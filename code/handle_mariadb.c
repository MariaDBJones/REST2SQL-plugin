// db connexions and session management
#include <common.h>
#include <microhttpd.h>

MYSQL *handle_authentication_request(struct MHD_Connection *connection)
{
    (void)connection;
    return g_conn;
}

static void connection_started(void *cls,
                                struct MHD_Connection *connection,
                                void **socket_context)
{
    (void)cls; (void)connection; (void)socket_context;
    mysql_thread_init();
}

static void connection_finished(void *cls,
                                 struct MHD_Connection *connection,
                                 void **socket_context,
                                 enum MHD_RequestTerminationCode toe)
{
    (void)cls; (void)connection; (void)socket_context; (void)toe;
    mysql_thread_end();
}
