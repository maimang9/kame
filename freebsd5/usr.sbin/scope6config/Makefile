SRCDIR=	${.CURDIR}/../../kame/scope6config
.PATH:	${SRCDIR}

PROG=	scope6config
CFLAGS+=	-DINET6
NOMAN=	yes

LDADD+=	-L${.OBJDIR}/../libinet6 -L${.OBJDIR}/../libinet6/obj \
	-L/usr/local/v6/lib -linet6
DPADD+=	${.OBJDIR}/../libinet6/libinet6.a \
	${.OBJDIR}/../libinet6/obj/libinet6.a \
	/usr/local/v6/lib/libinet6.a

.include <bsd.prog.mk>
