#include "webserver.h"

#define RESPONSE \
    "HTTP/1.1 200 OK\r\n" \
    "Content-Type: text/plain\r\n" \
    "Content-Length: 12\r\n" \
    "\r\n" \
    "Hello World\n" \

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
http_parser_settings settings;

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

    /* create tcp server and bind it to the address */
    uv_tcp_init(loop, &server);
    uv_tcp_bind(&server, addr);

    /* let the tcp server handle incoming connections */
    int r = uv_listen((uv_stream_t*) &server, 128, connection_cb);

    if (r) {
        fprintf(stderr, "Error on tcp listen: %s.\n", 
                uv_strerror(uv_last_error(loop)));
    }

    /* start the event loop */
    return uv_run(loop, UV_RUN_DEFAULT);
}

void connection_cb(uv_stream_t* server, int status) {
    /*
     * create a http_client instance - it mainly contains a collection
     * of structs we would else have to malloc after another 
     */
    http_client_t* client = malloc(sizeof(http_client_t));

    if (status == -1) {
        fprintf(stderr, "Error on connection: %s.\n",
                uv_strerror(uv_last_error(loop)));
    }

    /* create an extra tcp handle for the client */
    uv_tcp_init(loop, (uv_tcp_t*) &client->stream);
    
    /* set references so we can use our client in http_parser and libuv */
    client->stream.data = client;
    client->parser.data = client;

    /* use the http parser handlers defined in http.c */
    http_request_apply_settings(&settings);

    /* when finished call our complete callback */
    settings.on_message_complete = complete_cb;

    /* accept the created client */
    if (uv_accept(server, &client->stream) == 0) {
        /* initialize our http parser */
        http_parser_init(&client->parser, HTTP_REQUEST);
        /* start reading from the tcp client socket */
        uv_read_start(&client->stream, alloc_buffer, read_cb);
    } else {
        /* we seem to have an error and quit */
        uv_close((uv_handle_t*) &client->stream, NULL);
    }
}

void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
    /* get the client back */
    http_client_t* client = stream->data;

    /* handle error */
    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF) {
            fprintf(stderr, "Error on reading: %s.\n", 
                    uv_strerror(uv_last_error(loop)));
        }

        uv_close((uv_handle_t*) stream, NULL);
    }

    /*  call our http parser on the received tcp payload */
    int parsed = http_parser_execute(&client->parser, &settings, buf.base, 
            nread);

    if (parsed < nread) {
        fprintf(stderr, "Error on parsing HTTP request: \n");
        
        uv_close((uv_handle_t*) stream, NULL);
    }

    free(buf.base);
}

int complete_cb(http_parser* parser) {
    /* get ou client back and set shourtcut to our request struct */
    http_client_t* client = parser->data;
    http_request_t* request = &client->request;

    /* now print the ordered http request to console */
    printf("url: %s\n", request->url);
    printf("method: %s\n", request->method);
    for (int i = 0; i < 5; i++) {
        http_header_t* header = &client->request.headers[i];
        if (header->field)
            printf("Header: %s: %s\n", header->field, header->value);
    }
    printf("body: %s\n", request->body);
    printf("\r\n");

    /* lets send our short http hello world response and close the socket */
    uv_write(&client->req, &client->stream, &resp_buf, 1, NULL);
    uv_close((uv_handle_t*) &client->stream, NULL);

    return 0;
}

uv_buf_t alloc_buffer(uv_handle_t* handle, size_t size) {
    return uv_buf_init((char*) malloc(size), size);
}
