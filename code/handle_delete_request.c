#include "common.h"
#include "handle_http_request.h"
#include "handle_delete_request.h"

// in CRUD : DELETE => DELETE
// resources : DATA
cJSON* handle_delete_request(const char *url) {
    cJSON *r = cJSON_CreateObject();
    if (r == NULL) return NULL;
    cJSON_AddStringToObject(r, "url", url ? url : "");
#if DELETEMETHODCORK == 0
    http_set_error(r, "DELETE not available",
                   HTTP_NOT_FOUND);  
#else
    http_set_error(r, "DELETE disabled", 
                   HTTP_NOT_FOUND);
#endif
    return r;
}
