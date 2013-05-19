#include "webserver.h"
#include <unistd.h>

#define RESPONSE \
    "HTTP/1.1 200 OK\r\n" \
    "Content-Type: text/plain\r\n" \
    "Content-Length: 12\r\n" \
    "\r\n" \
    "Hello World\n" \

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
* HTTP-Parser.
*/
static http_parser* parser;

/**
* HTTP-Parser settings.
*/
static http_parser_settings parser_settings;

int main(int argc, const char** argv) {
    loop = uv_default_loop();

    struct sockaddr_in addr = uv_ip4_addr("127.0.0.1", 3000);

    uv_tcp_init(loop, &server);
    uv_tcp_bind(&server, addr);

    resp_buf.base = RESPONSE;
    resp_buf.len = sizeof(RESPONSE);

    parser_settings.on_headers_complete = headers_complete_cb;
    
    int r = uv_listen((uv_stream_t*) &server, 128, connection_cb);

    if (r) {
        fprintf(stderr, "Error on tcp listen: %s.\n", 
                uv_strerror(uv_last_error(loop)));
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}

void connection_cb(uv_stream_t* server, int status) {
    http_client_t* client = malloc(sizeof(http_client_t));

    if (status == -1) {
        fprintf(stderr, "Error on connection: %s.\n",
                uv_strerror(uv_last_error(loop)));
    }

    uv_tcp_init(loop, (uv_tcp_t*) &client->stream);
    
    client->stream.data = client;
    client->parser.data = client;

    if (uv_accept(server, &client->stream) == 0) {
        http_parser_init(&client->parser, HTTP_REQUEST);

        uv_read_start(&client->stream, alloc_buffer, read_cb);
    } else {
        uv_close((uv_handle_t*) &client->stream, NULL);
    }
}

void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
    http_client_t* client = stream->data;

    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF) {
            fprintf(stderr, "Error on reading: %s.\n", 
                    uv_strerror(uv_last_error(loop)));
        }

        uv_close((uv_handle_t*) stream, NULL);
    }

    int parsed = http_parser_execute(&client->parser, &parser_settings, 
            buf.base, nread); 

    if (parsed < nread) {
        fprintf(stderr, "Error on parsing HTTP request: \n");
    }

    write(1, buf.base, nread);

    free(buf.base);
}

void write_cb(uv_write_t* req, int status) {
    printf("sent response\n");
}

int headers_complete_cb(http_parser* parser) {
    http_client_t* client = (http_client_t*) parser->data;
    
    uv_write_t* req = malloc(sizeof(uv_write_t));
        
    uv_write(req, &client->stream, &resp_buf, 1, write_cb);

    return 1;
}

uv_buf_t alloc_buffer(uv_handle_t* handle, size_t size) {
    return uv_buf_init((char*) malloc(size), size);
}
