# $FreeBSD: src/sys/boot/i386/libi386/Makefile,v 1.15 1999/10/19 03:53:59 dcs Exp $
#
LIB=			i386
NOPIC=
NOPROFILE=
INTERNALLIB=		true
INTERNALSTATICLIB=	true

SRCS=	aout_freebsd.c biosdisk.c biosmem.c biospnp.c biospci.c \
	bootinfo.c comconsole.c devicename.c elf_freebsd.c gatea20.c \
	i386_copy.c i386_module.c time.c vidconsole.c

CFLAGS+=	-I${.CURDIR}/../../common -I${.CURDIR}/../btx/lib \
		-I${.CURDIR}/../../.. -I.

BOOT_COMCONSOLE_PORT?= 0x3f8
CFLAGS+=	-DCOMPORT=${BOOT_COMCONSOLE_PORT}

BOOT_COMCONSOLE_SPEED?= 9600
CFLAGS+=	-DCOMSPEED=${BOOT_COMCONSOLE_SPEED}

# Make the disk code more talkative
#CFLAGS+= -DDISK_DEBUG

# Include simple terminal emulation (cons25-compatible)
CFLAGS+= -DTERM_EMU

# Make "machine" required for all objects
# (based on the more complete case in sys/i386/boot/Makefile.inc)
${SRCS:M*.c:R:S/$/.o/g}: machine

# If it's not there, don't consider it a target
.if exists(${.CURDIR}/../../../i386/include)
beforedepend ${OBJS}: machine

machine:
	ln -sf ${.CURDIR}/../../../i386/include machine

.endif

CLEANFILES+=	machine

.include <bsd.lib.mk>
