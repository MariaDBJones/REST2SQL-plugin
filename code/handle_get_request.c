#include "common.h"
#include "handle_http.h"
#include "handle_get_request.h"

#define QUERY_MAX_LEN 1024

/* ============================================================
 *  Helpers DB locaux
 *  (seront migrés dans handle_mariadb.c dans la prochaine étape)
 * ============================================================ */

/**
 * Ouvre une connexion MariaDB via socket locale.
 * Credentials lus depuis les variables d'environnement.
 * Retourne le handle ou NULL en cas d'erreur (json_response rempli).
 */
static MYSQL *db_connect(cJSON *json_response)
{
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        http_set_error(json_response, "mysql_init failed",
                       HTTP_INTERNAL_SERVER_ERROR);
        return NULL;
    }

    unsigned int timeout = 5;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    const char *db_host   = getenv("DB_HOST")   ? getenv("DB_HOST")   : "localhost";
    const char *db_user   = getenv("DB_USER")   ? getenv("DB_USER")   : APIUSER;
    const char *db_passwd = getenv("DB_PASSWD") ? getenv("DB_PASSWD") : APIPASSWORD;
    const char *db_socket = getenv("DB_SOCKET") ? getenv("DB_SOCKET")
                                                : "/var/lib/mysql/mysql.sock";

    if (mysql_real_connect(conn, db_host, db_user, db_passwd,
                           NULL, 0, db_socket, 0) == NULL) {
        cJSON_AddStringToObject(json_response, "cnx", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_error(conn));
        cJSON_AddNumberToObject(json_response, "httpcode",
                                HTTP_INTERNAL_SERVER_ERROR);
        mysql_close(conn);
        return NULL;
    }
    return conn;
}

/**
 * Exécute une requête statique (sans paramètre utilisateur).
 * Retourne MYSQL_RES* ou NULL (json_response rempli).
 */
static MYSQL_RES *db_exec_static(MYSQL *conn, const char *query,
                                 cJSON *json_response)
{
    if (mysql_real_query(conn, query, (unsigned long)strlen(query))) {
        cJSON_AddStringToObject(json_response, "stmt exec", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_error(conn));
        cJSON_AddNumberToObject(json_response, "httpcode",
                                HTTP_INTERNAL_SERVER_ERROR);
        return NULL;
    }
    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        cJSON_AddStringToObject(json_response, "result fetch", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_error(conn));
        cJSON_AddNumberToObject(json_response, "httpcode",
                                HTTP_INTERNAL_SERVER_ERROR);
    }
    return result;
}

/**
 * Prépare, lie un seul paramètre STRING et exécute.
 * Retourne MYSQL_STMT* prêt à fetcher, ou NULL (json_response rempli).
 * L'appelant doit appeler mysql_stmt_close() après usage.
 */
static MYSQL_STMT *db_exec_prepared(MYSQL *conn, const char *query,
                                    const char *param_val,
                                    cJSON *json_response)
{
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        http_set_error(json_response, "mysql_stmt_init failed",
                       HTTP_INTERNAL_SERVER_ERROR);
        return NULL;
    }

    if (mysql_stmt_prepare(stmt, query, (unsigned long)strlen(query))) {
        cJSON_AddStringToObject(json_response, "stmt prepare", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_stmt_error(stmt));
        cJSON_AddNumberToObject(json_response, "httpcode",
                                HTTP_INTERNAL_SERVER_ERROR);
        mysql_stmt_close(stmt);
        return NULL;
    }

    MYSQL_BIND param;
    memset(&param, 0, sizeof(param));
    unsigned long plen = (unsigned long)strlen(param_val);
    param.buffer_type   = MYSQL_TYPE_STRING;
    param.buffer        = (void *)param_val;
    param.buffer_length = plen;
    param.length        = &plen;

    if (mysql_stmt_bind_param(stmt, &param) ||
        mysql_stmt_execute(stmt)            ||
        mysql_stmt_store_result(stmt)) {
        cJSON_AddStringToObject(json_response, "stmt exec", "KO");
        cJSON_AddStringToObject(json_response, "errno", mysql_stmt_error(stmt));
        cJSON_AddNumberToObject(json_response, "httpcode",
                                HTTP_INTERNAL_SERVER_ERROR);
        mysql_stmt_close(stmt);
        return NULL;
    }
    return stmt;
}

/**
 * Sérialise un MYSQL_RES* (requête statique) en JSON.
 */
