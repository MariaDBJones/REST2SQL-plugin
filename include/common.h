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
#define HTTP_UNAUTHORIZED           401 // SQL privileges no good
#define HTTP_FORBIDDEN              403 // resource not exposed
#define HTTP_NOT_FOUND              404 // resource does not exists
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
#define PORT 3000
#define ADDRESS "0.0.0.0"

// TODO : obfuscate
#define APIUSER "restapiplugin"
#define APIPASSWORD "rA9$Gx2!kF6oW&uP"

#define DEBUG  1

#endif // GLOBALS_H
