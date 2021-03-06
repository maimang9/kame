#	$NetBSD: Makefile,v 1.6.2.1 2004/06/22 07:55:00 tron Exp $

NOMAN= # defined

.include <bsd.own.mk>
.include <bsd.sys.mk>		# for HOST_SH

S=	${.CURDIR}/../../../..
MIPS=	${S}/arch/mips
COBALT=	${S}/arch/cobalt

# .PATH:	${.CURDIR}/../common

BINMODE?= 444

# XXX SHOULD NOT NEED TO DEFINE THESE!
LIBCRT0=
LIBC=
LIBCRTBEGIN=
LIBCRTEND=

.PHONY:		machine-links
beforedepend:	machine-links

machine-links:	machine cobalt mips
machine cobalt:
	-rm -f ${.TARGET}
	ln -s ${COBALT}/include ${.TARGET}

mips:
	-rm -f ${.TARGET}
	ln -s ${MIPS}/include ${.TARGET}

CLEANFILES+=	machine cobalt mips

realall: machine-links ${PROG}

# Load @15Mb boundary as most (all?) of the Cobalt boxes
# had been shipped with at least 16Mb.
#
# XXX The proper fix is to load at the kernel base address
# and to relocate itself at the end of available memory.
LOAD_ADDRESS?=0x80F00000

COMPORT?=0x0
COMBASE?=0xbc800000
COMSPEED?=115200
COMPROBE?=0xa020001c

AFLAGS+=	-D_LOCORE -D_KERNEL -DASSEMBLER -mno-abicalls

# -I${.CURDIR}/../.. done by Makefile.inc
CPPFLAGS+=	-nostdinc -D_STANDALONE -DNO_ABICALLS -D_NO_PROM_DEFINES
# CPPFLAGS+=	-D_DEBUG
CPPFLAGS+=	-I${.OBJDIR} -I${S} -I${S}/lib/libsa
CPPFLAGS+=	-DCONS_SERIAL -DCOMBASE=${COMBASE} -DCOMPORT=${COMPORT}
CPPFLAGS+=	-DCOMSPEED=${COMSPEED} -DCOMPROBE=${COMPROBE}

# compiler flags for smallest code size
CFLAGS=		-Os -mmemcpy -ffreestanding -mno-abicalls -msoft-float -G 128
OFORMAT=	--oformat elf32-littlemips

NETBSD_VERS!=	${HOST_SH} ${S}/conf/osrelease.sh
CPPFLAGS+=	-DNETBSD_VERS='"${NETBSD_VERS}"'

LDSCRIPT?=	${MIPS}/conf/stand.ldscript

PROG=		boot
# common sources
SRCS+=		start.S boot.c devopen.c conf.c clock.c bootinfo.c
SRCS+=		prf.c com.c cons.c ns16550.c pciide.c wdc.c wd.c

SRCS+=		vers.c
CLEANFILES+=	vers.c

### find out what to use for libkern
KERN_AS=	library
.include "${S}/lib/libkern/Makefile.inc"

### find out what to use for libz
Z_AS=		library
.include "${S}/lib/libz/Makefile.inc"

### find out what to use for libsa
SA_AS=		library
SAMISCMAKEFLAGS+=SA_USE_LOADFILE=yes SA_USE_CREAD=yes
.include "${S}/lib/libsa/Makefile.inc"

LIBS=		${KERNLIB} ${SALIB} ${ZLIB}

.PHONY: vers.c
vers.c: ${.CURDIR}/version
	${HOST_SH} ${S}/conf/newvers_stand.sh ${.CURDIR}/version "cobalt"

${PROG}: machine-links ${LDSCRIPT} ${OBJS} ${LIBS}
	${LD} ${OFORMAT} -Map ${PROG}.map -N -x -Ttext ${LOAD_ADDRESS} \
	    -T ${LDSCRIPT} -e start -o ${PROG} ${OBJS} ${LIBS}
	gzip -c9 ${PROG} > ${PROG}.gz
	@${SIZE} ${PROG}

CLEANFILES+=	${PROG}.map ${PROG}.elf ${PROG}.gz

cleandir distclean: cleanlibdir

cleanlibdir:
	-rm -rf lib

.include <bsd.prog.mk>
