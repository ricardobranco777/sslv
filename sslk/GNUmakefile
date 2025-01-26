CFLAGS	:= -Wall -Wextra -fPIC
LDFLAGS	:= -shared

LIB	:= libsslk.so

$(LIB):	sslk.c
	$(LINK.c) $^ -o $@

.PHONY:	clean
clean:
	$(RM) $(LIB)

.PHONY:	test
test:
	@./test.sh

.PHONY:	shellcheck
shellcheck:
	@shellcheck *.sh
