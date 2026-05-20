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
