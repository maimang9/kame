# Copyright (c) 1996 WIDE Project. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modifications, are permitted provided that the above copyright notice
# and this paragraph are duplicated in all such forms and that any
# documentation, advertising materials, and other materials related to
# such distribution and use acknowledge that the software was developed
# by the WIDE Project, Japan. The name of the Project may not be used to
# endorse or promote products derived from this software without
# specific prior written permission. THIS SOFTWARE IS PROVIDED ``AS IS''
# AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
# LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE.

.if exists(${.CURDIR}/../Makefile.opsys)
.include "${.CURDIR}/../Makefile.opsys"
.endif

BINDIR=	$(PREFIX)/sbin
PROG=	rtadvd
SRCS=	rtadvd.c rrenum.c advcap.c if.c config.c timer.c dump.c

CC=	gcc

.if (${OPSYS} != "NetBSD")
CFLAGS+=-Wall
.endif
.if (${OPSYS} != "NetBSD")
CFLAGS+=-DINET6
.else
WARNS?=2
CPPFLAGS+=-DINET6
.endif
.if (${OPSYS} == "NetBSD") || (${OPSYS} == "OpenBSD")
LDADD+=	-lutil
DPADD+=	${LIBUTIL}
.endif
.if exists(/usr/local/v6/lib/libinet6.a)
LDADD+=	-L${.OBJDIR}/../libinet6 -L${.OBJDIR}/../libinet6/obj \
	-L/usr/local/v6/lib -linet6
DPADD+=	${.OBJDIR}/../libinet6/libinet6.a \
	${.OBJDIR}/../libinet6/obj/libinet6.a \
	/usr/local/v6/lib/libinet6.a
.endif
LDADD+=	-lcompat
DPADD+=	${LIBCOMPAT}

.if (${OPSYS} != "NetBSD") && (${OPSYS} != "OpenBSD")
MAN5=	rtadvd.conf.5
MAN8=	rtadvd.8
.else
MAN=	rtadvd.8 rtadvd.conf.5
.endif

.include <bsd.prog.mk>
