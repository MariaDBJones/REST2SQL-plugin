// global headers for the project
#include "common.h"
#include <arpa/inet.h>
#include <mysql/plugin.h>

// MariaDB headers
#ifndef MYSQL_DYNAMIC_PLUGIN
#define MYSQL_DYNAMIC_PLUGIN
#endif

// micro httpd headers
#include <microhttpd.h>

// request handlers headers
#include "handle_get_request.h"
#include "handle_post_request.h"
#include "handle_put_request.h"
#include "handle_patch_request.h"
#include "handle_delete_request.h"

// Plugin declaration structure
struct st_mysql_daemon rest_api_plugin = {
    MYSQL_DAEMON_INTERFACE_VERSION
};

// defining the daemon structure
struct MHD_Daemon *listener = NULL;


// sends back HTTP + json to client
static int send_json_response(struct MHD_Connection *connection, cJSON *json_response) {
    char *response_string;
    int httpcode = HTTP_OK;

// Convert the cJSON object to a JSON string
    response_string = cJSON_Print(json_response);

// If conversion fails, set an error message
    if (response_string == NULL) {
        response_string = strdup("{\"error\": \"Internal Server Error\", \"httpcode\": HTTP_INTERNAL_SERVER_ERROR }");
        httpcode = HTTP_INTERNAL_SERVER_ERROR;
    } else {
        // Extract HTTP return code from the JSON string
        cJSON *parsed_json = cJSON_Parse(response_string);
        if (parsed_json) {
            cJSON *field = cJSON_GetObjectItemCaseSensitive(parsed_json, "httpcode");
            if (cJSON_IsNumber(field)) {
                httpcode = field->valueint;
            }
            cJSON_Delete(parsed_json);
        }
    }

// Create a response from the string
    struct MHD_Response *mhd_response = MHD_create_response_from_buffer(
        strlen(response_string),
        (void *)response_string,
        MHD_RESPMEM_MUST_COPY
    );

    if (mhd_response == NULL) {
        free(response_string);
        return MHD_NO;
    }

// Mandatory Allow header for HTTP 405 : added to all foir simplicity
    MHD_add_response_header(mhd_response, MHD_HTTP_HEADER_ALLOW, "GET, POST, PUT, PATCH, DELETE");
// content-type header  because all answer is json
    MHD_add_response_header(mhd_response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, httpcode, mhd_response);
    MHD_destroy_response(mhd_response);
    free(response_string);

    return ret;
}

// directing requests to the right handling function
static int request_handler(void *cls, struct MHD_Connection *connection,
                           const char *url, const char *method,
                           const char *version, const char *upload_data,
                           size_t *upload_data_size, void **con_cls) {

     cJSON *response = cJSON_CreateObject();

// MariaDB init for this thread
     mysql_thread_init();

    if (strcmp(method, "GET") == 0) {
// SELECT
        response = handle_get_request(url);
//  } else if (strcmp(method, "POST") == 0) {
// INSERT
//        response = handle_post_request(url, upload_data, upload_data_size);
//  } else if (strcmp(method, "PATCH") == 0) {
// UPDATE
//        response = handle_patch_request(url, upload_data, upload_data_size);
//  } else if (strcmp(method, "PUT") == 0) {
// CALL
//        response = handle_put_request(url, upload_data, upload_data_size);
    } else if (strcmp(method, "DELETE") == 0) {
// DELETE
        response = handle_delete_request(url);
    } else {
// Method not supported
          cJSON_AddStringToObject(response, "status", "METHODNOTALLOWED");
          cJSON_AddStringToObject(response, "error", "method not allowed");
          cJSON_AddStringToObject(response, "method", method);
          cJSON_AddStringToObject(response, "url", url);
          cJSON_AddNumberToObject(response, "httpcode", HTTP_METHOD_NOT_ALLOWED);
    }

// clean exit procedure w/ housekeeping
    int ret = send_json_response(connection, response);
// Free the allocated JSON string
    free(response); 
// Nettoyage avant sortie du thread
    mysql_thread_end();

    return ret;
}

// API bootstrap : httpd startup
static int rest_api_plugin_init(void *p) {

// Initialisation globale MySQL - UNE SEULE FOIS
    if (mysql_library_init(0, NULL, NULL)) {
       fprintf(stderr, "Failed to initialize mariadb lib\n");
        return 2;
    }

//default behaviour : bind to local address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ADDRESS, &(addr.sin_addr));

    listener = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,     // 1 thread per connexion
                                PORT,                              // listening to PORT
                                NULL, NULL,
                                &request_handler,                  // answering function
                                NULL,
                                MHD_OPTION_SOCK_ADDR, &addr,       // bind address
 //                               MHD_OPTION_THREAD_POOL_SIZE, 5,    // max number of concurrent threads
                                MHD_OPTION_END);

    if (listener == NULL) {
       fprintf(stderr, "Failed to start http server\n");
       return 1;
    }
    fprintf(stderr,"Server running on port %d\n", PORT);
    return 0;
}

// API shutdown : housekeeping
static int rest_api_plugin_deinit(void *p) {
    if (listener != NULL) {
        MHD_stop_daemon(listener);
        listener = NULL;
        fprintf(stderr, "HTTP server stopped.\n");
    }
    mysql_library_end();
    return 0;
}

// TODO:variables
//+--------------+
// bind-address
// port
// user
// password
// socket
// sslca
// sslcert
// sslkey
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

// Plugin descriptor
maria_declare_plugin(json_api)
{
    MYSQL_DAEMON_PLUGIN,              /* the plugin type (a MYSQL_XXX_PLUGIN value)     */
    &rest_api_plugin,                 /* pointer to type-specific plugin descriptor     */
    PLUGIN_NAME,                      /* plugin name                                    */
    PLUGIN_AUTHOR,                    /* plugin author (for I_S.PLUGINS)                */
    PLUGIN_DESCRIPTION,               /* general descriptive text (for I_S.PLUGINS)     */
    PLUGIN_LICENSE_GPL,               /* the plugin license (PLUGIN_LICENSE_XXX)        */
    rest_api_plugin_init,             /* the function to invoke when plugin is loaded   */
    rest_api_plugin_deinit,           /* the function to invoke when plugin is unloaded */
    0x0100,                           /* plugin version (for I_S.PLUGINS)               */
    NULL,                             /* for status vars                                */
    NULL,                             /* for system vars                                */
    NULL,                             /* reserved for dependency checking               */
    MariaDB_PLUGIN_MATURITY_ALPHA     /* maturity flags for plugin                      */
}
maria_declare_plugin_end;
