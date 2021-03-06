/*	$OpenBSD: inet.c,v 1.84 2004/03/13 22:02:13 deraadt Exp $	*/
/*	$NetBSD: inet.c,v 1.14 1995/10/03 21:42:37 thorpej Exp $	*/

/*
 * Copyright (c) 1983, 1988, 1993
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
 * 3. Neither the name of the University nor the names of its contributors
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
 */

#ifndef lint
#if 0
static char sccsid[] = "from: @(#)inet.c	8.4 (Berkeley) 4/20/94";
#else
static const char *rcsid = "$OpenBSD: inet.c,v 1.84 2004/03/13 22:02:13 deraadt Exp $";
#endif
#endif /* not lint */

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>

#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/igmp_var.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_seq.h>
#define TCPSTATES
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_debug.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/ip_ipsp.h>
#include <netinet/ip_ah.h>
#include <netinet/ip_esp.h>
#include <netinet/ip_ipip.h>
#include <netinet/ip_ipcomp.h>
#include <netinet/ip_ether.h>
#ifdef DCCP
#include <netinet/dccp.h>
#define DCCPSTATES
#include <netinet/dccp_var.h>
#endif
#include <netinet/ip_carp.h>
#include <net/if.h>
#include <net/pfvar.h>
#include <net/if_pfsync.h>

#include <arpa/inet.h>
#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "netstat.h"

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>

struct	inpcb inpcb;
struct	tcpcb tcpcb;
#ifdef DCCP
struct  dccpcb dccpcb;
#endif
struct	socket sockb;

static void protopr0(u_long, char *, int);

char	*inetname(struct in_addr *);
void	inetprint(struct in_addr *, in_port_t, char *, int);
#ifdef INET6
char	*inet6name(struct in6_addr *);
void	inet6print(struct in6_addr *, int, char *, int);
#endif

/*
 * Print a summary of connections related to an Internet
 * protocol.  For TCP, also give state of connection.
 * Listening processes (aflag) are suppressed unless the
 * -a (all) flag is specified.
 */
void
protopr(u_long off, char *name)
{
	protopr0(off, name, AF_INET);
}

#ifdef INET6
void
ip6protopr(u_long off, char *name)
{
	protopr0(off, name, AF_INET6);
}
#endif

static void
protopr0(u_long off, char *name, int af)
{
	struct inpcbtable table;
	struct inpcb *head, *next, *prev;
	struct inpcb inpcb;
	int istcp, israw;
#ifdef DCCP
	int isdccp;
#endif
	int first = 1;
	char *name0;
	char namebuf[20];

	name0 = name;
	if (off == 0)
		return;
	istcp = strcmp(name, "tcp") == 0;
	israw = strncmp(name, "ip", 2) == 0;
#ifdef DCCP
	isdccp = strcmp(name, "dccp") == 0;
#endif
	kread(off, (char *)&table, sizeof table);
	prev = head =
	    (struct inpcb *)&((struct inpcbtable *)off)->inpt_queue.cqh_first;
	next = table.inpt_queue.cqh_first;

	while (next != head) {
		kread((u_long)next, (char *)&inpcb, sizeof inpcb);
		if (inpcb.inp_queue.cqe_prev != prev) {
			printf("???\n");
			break;
		}
		prev = next;
		next = inpcb.inp_queue.cqe_next;

		switch (af) {
		case AF_INET:
			if ((inpcb.inp_flags & INP_IPV6) != 0)
				continue;
			break;
		case AF_INET6:
			if ((inpcb.inp_flags & INP_IPV6) == 0)
				continue;
			break;
		default:
			break;
		}

		if (!aflag &&
		    inet_lnaof(inpcb.inp_laddr) == INADDR_ANY)
			continue;
		kread((u_long)inpcb.inp_socket, (char *)&sockb, sizeof (sockb));
		if (istcp) {
			kread((u_long)inpcb.inp_ppcb,
			    (char *)&tcpcb, sizeof (tcpcb));
		}
#ifdef DCCP
		if (isdccp) {
			kread((u_long)inpcb.inp_ppcb,
			    (char *)&dccpcb, sizeof (dccpcb));
		}
#endif
		if (first) {
			printf("Active Internet connections");
			if (aflag)
				printf(" (including servers)");
			putchar('\n');
			if (Aflag)
				printf("%-*.*s %-5.5s %-6.6s %-6.6s  %-18.18s %-18.18s %s\n",
				    PLEN, PLEN, "PCB", "Proto", "Recv-Q",
				    "Send-Q", "Local Address",
				    "Foreign Address", "(state)");
			else
				printf("%-5.5s %-6.6s %-6.6s  %-22.22s %-22.22s %s\n",
				    "Proto", "Recv-Q", "Send-Q",
				    "Local Address", "Foreign Address",
				    "(state)");
			first = 0;
		}
		if (Aflag) {
#ifdef DCCP
			if (istcp || isdccp)
#else
			if (istcp)
#endif
				printf("%*p ", PLEN, inpcb.inp_ppcb);
			else
				printf("%*p ", PLEN, prev);
		}
#ifdef INET6
		if (inpcb.inp_flags & INP_IPV6 && !israw) {
			strlcpy(namebuf, name0, sizeof namebuf);
			strlcat(namebuf, "6", sizeof namebuf);
			name = namebuf;
		} else
			name = name0;
#endif
		printf("%-5.5s %6ld %6ld ", name, sockb.so_rcv.sb_cc,
		    sockb.so_snd.sb_cc);
#ifdef INET6
		if (inpcb.inp_flags & INP_IPV6) {
			inet6print(&inpcb.inp_laddr6, (int)inpcb.inp_lport,
			    name, 1);
			inet6print(&inpcb.inp_faddr6, (int)inpcb.inp_fport,
			    name, 0);
		} else
#endif
		{
			inetprint(&inpcb.inp_laddr, (int)inpcb.inp_lport,
			    name, 1);
			inetprint(&inpcb.inp_faddr, (int)inpcb.inp_fport,
			    name, 0);
		}
		if (istcp) {
			if (tcpcb.t_state < 0 || tcpcb.t_state >= TCP_NSTATES)
				printf(" %d", tcpcb.t_state);
			else
				printf(" %s", tcpstates[tcpcb.t_state]);
		} else if (israw) {
			struct protoent *pe = NULL;
			u_int8_t proto;
#ifdef INET6
			if (inpcb.inp_flags & INP_IPV6)
				proto = inpcb.inp_ipv6.ip6_nxt;
			else
#endif
				proto = inpcb.inp_ip.ip_p;
			if (!nflag)
				pe = getprotobynumber(proto);
			if (pe)
				printf(" %s", pe->p_name);
			else
				printf(" %u", proto);
		}
#ifdef DCCP
		if (isdccp) {
			if (dccpcb.state >= DCCP_NSTATES)
				printf(" %d", dccpcb.state);
			else
				printf(" %s", dccpstates[dccpcb.state]);
		}
#endif
		putchar('\n');
	}
}