static void db_static_result_to_json(MYSQL_RES *result, cJSON *json_response)
{
    unsigned int  num_fields = mysql_num_fields(result);
    my_ulonglong  num_rows   = mysql_num_rows(result);
    MYSQL_FIELD  *fields     = mysql_fetch_fields(result);
    MYSQL_ROW     row;

    cJSON *fields_array = cJSON_CreateArray();
    for (unsigned int i = 0; i < num_fields; i++)
        cJSON_AddItemToArray(fields_array,
                             cJSON_CreateString(fields[i].name));

    cJSON *fields_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(fields_obj, "num_fields", (double)num_fields);
    cJSON_AddItemToObject(fields_obj, "names", fields_array);

    cJSON *data_array = cJSON_CreateArray();
    while ((row = mysql_fetch_row(result)) != NULL) {
        cJSON *row_obj = cJSON_CreateObject();
        for (unsigned int i = 0; i < num_fields; i++) {
            cJSON_AddStringToObject(row_obj, fields[i].name,
                                    row[i] ? row[i] : "NULL");
        }
        cJSON_AddItemToArray(data_array, row_obj);
    }

    cJSON_AddItemToObject(json_response, "fields", fields_obj);
    cJSON_AddNumberToObject(json_response, "rows",  (double)num_rows);
    cJSON_AddItemToObject(json_response, "data",   data_array);
}

/**
 * Sérialise un MYSQL_STMT* (prepared statement) en JSON.
 */
