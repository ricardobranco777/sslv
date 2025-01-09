
BIN	= sslv
CC	:= gcc
CFLAGS	+= -O2 -Wall -Wextra

$(BIN):	sslv.c
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	@rm -f $(BIN)
