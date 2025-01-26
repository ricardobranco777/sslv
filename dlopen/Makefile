OPSYS:sh = uname

PROG= test
SRCS= test.c xdlopen.c procmap.c

WARNS= 3
CFLAGS+= -O2
.if ${OPSYS} == "NetBSD"
LDADD+= -lrt
.endif

MK_DEBUG_FILES= no
MAN=

.include <bsd.prog.mk>
