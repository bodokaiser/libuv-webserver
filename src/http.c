#ifndef HTTP_C
#define HTTP_C
#include "http.h"
#include <stdio.h>
#include <string.h>

void http_request_apply_settings(http_parser_settings* settings) {
    settings->on_header_field = header_field_cb;
    settings->on_header_value = header_value_cb;
    settings->on_headers_complete = headers_complete_cb;
    //http_request_settings.on_body = body_cb;
};

int url_cb(http_parser* parser, const char* chunk, size_t len) {
    http_client_t* client = parser->data;

    struct http_parser_url* url_s;
    http_parser_parse_url(chunk, len, 1, url_s);

    //client->request.url = url_s

    return 0;
}

int current_header_index = 0;

int header_field_cb(http_parser* parser, const char* chunk, size_t len) {
    http_client_t* client = parser->data;

    http_header_t* header = &client->request.headers[current_header_index];

    header->field_length = len;
    header->field = malloc(len+1);

    strncpy((char*) header->field, chunk, len);

    return 0;
}

int header_value_cb(http_parser* parser, const char* chunk, size_t len) {
    http_client_t* client = parser->data;

    http_header_t* header = &client->request.headers[current_header_index];

    header->value = malloc(len+1);
    header->value_length = len;

    strncpy((char*) header->value, chunk, len);

    current_header_index += 1;

    return 0;
}

int headers_complete_cb(http_parser* parser) {
    http_client_t* client = parser->data;

    return 0;
}

int body_cb(http_parser* parser) {

    return 0;
}

#endif
