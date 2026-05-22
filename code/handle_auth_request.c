#include "handle_auth_request.h"
#include "handle_http_request.h"

/* ============================================================
 *  open_db_connection
 *  MVP  : connexion socket Unix, utilisateur OS (pas de credentials).
 *  Beta : parser le header Authorization: Basic <base64>,
 *         décoder user:pass, passer à mysql_real_connect().
 * ============================================================ */
MYSQL *open_db_connection(struct MHD_Connection *connection)
{
    /* Beta : extraire les credentials ici depuis le header HTTP
     * const char *auth = MHD_lookup_connection_value(
     *     connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_AUTHORIZATION);
     * puis décoder base64 → user / pass
     */
    (void)connection; /* MVP : header non utilisé */

    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL)
        return NULL;

    unsigned int timeout = (unsigned int)rest2sql_config.connect_timeout;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    if (mysql_real_connect(conn,
                           NULL,   /* host → socket Unix */
                           NULL,   /* user → OS (MVP) | Basic Auth user (Beta) */
                           NULL,   /* pass → none (MVP) | Basic Auth pass (Beta) */
                           NULL,   /* db   */
                           0,      /* port */
                           NULL,   /* socket → défaut mysqld */
                           0) == NULL)
    {
        mysql_close(conn);
        return NULL;
    }
    return conn;
}

/* ============================================================
 *  Auth endpoints — stubs MVP
 * ============================================================ */
cJSON *handle_session_request(const char *url,
                              const char *upload_data,
                              size_t     *upload_data_size)
{
    (void)upload_data;
    (void)upload_data_size;

    cJSON *r = cJSON_CreateObject();
    if (r == NULL) return NULL;

    cJSON_AddStringToObject(r, "url", url ? url : "");
    http_set_error(r, "Auth not yet implemented", HTTP_METHOD_NOT_ALLOWED);
    return r;
}

/* ============================================================
 *  MHD connection lifecycle callbacks
 *  Gèrent uniquement mysql_thread_init/end pour le thread MHD.
 *  Distincts de con_cls — contexte TCP, pas contexte requête.
 * ============================================================ */
void connection_start(void *cls,
                      struct MHD_Connection *connection,
                      void **socket_context)
{
    (void)cls; (void)connection; (void)socket_context;
    mysql_thread_init();
}

void connection_finish(void *cls,
                       struct MHD_Connection *connection,
                       void **socket_context,
                       enum MHD_RequestTerminationCode toe)
{
    (void)cls; (void)connection; (void)socket_context; (void)toe;
    mysql_thread_end();
}
