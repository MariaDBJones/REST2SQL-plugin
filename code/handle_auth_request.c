#include "handle_auth_request.h"
#include "handle_http_request.h"

// Auth related endpoints — full implementation is tracked in doc/06-digest-authentication.md
// /auth/login
// /auth/logoff
// /auth/renew
// /auth/create

cJSON* handle_session_request(const char *url,
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

MYSQL *handle_auth_request(struct MHD_Connection *connection)
{
    (void)connection;
    return g_conn;
}

static void connection_start(void *cls,
                                struct MHD_Connection *connection,
                                void **socket_context)
{
    (void)cls; (void)connection; (void)socket_context;
    mysql_thread_init();
}

static void connection_finish(void *cls,
                                 struct MHD_Connection *connection,
                                 void **socket_context,
                                 enum MHD_RequestTerminationCode toe)
{
    (void)cls; (void)connection; (void)socket_context; (void)toe;
    mysql_thread_end();
}
