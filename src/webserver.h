#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include "http.c"

/**
 * New tcp connection callback.
 */
void connection_cb(uv_stream_t* server, int status);

/**
 * Reads data from tcp client.
 */
void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t);

/**
 * Is executed when request fully parsed.
 * User can read all request options from "&parser->data.request".
 */
int complete_cb(http_parser* parser);

/**
 * Allocates a new buffer for reading.
 */
uv_buf_t alloc_buffer(uv_handle_t* handle, size_t size);

#endif
