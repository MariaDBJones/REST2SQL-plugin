#ifndef HANDLE_GET_REQUEST_H
#define HANDLE_GET_REQUEST_H

#include "common.h"

cJSON *handle_get_request(MYSQL *conn, const char *url);

#endif // HANDLE_GET_REQUEST_H
