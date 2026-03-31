#include "common.h"
#include "handle_http_request.h"
#include "handle_get_request.h"
#include "handle_post_request.h"
#include "handle_put_request.h"
#include "handle_patch_request.h"
#include "handle_delete_request.h"

#include <arpa/inet.h>
#include <mysql/plugin.h>
#include <microhttpd.h>

#include <my_default.h>

#ifndef MYSQL_DYNAMIC_PLUGIN
#define MYSQL_DYNAMIC_PLUGIN
#endif

/* ============================================================
 *  Plugin declaration
 * ============================================================ */
struct st_mysql_daemon rest_api_plugin = {
    MYSQL_DAEMON_INTERFACE_VERSION
};

static struct MHD_Daemon *listener = NULL;

// valeurs par defaut
rest2sql_config_t rest2sql_config = {
    .address             = "0.0.0.0",
    .port                = 3000,
    .sslcert             = "",
    .sslkey              = "",
    .sslca               = "",
    .default_exposition  = "blacklist",
    .jwt_expiry          = 3600,
    .concurrency         = 10,
    .connect_timeout     = 5,
    .debug               = 0
};

/* ============================================================
 *  Plugin variables init
 * ============================================================ */

static void load_plugin_config(void) {
    const char *groups[] = { APIGROUP, NULL };
    char **argv_ptr = NULL;
    int    argc_ptr = 0;

    if (load_defaults("my", groups, &argc_ptr, &argv_ptr) != 0) {
        fprintf(stderr, "[rest2sql] WARNING: could not read config file\n");
        return;
    }

    for (int i = 0; argv_ptr[i] != NULL; i++) {
        char key[64]   = {0};
        char value[256] = {0};

        // parser "--key=value"
        if (sscanf(argv_ptr[i], "--%63[^=]=%255s", key, value) != 2)
            continue;

        if      (strcmp(key, "rest2sql_port") == 0)
            rest2sql_config.port = atoi(value);
        else if (strcmp(key, "rest2sql_address") == 0)
            strncpy(rest2sql_config.address, value, sizeof(rest2sql_config.address) - 1);
        else if (strcmp(key, "rest2sql_sslcert") == 0)
            strncpy(rest2sql_config.sslcert, value, sizeof(rest2sql_config.sslcert) - 1);
        else if (strcmp(key, "rest2sql_sslkey") == 0)
            strncpy(rest2sql_config.sslkey, value, sizeof(rest2sql_config.sslkey) - 1);
        else if (strcmp(key, "rest2sql_sslca") == 0)
            strncpy(rest2sql_config.sslca, value, sizeof(rest2sql_config.sslca) - 1);
        else if (strcmp(key, "rest2sql_default_exposition") == 0)
            strncpy(rest2sql_config.default_exposition, value, sizeof(rest2sql_config.default_exposition) - 1);
        else if (strcmp(key, "rest2sql_jwt_expiry") == 0)
            rest2sql_config.jwt_expiry = atoi(value);
        else if (strcmp(key, "rest2sql_concurrency") == 0)
            rest2sql_config.concurrency = atoi(value);
        else if (strcmp(key, "rest2sql_connect_timeout") == 0)
            rest2sql_config.connect_timeout = atoi(value);
        else if (strcmp(key, "rest2sql_debug") == 0)
            rest2sql_config.debug = atoi(value);
    }

    free_defaults(argv_ptr);
}

/* ============================================================
 *  Plugin init
 * ============================================================ */
static int rest_api_plugin_init(void *p)
{
    (void)p;

    /* Initialisation globale MariaDB — une seule fois */
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "[rest2sql] Failed to initialize MariaDB lib\n");
        return 1;
    }

    /* Binding ADDRESS:PORT */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    inet_pton(AF_INET, ADDRESS, &addr.sin_addr);

    listener = MHD_start_daemon(
        MHD_USE_THREAD_PER_CONNECTION,
        PORT,
        NULL, NULL,
        &request_handler, NULL,
        MHD_OPTION_SOCK_ADDR, &addr,
        /* MHD_OPTION_THREAD_POOL_SIZE, 5, */  /* TODO: system variable */
        MHD_OPTION_END
    );

    if (listener == NULL) {
        fprintf(stderr, "[rest2sql] Failed to start HTTP server on %s:%d\n",
                ADDRESS, PORT);
        mysql_library_end();
        return 1;
    }

    fprintf(stderr, "[rest2sql] Server running on %s:%d\n", ADDRESS, PORT);
    return 0;
}

/* ============================================================
 *  Plugin deinit
 * ============================================================ */
static int rest_api_plugin_deinit(void *p)
{
    (void)p;

    if (listener != NULL) {
        MHD_stop_daemon(listener);
        listener = NULL;
        fprintf(stderr, "[rest2sql] HTTP server stopped.\n");
    }
    mysql_library_end();
    return 0;
}

// TODO:variables
//+--------------+
// bind-address *
// port *
// plugin-user *
// plugin-password *
// sslca
// sslcert
// sslkey
// default exposition : blacklist/whitelist
// allow insert : PUTTMETHODCORK
// allow update : PATCHMETHODCORK
// allow delete : DELETEMETHODCORK
// allow ping   : PINGMETHODCORK
// allow status : SATUSMETHODCORK
// allow select : GETMETHODCORK
// concurrency
// cnx timeout
// read timeout
// write timeout

// TODO:status
//+-----------+
// GET
// POST
// PATCH
// PUT
// DELETE
// CREATE
// READ
// UPDATE
// DELETE
// PING
// HEALTH
// STATUS
// OK
// INTERNAL_SERVER_ERROR
// BAD_REQUEST
// UNAUTHORIZED
// FORBIDDEN
// NOT_FOUND
// METHOD_NOT_ALLOWED
// UNSUPPORTED_MEDIA_TYPE
// LINEREADS
// LINEINSERTED
// LINEUPDATED
// LINEDELETED

/* ============================================================
 *  Plugin descriptor
 *
 *  TODO system variables :
 *    bind-address, port, user, password, socket,
 *    sslca, sslcert, sslkey, concurrency,
 *    cnx_timeout, read_timeout, write_timeout
 *
 *  TODO status vars :
 *    requests_{get,post,patch,put,delete}
 *    responses_{ok,bad_request,unauthorized,forbidden,
 *               not_found,method_not_allowed,internal_error}
 * ============================================================ */
maria_declare_plugin(json_api)
{
    MYSQL_DAEMON_PLUGIN,
    &rest_api_plugin,
    PLUGIN_NAME,
    PLUGIN_AUTHOR,
    PLUGIN_DESCRIPTION,
    PLUGIN_LICENSE_GPL,
    rest_api_plugin_init,
    rest_api_plugin_deinit,
    0x0100,
    NULL,   /* status vars  — TODO */
    NULL,   /* system vars  — TODO */
    NULL,
    MariaDB_PLUGIN_MATURITY_BETA
}
maria_declare_plugin_end;
