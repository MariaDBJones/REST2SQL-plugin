#include "common.h"

// in CRUD : DELETE => DELETE
// resources : DATA 
cJSON* handle_delete_request(const char *url) {
{
    cJSON *r = cJSON_CreateObject();
    if (r == NULL) return NULL;
    cJSON_AddStringToObject(r, "url", url ? url : "");
    http_set_error(r, "DELETE not available in this version",
                   HTTP_METHOD_NOT_ALLOWED);
    return r;
}