/*
 * Dump TCP statistics structure.
 */
void
tcp_stats(u_long off, char *name)
{
	struct tcpstat tcpstat;

	if (off == 0)
		return;
	printf("%s:\n", name);
	kread(off, (char *)&tcpstat, sizeof (tcpstat));

#define	p(f, m) if (tcpstat.f || sflag <= 1) \
	printf(m, tcpstat.f, plural(tcpstat.f))
#define	p1(f, m) if (tcpstat.f || sflag <= 1) \
	printf(m, tcpstat.f)
#define	p2(f1, f2, m) if (tcpstat.f1 || tcpstat.f2 || sflag <= 1) \
	printf(m, tcpstat.f1, plural(tcpstat.f1), tcpstat.f2, plural(tcpstat.f2))
#define	p2a(f1, f2, m) if (tcpstat.f1 || tcpstat.f2 || sflag <= 1) \
	printf(m, tcpstat.f1, plural(tcpstat.f1), tcpstat.f2)
#define	p3(f, m) if (tcpstat.f || sflag <= 1) \
	printf(m, tcpstat.f, plurales(tcpstat.f))

	p(tcps_sndtotal, "\t%u packet%s sent\n");
	p2(tcps_sndpack,tcps_sndbyte,
	    "\t\t%u data packet%s (%qd byte%s)\n");
	p2(tcps_sndrexmitpack, tcps_sndrexmitbyte,
	    "\t\t%u data packet%s (%qd byte%s) retransmitted\n");
	p(tcps_sndrexmitfast, "\t\t%qd fast retransmitted packet%s\n");
	p2a(tcps_sndacks, tcps_delack,
	    "\t\t%u ack-only packet%s (%u delayed)\n");
	p(tcps_sndurg, "\t\t%u URG only packet%s\n");
	p(tcps_sndprobe, "\t\t%u window probe packet%s\n");
	p(tcps_sndwinup, "\t\t%u window update packet%s\n");
	p(tcps_sndctrl, "\t\t%u control packet%s\n");
	p(tcps_outhwcsum, "\t\t%u packet%s hardware-checksummed\n");
	p(tcps_rcvtotal, "\t%u packet%s received\n");
	p2(tcps_rcvackpack, tcps_rcvackbyte, "\t\t%u ack%s (for %qd byte%s)\n");
	p(tcps_rcvdupack, "\t\t%u duplicate ack%s\n");
	p(tcps_rcvacktoomuch, "\t\t%u ack%s for unsent data\n");
	p2(tcps_rcvpack, tcps_rcvbyte,
	    "\t\t%u packet%s (%qu byte%s) received in-sequence\n");
	p2(tcps_rcvduppack, tcps_rcvdupbyte,
	    "\t\t%u completely duplicate packet%s (%qd byte%s)\n");
	p(tcps_pawsdrop, "\t\t%u old duplicate packet%s\n");
	p2(tcps_rcvpartduppack, tcps_rcvpartdupbyte,
	    "\t\t%u packet%s with some duplicate data (%qd byte%s duplicated)\n");
	p2(tcps_rcvoopack, tcps_rcvoobyte,
	    "\t\t%u out-of-order packet%s (%qd byte%s)\n");
	p2(tcps_rcvpackafterwin, tcps_rcvbyteafterwin,
	    "\t\t%u packet%s (%qd byte%s) of data after window\n");
	p(tcps_rcvwinprobe, "\t\t%u window probe%s\n");
	p(tcps_rcvwinupd, "\t\t%u window update packet%s\n");
	p(tcps_rcvafterclose, "\t\t%u packet%s received after close\n");
	p(tcps_rcvbadsum, "\t\t%u discarded for bad checksum%s\n");
	p(tcps_rcvbadoff, "\t\t%u discarded for bad header offset field%s\n");
	p1(tcps_rcvshort, "\t\t%u discarded because packet too short\n");
	p1(tcps_rcvnosec, "\t\t%u discarded for missing IPsec protection\n");
	p1(tcps_rcvmemdrop, "\t\t%u discarded due to memory shortage\n");
	p(tcps_inhwcsum, "\t\t%u packet%s hardware-checksummed\n");
	p(tcps_rcvbadsig, "\t\t%u bad/missing md5 checksum%s\n");
	p(tcps_rcvgoodsig, "\t\t%qd good md5 checksum%s\n");
	p(tcps_connattempt, "\t%u connection request%s\n");
	p(tcps_accepts, "\t%u connection accept%s\n");
	p(tcps_connects, "\t%u connection%s established (including accepts)\n");
	p2(tcps_closed, tcps_drops,
	    "\t%u connection%s closed (including %u drop%s)\n");
	p(tcps_conndrained, "\t%qd connection%s drained\n");
	p(tcps_conndrops, "\t%u embryonic connection%s dropped\n");
	p2(tcps_rttupdated, tcps_segstimed,
	    "\t%u segment%s updated rtt (of %u attempt%s)\n");
	p(tcps_rexmttimeo, "\t%u retransmit timeout%s\n");
	p(tcps_timeoutdrop, "\t\t%u connection%s dropped by rexmit timeout\n");
	p(tcps_persisttimeo, "\t%u persist timeout%s\n");
	p(tcps_keeptimeo, "\t%u keepalive timeout%s\n");
	p(tcps_keepprobe, "\t\t%u keepalive probe%s sent\n");
	p(tcps_keepdrops, "\t\t%u connection%s dropped by keepalive\n");
	p(tcps_predack, "\t%u correct ACK header prediction%s\n");
	p(tcps_preddat, "\t%u correct data packet header prediction%s\n");
	p3(tcps_pcbhashmiss, "\t%u PCB cache miss%s\n");

	p(tcps_ecn_accepts, "\t%u ECN connection%s accepted\n");
	p(tcps_ecn_rcvece, "\t\t%u ECE packet%s received\n");
	p(tcps_ecn_rcvcwr, "\t\t%u CWR packet%s received\n");
	p(tcps_ecn_rcvce, "\t\t%u CE packet%s received\n");
	p(tcps_ecn_sndect, "\t\t%u ECT packet%s sent\n");
	p(tcps_ecn_sndece, "\t\t%u ECE packet%s sent\n");
	p(tcps_ecn_sndcwr, "\t\t%u CWR packet%s sent\n");
	p1(tcps_cwr_frecovery, "\t\t\tcwr by fastrecovery: %u\n");
	p1(tcps_cwr_timeout, "\t\t\tcwr by timeout: %u\n");
	p1(tcps_cwr_ecn, "\t\t\tcwr by ecn: %u\n");

	p(tcps_badsyn, "\t%u bad connection attempt%s\n");
	p1(tcps_sc_added, "\t%qd SYN cache entries added\n");
	p(tcps_sc_collisions, "\t\t%qd hash collision%s\n");
	p1(tcps_sc_completed, "\t\t%qd completed\n");
	p1(tcps_sc_aborted, "\t\t%qd aborted (no space to build PCB)\n");
	p1(tcps_sc_timed_out, "\t\t%qd timed out\n");
	p1(tcps_sc_overflowed, "\t\t%qd dropped due to overflow\n");
	p1(tcps_sc_bucketoverflow, "\t\t%qd dropped due to bucket overflow\n");
	p1(tcps_sc_reset, "\t\t%qd dropped due to RST\n");
	p1(tcps_sc_unreach, "\t\t%qd dropped due to ICMP unreachable\n");
	p(tcps_sc_retransmitted, "\t%qd SYN,ACK%s retransmitted\n");
	p(tcps_sc_dupesyn, "\t%qd duplicate SYN%s received for entries "
		"already in the cache\n");
	p(tcps_sc_dropped, "\t%qd SYN%s dropped (no route or no space)\n");

#undef p
#undef p1
#undef p2
#undef p2a
#undef p3
}

