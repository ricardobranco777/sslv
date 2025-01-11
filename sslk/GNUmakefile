CFLAGS	:= -Wall -Wextra -fPIC
LDFLAGS	:= -shared

LIB	:= sslk.so

$(LIB):	sslk.c
	$(LINK.c) $^ -o $@

.PHONY:	clean
clean:
	$(RM) $(LIB)
