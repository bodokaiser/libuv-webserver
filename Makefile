LDFLAGS = -luv

main: libuv http-parser
	$(CC) src/main.c -o main.out $(LDFLAGS) deps/libuv/libuv.a deps/http-parser/http_parser.o

libuv:
	$(MAKE) -C deps/libuv libuv.a

http-parser:
	$(MAKE) -C deps/http-parser http_parser.o

clean:
	rm deps/libuv/libuv.a
	rm deps/http-parser/http_parser.o
