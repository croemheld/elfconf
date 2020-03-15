CC	:= gcc

all: global.o

global.o: global.c
	$(CC) -o $@ $<

.PHONY: clean
clean:
	rm -f global.o
