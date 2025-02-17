#ifndef REQUESTS_H
#define REQUESTS_H

char *compute_get_request(const char *host, const char *url, const char *query_params, char **cookies, int cookies_count, const char *auth_token);
char *compute_post_request(const char *host, const char *url, const char *content_type, const char **body_data, int body_data_fields_count, char **cookies, int cookies_count, const char *auth_token);
char *compute_delete_request(const char *host, const char *url, const char *query_params, char **cookies, int cookies_count, const char *auth_token);

#endif
