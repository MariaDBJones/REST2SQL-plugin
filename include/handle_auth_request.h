#ifndef HANDLE_AUTH_REQUEST_H
#define HANDLE_AUTH_REQUEST_H

#include "common.h"
#include "handle_http_request.h"

// Auth endpoints :
//   POST /auth/login   — exchange credentials for a JWT
//   POST /auth/logoff  — invalidate current JWT
//   POST /auth/renew   — renew a JWT before expiry
//   POST /auth/create  — create a new API user (admin only)

cJSON* handle_session_request(const char *url,
                           const char *upload_data,
                           size_t     *upload_data_size);

MYSQL *handle_auth_request(struct MHD_Connection *connection);

void connection_start(void *cls,
                         struct MHD_Connection *connection,
                         void **socket_context);

void connection_finish(void *cls,
                          struct MHD_Connection *connection,
                          void **socket_context,
                          enum MHD_RequestTerminationCode toe);

#endif // HANDLE_AUTH_REQUEST_H
