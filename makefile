CC=gcc
CFLAGS = -Wall
all:
	$(CC) $(CFLAGS) main2.c -o family
clean:
	rm family