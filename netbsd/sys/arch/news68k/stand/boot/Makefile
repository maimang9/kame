#	$NetBSD: Makefile,v 1.6 2000/04/13 14:44:02 tsutsui Exp $

COMMON= ${.CURDIR}/../common
.ifnmake(print-objdir)
COMMONOBJ!=	cd ${COMMON}; ${MAKE} print-objdir
.endif

.PATH: ${COMMON}

PROG= boot
SRCS= locore.S boot.c devopen.c
SRCS+= romcalls.S putchar.c
MKMAN= no
STRIPFLAG=
BINMODE= 444

SIZE?= size
STRIP?= strip
OBJCOPY?= objcopy

S= ${.CURDIR}/../../../..

LDFLAGS= -x -N -Ttext 3e0000
CFLAGS= -Os -Wall

CPPFLAGS+= -D_STANDALONE # -DBOOT_DEBUG
CPPFLAGS+= -nostdinc -I${.OBJDIR} -I${S}
AFLAGS= -D_LOCORE

CLEANFILES+= ${PROG}.tmp machine m68k

L= ${COMMONOBJ}/lib
LIBS= ${L}/sa/libsa.a ${L}/kern/libkern.a ${L}/z/libz.a

.BEGIN: machine m68k
.NOPATH: machine m68k
realdepend realall: machine m68k

machine::
	-rm -f $@
	ln -s ${S}/arch/${MACHINE}/include $@

m68k::
	-rm -f $@
	ln -s ${S}/arch/m68k/include $@

${PROG}: ${OBJS} machine m68k
	${LD} ${LDFLAGS} -o ${PROG} ${OBJS} ${LIBS}
	-${SIZE} ${PROG}
	${STRIP} ${PROG}
	${OBJCOPY} -O binary ${PROG} ${PROG}.tmp
	mv ${PROG}.tmp ${PROG}

.include <bsd.prog.mk>
