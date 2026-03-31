#include "common.h"

cJSON *handle_post_request(const char *url,
                           const char *upload_data,
                           size_t *upload_data_size)
{
    (void)upload_data;
    (void)upload_data_size;
    cJSON *r = cJSON_CreateObject();
    if (r == NULL) return NULL;

    cJSON_AddStringToObject(r, "url", url ? url : "");

#if POSTMETHODCORK == 0
    http_set_error(r, "POST not available",
                   HTTP_METHOD_NOT_ALLOWED);  
#else
    http_set_error(r, "POST disabled", 
                   HTTP_METHOD_NOT_ALLOWED);
#endif
    
    return r;
}
