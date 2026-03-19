#include "common.h"
#include "handle_http.h"
#include "handle_get_request.h"
#include "handle_post_request.h"
#include "handle_put_request.h"
#include "handle_patch_request.h"
#include "handle_delete_request.h"

#include <arpa/inet.h>
#include <mysql/plugin.h>
#include <microhttpd.h>

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
 *  Request dispatcher
 * ============================================================ */
static int request_handler(void *cls,
                            struct MHD_Connection *connection,
                            const char *url,
                            const char *method,
                            const char *version,
                            const char *upload_data,
                            size_t *upload_data_size,
                            void **con_cls)
{
    (void)cls;
    (void)version;

    cJSON *response = NULL;

    /* Init MariaDB pour ce thread */
    mysql_thread_init();

#if HANDLERCORK == 0

    if (strcmp(method, "GET") == 0) {
#if GETMETHODCORK == 0
        response = handle_get_request(url);
#else
        response = cJSON_CreateObject();
        http_set_error(response, "GET method disabled", HTTP_METHOD_NOT_ALLOWED);
#endif

    } else if (strcmp(method, "POST") == 0) {
#if POSTMETHODCORK == 0
        response = handle_post_request(url, upload_data, upload_data_size);
#else
        response = cJSON_CreateObject();
        http_set_error(response, "POST method disabled", HTTP_METHOD_NOT_ALLOWED);
#endif

    } else if (strcmp(method, "PATCH") == 0) {
#if PATCHMETHODCORK == 0
        response = handle_patch_request(url, upload_data, upload_data_size);
#else
        response = cJSON_CreateObject();
        http_set_error(response, "PATCH method disabled", HTTP_METHOD_NOT_ALLOWED);
#endif

    } else if (strcmp(method, "PUT") == 0) {
#if PUTMETHODCORK == 0
        response = handle_put_request(url, upload_data, upload_data_size);
#else
        response = cJSON_CreateObject();
        http_set_error(response, "PUT method disabled", HTTP_METHOD_NOT_ALLOWED);
#endif

    } else if (strcmp(method, "DELETE") == 0) {
#if DELETEMETHODCORK == 0
        response = handle_delete_request(url);
#else
        response = cJSON_CreateObject();
        http_set_error(response, "DELETE method disabled", HTTP_METHOD_NOT_ALLOWED);
#endif

    } else {
        response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "method", method);
        cJSON_AddStringToObject(response, "url",    url);
        http_set_error(response, "Method not allowed", HTTP_METHOD_NOT_ALLOWED);
    }

#else
    /* HANDLERCORK == 1 : plugin entier désactivé */
    response = cJSON_CreateObject();
    http_set_error(response, "Plugin disabled", HTTP_METHOD_NOT_ALLOWED);
#endif

    /* Garde-fou : si un handler retourne NULL (OOM), on répond 500 */
    if (response == NULL) {
        response = cJSON_CreateObject();
        if (response != NULL)
            http_set_error(response, "Internal Server Error",
                           HTTP_INTERNAL_SERVER_ERROR);
        else {
            mysql_thread_end();
            return MHD_NO;
        }
    }

    int ret = http_send_json_response(connection, response);

    cJSON_Delete(response);   /* cJSON_Delete, pas free() */
    mysql_thread_end();

    return ret;
}

/* ============================================================
 *  Helpers init : gestion du user API
 * ============================================================ */

/**
 * Vérifie si le user APIUSER@localhost existe.
 * Retourne 1 si oui, 0 si non, -1 en cas d'erreur.
 */
static int api_user_exists(MYSQL *admin)
{
    /* Requête préparée pour éviter toute injection sur APIUSER */
    const char *query = "SELECT 1 FROM mysql.user "
                        "WHERE user = ? AND host = 'localhost'";

    MYSQL_STMT *stmt = mysql_stmt_init(admin);
    if (stmt == NULL) {
        fprintf(stderr, "[rest2sql] mysql_stmt_init failed: %s\n",
                mysql_error(admin));
        return -1;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "[rest2sql] stmt prepare failed: %s\n",
                mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    MYSQL_BIND param;
    memset(&param, 0, sizeof(param));
    unsigned long ulen = (unsigned long)strlen(APIUSER);
    param.buffer_type   = MYSQL_TYPE_STRING;
    param.buffer        = (void *)APIUSER;
    param.buffer_length = ulen;
    param.length        = &ulen;

    if (mysql_stmt_bind_param(stmt, &param) ||
        mysql_stmt_execute(stmt)            ||
        mysql_stmt_store_result(stmt)) {
        fprintf(stderr, "[rest2sql] stmt exec failed: %s\n",
                mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    int exists = (mysql_stmt_num_rows(stmt) > 0) ? 1 : 0;
    mysql_stmt_close(stmt);
    return exists;
}

/**
 * Crée le user APIUSER avec les privilèges minimaux nécessaires.
 * Retourne 0 en cas de succès, -1 en cas d'erreur.
 *
 * NOTE : APIUSER et APIPASSWORD sont des constantes de compilation.
 * TODO : les lire depuis les variables système MariaDB.
 */
static int api_user_create(MYSQL *admin)
{
    /*
     * Les noms d'utilisateurs ne peuvent pas être des paramètres ?
     * dans un prepared statement MariaDB.
     * APIUSER et APIPASSWORD sont des constantes de compilation
     * contrôlées par le développeur → risque d'injection nul ici.
     * On valide quand même le format pour se prémunir d'une mauvaise
     * configuration accidentelle.
     */
    for (const char *p = APIUSER; *p != '\0'; p++) {
        if (!isalnum((unsigned char)*p) && *p != '_') {
            fprintf(stderr, "[rest2sql] APIUSER contains invalid characters\n");
            return -1;
        }
    }

    char grant[512];
    int n = snprintf(grant, sizeof(grant),
                     "GRANT PROCESS, SELECT ON *.* TO `%s`@`localhost` "
                     "IDENTIFIED BY '%s'",
                     APIUSER, APIPASSWORD);
    if (n < 0 || (size_t)n >= sizeof(grant)) {
        fprintf(stderr, "[rest2sql] GRANT query truncated\n");
        return -1;
    }

    if (mysql_real_query(admin, grant, (unsigned long)strlen(grant))) {
        fprintf(stderr, "[rest2sql] GRANT failed: %s\n", mysql_error(admin));
        return -1;
    }
    return 0;
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

    /* Connexion admin locale pour vérifier / créer le user API */
    MYSQL *admin = mysql_init(NULL);
    if (admin == NULL) {
        fprintf(stderr, "[rest2sql] mysql_init failed\n");
        mysql_library_end();
        return 1;
    }

    if (mysql_real_connect_local(admin) == NULL) {
        fprintf(stderr, "[rest2sql] Local connect failed: %s\n",
                mysql_error(admin));
        mysql_close(admin);
        mysql_library_end();
        return 1;
    }

    int exists = api_user_exists(admin);
    if (exists < 0) {
        mysql_close(admin);
        mysql_library_end();
        return 1;
    }

    if (exists == 0) {
        fprintf(stderr, "[rest2sql] API user not found, creating...\n");
        if (api_user_create(admin) < 0) {
            mysql_close(admin);
            mysql_library_end();
            return 1;
        }
        fprintf(stderr, "[rest2sql] API user '%s'@'localhost' created.\n",
                APIUSER);
    }

    mysql_close(admin);

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
// bind-address
// port
// plugin-user
// plugin-password
// socket
// sslca
// sslcert
// sslkey
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
