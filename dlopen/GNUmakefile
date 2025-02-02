
BIN	= test
CC	:= gcc
CFLAGS	+= -O2 -Wall -Wextra

OSTYPE	!= uname
SRCS	= test.c xdlopen.c

$(BIN):	$(SRCS)
	$(CC) -o $@ $(SRCS) $(CFLAGS)

.PHONY: testit
testit: test
	@./test | grep -e FAIL -e PASS -e deleted -e libcrypto -e fd/

.PHONY: clean
clean:
	$(RM) $(BIN) *.o
