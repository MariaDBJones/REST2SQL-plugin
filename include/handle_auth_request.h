#ifndef HANDLE_AUTH_REQUEST_H
#define HANDLE_AUTH_REQUEST_H

#include "common.h"

// Auth endpoints :
//   POST /auth/login   — exchange credentials for a JWT
//   POST /auth/logoff  — invalidate current JWT
//   POST /auth/renew   — renew a JWT before expiry
//   POST /auth/create  — create a new API user (admin only)

cJSON* handle_auth_request(const char *url,
                           const char *upload_data,
                           size_t     *upload_data_size);

#endif // HANDLE_AUTH_REQUEST_H
