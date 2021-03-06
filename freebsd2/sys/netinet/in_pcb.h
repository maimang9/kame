/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982, 1986, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)in_pcb.h	8.1 (Berkeley) 6/10/93
 * $Id: in_pcb.h,v 1.14.2.3 1997/09/30 16:25:04 davidg Exp $
 */

#ifndef _NETINET_IN_PCB_H_
#define _NETINET_IN_PCB_H_

#include <sys/queue.h>
#if 1 /*IPSEC*/
#include <netinet6/ipsec.h>
#endif

/*
 * Common structure pcb for internet protocol implementation.
 * Here are stored pointers to local and foreign host table
 * entries, local and foreign socket numbers, and pointers
 * up (to a socket structure) and down (to a protocol-specific)
 * control block.
 */
LIST_HEAD(inpcbhead, inpcb);

struct inpcb {
	LIST_ENTRY(inpcb) inp_list;		/* list for all PCBs of this proto */
	LIST_ENTRY(inpcb) inp_hash;		/* hash list */
	struct	inpcbinfo *inp_pcbinfo;	/* PCB list info */
	struct	in_addr inp_faddr;	/* foreign host table entry */
	struct	in_addr inp_laddr;	/* local host table entry */
	u_short	inp_fport;		/* foreign port */
	u_short	inp_lport;		/* local port */
	caddr_t	inp_ppcb;		/* pointer to per-protocol pcb */
	struct	socket *inp_socket;	/* back pointer to socket */
	struct	mbuf *inp_options;	/* IP options */
	struct	route inp_route;	/* placeholder for routing entry */
	int	inp_flags;		/* generic IP/datagram flags */
	u_char	inp_ip_tos;		/* type of service proto */
	u_char	inp_ip_ttl;		/* time to live proto */
	u_char	inp_ip_p;		/* protocol proto */
	u_char	pad[1];			/* alignment */
	struct	ip_moptions *inp_moptions; /* IP multicast options */
#if 0 /* Someday, perhaps... */
	struct	ip inp_ip;		/* header prototype; should have more */
#endif
#if 1 /*IPSEC*/
	struct inpcbpolicy *inp_sp;	/* security policy. */
#endif
};

struct inpcbinfo {
	struct inpcbhead *listhead;
	struct inpcbhead *hashbase;
	unsigned long hashmask;
	unsigned short lastport;
	unsigned short lastlow;
	unsigned short lasthi;
};

#define INP_PCBHASH(faddr, lport, fport, mask) \
	(((faddr) ^ ((faddr) >> 16) ^ (lport) ^ (fport)) & (mask))

/* flags in inp_flags: */
#define	INP_RECVOPTS		0x01	/* receive incoming IP options */
#define	INP_RECVRETOPTS		0x02	/* receive IP options for reply */
#define	INP_RECVDSTADDR		0x04	/* receive IP dst address */
#define	INP_HDRINCL		0x08	/* user supplies entire IP header */
#define	INP_HIGHPORT		0x10	/* user wants "high" port binding */
#define	INP_LOWPORT		0x20	/* user wants "low" port binding */
#define	INP_ANONPORT		0x40	/* port chosen for user */
#define	INP_RECVIF		0x80	/* receive incoming interface */
#define INP_FAITH		0x100	/* accept FAITH'ed connections */
#define	INP_CONTROLOPTS		(INP_RECVOPTS|INP_RECVRETOPTS|INP_RECVDSTADDR|\
					INP_RECVIF)
#define	INPLOOKUP_WILDCARD	1

#define	sotoinpcb(so)	((struct inpcb *)(so)->so_pcb)

#ifdef KERNEL
void	 in_losing __P((struct inpcb *));
int	 in_pcballoc __P((struct socket *, struct inpcbinfo *));
int	 in_pcbbind __P((struct inpcb *, struct mbuf *));
int	 in_pcbconnect __P((struct inpcb *, struct mbuf *));
void	 in_pcbdetach __P((struct inpcb *));
void	 in_pcbdisconnect __P((struct inpcb *));
int	 in_pcbladdr __P((struct inpcb *, struct mbuf *,
	    struct sockaddr_in **));
struct inpcb *
	 in_pcblookup __P((struct inpcbinfo *,
	    struct in_addr, u_int, struct in_addr, u_int, int));
struct inpcb *
	 in_pcblookuphash __P((struct inpcbinfo *,
	    struct in_addr, u_int, struct in_addr, u_int, int));
void	 in_pcbnotify __P((struct inpcbhead *, struct sockaddr *,
	    u_int, struct in_addr, u_int, int, void (*)(struct inpcb *, int)));
void	 in_pcbrehash __P((struct inpcb *));
void	 in_setpeeraddr __P((struct inpcb *, struct mbuf *));
void	 in_setsockaddr __P((struct inpcb *, struct mbuf *));
extern struct sockaddr_in *in_selectsrc __P((struct sockaddr_in *,
	struct route *, int, struct ip_moptions *, int *));
#endif

#endif
