#include "common.h"
#include "handle_http_request.h"

#include <arpa/inet.h>
#include <mysql/plugin.h>
#include <microhttpd.h>

#include <mysql/mariadb_com.h>   /* mariadb_load_defaults() — résolu via mariadb_config */

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

/* ============================================================
 *  Plugin status variables — backing storage
 * ============================================================ */

/* have_rest2sql : always "YES" while the plugin is loaded.
 * Disappears from SHOW VARIABLES automatically when unloaded. */
char have_rest2sql_val[] = "YES";

/* rest2sql_enabled : gates all request processing.
 * SET GLOBAL rest2sql_on = OFF disables without unloading. */
my_bool rest2sql_enabled = TRUE;

/* ============================================================
 *  Plugin configuration — defaults (overridden by my.cnf)
 * ============================================================ */

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
 *  Plugin sysvars init
 * ============================================================ */
/* have_rest2sql — read only, type string */
static MYSQL_SYSVAR_STR(
  have_rest2sql,            /* nom → SHOW VARIABLES LIKE 'have_rest2sql' */
  have_rest2sql_val,        /* value pointer */
  PLUGIN_VAR_READONLY | PLUGIN_VAR_NOSYSVAR,
  "Whether rest2sql plugin is installed (YES/NO)",
  NULL,                     /* check function */
  NULL,                     /* update function */
  "YES"                     /* valeur par défaut */
);

/* rest2sql_on — lecture/écriture, type bool */
static MYSQL_SYSVAR_BOOL(
  on,                       /* suffixe → rest2sql_on */
  rest2sql_enabled,         /* pointeur vers la valeur */
  PLUGIN_VAR_RQCMDARG,
  "Enable or disable rest2sql request processing",
  NULL,                     /* check function */
  NULL,                     /* update function */
  TRUE                      /* activé par défaut */
);

static struct st_mysql_sys_var *rest2sql_sysvars[] = {
  MYSQL_SYSVAR(have_rest2sql),
  MYSQL_SYSVAR(on),
  NULL
};

/* ============================================================
 *  Plugin variables init
 * ============================================================ */

static void load_plugin_config(void) {
    const char *groups[] = { APIGROUP, NULL };
    char **argv_ptr = NULL;
    int    argc_ptr = 0;

    /* mariadb_load_defaults() est l'API cliente publique (mariadb_com.h).
     * Elle retourne void — pas de code d'erreur, argv_ptr reste NULL si
     * le fichier de config est absent, ce qui est géré ci-dessous. */
    mariadb_load_defaults("my", groups, &argc_ptr, &argv_ptr);
    if (argv_ptr == NULL) {
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

    free(argv_ptr);
}

/* ============================================================
 *  Plugin init
 * ============================================================ */
static int rest2sql_init(void *p)
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
        &http_request_handler, NULL,
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
static int rest2sql_deinit(void *p)
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
// rest2sql_on 
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
 *    rest2sql_{get,post,patch,put,delete}
 *    rest2sql_{ok,bad_request,unauthorized,forbidden,
 *               not_found,method_not_allowed,internal_error}
 *    rest2sql_on
 *    have_rest2sql
 * ============================================================ */
maria_declare_plugin(rest2sql)
{
    MYSQL_DAEMON_PLUGIN,
    &rest_api_plugin,
    PLUGIN_NAME,
    PLUGIN_AUTHOR,
    PLUGIN_DESCRIPTION,
    PLUGIN_LICENSE_GPL,
    rest2sql_init,
    rest2sql_deinit,
    0x0100,
    NULL,   /* status vars  — TODO */
    rest2sql_sysvars,   /* system vars : have_rest2sql, rest2sql_on */
    NULL,
    MariaDB_PLUGIN_MATURITY_BETA
}
maria_declare_plugin_end;
