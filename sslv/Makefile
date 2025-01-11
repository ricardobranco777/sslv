OPSYS:sh = uname

PROG= sslv
WARNS= 3
CFLAGS+= -O2
.if ${OPSYS} == "OpenBSD"
CFLAGS+= -Wall -Wmissing-prototypes -Wno-uninitialized -Wstrict-prototypes
.endif

MK_DEBUG_FILES= no
MAN=

.include <bsd.prog.mk>
