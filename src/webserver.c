#include "webserver.h"

uv_loop_t* loop;

int main(int argc, const char** argv) {
    loop = uv_default_loop();

    struct sockaddr_in addr = uv_ip4_addr("127.0.0.1", 3000);

    uv_tcp_t server;
    uv_tcp_init(loop, &server);
    uv_tcp_bind(&server, addr);
    
    int r = uv_listen((uv_stream_t*) &server, 128, on_connection_cb);

    if (r) {
        fprintf(stderr, "Error on tcp listen: %s.\n", 
                uv_strerror(uv_last_error(loop)));
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}

void on_connection_cb(uv_stream_t* server, int status) {
    uv_stream_t* stream = malloc(sizeof(uv_tcp_t));

    if (status == -1) {
        fprintf(stderr, "Error on connection: %s.\n",
                uv_strerror(uv_last_error(loop)));
    }

    uv_tcp_init(loop, (uv_tcp_t*) stream);
                    
    if (uv_accept(server, stream) == 0) {
        uv_read_start(stream, alloc_buffer, read_cb);
    } else {
        uv_close((uv_handle_t*) stream, NULL);
    }
}

void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
    uv_write_t* req = malloc(sizeof(uv_write_t));

    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF) {
            fprintf(stderr, "Error on reading: %s.\n", 
                    uv_strerror(uv_last_error(loop)));
        }

        uv_close((uv_handle_t*) stream, NULL);
    }
    
    uv_write(req, stream, &buf, 1, write_cb);
}

void write_cb(uv_write_t* req, int status) {
    if (status == -1) {
        fprintf(stderr, "Error on writing: %s.\n", 
                uv_strerror(uv_last_error(loop)));
    }
    
    free(req);
}

uv_buf_t alloc_buffer(uv_handle_t* handle, size_t size) {
    return uv_buf_init((char*) malloc(size), size);
}