/*
 * Dump UDP statistics structure.
 */
void
udp_stats(u_long off, char *name)
{
	struct udpstat udpstat;
	u_long delivered;

	if (off == 0)
		return;
	kread(off, (char *)&udpstat, sizeof (udpstat));
	printf("%s:\n", name);
#define	p(f, m) if (udpstat.f || sflag <= 1) \
	printf(m, udpstat.f, plural(udpstat.f))
#define	p1(f, m) if (udpstat.f || sflag <= 1) \
	printf(m, udpstat.f)

	p(udps_ipackets, "\t%lu datagram%s received\n");
	p1(udps_hdrops, "\t%lu with incomplete header\n");
	p1(udps_badlen, "\t%lu with bad data length field\n");
	p1(udps_badsum, "\t%lu with bad checksum\n");
	p1(udps_nosum, "\t%lu with no checksum\n");
	p(udps_inhwcsum, "\t%lu input packet%s hardware-checksummed\n");
	p(udps_outhwcsum, "\t%lu output packet%s hardware-checksummed\n");
	p1(udps_noport, "\t%lu dropped due to no socket\n");
	p(udps_noportbcast, "\t%lu broadcast/multicast datagram%s dropped due to no socket\n");
	p1(udps_nosec, "\t%lu dropped due to missing IPsec protection\n");
	p1(udps_fullsock, "\t%lu dropped due to full socket buffers\n");
	delivered = udpstat.udps_ipackets - udpstat.udps_hdrops -
	    udpstat.udps_badlen - udpstat.udps_badsum -
	    udpstat.udps_noport - udpstat.udps_noportbcast -
	    udpstat.udps_fullsock;
	if (delivered || sflag <= 1)
		printf("\t%lu delivered\n", delivered);
	p(udps_opackets, "\t%lu datagram%s output\n");
	p1(udps_pcbhashmiss, "\t%lu missed PCB cache\n");
#undef p
#undef p1
}

