#ifndef HANDLE_AUTH_REQUEST_H
#define HANDLE_AUTH_REQUEST_H

#include "common.h"
#include "handle_http_request.h"

// Auth endpoints — MVP : stubs returning 405
// Beta : Basic Auth stateless via mysql_real_connect() per request
//   POST /auth/login   — placeholder
//   POST /auth/logoff  — placeholder
//   POST /auth/renew   — placeholder
//   POST /auth/create  — placeholder

cJSON* handle_session_request(const char *url,
                              const char *upload_data,
                              size_t     *upload_data_size);

// MHD connection lifecycle callbacks — passed to MHD_OPTION_NOTIFY_CONNECTION
void connection_start(void *cls,
                      struct MHD_Connection *connection,
                      void **socket_context);

void connection_finish(void *cls,
                       struct MHD_Connection *connection,
                       void **socket_context,
                       enum MHD_RequestTerminationCode toe);

#endif // HANDLE_AUTH_REQUEST_H
