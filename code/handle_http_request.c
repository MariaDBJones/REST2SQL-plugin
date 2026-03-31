#include "handle_http_request.h"

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

    /* Extrait le httpcode depuis le JSON */
    cJSON *field = cJSON_GetObjectItemCaseSensitive(json_response, "httpcode");
    if (cJSON_IsNumber(field))
        httpcode = field->valueint;

    /* Sérialise le JSON */
    char *json_str = cJSON_PrintUnformatted(json_response);
    if (json_str == NULL) {
        httpcode = HTTP_INTERNAL_SERVER_ERROR;
        if (asprintf(&json_str,
                     "{\"error\":\"Internal Server Error\",\"httpcode\":%d}",
                     httpcode) < 0)
            return MHD_NO;
    }

    /* Crée la réponse HTTP */
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
static int http_request_handler(void *cls,
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

    cJSON *response = NULL;

    /* Init MariaDB pour ce thread */
    mysql_thread_init();
 
#if HANDLERCORK == 0

    if (strcmp(method, "GET") == 0) {

        response = handle_get_request(url);

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
    /* HANDLERCORK == 1 : plugin entier désactivé */
    response = cJSON_CreateObject();
    http_set_error(response, "Plugin disabled", HTTP_METHOD_NOT_ALLOWED);
#endif

    /* Garde-fou : si un handler retourne NULL (OOM), on répond 500 */
    if (response == NULL) {
        response = cJSON_CreateObject();
        if (response != NULL)
            http_set_error(response, "Internal Server Error",
                           HTTP_INTERNAL_SERVER_ERROR);
        else {
            mysql_thread_end();
            return MHD_NO;
        }
    }

    int ret = http_send_json_response(connection, response);

    cJSON_Delete(response);   /* cJSON_Delete, pas free() */
    mysql_thread_end();

    return ret;
}