#ifdef DCCP
void
dccp_stats(off, name)
	u_long off;
	char *name;
{
	struct dccpstat dccpstat;

	if (off == 0)
		return;
	printf("%s:\n", name);
	kread(off, (char *)&dccpstat, sizeof (dccpstat));

#define	p(f, m) if (dccpstat.f || sflag <= 1) \
    printf(m, dccpstat.f, plural(dccpstat.f))
#define	p1a(f, m) if (dccpstat.f || sflag <= 1) \
    printf(m, dccpstat.f)
	p(dccps_ipackets, "\t%lu packet%s received\n");
	p(dccps_ibytes, "\t%lu byte%s received\n");
	p(dccps_connattempt, "\t%lu connection request%s\n");
	p(dccps_connects, "\t%lu connection%s established\n");
	p(dccps_drops, "\t%lu packet%s dropped\n");
	p(dccps_badlen, "\t%lu packet%s with bad data length field\n");
	p(dccps_badsum, "\t%lu packet%s with bad checksum\n");
	p(dccps_badseq, "\t%lu packet%s with bad sequencenumber\n");
	p(dccps_noport, "\t%lu packet%s dropped due to no socket\n");
	p(dccps_opackets, "\t%lu packet%s output\n");
	p(dccps_obytes, "\t%lu byte%s output\n");

	printf("\n\tTCPlike Sender:\n");
	p(tcplikes_send_conn, "\t%lu connection%s established\n");
	p(tcplikes_send_reploss, "\t%lu data packet%s reported lost\n");
	p(tcplikes_send_assloss, "\t%lu data packet%s assumed lost\n");
	p(tcplikes_send_ackrecv, "\t%lu acknowledgement%s received\n");
	p(tcplikes_send_missack, "\t%lu ack packet%s assumed lost\n");
	p(tcplikes_send_badseq, "\t%lu bad sequence number on outgoing packet%s\n");
	p(tcplikes_send_memerr, "\t%lu memory allocation error%s\n");

	printf("\tTCPlike Receiver:\n");
	p(tcplikes_recv_conn, "\t%lu connection%s established\n");
	p(tcplikes_recv_datarecv, "\t%lu data packet%s received\n");
	p(tcplikes_recv_ackack, "\t%lu Ack-on-ack%s received\n");
	p(tcplikes_recv_acksent, "\t%lu acknowledgement packet%s sent\n");
	p(tcplikes_recv_memerr, "\t%lu memory allocation error%s\n");

#undef p
#undef p1a
}
#endif

/*
 * Dump IP statistics structure.
 */
void
ip_stats(u_long off, char *name)
{
	struct ipstat ipstat;

	if (off == 0)
		return;
	kread(off, (char *)&ipstat, sizeof (ipstat));
	printf("%s:\n", name);

#define	p(f, m) if (ipstat.f || sflag <= 1) \
	printf(m, ipstat.f, plural(ipstat.f))
#define	p1(f, m) if (ipstat.f || sflag <= 1) \
	printf(m, ipstat.f)

	p(ips_total, "\t%lu total packet%s received\n");
	p(ips_badsum, "\t%lu bad header checksum%s\n");
	p1(ips_toosmall, "\t%lu with size smaller than minimum\n");
	p1(ips_tooshort, "\t%lu with data size < data length\n");
	p1(ips_badhlen, "\t%lu with header length < data size\n");
	p1(ips_badlen, "\t%lu with data length < header length\n");
	p1(ips_badoptions, "\t%lu with bad options\n");
	p1(ips_badvers, "\t%lu with incorrect version number\n");
	p(ips_fragments, "\t%lu fragment%s received\n");
	p(ips_fragdropped, "\t%lu fragment%s dropped (duplicates or out of space)\n");
	p(ips_badfrags, "\t%lu malformed fragment%s dropped\n");
	p(ips_fragtimeout, "\t%lu fragment%s dropped after timeout\n");
	p(ips_reassembled, "\t%lu packet%s reassembled ok\n");
	p(ips_delivered, "\t%lu packet%s for this host\n");
	p(ips_noproto, "\t%lu packet%s for unknown/unsupported protocol\n");
	p(ips_forward, "\t%lu packet%s forwarded\n");
	p(ips_cantforward, "\t%lu packet%s not forwardable\n");
	p(ips_redirectsent, "\t%lu redirect%s sent\n");
	p(ips_localout, "\t%lu packet%s sent from this host\n");
	p(ips_rawout, "\t%lu packet%s sent with fabricated ip header\n");
	p(ips_odropped, "\t%lu output packet%s dropped due to no bufs, etc.\n");
	p(ips_noroute, "\t%lu output packet%s discarded due to no route\n");
	p(ips_fragmented, "\t%lu output datagram%s fragmented\n");
	p(ips_ofragments, "\t%lu fragment%s created\n");
	p(ips_cantfrag, "\t%lu datagram%s that can't be fragmented\n");
	p1(ips_rcvmemdrop, "\t%lu fragment floods\n");
	p(ips_toolong, "\t%lu packet%s with ip length > max ip packet size\n");
	p(ips_nogif, "\t%lu tunneling packet%s that can't find gif\n");
	p(ips_badaddr, "\t%lu datagram%s with bad address in header\n");
	p(ips_inhwcsum, "\t%lu input datagram%s checksum-processed by hardware\n");
	p(ips_outhwcsum, "\t%lu output datagram%s checksum-processed by hardware\n");
#undef p
#undef p1
}

