#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include "http_parser.h"

/**
 * TCP on new connection callback.
 */
void connection_cb(uv_stream_t* server, int status);

/**
 * Reads data from tcp client.
 */
void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t);

/**
 * Writes data from read to the tcp client.
 */
void write_cb(uv_write_t* req, int status);

/**
 * Reads url from http request.
 */
void url_cb();

/**
 * Readers header field from http request.
 */
void header_field_cb();

/**
 * Allocates a new buffer for reading.
 */
uv_buf_t alloc_buffer(uv_handle_t* handle, size_t size);
