CC=gcc
COPTS=-g -Wall

all: watch

watch: watch.c
	$(CC) $(COPTS) -o watch watch.c

clean:
	rm -f watch watch.o
	
install:
	strip watch
	install watch /usr/local/bin/watch