static	char *icmpnames[ICMP_MAXTYPE + 1] = {
	"echo reply",
	"#1",
	"#2",
	"destination unreachable",
	"source quench",
	"routing redirect",
	"#6",
	"#7",
	"echo",
	"router advertisement",
	"router solicitation",
	"time exceeded",
	"parameter problem",
	"time stamp",
	"time stamp reply",
	"information request",
	"information request reply",
	"address mask request",
	"address mask reply",
	"#19",
	"#20",
	"#21",
	"#22",
	"#23",
	"#24",
	"#25",
	"#26",
	"#27",
	"#28",
	"#29",
	"traceroute",
	"data conversion error",
	"mobile host redirect",
	"IPv6 where-are-you",
	"IPv6 i-am-here",
	"mobile registration request",
	"mobile registration reply",
	"#37",
	"#38",
	"SKIP",
	"Photuris",
};

/*
 * Dump ICMP statistics.
 */
void
icmp_stats(u_long off, char *name)
{
	struct icmpstat icmpstat;
	int i, first;

	if (off == 0)
		return;
	kread(off, (char *)&icmpstat, sizeof (icmpstat));
	printf("%s:\n", name);

#define	p(f, m) if (icmpstat.f || sflag <= 1) \
	printf(m, icmpstat.f, plural(icmpstat.f))

	p(icps_error, "\t%lu call%s to icmp_error\n");
	p(icps_oldicmp,
	    "\t%lu error%s not generated because old message was icmp\n");
	for (first = 1, i = 0; i < ICMP_MAXTYPE + 1; i++)
		if (icmpstat.icps_outhist[i] != 0) {
			if (first) {
				printf("\tOutput packet histogram:\n");
				first = 0;
			}
			if (icmpnames[i])
				printf("\t\t%s:", icmpnames[i]);
			else
				printf("\t\t#%d:", i);
			printf(" %lu\n", icmpstat.icps_outhist[i]);
		}
	p(icps_badcode, "\t%lu message%s with bad code fields\n");
	p(icps_tooshort, "\t%lu message%s < minimum length\n");
	p(icps_checksum, "\t%lu bad checksum%s\n");
	p(icps_badlen, "\t%lu message%s with bad length\n");
	for (first = 1, i = 0; i < ICMP_MAXTYPE + 1; i++)
		if (icmpstat.icps_inhist[i] != 0) {
			if (first) {
				printf("\tInput packet histogram:\n");
				first = 0;
			}
			if (icmpnames[i])
				printf("\t\t%s:", icmpnames[i]);
			else
				printf("\t\t#%d:", i);
			printf(" %lu\n", icmpstat.icps_inhist[i]);
		}
	p(icps_reflect, "\t%lu message response%s generated\n");
#undef p
}

/*
 * Dump IGMP statistics structure.
 */
void
igmp_stats(u_long off, char *name)
{
	struct igmpstat igmpstat;

	if (off == 0)
		return;
	kread(off, (char *)&igmpstat, sizeof (igmpstat));
	printf("%s:\n", name);

#define	p(f, m) if (igmpstat.f || sflag <= 1) \
	printf(m, igmpstat.f, plural(igmpstat.f))
#define	py(f, m) if (igmpstat.f || sflag <= 1) \
	printf(m, igmpstat.f, igmpstat.f != 1 ? "ies" : "y")

	p(igps_rcv_total, "\t%lu message%s received\n");
	p(igps_rcv_tooshort, "\t%lu message%s received with too few bytes\n");
	p(igps_rcv_toolong, "\t%lu message%s received over MTU size\n");
	p(igps_rcv_badsum, "\t%lu message%s received with bad checksum\n");
	py(igps_rcv_badqueries, "\t%lu membership quer%s received with invalid field(s)\n");
	p(igps_rcv_nora, "\t%u message%s received with no router alert\n");
	py(igps_rcv_v1_queries, "\t%u v1 membership quer%s received\n");
	py(igps_rcv_v2_queries, "\t%u v2 membership quer%s received\n");
	py(igps_rcv_v3_queries, "\t%u v3 membership quer%s received\n");
	p(igps_rcv_reports, "\t%lu membership report%s received\n");
	p(igps_rcv_badreports, "\t%lu membership report%s received with invalid field(s)\n");
	p(igps_rcv_ourreports, "\t%lu membership report%s received for groups to which we belong\n");
	p(igps_snd_v1v2_reports, "\t%lu v1/v2 membership report%s sent\n");
	p(igps_snd_v3_reports, "\t%lu v3 membership report%s sent\n");
#undef p
#undef py
}

struct rpcnams {
	struct rpcnams *next;
	in_port_t port;
	int	  proto;
	char	*rpcname;
};

