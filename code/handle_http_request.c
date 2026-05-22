#include "handle_http_request.h"
#include "handle_get_request.h"
#include "handle_put_request.h"
#include "handle_post_request.h"
#include "handle_patch_request.h"
#include "handle_delete_request.h"
#include "handle_auth_request.h"
// #include "handle_subscription_request.h"


/* ============================================================
 *  Validation
 * ============================================================ */

int http_is_valid_sql_ident(const char *s)
{
    if (s == NULL || *s == '\0')
        return 0;
    for (const char *p = s; *p != '\0'; p++) {
        if (!isalnum((unsigned char)*p) && *p != '_')
            return 0;
    }
    return 1;
}

int http_validate_url(const char *url, cJSON *json_response)
{
    if (url == NULL) {
        http_set_error(json_response, "NULL URL", HTTP_BAD_REQUEST);
        return 0;
    }
    if (strlen(url) > URL_MAX_LEN) {
        http_set_error(json_response, "URL too long", HTTP_BAD_REQUEST);
        return 0;
    }
    return 1;
}

/* ============================================================
 *  Construction de réponses JSON
 * ============================================================ */

void http_set_error(cJSON *json_response, const char *error_msg, int httpcode)
{
    cJSON_AddStringToObject(json_response, "error", error_msg);
    cJSON_AddNumberToObject(json_response, "httpcode", httpcode);
}

int http_send_json_response(struct MHD_Connection *connection,
                            cJSON *json_response)
{
    int httpcode = HTTP_OK;

    cJSON *field = cJSON_GetObjectItemCaseSensitive(json_response, "httpcode");
    if (cJSON_IsNumber(field))
        httpcode = field->valueint;

    char *json_str = cJSON_PrintUnformatted(json_response);
    if (json_str == NULL) {
        httpcode = HTTP_INTERNAL_SERVER_ERROR;
        if (asprintf(&json_str,
                     "{\"error\":\"Internal Server Error\",\"httpcode\":%d}",
                     httpcode) < 0)
            return MHD_NO;
    }

    struct MHD_Response *mhd_response = MHD_create_response_from_buffer(
        strlen(json_str),
        (void *)json_str,
        MHD_RESPMEM_MUST_COPY
    );
    if (mhd_response == NULL) {
        free(json_str);
        return MHD_NO;
    }

    MHD_add_response_header(mhd_response, MHD_HTTP_HEADER_ALLOW,
                            ALLOWED_METHODS);
    MHD_add_response_header(mhd_response, "Content-Type",
                            CONTENT_TYPE_JSON);

    int ret = MHD_queue_response(connection, httpcode, mhd_response);
    MHD_destroy_response(mhd_response);
    free(json_str);
    return ret;
}

/* ============================================================
 *  Request dispatcher
 * ============================================================ */
int http_request_handler(void *cls,
                         struct MHD_Connection *connection,
                         const char *url,
                         const char *method,
                         const char *version,
                         const char *upload_data,
                         size_t *upload_data_size,
                         void **con_cls)
{
    (void)cls;
    (void)version;

    /* ----------------------------------------------------------
     * 1er appel : con_cls == NULL
     * Ouvrir la connexion MariaDB = étape d'authentification.
     * MVP  : socket Unix, utilisateur OS.
     * Beta : open_db_connection() extraira les credentials Basic Auth.
     * ---------------------------------------------------------- */
    if (*con_cls == NULL) {
        mhd_conn_ctx_t *ctx = calloc(1, sizeof(mhd_conn_ctx_t));
        if (ctx == NULL)
            return MHD_NO;

        ctx->conn = open_db_connection(connection);
        *con_cls  = ctx;

        /* MHD rappelle immédiatement pour traiter la requête */
        return MHD_YES;
    }

    /* ----------------------------------------------------------
     * 2ème appel : traitement de la requête
     * ---------------------------------------------------------- */
    mhd_conn_ctx_t *ctx  = *con_cls;
    MYSQL          *conn = ctx->conn;

    /* Connexion NULL = authentification échouée → 401 */
    if (conn == NULL) {
        cJSON *r = cJSON_CreateObject();
        if (r == NULL) { free(ctx); *con_cls = NULL; return MHD_NO; }
        http_set_error(r, "Unauthorized", HTTP_UNAUTHORIZED);
        int ret = http_send_json_response(connection, r);
        cJSON_Delete(r);
        free(ctx);
        *con_cls = NULL;
        return ret;
    }

    cJSON *response = NULL;

    if (strncmp(url, "/auth/", 6) == 0) {

        response = handle_session_request(url, upload_data, upload_data_size);

    } else if (strncmp(url, "/subscription/", 14) == 0) {

        response = handle_subscription_request(method, url, upload_data, upload_data_size);

    } else {
#if HANDLERCORK == 0

        if (strcmp(method, "GET") == 0) {

            response = handle_get_request(conn, url);

        } else if (strcmp(method, "POST") == 0) {

            response = handle_post_request(url, upload_data, upload_data_size);

        } else if (strcmp(method, "PATCH") == 0) {

            response = handle_patch_request(url, upload_data, upload_data_size);

        } else if (strcmp(method, "PUT") == 0) {

            response = handle_put_request(url, upload_data, upload_data_size);

        } else if (strcmp(method, "DELETE") == 0) {

            response = handle_delete_request(url);

        } else {
            response = cJSON_CreateObject();
            cJSON_AddStringToObject(response, "method", method);
            cJSON_AddStringToObject(response, "url",    url);
            http_set_error(response, "Method not allowed", HTTP_METHOD_NOT_ALLOWED);
        }

#else
        response = cJSON_CreateObject();
        http_set_error(response, "Plugin disabled", HTTP_METHOD_NOT_ALLOWED);
#endif
    }

    /* Safeguard OOM */
    if (response == NULL) {
        response = cJSON_CreateObject();
        if (response == NULL) {
            mysql_close(conn);
            free(ctx);
            *con_cls = NULL;
            return MHD_NO;
        }
        http_set_error(response, "Internal Server Error",
                       HTTP_INTERNAL_SERVER_ERROR);
    }

    int ret = http_send_json_response(connection, response);
    cJSON_Delete(response);

    /* Fermeture de la connexion MariaDB — fin du cycle de vie */
    mysql_close(conn);
    free(ctx);
    *con_cls = NULL;

    return ret;
}
