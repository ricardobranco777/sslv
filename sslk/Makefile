OS:sh = uname

SRCS=	sslk.c

.if $(OS) == "FreeBSD" || $(OS) == "MidnightBSD" || $(OS) == "DragonFly"

SHLIB_NAME= libsslk.so

WITHOUT_DEBUG_FILES=

.elif $(OS) == "NetBSD"

LIB=	sslk

SHLIB_MAJOR=	0
SHLIB_MINOR=	0
MKPICLIB=no
NOCTF=
NODEBUGLIB=
NOLINKLIB=
NOPROFILE=
NOSTATICLIB=

.elif $(OS) == "OpenBSD"

LIB=	sslk

SHLIB_MAJOR=	0
SHLIB_MINOR=	0
NOLIBSTATIC=
CLEANFILES+=	lib$(LIB).so

# OpenBSD doesn't let us create unversioned shared-libraries
# so we hijack the main target to make a hard-link later
.MAIN: ALL
ALL: lib$(LIB).so

.endif

.include <bsd.lib.mk>

.if $(OS) == "OpenBSD"
lib$(LIB).so: lib$(LIB).so.$(SHLIB_MAJOR).$(SHLIB_MINOR)
	ln lib$(LIB).so.$(SHLIB_MAJOR).$(SHLIB_MINOR) $@
.endif
