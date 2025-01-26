
BIN	= test
CC	:= gcc
CFLAGS	+= -O2 -Wall -Wextra

OSTYPE	!= uname
SRCS	= test.c xdlopen.c
ifeq ($(OSTYPE),SunOS)
SRCS	+= solaris.c
else
SRCS	+= procmap.c
endif

$(BIN):	$(SRCS)
	$(CC) -o $@ $(SRCS) $(CFLAGS)

.PHONY: clean
clean:
	@rm -f $(BIN)
