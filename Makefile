LDFLAGS = -luv

all:
	$(CC) -o main.out src/main.c $(LDFLAGS)