static char *
getrpcportnam(in_port_t port, int proto)
{
	struct sockaddr_in server_addr;
	struct hostent *hp;
	static struct pmaplist *head;
	int socket = RPC_ANYSOCK;
	struct timeval minutetimeout;
	CLIENT *client;
	struct rpcent *rpc;
	static int first;
	static struct rpcnams *rpcn;
	struct rpcnams *n;
	char num[20];

	if (first == 0) {
		first = 1;
		memset((char *)&server_addr, 0, sizeof server_addr);
		server_addr.sin_family = AF_INET;
		if ((hp = gethostbyname("localhost")) != NULL)
			memmove((caddr_t)&server_addr.sin_addr, hp->h_addr,
			    hp->h_length);
		else
			(void) inet_aton("0.0.0.0", &server_addr.sin_addr);

		minutetimeout.tv_sec = 60;
		minutetimeout.tv_usec = 0;
		server_addr.sin_port = htons(PMAPPORT);
		if ((client = clnttcp_create(&server_addr, PMAPPROG,
		    PMAPVERS, &socket, 50, 500)) == NULL)
			return (NULL);
		if (clnt_call(client, PMAPPROC_DUMP, xdr_void, NULL,
		    xdr_pmaplist, &head, minutetimeout) != RPC_SUCCESS) {
			clnt_destroy(client);
			return (NULL);
		}
		for (; head != NULL; head = head->pml_next) {
			n = (struct rpcnams *)malloc(sizeof(struct rpcnams));
			if (n == NULL)
				continue;
			n->next = rpcn;
			rpcn = n;
			n->port = head->pml_map.pm_port;
			n->proto = head->pml_map.pm_prot;

			rpc = getrpcbynumber(head->pml_map.pm_prog);
			if (rpc)
				n->rpcname = strdup(rpc->r_name);
			else {
				snprintf(num, sizeof num, "%ld",
				    head->pml_map.pm_prog);
				n->rpcname = strdup(num);
			}
		}
		clnt_destroy(client);
	}

	for (n = rpcn; n; n = n->next)
		if (n->port == port && n->proto == proto)
			return (n->rpcname);
	return (NULL);
}

/*
 * Pretty print an Internet address (net address + port).
 * If the nflag was specified, use numbers instead of names.
 */
void
inetprint(struct in_addr *in, in_port_t port, char *proto, int local)
{
	struct servent *sp = 0;
	char line[80], *cp, *nam;
	int width;

	snprintf(line, sizeof line, "%.*s.", (Aflag && !nflag) ? 12 : 16,
	    inetname(in));
	cp = strchr(line, '\0');
	if (!nflag && port)
		sp = getservbyport((int)port, proto);
	if (sp || port == 0)
		snprintf(cp, line + sizeof line - cp, "%.8s",
		    sp ? sp->s_name : "*");
	else if (local && !nflag && (nam = getrpcportnam(ntohs(port),
	    (strcmp(proto, "tcp") == 0 ? IPPROTO_TCP : IPPROTO_UDP))))
		snprintf(cp, line + sizeof line - cp, "%d[%.8s]",
		    ntohs(port), nam);
	else
		snprintf(cp, line + sizeof line - cp, "%d", ntohs(port));
	width = Aflag ? 18 : 22;
	printf(" %-*.*s", width, width, line);
}

/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give
 * numeric value, otherwise try for symbolic name.
 */
char *
inetname(struct in_addr *inp)
{
	char *cp;
	static char line[50];
	struct hostent *hp;
	struct netent *np;
	static char domain[MAXHOSTNAMELEN];
	static int first = 1;

	if (first && !nflag) {
		first = 0;
		if (gethostname(domain, sizeof(domain)) == 0 &&
		    (cp = strchr(domain, '.')))
			(void) strlcpy(domain, cp + 1, sizeof domain);
		else
			domain[0] = '\0';
	}
	cp = NULL;
	if (!nflag && inp->s_addr != INADDR_ANY) {
		int net = inet_netof(*inp);
		int lna = inet_lnaof(*inp);

		if (lna == INADDR_ANY) {
			np = getnetbyaddr(net, AF_INET);
			if (np)
				cp = np->n_name;
		}
		if (cp == NULL) {
			hp = gethostbyaddr((char *)inp, sizeof (*inp), AF_INET);
			if (hp) {
				if ((cp = strchr(hp->h_name, '.')) &&
				    !strcmp(cp + 1, domain))
					*cp = '\0';
				cp = hp->h_name;
			}
		}
	}
	if (inp->s_addr == INADDR_ANY)
		snprintf(line, sizeof line, "*");
	else if (cp)
		snprintf(line, sizeof line, "%s", cp);
	else {
		inp->s_addr = ntohl(inp->s_addr);
#define C(x)	((x) & 0xff)
		snprintf(line, sizeof line, "%u.%u.%u.%u",
		    C(inp->s_addr >> 24), C(inp->s_addr >> 16),
		    C(inp->s_addr >> 8), C(inp->s_addr));
	}
	return (line);
}

/*
 * Dump AH statistics structure.
 */
