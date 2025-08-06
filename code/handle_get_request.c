#include <common.h>

// in CRUD : GET => READ
// resources : DATA , STRUCT(URE)
cJSON* handle_get_request(const char *url) {
// 1. URL parsing w/ sscanf
    char version[64] = {0};
    char resource[64] = {0};
    char schema[64] = {0};
    char table[64] = {0};
    char column[64] = {0};
    char value[256] = {0};
    char query[1024] = {0};
#if DEBUG == 1
    struct timeval tv;
    char timestamp[30];
    char microtimestamp[40];
    struct tm *local;
#endif
    
    cJSON *json_response = cJSON_CreateObject();

#if DEBUG == 1
//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "begin", microtimestamp);
#endif

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
// - resource in (ping, status) && 2 tokens
// - resource = structure && 4 tokens
// - resource = data && 6 tokens
// - bad request format
    
    if (strcasecmp(resource, "ping") == 0 && nb_tokens == 2) {
// PING
        cJSON_AddStringToObject(json_response, "ping", "pong");
       snprintf(query, sizeof(query), "SELECT now()");
        cJSON_AddStringToObject(json_response, "SQL", query);

    } else if (strcasecmp(resource, "status") == 0 && nb_tokens == 2) {
// STATUS
        snprintf(query, sizeof(query), "SHOW GLOBAL STATUS");
        cJSON_AddStringToObject(json_response, "SQL", query);

    } else if (strcasecmp(resource, "struct") == 0 && nb_tokens == 4) {
// STRUCTURE
        snprintf(query, sizeof(query), "SHOW COLUMNS FROM %s.%s", schema, table);
        cJSON_AddStringToObject(json_response, "SQL", query);

    } else if (strcasecmp(resource, "data") == 0 && nb_tokens == 6) {
// DATA
        snprintf(query, sizeof(query), "SELECT * FROM %s.%s WHERE %s='%s'", schema, table, column, value);
        cJSON_AddStringToObject(json_response, "SQL", query);

   } else {
// BAD REQUEST
        cJSON_AddStringToObject(json_response, "error", "BAD REQUEST");
        cJSON_AddNumberToObject(json_response, "httpcode", HTTP_BAD_REQUEST);

#if DEBUG == 1
//get current time with microsecond precision
        gettimeofday(&tv, NULL);
// Convert to local time and format as a string
        local = localtime(&tv.tv_sec);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
        snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
        cJSON_AddStringToObject(json_response, "end", microtimestamp);
#endif

        return json_response;
    }
        
#if DEBUG == 1
//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "after resource check", microtimestamp);
#endif

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

#if DEBUG == 1
//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "before cnx", microtimestamp);
#endif

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
#if DEBUG == 1
//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "after cnx handle", microtimestamp);
#endif

//mysql_options(conn, MYSQL_OPT_PROTOCOL, MYSQL_PROTOCOL_SOCKET);  // Force socket
// 4. statement execution
//    if(mysql_query(conn, query)) {
      if (mysql_real_query(conn, STRING_WITH_LEN(query))) {
        cJSON_AddStringToObject(json_response, "stmt exec", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_error(conn));
        cJSON_AddNumberToObject(json_response, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
        mysql_close(conn);

#if DEBUG == 1
//get current time with microsecond precision
        gettimeofday(&tv, NULL);
// Convert to local time and format as a string
        local = localtime(&tv.tv_sec);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
        snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
        cJSON_AddStringToObject(json_response, "after stmt exec KO", microtimestamp);
#endif

        return json_response;
    }
    cJSON_AddStringToObject(json_response, "stmt exec", "OK");

#if DEBUG == 1
//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "after stmt exec", microtimestamp);
#endif

//// creating the result buffer
    MYSQL_RES *result = mysql_store_result(conn);
    if(result == NULL) {
        cJSON_AddStringToObject(json_response, "result fetch", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_error(conn));
        cJSON_AddNumberToObject(json_response, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
        mysql_close(conn);
#if DEBUG == 1
//get current time with microsecond precision
        gettimeofday(&tv, NULL);
// Convert to local time and format as a string
        local = localtime(&tv.tv_sec);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
        snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
        cJSON_AddStringToObject(json_response, "after result buffer KO", microtimestamp);
#endif

        return json_response;
    }
    cJSON_AddStringToObject(json_response, "result fetch", "OK");

#if DEBUG == 1
//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "after result buffer", microtimestamp);
#endif

// 5. initializing resulset
    int num_rows = mysql_num_rows(result);
    int num_fields = mysql_num_fields(result);
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    MYSQL_ROW row;

// 6. resultset => JSON
    cJSON *data_array = cJSON_CreateArray();
    cJSON *fields_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(fields_obj, "num_fields", num_fields);
    cJSON *fields_array = cJSON_CreateArray();

    while ((row = mysql_fetch_row(result))) {
        cJSON *row_object = cJSON_CreateObject();

        for (int i = 0; i < num_fields; i++) {
            const char *field_name = fields[i].name;
            const char *field_value = row[i] ? row[i] : "NULL";
            cJSON_AddStringToObject(row_object, field_name, field_value);
            cJSON_AddItemToArray(fields_array, cJSON_CreateString(fields[i].name));
        }

        cJSON_AddItemToArray(data_array, row_object);
        cJSON_AddItemToObject(fields_obj, "names", fields_array);
    }

    cJSON_AddItemToObject(json_response, "fields", fields_obj);
    cJSON_AddNumberToObject(json_response, "rows", num_rows);
    cJSON_AddItemToObject(json_response, "data", data_array);

#if DEBUG == 1
//get current time with microsecond precision
    gettimeofday(&tv, NULL);
// Convert to local time and format as a string
    local = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
    snprintf(microtimestamp, sizeof(microtimestamp), "%s.%06ld", timestamp, tv.tv_usec);
    cJSON_AddStringToObject(json_response, "end", microtimestamp);
#endif

    cJSON_AddNumberToObject(json_response, "httpcode", HTTP_OK);

// Nettoyage
    mysql_free_result(result);
    mysql_close(conn);

    return json_response;
}
