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
    http_set_error(r, "POST not available in this version",
                   HTTP_METHOD_NOT_ALLOWED);
    return r;
}
