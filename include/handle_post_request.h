#ifndef HANDLE_POST_REQUEST_H
#define HANDLE_POST_REQUEST_H

cJSON* handle_post_request(const char *url, const char *upload_data, size_t *upload_data_size);

#endif // HANDLE_POST_REQUEST_H
