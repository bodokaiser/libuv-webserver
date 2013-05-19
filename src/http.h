#ifndef HTTP_H
#define HTTP_H
#include "http_parser.h"
#include "webserver.h"

/**
 * Represents a single http header.
 */
typedef struct {
    const char* field;
    const char* value;
    size_t field_length;
    size_t value_length;
} http_header_t;

/**
 * Represents a http request.
 */
typedef struct {
    int version_major;
    int version_minor;

    char* url;
    char* method;
    
    http_header_t headers[20];
 
    const char* body;
} http_request_t;

/**
 * HTTP Client.
 *
 * The HTTP Client uses a TCP socket stream, a instance of HTTP Parser and a
 * usable write request to write our response to the socket stream.
 */
typedef struct {
    uv_write_t req;
    uv_stream_t stream;
    http_parser parser;
    http_request_t request;
} http_client_t;

int url_cb();
int status_cb();
int header_field_cb();
int header_value_cb();
int headers_complete_cb();
int body_cb();

#endif