static void db_stmt_result_to_json(MYSQL_STMT *stmt, cJSON *json_response)
{
    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    if (meta == NULL) {
        cJSON_AddNumberToObject(json_response, "rows", 0);
        return;
    }

    unsigned int  num_fields = mysql_num_fields(meta);
    MYSQL_FIELD  *fields     = mysql_fetch_fields(meta);
    my_ulonglong  num_rows   = mysql_stmt_num_rows(stmt);

    const size_t COL_BUF_SIZE = 4096;

    MYSQL_BIND    *bind_res   = calloc(num_fields, sizeof(MYSQL_BIND));
    char         **row_bufs   = calloc(num_fields, sizeof(char *));
    unsigned long *lengths    = calloc(num_fields, sizeof(unsigned long));
    my_bool       *is_nulls   = calloc(num_fields, sizeof(my_bool));

    if (!bind_res || !row_bufs || !lengths || !is_nulls) {
        free(bind_res); free(row_bufs); free(lengths); free(is_nulls);
        mysql_free_result(meta);
        http_set_error(json_response, "Result buffer allocation failed",
                       HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    for (unsigned int i = 0; i < num_fields; i++) {
        row_bufs[i] = malloc(COL_BUF_SIZE);
        if (row_bufs[i] == NULL) {
            for (unsigned int j = 0; j < i; j++) free(row_bufs[j]);
            free(bind_res); free(row_bufs); free(lengths); free(is_nulls);
            mysql_free_result(meta);
            http_set_error(json_response, "Column buffer allocation failed",
                           HTTP_INTERNAL_SERVER_ERROR);
            return;
        }
        bind_res[i].buffer_type   = MYSQL_TYPE_STRING;
        bind_res[i].buffer        = row_bufs[i];
        bind_res[i].buffer_length = COL_BUF_SIZE;
        bind_res[i].length        = &lengths[i];
        bind_res[i].is_null       = &is_nulls[i];
    }

    mysql_stmt_bind_result(stmt, bind_res);

    cJSON *fields_array = cJSON_CreateArray();
    for (unsigned int i = 0; i < num_fields; i++)
        cJSON_AddItemToArray(fields_array,
                             cJSON_CreateString(fields[i].name));

    cJSON *fields_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(fields_obj, "num_fields", (double)num_fields);
    cJSON_AddItemToObject(fields_obj, "names", fields_array);

    cJSON *data_array = cJSON_CreateArray();
    while (mysql_stmt_fetch(stmt) == 0) {
        cJSON *row_obj = cJSON_CreateObject();
        if (row_obj == NULL) break;
        for (unsigned int i = 0; i < num_fields; i++) {
            cJSON_AddStringToObject(row_obj, fields[i].name,
                                    is_nulls[i] ? "NULL" : row_bufs[i]);
        }
        cJSON_AddItemToArray(data_array, row_obj);
    }

    cJSON_AddItemToObject(json_response, "fields", fields_obj);
    cJSON_AddNumberToObject(json_response, "rows",  (double)num_rows);
    cJSON_AddItemToObject(json_response, "data",   data_array);

    for (unsigned int i = 0; i < num_fields; i++) free(row_bufs[i]);
    free(bind_res); free(row_bufs); free(lengths); free(is_nulls);
    mysql_free_result(meta);
}

/* ============================================================
 *  handle_get_request
 * ============================================================ */
cJSON *handle_get_request(const char *url)
{
    cJSON *json_response = cJSON_CreateObject();
    if (json_response == NULL)
        return NULL;

    /* Validation URL */
    if (!http_validate_url(url, json_response)) {
        HTTP_DEBUG_STAMP(json_response, "end");
        return json_response;
    }

    HTTP_DEBUG_STAMP(json_response, "begin");

    /* Parsing */
    char version[64]  = {0};
    char resource[64] = {0};
    char schema[64]   = {0};
    char table[64]    = {0};
    char column[64]   = {0};
    char value[296]   = {0};

    int nb_tokens = sscanf(url,
        "/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%63[^/]/%295s",
        version, resource, schema, table, column, value);

    cJSON_AddStringToObject(json_response, "apiversion", version);
    cJSON_AddStringToObject(json_response, "url", url);

    /* Suppression du slash final sur value */
    size_t vlen = strlen(value);
    while (vlen > 0 && value[vlen - 1] == '/') value[--vlen] = '\0';

    /* Validation identifiants SQL (ne peuvent pas être des paramètres ?) */
    const char *idents[] = { schema, table, column, NULL };
    for (int k = 0; idents[k] != NULL; k++) {
        if (*idents[k] != '\0' && !http_is_valid_sql_ident(idents[k])) {
            http_set_error(json_response, "Invalid identifier",
                           HTTP_BAD_REQUEST);
            HTTP_DEBUG_STAMP(json_response, "end");
            return json_response;
        }
    }

    HTTP_DEBUG_STAMP(json_response, "after validation");

    /* Connexion */
    MYSQL *conn = db_connect(json_response);
    if (conn == NULL) {
        HTTP_DEBUG_STAMP(json_response, "end");
        return json_response;
    }

    HTTP_DEBUG_STAMP(json_response, "after cnx");

    /* Dispatch */
    if (strcasecmp(resource, "ping") == 0 && nb_tokens == 2) {
        const char *q = "SELECT now()";
        cJSON_AddStringToObject(json_response, "SQL", q);
        MYSQL_RES *res = db_exec_static(conn, q, json_response);
        if (res == NULL) { mysql_close(conn); HTTP_DEBUG_STAMP(json_response, "end"); return json_response; }
        db_static_result_to_json(res, json_response);
        mysql_free_result(res);

    } else if (strcasecmp(resource, "status") == 0 && nb_tokens == 2) {
        const char *q = "SHOW GLOBAL STATUS";
        cJSON_AddStringToObject(json_response, "SQL", q);
        MYSQL_RES *res = db_exec_static(conn, q, json_response);
        if (res == NULL) { mysql_close(conn); HTTP_DEBUG_STAMP(json_response, "end"); return json_response; }
        db_static_result_to_json(res, json_response);
        mysql_free_result(res);

    } else if (strcasecmp(resource, "status") == 0 && nb_tokens == 3) {
        /* ✅ Prepared statement : filtre LIKE = paramètre ? */
        char like_val[sizeof(schema) + 2];
        snprintf(like_val, sizeof(like_val), "%%%s%%", schema);
        const char *q = "SHOW GLOBAL STATUS LIKE ?";
        cJSON_AddStringToObject(json_response, "SQL", q);
        MYSQL_STMT *stmt = db_exec_prepared(conn, q, like_val, json_response);
        if (stmt == NULL) { mysql_close(conn); HTTP_DEBUG_STAMP(json_response, "end"); return json_response; }
        db_stmt_result_to_json(stmt, json_response);
        mysql_stmt_close(stmt);

    } else if (strcasecmp(resource, "struct") == 0 && nb_tokens == 4) {
        /* schema et table validés [A-Za-z0-9_] → backticks suffisent */
        char q[QUERY_MAX_LEN];
        snprintf(q, sizeof(q), "SHOW COLUMNS FROM `%s`.`%s`", schema, table);
        cJSON_AddStringToObject(json_response, "SQL", q);
        MYSQL_RES *res = db_exec_static(conn, q, json_response);
        if (res == NULL) { mysql_close(conn); HTTP_DEBUG_STAMP(json_response, "end"); return json_response; }
        db_static_result_to_json(res, json_response);
        mysql_free_result(res);

    } else if (strcasecmp(resource, "data") == 0 && nb_tokens == 6) {
        /* ✅ Prepared statement : value = paramètre ?              */
        /* schema, table, column validés [A-Za-z0-9_] + backticks  */
        char q[QUERY_MAX_LEN];
        snprintf(q, sizeof(q),
                 "SELECT * FROM `%s`.`%s` WHERE `%s` = ?",
                 schema, table, column);
        cJSON_AddStringToObject(json_response, "SQL", q);
        MYSQL_STMT *stmt = db_exec_prepared(conn, q, value, json_response);
        if (stmt == NULL) { mysql_close(conn); HTTP_DEBUG_STAMP(json_response, "end"); return json_response; }
        db_stmt_result_to_json(stmt, json_response);
        mysql_stmt_close(stmt);

    } else {
        http_set_error(json_response, "Bad request", HTTP_BAD_REQUEST);
        mysql_close(conn);
        HTTP_DEBUG_STAMP(json_response, "end");
        return json_response;
    }

    cJSON_AddNumberToObject(json_response, "httpcode", HTTP_OK);
    HTTP_DEBUG_STAMP(json_response, "end");

    mysql_close(conn);
    return json_response;
}
