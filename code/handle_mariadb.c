// db connexions and session management
#include <common.h>
#include <microhttpd.h>

MYSQL *handle_authentication_request(struct MHD_Connection *connection)
{
    (void)connection;
    return g_conn;
}
