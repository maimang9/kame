#	$Id: Makefile,v 1.2 1995/05/30 07:58:41 rgrimes Exp $
#

MPROG=	fbsdboot.exe
CLEANFILES+=	${MPROG}
BINDIR=		/usr/mdec
BINMODE=	444
NOMAN=
STRIP=

all:	${MPROG}

install:	${MPROG}
	${INSTALL} ${COPY} ${STRIP} -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} \
	    ${INSTALLFLAGS} ${MPROG} ${DESTDIR}${BINDIR}


${MPROG}:	${MPROG}.uu
	uudecode < ${.CURDIR}/${MPROG}.uu

.include <bsd.prog.mk>
