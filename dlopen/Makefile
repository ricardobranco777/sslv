OPSYS:sh = uname

PROG=	test
SRCS=	test.c xdlopen.c

WARNS=	3
CFLAGS+= -O2
.if ${OPSYS} == "NetBSD"
WARNS= 2
LDADD+= -lrt
.endif

MK_DEBUG_FILES= no
MAN=

testit: test
	@./test | grep -e FAIL -e PASS -e deleted -e libcrypto -e fd/ -e shm/

.include <bsd.prog.mk>
