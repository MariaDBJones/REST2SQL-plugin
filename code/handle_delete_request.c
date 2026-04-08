#include "handle_delete_request.h"

// in CRUD : DELETE => DELETE
// resources : DATA 
cJSON* handle_delete_request(const char *url) {
{
    cJSON *r = cJSON_CreateObject();
    if (r == NULL) return NULL;
    cJSON_AddStringToObject(r, "url", url ? url : "");
#if DELETEMETHODCORK == 0
    http_set_error(r, "DELETE not available",
                   HTTP_METHOD_NOT_ALLOWED);  
#else
    http_set_error(r, "DELETE disabled", 
                   HTTP_METHOD_NOT_ALLOWED);
#endif
    return r;
}
