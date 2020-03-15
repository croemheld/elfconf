CC	:= gcc

all: elfconf examples

elfconf: elfconf.c
	$(CC) -o $@ $<

.PHONY: examples
examples:
	$(MAKE) -C examples

.PHONY: clean
clean:
	rm -f elfconf
	$(MAKE) -C examples clean
