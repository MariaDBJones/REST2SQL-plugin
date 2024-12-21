#include "common.h"

#define GETLOG 1

char* handle_get_request(const char *url) {
// initialize the JSON answer
    cJSON *json = cJSON_CreateObject();
    char *answerget;
    char version[64]="v1";
    char resource[64]="";
    char schema[64]="";
    char object[64]="";
    char column[64]="";
    char value[64]="";
    char *query=NULL;
    unsigned int action = 0;
    MYSQL *cnx;
// 2 = sql needed
// 1 = direct answere
// 0 = exec not possible

// extracting tokens from url
    int tokens = sscanf(url, "/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]", version, resource, schema, object, column, value);
// end token extraction

int isresourcent=0;
if (strchr(resource, '\0') != NULL) {
// String is null-terminated
    isresourcent=1;
}
    cJSON_AddStringToObject(json, "status", "HANDLER");
    cJSON_AddStringToObject(json, "method", "GET");
    cJSON_AddStringToObject(json, "url", url);
    cJSON_AddNumberToObject(json, "tokens", tokens);

// analyzing toeksn to match patterns and resources
// 4 possibilites :
// - tokens = 2 && resource in (ping, healthcheck, status)
// - tokens = 4 && resource = tables
// - tokens = 6 && resource = tables
// - bad request format
switch(tokens) {
    case 2:
#if GETLOG ==1
      cJSON_AddStringToObject(json, "version", version);
      cJSON_AddStringToObject(json, "resource", resource);
      cJSON_AddNumberToObject(json, "strlen resource",strlen(resource));
      cJSON_AddNumberToObject(json, "sizeof resrouce",sizeof(resource));
      cJSON_AddNumberToObject(json, "strchr resource null is not null", isresourcent);
#endif // GETLOG
      if ( strncasecmp(resource, "ping", 63) == 0 ) {
         cJSON_AddStringToObject(json, "action", "PING");
         cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
         cJSON_AddStringToObject(json, "message", "pong");
      } else if ( strncasecmp(resource, "status", 63) == 0 ) {
         char tmp[1]="";
         int length = snprintf(tmp, 1, "SELECT * FROM INFORMATION_SCHEMA.GLOBAL_STATUS");
         query=malloc(length+1);
         snprintf(query, length+1 , "SELECT * FROM INFORMATION_SCHEMA.GLOBAL_STATUS");
#if GETLOG == 1
         cJSON_AddStringToObject(json, "action", "STATUS");
         cJSON_AddNumberToObject(json, "query length", length);
         cJSON_AddStringToObject(json, "expected query", "SELECT * FROM INFORMATION_SCHEMA.GLOBAL_STATUS");
         cJSON_AddNumberToObject(json, "final query length", strlen(query));
#endif // GETLOG
      }  else if ( strncasecmp(resource, "healthcheck",63) == 0 ) {
         char tmp[1]="";
         int length = snprintf(tmp, 1, "SELECT now() as NOW");
         query=malloc(length+1);
         snprintf(query, length+1, "SELECT now() as NOW");
#if GETLOG == 1
         cJSON_AddStringToObject(json, "action", "HEALTHCHECK");
         cJSON_AddStringToObject(json, "expected query", "SELECT now() as NOW");
         cJSON_AddNumberToObject(json, "expected query length", length);
         cJSON_AddNumberToObject(json, "final query length", strlen(query));
#endif // GETLOG
      } else {
         cJSON_AddStringToObject(json, "action", "ERROR");
         cJSON_AddStringToObject(json, "error", "Invalid GET request");
         cJSON_AddNumberToObject(json, "httpcode", HTTP_BAD_REQUEST);
         answerget = cJSON_PrintUnformatted(json);
         free(query);
         cJSON_Delete(json);
         return answerget;
      }
    break; // end tokens=2
    case 4:
#if GETLOG == 1
      cJSON_AddStringToObject(json, "version", version);
      cJSON_AddStringToObject(json, "resource", resource);
      cJSON_AddStringToObject(json, "schema", schema);
      cJSON_AddStringToObject(json, "table", object);
      cJSON_AddNumberToObject(json, "sizeof resrouce",sizeof(resource));
      cJSON_AddNumberToObject(json, "strchr resource null is not null", isresourcent);
      cJSON_AddStringToObject(json, "action",   "QUERY");
#endif // GETLOG
      cJSON_AddStringToObject(json, "message",  "developped later");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
    break; // end tokens=4
   case 6:
#if GETLOG == 1
      cJSON_AddStringToObject(json, "version", version);
      cJSON_AddStringToObject(json, "resource", resource);
      cJSON_AddStringToObject(json, "schema", schema);
      cJSON_AddStringToObject(json, "table", object);
      cJSON_AddStringToObject(json, "column", column);
      cJSON_AddStringToObject(json, "value", value);
      cJSON_AddNumberToObject(json, "strlen resource",strlen(resource));
      cJSON_AddNumberToObject(json, "sizeof resrouce",sizeof(resource));
      cJSON_AddNumberToObject(json, "strchr resource null is not null", isresourcent);
      cJSON_AddStringToObject(json, "action",   "QUERY");
#endif // GETLOG
      char tmp[1]="";
      int length = snprintf(tmp, 1, "SELECT * FROM %s.%s WHERE %s = '%s'", schema, object, column, value);
      query=malloc(length+1);
      snprintf(query, length+1 , "SELECT * FROM %s.%s WHERE %s = '%s'", schema, object, column, value);
#if GETLOG == 1
      cJSON_AddNumberToObject(json, "query length", length);
      cJSON_AddNumberToObject(json, "final query length", strlen(query));
      cJSON_AddStringToObject(json, "query", query);
#endif
    break; // end tokens=6
    default:
      cJSON_AddStringToObject(json, "action", "ERROR");
      cJSON_AddStringToObject(json, "error", "Invalid GET request");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_BAD_REQUEST);
      answerget = cJSON_PrintUnformatted(json);
      cJSON_Delete(json);
      free(query);
      return answerget;
}
// end decision making

