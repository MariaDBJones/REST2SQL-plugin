#include "common.h"

cJSON *handle_patch_request(const char *url,
                            const char *upload_data,
                            size_t *upload_data_size)
{
    (void)upload_data;
    (void)upload_data_size;
    cJSON *r = cJSON_CreateObject();
    if (r == NULL) return NULL;
    cJSON_AddStringToObject(r, "url", url ? url : "");
#if PATCHMETHODCORK == 0
    http_set_error(r, "PATCH not available",
                   HTTP_NOT_FOUND);  
#else
    http_set_error(r, "PATCH disabled", 
                   HTTP_NOT_FOUND);
#endif
    return r;
}
