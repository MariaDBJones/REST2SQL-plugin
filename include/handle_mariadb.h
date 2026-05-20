#ifndef HANDLE_MARIADB_H
#define HANDLE_MARIADB_H

#include <mysql.h>
#include <microhttpd.h>

extern MYSQL *g_conn;  /* defined in rest2sql.c */
MYSQL *handle_authentication_request(struct MHD_Connection *connection);

#endif
