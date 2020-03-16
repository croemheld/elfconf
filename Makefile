CC	:= gcc

# Build debug version with debug information (prints)
#
# Default configuration: D=0. To enable printing more
# information about the ELF to be configured, use D=1.

ifeq ("$(origin D)", "command line")
  DEBUG = $(D)
endif
ifndef DEBUG
  DEBUG = 0
endif

ifeq ($(DEBUG),1)
	CFLAGS += -DELFCONF_DEBUG
endif

all: elfconf examples

elfconf: elfconf.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: examples
examples:
	$(MAKE) -C examples

.PHONY: clean
clean:
	rm -f elfconf
	$(MAKE) -C examples clean