log_message("query time");
    if (query != NULL) {
// assigned
        cJSON_AddStringToObject(json, "todo", query);
log_message("cnx time");
// we establish internal local connexion
        cnx = mysql_init(NULL);
if (!cnx) {
log_message("cnx init failed");
          cJSON_AddStringToObject(json, "cnx status", "CONNECTION failed");
          cJSON_AddNumberToObject(json, "mariadbcode", mysql_errno(cnx));
          cJSON_AddNumberToObject(json, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
          // clean exit procedure
          char *json_string = cJSON_PrintUnformatted(json);
          cJSON_Delete(json);
          return json_string; // Caller is responsible for freeing this memory
}
log_message("cnx init ok");
//        if (mysql_real_connect_local(cnx) == NULL) {
          if (mysql_real_connect(cnx, "localhost", NULL, NULL, NULL, 3306, NULL, 0) == NULL ) {
log_message("cnx failed");
          cJSON_AddStringToObject(json, "cnx status", "CONNECTION failed");
          cJSON_AddNumberToObject(json, "mariadbcode", mysql_errno(cnx));
          cJSON_AddNumberToObject(json, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
          // clean exit procedure
          char *json_string = cJSON_PrintUnformatted(json);
          cJSON_Delete(json);
          return json_string; // Caller is responsible for freeing this memory
        } else {
log_message("cnx ok");
          cJSON_AddStringToObject(json, "cnx status", "CONNECTION OK");
          cJSON_AddNumberToObject(json, "mariadbcode", mysql_errno(cnx));
          cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
          mysql_close(cnx);
        }
    }
#if GETLOG == 1
else {
//not assigned
log_message("no query");
        cJSON_AddStringToObject(json, "todo", "nothing");
    }
#endif // GETLOG

// housekeeping
    answerget = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    free(query);
//end housekeeping

return answerget;
} // end function
