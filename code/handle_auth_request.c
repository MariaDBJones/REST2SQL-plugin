#include "handle_auth_request.h"
#include "handle_http_request.h"

// Auth related endpoints — full implementation tracked in doc/06-digest-authentication.md
// Beta : Basic Auth via mysql_real_connect() per request (stateless, no sessions)
// /auth/* routes are stubs for MVP — returns 405

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

/* connection_start/finish sont passées comme callbacks à
 * MHD_OPTION_NOTIFY_CONNECTION dans rest2sql.c — pas de static,
 * elles doivent être visibles à l'édition de liens. */
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
