#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(const char *host, const char *url, const char *query_params,
                          char **cookies, int cookies_count, const char *auth_token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *cookie_header = calloc(LINELEN, sizeof(char));

    // Write the method, URL, and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add Authorization header if present
    if (auth_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_token);
        compute_message(message, line);
    }

    // Add cookies if present
    if (cookies != NULL) {
        strcat(cookie_header, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(cookie_header, cookies[i]);
            if (i != cookies_count - 1) {
                strcat(cookie_header, "; ");
            }
        }
        compute_message(message, cookie_header);
    }

    // End the headers
    compute_message(message, "");

    free(line);
    free(cookie_header);

    return message;
}

char *compute_post_request(const char *host, const char *url, const char *content_type, 
                           const char **body_data, int body_data_fields_count, 
                           char **cookies, int cookies_count, const char *auth_token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body = calloc(BUFLEN, sizeof(char));
    char *cookie_header = calloc(LINELEN, sizeof(char));

    // Write the method, URL, and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add Authorization header if present
    if (auth_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_token);
        compute_message(message, line);
    }

    // Add Content-Type header
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    // Add cookies if present
    if (cookies != NULL) {
        strcat(cookie_header, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(cookie_header, cookies[i]);
            if (i != cookies_count - 1) {
                strcat(cookie_header, "; ");
            }
        }
        compute_message(message, cookie_header);
    }

    // Create the body
    for (int i = 0; i < body_data_fields_count; i++) {
        strcat(body, body_data[i]);
    }
    sprintf(line, "Content-Length: %zu", strlen(body));
    compute_message(message, line);

    // End the headers
    compute_message(message, "");

    // Add the body
    strcat(message, body);

    free(line);
    free(body);
    free(cookie_header);

    return message;
}

char *compute_delete_request(const char *host, const char *url, const char *query_params,
                             char **cookies, int cookies_count, const char *auth_token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *cookie_header = calloc(LINELEN, sizeof(char));

    // Write the method, URL, and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add Authorization header if present
    if (auth_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_token);
        compute_message(message, line);
    }

    // Add cookies if present
    if (cookies != NULL) {
        strcat(cookie_header, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(cookie_header, cookies[i]);
            if (i != cookies_count - 1) {
                strcat(cookie_header, "; ");
            }
        }
        compute_message(message, cookie_header);
    }

    // End the headers
    compute_message(message, "");

    free(line);
    free(cookie_header);

    return message;
}
