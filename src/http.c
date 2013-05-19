#ifndef HTTP_C
#define HTTP_C
#include "http.h"
#include <stdio.h>
#include <string.h>

/**
 * Bind our parser helpers to the parser settings.
 */
void http_request_apply_settings(http_parser_settings* settings) {
    settings->on_url = url_cb;
    
    settings->on_header_field = header_field_cb;
    settings->on_header_value = header_value_cb;
    settings->on_headers_complete = headers_complete_cb;
    
    settings->on_body = body_cb;
};

/**
 * Copies url string to request->url.
 */
int url_cb(http_parser* parser, const char* chunk, size_t len) {
    http_client_t* client = parser->data;
    http_request_t* request = &client->request;
    
    request->url = malloc(len+1);

    strncpy((char*) request->url, chunk, len);

    return 0;
}

/**
 * Header counter.
 */
int current_header_index = 0;

/**
 * Copy the header field name to the current header item.
 */
int header_field_cb(http_parser* parser, const char* chunk, size_t len) {
    http_client_t* client = parser->data;

    http_header_t* header = &client->request.headers[current_header_index];

    header->field_length = len;
    header->field = malloc(len+1);

    strncpy((char*) header->field, chunk, len);

    return 0;
}

/**
 * Now copy its assigned value.
 */
int header_value_cb(http_parser* parser, const char* chunk, size_t len) {
    http_client_t* client = parser->data;

    http_header_t* header = &client->request.headers[current_header_index];

    header->value = malloc(len+1);
    header->value_length = len;

    strncpy((char*) header->value, chunk, len);

    current_header_index += 1;

    return 0;
}

/**
 * Extract the method name.
 */
int headers_complete_cb(http_parser* parser) {
    http_client_t* client = parser->data;
    http_request_t* request = &client->request;

    const char* method = http_method_str(parser->method);

    request->method = malloc(sizeof(method));
    strncpy(request->method, method, strlen(method));

    return 0;
}

/**
 * And copy the body content.
 */
int body_cb(http_parser* parser, const char* chunk, size_t len) {
    http_client_t* client = parser->data;
    http_request_t* request = &client->request;

    request->body = malloc(len+1);
    request->body = chunk;

    return 0;
}

#endif