void
ah_stats(u_long off, char *name)
{
	struct ahstat ahstat;

	if (off == 0)
		return;
	kread(off, (char *)&ahstat, sizeof (ahstat));
	printf("%s:\n", name);

#define p(f, m) if (ahstat.f || sflag <= 1) \
	printf(m, ahstat.f, plural(ahstat.f))
#define p1(f, m) if (ahstat.f || sflag <= 1) \
	printf(m, ahstat.f)

	p1(ahs_input, "\t%u input AH packets\n");
	p1(ahs_output, "\t%u output AH packets\n");
	p(ahs_nopf, "\t%u packet%s from unsupported protocol families\n");
	p(ahs_hdrops, "\t%u packet%s shorter than header shows\n");
	p(ahs_pdrops, "\t%u packet%s dropped due to policy\n");
	p(ahs_notdb, "\t%u packet%s for which no TDB was found\n");
	p(ahs_badkcr, "\t%u input packet%s that failed to be processed\n");
	p(ahs_badauth, "\t%u packet%s that failed verification received\n");
	p(ahs_noxform, "\t%u packet%s for which no XFORM was set in TDB received\n");
	p(ahs_qfull, "\t%u packet%s were dropped due to full output queue\n");
	p(ahs_wrap, "\t%u packet%s where counter wrapping was detected\n");
	p(ahs_replay, "\t%u possibly replayed packet%s received\n");
	p(ahs_badauthl, "\t%u packet%s with bad authenticator length received\n");
	p(ahs_invalid, "\t%u packet%s attempted to use an invalid TDB\n");
	p(ahs_toobig, "\t%u packet%s got larger than max IP packet size\n");
	p(ahs_crypto, "\t%u packet%s that failed crypto processing\n");
	p(ahs_ibytes, "\t%qu input byte%s\n");
	p(ahs_obytes, "\t%qu output byte%s\n");

#undef p
#undef p1
}

/*
 * Dump etherip statistics structure.
 */
void
etherip_stats(u_long off, char *name)
{
	struct etheripstat etheripstat;

	if (off == 0)
		return;
	kread(off, (char *)&etheripstat, sizeof (etheripstat));
	printf("%s:\n", name);

#define p(f, m) if (etheripstat.f || sflag <= 1) \
	printf(m, etheripstat.f, plural(etheripstat.f))

	p(etherip_hdrops, "\t%u packet%s shorter than header shows\n");
	p(etherip_qfull, "\t%u packet%s were dropped due to full output queue\n");
	p(etherip_noifdrops, "\t%u packet%s were dropped because of no interface/bridge information\n");
	p(etherip_pdrops, "\t%u packet%s dropped due to policy\n");
	p(etherip_adrops, "\t%u packet%s dropped for other reasons\n");
	p(etherip_ipackets, "\t%u input ethernet-in-IP packet%s\n");
	p(etherip_opackets, "\t%u output ethernet-in-IP packet%s\n");
	p(etherip_ibytes, "\t%qu input byte%s\n");
	p(etherip_obytes, "\t%qu output byte%s\n");
#undef p
}

/*
 * Dump ESP statistics structure.
 */
void
esp_stats(u_long off, char *name)
{
	struct espstat espstat;

	if (off == 0)
		return;
	kread(off, (char *)&espstat, sizeof (espstat));
	printf("%s:\n", name);

#define p(f, m) if (espstat.f || sflag <= 1) \
	printf(m, espstat.f, plural(espstat.f))

	p(esps_input, "\t%u input ESP packet%s\n");
	p(esps_output, "\t%u output ESP packet%s\n");
	p(esps_nopf, "\t%u packet%s from unsupported protocol families\n");
	p(esps_hdrops, "\t%u packet%s shorter than header shows\n");
	p(esps_pdrops, "\t%u packet%s dropped due to policy\n");
	p(esps_notdb, "\t%u packet%s for which no TDB was found\n");
	p(esps_badkcr, "\t%u input packet%s that failed to be processed\n");
	p(esps_badenc, "\t%u packet%s with bad encryption received\n");
	p(esps_badauth, "\t%u packet%s that failed verification received\n");
	p(esps_noxform, "\t%u packet%s for which no XFORM was set in TDB received\n");
	p(esps_qfull, "\t%u packet%s were dropped due to full output queue\n");
	p(esps_wrap, "\t%u packet%s where counter wrapping was detected\n");
	p(esps_replay, "\t%u possibly replayed packet%s received\n");
	p(esps_badilen, "\t%u packet%s with bad payload size or padding received\n");
	p(esps_invalid, "\t%u packet%s attempted to use an invalid TDB\n");
	p(esps_toobig, "\t%u packet%s got larger than max IP packet size\n");
	p(esps_crypto, "\t%u packet%s that failed crypto processing\n");
	p(esps_udpencin, "\t%u input UDP encapsulated ESP packet%s\n");
	p(esps_udpencout, "\t%u output UDP encapsulated ESP packet%s\n");
	p(esps_udpinval, "\t%u UDP packet%s for non-encapsulating TDB received\n");
	p(esps_ibytes, "\t%qu input byte%s\n");
	p(esps_obytes, "\t%qu output byte%s\n");

#undef p
}

/*
 * Dump IP-in-IP statistics structure.
 */
void
ipip_stats(u_long off, char *name)
{
	struct ipipstat ipipstat;

	if (off == 0)
		return;
	kread(off, (char *)&ipipstat, sizeof (ipipstat));
	printf("%s:\n", name);

#define p(f, m) if (ipipstat.f || sflag <= 1) \
	printf(m, ipipstat.f, plural(ipipstat.f))

	p(ipips_ipackets, "\t%u total input packet%s\n");
	p(ipips_opackets, "\t%u total output packet%s\n");
	p(ipips_hdrops, "\t%u packet%s shorter than header shows\n");
	p(ipips_pdrops, "\t%u packet%s dropped due to policy\n");
	p(ipips_spoof, "\t%u packet%s with possibly spoofed local addresses\n");
	p(ipips_qfull, "\t%u packet%s were dropped due to full output queue\n");
	p(ipips_ibytes, "\t%qu input byte%s\n");
	p(ipips_obytes, "\t%qu output byte%s\n");
	p(ipips_family, "\t%u protocol family mismatche%s\n");
	p(ipips_unspec, "\t%u attempt%s to use tunnel with unspecified endpoint(s)\n");
#undef p
}

