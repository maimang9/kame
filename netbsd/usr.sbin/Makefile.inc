#	$NetBSD: Makefile.inc,v 1.5 1997/10/18 06:42:25 lukem Exp $
#	from: @(#)Makefile.inc	8.1 (Berkeley) 6/6/93

BINDIR?=	/usr/local/v6/sbin
MANDIR=	/usr/local/v6/man
ETCDIR=	/usr/local/v6/etc
DOCDIR=	/usr/local/v6/share/doc
CFLAGS+=-DHAVE_SCOPELIB
CFLAGS+=-DNO_PF
