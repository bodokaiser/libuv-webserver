#include "webserver.h"

/**
 * Storage for our buffer with the http response.
 */
static uv_buf_t resp_buf;

/**
* Pointer to libuv event loop.
*/
static uv_loop_t* loop;

/**
* libuv TCP server.
*/
static uv_tcp_t server;

/**
 * Settings for http_parser.
 *
 * The settings mostly are only a map of assigned callbacks.
 * In http.c we define a function "http_request_apply_settings" which set all
 * necessary parser handlers for us.
 */
static http_parser_settings settings;

/**
 * Executed when calling from shell.
 */
int main(int argc, const char** argv) {
    /* transforms an ip4 addr to a libuv usable struct */
    struct sockaddr_in addr = uv_ip4_addr("127.0.0.1", 3000);
    
    loop = uv_default_loop();

    /* set the http response to the buffer */
    resp_buf.base = RESPONSE;
    resp_buf.len = sizeof(RESPONSE);

    /* assign our parser handlers */
    settings.on_url = http_url_cb;
    settings.on_body = http_body_cb;
    settings.on_header_field = http_header_field_cb;
    settings.on_header_value = http_header_value_cb;
    settings.on_headers_complete = http_headers_complete_cb;
    settings.on_message_begin = http_message_begin_cb;
    settings.on_message_complete = http_message_complete_cb;

    /* create tcp server and bind it to the address */
    uv_tcp_init(loop, &server);
    uv_tcp_bind(&server, addr);

    /* let the tcp server handle incoming connections */
    int r = uv_listen((uv_stream_t*) &server, 128, tcp_new_connection_cb);

    if (r) {
        fprintf(stderr, "Error on tcp listen: %s.\n", 
                uv_strerror(uv_last_error(loop)));
    }

    /* start the event loop */
    return uv_run(loop, UV_RUN_DEFAULT);
}

void tcp_new_connection_cb(uv_stream_t* server, int status) {
    /* initialize a new http http_request struct */
    http_request_t* http_request = malloc(sizeof(http_request_t));

    if (status == -1) {
        fprintf(stderr, "Error on connection: %s.\n",
                uv_strerror(uv_last_error(loop)));
    }

    /* create an extra tcp handle for the http_request */
    uv_tcp_init(loop, (uv_tcp_t*) &http_request->stream);
    
    /* set references so we can use our http_request in http_parser and libuv */
    http_request->stream.data = http_request;
    http_request->parser.data = http_request;

    /* accept the created http_request */
    if (uv_accept(server, &http_request->stream) == 0) {
        /* initialize our http parser */
        http_parser_init(&http_request->parser, HTTP_REQUEST);
        /* start reading from the tcp http_request socket */
        uv_read_start(&http_request->stream, alloc_buffer, tcp_read_cb);
    } else {
        /* we seem to have an error and quit */
        uv_close((uv_handle_t*) &http_request->stream, NULL);
    }
}

void tcp_read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
    /* get back our http request*/
    http_request_t* http_request = stream->data;

    /* handle error */
    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF) {
            fprintf(stderr, "Error on reading: %s.\n", 
                    uv_strerror(uv_last_error(loop)));
        }

        uv_close((uv_handle_t*) stream, NULL);
    }

    /*  call our http parser on the received tcp payload */
    int parsed = http_parser_execute(&http_request->parser, &settings, 
            buf.base, nread);

    if (parsed < nread) {
        fprintf(stderr, "Error on parsing HTTP request: \n");
        
        uv_close((uv_handle_t*) stream, NULL);
    }

    free(buf.base);
}

/**
 * Closes current tcp socket after write.
 */
void tcp_write_cb(uv_write_t* req, int status) {
    uv_close((uv_handle_t*) req->handle, NULL);
}

/**
 * Initializes default values, counters.
 */
int http_message_begin_cb(http_parser* parser) {
    http_request_t* http_request = parser->data;

    http_request->header_lines = 0;

    return 0;
}

/**
 * Copies url string to http_request->url.
 */
int http_url_cb(http_parser* parser, const char* chunk, size_t len) {
    http_request_t* http_request = parser->data;
    
    http_request->url = malloc(len+1);

    strncpy((char*) http_request->url, chunk, len);

    return 0;
}

/**
 * Copy the header field name to the current header item.
 */
int http_header_field_cb(http_parser* parser, const char* chunk, size_t len) {
    http_request_t* http_request = parser->data;

    http_header_t* header = &http_request->headers[http_request->header_lines];

    header->field = malloc(len+1);
    header->field_length = len;

    strncpy((char*) header->field, chunk, len);

    return 0;
}

/**
 * Now copy its assigned value.
 */
int http_header_value_cb(http_parser* parser, const char* chunk, size_t len) {
    http_request_t* http_request = parser->data;

    http_header_t* header = &http_request->headers[http_request->header_lines];

    header->value_length = len;
    header->value = malloc(len+1);

    strncpy((char*) header->value, chunk, len);
    
    ++http_request->header_lines;

    return 0;
}

/**
 * Extract the method name.
 */
int http_headers_complete_cb(http_parser* parser) {
    http_request_t* http_request = parser->data;

    const char* method = http_method_str(parser->method);

    http_request->method = malloc(sizeof(method));
    strncpy(http_request->method, method, strlen(method));

    return 0;
}

/**
 * And copy the body content.
 */
int http_body_cb(http_parser* parser, const char* chunk, size_t len) {
    http_request_t* http_request = parser->data;

    http_request->body = malloc(len+1);
    http_request->body = chunk;

    return 0;
}

/**
 * Last cb executed by http_parser.
 *
 * In our case just logs the whole request to stdou.
 */
int http_message_complete_cb(http_parser* parser) {
    http_request_t* http_request = parser->data;

    /* now print the ordered http http_request to console */
    printf("url: %s\n", http_request->url);
    printf("method: %s\n", http_request->method);
    for (int i = 0; i < 5; i++) {
        http_header_t* header = &http_request->headers[i];
        if (header->field)
            printf("Header: %s: %s\n", header->field, header->value);
    }
    printf("body: %s\n", http_request->body);
    printf("\r\n");

    /* lets send our short http hello world response and close the socket */
    uv_write(&http_request->req, &http_request->stream, &resp_buf, 1, 
            tcp_write_cb);

    return 0;
}

/**
 * Allocates a buffer for us.
 */
uv_buf_t alloc_buffer(uv_handle_t* handle, size_t size) {
    return uv_buf_init((char*) malloc(size), size);
}