/*
 * Dump CARP statistics structure.
 */
void
carp_stats(u_long off, char *name)
{
	struct carpstats carpstat;

	if (off == 0)
		return;
	kread(off, (char *)&carpstat, sizeof(carpstat));
	printf("%s:\n", name);

#define p(f, m) if (carpstat.f || sflag <= 1) \
	printf(m, carpstat.f, plural(carpstat.f))
#define p2(f, m) if (carpstat.f || sflag <= 1) \
	printf(m, carpstat.f)

	p(carps_ipackets, "\t%lu packet%s received (IPv4)\n");
	p(carps_ipackets6, "\t%lu packet%s received (IPv6)\n");
	p(carps_badif, "\t\t%lu packet%s discarded for bad interface\n");
	p(carps_hdrops, "\t\t%lu packet%s shorter than header\n");
	p(carps_badsum, "\t\t%lu discarded for bad checksum%s\n");
	p(carps_badver,	"\t\t%lu discarded packet%s with a bad version\n");
	p2(carps_badlen, "\t\t%lu discarded because packet too short\n");
	p2(carps_badauth, "\t\t%lu discarded for bad authentication\n");
	p2(carps_badvhid, "\t\t%lu discarded for bad vhid\n");
	p2(carps_badaddrs, "\t\t%lu discarded because of a bad address list\n");
	p(carps_opackets, "\t%lu packet%s sent (IPv4)\n");
	p(carps_opackets6, "\t%lu packet%s sent (IPv6)\n");
#if notyet
	p(carps_ostates, "\t\t%s state update%s sent\n");
#endif
#undef p
#undef p2
}

#if 0
/* 
 * Dump pfsync statistics structure.
 */
void
pfsync_stats(u_long off, char *name)
{
	struct pfsyncstats pfsyncstat;

	if (off == 0)
		return;
	kread(off, (char *)&pfsyncstat, sizeof(pfsyncstat));
	printf("%s:\n", name);

#define p(f, m) if (pfsyncstat.f || sflag <= 1) \
	printf(m, pfsyncstat.f, plural(pfsyncstat.f))
#define p2(f, m) if (pfsyncstat.f || sflag <= 1) \
	printf(m, pfsyncstat.f)

	p(pfsyncs_ipackets, "\t%lu packet%s received (IPv4)\n");
	p(pfsyncs_ipackets6, "\t%lu packet%s received (IPv6)\n");
	p(pfsyncs_badif, "\t\t%lu packet%s discarded for bad interface\n");
	p(pfsyncs_badttl, "\t\t%lu packet%s discarded for bad ttl\n");
	p(pfsyncs_hdrops, "\t\t%lu packet%s shorter than header\n");
	p(pfsyncs_badver,	"\t\t%lu discarded packet%s with a bad version\n");
	p(pfsyncs_badact,	"\t\t%lu discarded packet%s with a bad action\n");
	p2(pfsyncs_badlen, "\t\t%lu discarded because packet too short\n");
	p2(pfsyncs_badauth, "\t\t%lu discarded for bad authentication\n");
	p(pfsyncs_badstate, "\t%lu failed state lookup/insert%s\n");
	p(pfsyncs_opackets, "\t%lu packet%s sent (IPv4)\n");
	p(pfsyncs_opackets6, "\t%lu packet%s sent (IPv6)\n");
	p2(pfsyncs_onomem, "\t\t%lu send failed due to mbuf memory error\n");
	p2(pfsyncs_oerrors, "\t\t%lu send error\n");
#undef p
#undef p2
}
#endif

/*
 * Dump IPCOMP statistics structure.
 */
void
ipcomp_stats(u_long off, char *name)
{
	struct ipcompstat ipcompstat;

	if (off == 0)
		return;
	kread(off, (char *)&ipcompstat, sizeof (ipcompstat));
	printf("%s:\n", name);

#define p(f, m) if (ipcompstat.f || sflag <= 1) \
	printf(m, ipcompstat.f, plural(ipcompstat.f))

	p(ipcomps_input, "\t%u input IPCOMP packet%s\n");
	p(ipcomps_output, "\t%u output IPCOMP packet%s\n");
	p(ipcomps_nopf, "\t%u packet%s from unsupported protocol families\n");
	p(ipcomps_hdrops, "\t%u packet%s shorter than header shows\n");
	p(ipcomps_pdrops, "\t%u packet%s dropped due to policy\n");
	p(ipcomps_notdb, "\t%u packet%s for which no TDB was found\n");
	p(ipcomps_badkcr, "\t%u input packet%s that failed to be processed\n");
	p(ipcomps_noxform, "\t%u packet%s for which no XFORM was set in TDB received\n");
	p(ipcomps_qfull, "\t%u packet%s were dropped due to full output queue\n");
	p(ipcomps_wrap, "\t%u packet%s where counter wrapping was detected\n");
	p(ipcomps_invalid, "\t%u packet%s attempted to use an invalid TDB\n");
	p(ipcomps_toobig, "\t%u packet%s got larger than max IP packet size\n");
	p(ipcomps_crypto, "\t%u packet%s that failed (de)compression processing\n");
	p(ipcomps_minlen, "\t%u packet%s less than minimum compression length\n");
	p(ipcomps_ibytes, "\t%qu input byte%s\n");
	p(ipcomps_obytes, "\t%qu output byte%s\n");

#undef p
}
