/*	$KAME: common.h,v 1.23 2002/05/08 15:53:18 jinmei Exp $	*/
/*
 * Copyright (C) 1998 and 1999 WIDE Project.
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

#define IN6_IFF_INVALID (IN6_IFF_ANYCAST|IN6_IFF_TENTATIVE|\
		IN6_IFF_DUPLICATED|IN6_IFF_DETACHED)

extern int foreground;
extern int debug_thresh;
extern char *device;

/* common.c */
extern int getifaddr __P((struct in6_addr *, char *, struct in6_addr *,
			  int, int, int));
extern int transmit_sa __P((int, struct sockaddr *, char *, size_t));
extern long random_between __P((long, long));
extern int prefix6_mask __P((struct in6_addr *, int));
extern int sa6_plen2mask __P((struct sockaddr_in6 *, int));
extern char *addr2str __P((struct sockaddr *));
extern char *in6addr2str __P((struct in6_addr *, int));
extern const char *getdev __P((struct sockaddr_in6 *));
extern int in6_addrscopebyif __P((struct in6_addr *, char *));
extern int in6_scope __P((struct in6_addr *));
extern void setloglevel __P((int));
extern void dprintf __P((int, const char *, ...));
extern int get_duid __P((char *, struct duid *));
extern void dhcp6_init_options __P((struct dhcp6_optinfo *));
extern void dhcp6_clear_options __P((struct dhcp6_optinfo *));
extern int dhcp6_get_options __P((struct dhcp6opt *, struct dhcp6opt *,
				  struct dhcp6_optinfo *));
extern int dhcp6_set_options __P((struct dhcp6opt *, struct dhcp6opt *,
				  struct dhcp6_optinfo *));
extern char *dhcpoptstr __P((int));
extern char *dhcpmsgstr __P((int));
extern char *duidstr __P((struct duid *));

/* missing */
#ifndef HAVE_STRLCAT
extern size_t strlcat __P((char *, const char *, size_t));
#endif
#ifndef HAVE_STRLCPY
extern size_t strlcpy __P((char *, const char *, size_t));
#endif
