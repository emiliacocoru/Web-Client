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

// preluat din laboratorul 10: Protocolul HTTP 

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char** token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);
    sprintf(line, "Host: %s:8080", host);
    compute_message(message, line);

    // adaug heade-ul pentru token
    // în cazul în care acesta este necesar
    // și este dat ca parametru
    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s",token[0]);
        compute_message(message, line);
    }

    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Cookie: %s", cookies[0]);
        compute_message(message, line);
    }
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char** token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // header pentru token
    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s",token[0]);
        compute_message(message, line);
    }

    sprintf(line, "Host :%s:8080", host);
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", strlen(body_data[0]));
    compute_message(message, line);

    // header pentru cookie
    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Cookie: %s", cookies[0]);
        compute_message(message, line);
    }

    compute_message(message, "");

    if (body_data != NULL) {
        compute_message(message, body_data[0]);
    }
    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char** token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // singura diferență fața de get
    // este că realizez DELETE
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);
    sprintf(line, "Host: %s:8080", host);
    compute_message(message, line);

    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s",token[0]);
        compute_message(message, line);
    }
    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Cookie: %s", cookies[0]);
        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}