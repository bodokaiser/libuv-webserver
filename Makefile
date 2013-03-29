LDFLAGS = -framework Foundation \
          -framework CoreFoundation \
          -framework ApplicationServices \
          -Ldeps/libuv

INCLFLAGS = -Ideps/http-parser

main: libuv.a http-parser.o
	$(CC) -v -o main src/main.c $(INCLFLAGS) deps/libuv/libuv.a deps/http-parser/http_parser.o $(LDFLAGS)

libuv.a:
	$(MAKE) -C deps/libuv libuv.a

http-parser.o:
	$(MAKE) -C deps/http-parser http_parser.o

clean:
	rm deps/libuv/libuv.a
	rm deps/http-parser/http_parser.o
