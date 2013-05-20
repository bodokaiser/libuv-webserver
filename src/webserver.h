#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include "http_parser.h"

#define RESPONSE \
    "HTTP/1.1 200 OK\r\n" \
    "Content-Type: text/plain\r\n" \
    "Content-Length: 12\r\n" \
    "\r\n" \
    "Hello World\n" \

#define MAX_HTTP_HEADERS 20

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
 * Represents a http request with internal dependencies.
 *
 * - write request for sending the response
 * - reference to tcp socket as write stream
 * - instance of http_parser parser
 * - string of the http url
 * - string of the http method
 * - amount of total header lines
 * - http header array
 * - body content
 */
typedef struct {
    uv_write_t req;
    uv_stream_t stream;
    http_parser parser;
    char* url;
    char* method;
    int header_lines;
    http_header_t headers[MAX_HTTP_HEADERS];
    const char* body;
} http_request_t;

/**
 * New tcp connection callback.
 */
void tcp_new_connection_cb(uv_stream_t* server, int status);

/**
 * Reads data from tcp client.
 */
void tcp_read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t);

/**
 * Executed after write to tcp socket.
 */
void tcp_write_cb(uv_write_t* req, int status);

/**
 * Executed at begin of message.
 */
int http_message_begin_cb(http_parser* parser);

/**
 * Executed when parsed the url.
 */
int http_url_cb(http_parser* parser, const char* chunk, size_t len);

/**
 * Executed on each header field.
 */
int http_header_field_cb(http_parser* parser, const char* chunk, size_t len);

/**
 * Executed on each header value.
 */
int http_header_value_cb(http_parser* parser, const char* chunk, size_t len);

/**
 * Executed when completed header parsing.
 */
int http_headers_complete_cb(http_parser* parser);

/**
 * Executed on body
 */
int http_body_cb(http_parser* parser, const char* chunk, size_t len);

/**
 * Is executed when request fully parsed.
 * User can read all request options from "&parser->data.request".
 */
int http_message_complete_cb(http_parser* parser);

/**
 * Allocates a new buffer for reading.
 */
uv_buf_t alloc_buffer(uv_handle_t* handle, size_t size);

#endif
