#include <common.h>
#include <mysql/mysql.h>
#include <time.h>
#include <sys/time.h>

// in CRUD : GET => READ
cJSON* handle_get_request(const char *url) {
// 1. URL parsing w/ sscanf
    char version[64] = {0};
    char resource[64] = {0};
    char schema[64] = {0};
    char table[64] = {0};
    char column[64] = {0};
    char value[256] = {0};
    char sql_query[1024] = {0};
    struct timeval tv;
    char timestamp[30];
    char microtimestamp[40];
    struct tm *local;

    cJSON *json_response = cJSON_CreateObject();

//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "begin", microtimestamp);

    int nb_tokens = sscanf(url, "/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%255s",
                          version, resource, schema, table, column, value);

    cJSON_AddStringToObject(json_response, "apiversion", version);
    cJSON_AddStringToObject(json_response, "url", url);

// removing eventual trailing / to value
    size_t length = strlen(value);
    while (length > 0 && value[length - 1] == '/') {
        value[length - 1] = '\0';
        length--;
    }

// 2. analyzing tokens to match patterns and resources
// 3 possibilites :
// - resource in (ping, health, status)
// - resource = READ && 6 tokens
// - bad request format

//// Testing resource type
    if (strcasecmp(resource, "ping") == 0) {
// PING
        cJSON_AddStringToObject(json_response, "ping", "pong");
        cJSON_AddNumberToObject(json_response, "httpcode", HTTP_OK);
        return json_response;
    } else if (strcasecmp(resource, "health") == 0) {
// HEALTHCHECK
       snprintf(sql_query, sizeof(sql_query),
             "SELECT now()");
        cJSON_AddStringToObject(json_response, "SQL", sql_query);
    } else if (strcasecmp(resource, "status") == 0) {
// GLOBAL STATUS
        snprintf(sql_query, sizeof(sql_query),
             "SHOW GLOBAL STATUS");
        cJSON_AddStringToObject(json_response, "SQL", sql_query);
    } else if (strcasecmp(resource, "read") == 0 && nb_tokens == 6) {
// READ
        snprintf(sql_query, sizeof(sql_query),
             "SELECT * FROM %s.%s WHERE %s='%s'",
             schema, table, column, value);
        cJSON_AddStringToObject(json_response, "SQL", sql_query);
   } else {
// BAD REQUEST
        cJSON_AddStringToObject(json_response, "error", "BAD REQUEST");
        cJSON_AddNumberToObject(json_response, "httpcode", HTTP_BAD_REQUEST);
        return json_response;
    }

//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "after resource check", microtimestamp);

//// 3. Local connexion to MariaDB
// connexion handle
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        cJSON_AddStringToObject(json_response, "cnx handle", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_error(conn));
        cJSON_AddNumberToObject(json_response, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
        return json_response;
    }
     cJSON_AddStringToObject(json_response, "cnx handle", "OK");

//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "before cnx", microtimestamp);

// actual connexion
// through socket
    if(mysql_real_connect(conn, "localhost", "api", "passw0rd",
                      NULL, 0, "/var/lib/mysql/mysql.sock", 0) == NULL ) {
// through network port
//    if(mysql_real_connect(conn, "localhost", "api", "passw0rd",
//                  NULL, 3306, NULL, 0) == NULL ) {
        cJSON_AddStringToObject(json_response, "cnx", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_error(conn));
        cJSON_AddNumberToObject(json_response, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
        mysql_close(conn);
        return json_response;
    }
    cJSON_AddStringToObject(json_response, "cnx", "OK");

//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "after cnx handle", microtimestamp);

// Avant mysql_query, ajoutez :
//unsigned int timeout = 1;
//mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
//mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
//mysql_options(conn, MYSQL_OPT_PROTOCOL, MYSQL_PROTOCOL_SOCKET);  // Force socket
// 4. statement execution
    if(mysql_query(conn, sql_query)) {
        cJSON_AddStringToObject(json_response, "stmt exec", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_error(conn));
        cJSON_AddNumberToObject(json_response, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
        mysql_close(conn);
        return json_response;
    }
    cJSON_AddStringToObject(json_response, "stmt exec", "OK");

//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "after stmt exec", microtimestamp);

//// creating the result buffer
    MYSQL_RES *result = mysql_store_result(conn);
    if(result == NULL) {
        cJSON_AddStringToObject(json_response, "result fetch", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_error(conn));
        cJSON_AddNumberToObject(json_response, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
        mysql_close(conn);

//get current time with microsecond precision
        gettimeofday(&tv, NULL);
// Convert to local time and format as a string
        local = localtime(&tv.tv_sec);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
        snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
        cJSON_AddStringToObject(json_response, "after result buffer KO", microtimestamp);

        return json_response;
    }
    cJSON_AddStringToObject(json_response, "result fetch", "OK");

//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "after result buffer", microtimestamp);

// 5. answer fetching
    int num_rows = mysql_num_rows(result);
    int num_fields = mysql_num_fields(result);
//    MYSQL_FIELD *fields = mysql_fetch_fields(result);
//    MYSQL_ROW row;

// 6. Composition du JSON
//    cJSON *data_array = cJSON_CreateArray();

//    while ((row = mysql_fetch_row(result))) {
//        cJSON *row_object = cJSON_CreateObject();
//
//        for (int i = 0; i < num_fields; i++) {
//            const char *field_name = fields[i].name;
//            const char *field_value = row[i] ? row[i] : "NULL";
//            cJSON_AddStringToObject(row_object, field_name, field_value);
//        }
//
//        cJSON_AddItemToArray(data_array, row_object);
//    }

//    cJSON_AddItemToObject(json_response, "data", data_array);
    cJSON_AddNumberToObject(json_response, "rows", num_rows);
    cJSON_AddNumberToObject(json_response, "fields", num_fields);

//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "end", microtimestamp);

    cJSON_AddNumberToObject(json_response, "httpcode", HTTP_OK);

// Nettoyage
    mysql_free_result(result);
    mysql_close(conn);

    return json_response;
}
