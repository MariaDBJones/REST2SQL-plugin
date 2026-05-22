// global libs inclusion
//preventing inclusion duplication 
#ifndef GLOBALS_H
#define GLOBALS_H

// MariaDB headers
#ifndef MYSQL_DYNAMIC_PLUGIN
#define MYSQL_DYNAMIC_PLUGIN
#endif

// C shenanigans
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// MariaDB headers
#include <mysql.h>
#include <mysql/service_sql.h>

// time logging
#include <time.h>
#include <sys/time.h>

// cJSON headers
#include <cjson/cJSON.h>

//logging func header
#include "log.h"

// global variables sharing
// Defining the resources we want to check
// TODO : ability to declare the list in a config file / system variable
// extern const char *resources[] ;
// extern const int num_resources;

// grant verification
// extern int has_required_privilege(const char *user);

// check for resource exposition
// extern int is_exposed_resource(const char *url);

#define PLUGIN_NAME          "rest2sql"
#define PLUGIN_AUTHOR        "Sylvain Arbaudie <sylvain@arbaudie.it>"
#define PLUGIN_DESCRIPTION   "simple JSON-to-SQL CRUD REST API Plugin for MariaDB"

// corks definitions
#define GETCORK           0 // inside method handler
#define PUTCORK           1 // inside method handler
#define POSTCORK          1 // inside method handler
#define PATCHCORK         1 // inside method handler
#define DELETECORK        1 // inside method handler
#define HANDLERCORK       0 // global handler cork
#define GETMETHODCORK     0 // around method handler call
#define PUTMETHODCORK     1 // around method handler call
#define POSTMETHODCORK    1 // around method handler call
#define PATCHMETHODCORK   1 // around method handler call
#define DELETEMETHODCORK  1 // around method handler call

// defining use HTTP response codes
#define HTTP_OK                     200 // ok
#define HTTP_INTERNAL_SERVER_ERROR  500 // something went wrong
#define HTTP_BAD_REQUEST            400 // url bad format (tokens / resources no good)
#define HTTP_UNAUTHORIZED           401 // SQL privileges no good (merge into 403 ? )
#define HTTP_FORBIDDEN              403 // resource not exposed
#define HTTP_NOT_FOUND              404 // resource does not exists ( merge into 403 to prevent resource exposition scanning ? )
#define HTTP_METHOD_NOT_ALLOWED     405 // method & resource not compmatible
#define HTTP_UNSUPPORTED_MEDIA_TYPE 415 // request body format not json

// method <=> privilege translation
#define GET    "READ"
#define PUT    "EXECUTE" // TBD
#define POST   "CREATE"
#define PATCH  "UPDATE"
#define DELETE "DELETE"

// constants for http answers
#define ALLOWED_METHODS "GET"
#define CONTENT_TYPE_JSON "application/json"

// ease the use of mysql_real_query
#define STRING_WITH_LEN(X) (X), ((size_t) (sizeof(X) - 1))
#define STRING_WITH_STRLEN(X) (X), (strlen(X))

// TODO : managing this via system variables
#define DEFAULT_PORT    8080
#define DEFAULT_ADDRESS "0.0.0.0"

// TODO : obfuscate
// read variables from [rest2sql] group in cnf files
// all variables will be prefixed with rest2sql :
// rest2sql_user, rest2sql_password, rest2sql_
#define APIGROUP "rest2sql"

// Fallback credentials used when env vars DB_USER / DB_PASSWD are not set.
// Override via [rest2sql] section in my.cnf (rest2sql_user / rest2sql_password).
#ifndef APIUSER
#define APIUSER     "restapi-user"
#endif
#ifndef APIPASSWORD
#define APIPASSWORD "re$t2SQ!"
#endif

#define DEBUG  1

/* ============================================================
 *  Plugin configuration struct
 *  Populated from [rest2sql] group in my.cnf at init time.
 * ============================================================ */
typedef struct {
    char    address[64];
    int     port;
    char    sslcert[256];
    char    sslkey[256];
    char    sslca[256];
    char    default_exposition[16];  /* "blacklist" | "whitelist" */
    int     jwt_expiry;              /* seconds */
    int     concurrency;
    int     connect_timeout;         /* seconds */
    int     debug;
} rest2sql_config_t;

extern rest2sql_config_t rest2sql_config;

/* ============================================================
 *  Plugin system variables (sysvars)
 *
 *  have_rest2sql  — read-only string, always "YES" when loaded.
 *                   Mirrors the pattern of have_ssl / have_openssl.
 *  rest2sql_enabled — bool, togglable at runtime via SET GLOBAL.
 *                   Controls whether the plugin processes requests.
 * ============================================================ */
extern char     have_rest2sql_val[];   /* defined in rest2sql.c */
extern my_bool  rest2sql_enabled;      /* defined in rest2sql.c */

#endif // GLOBALS_H
