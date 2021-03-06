/*-
 * Copyright (c) 2004 Mark R V Murray
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/dev/random/probe.c,v 1.2 2004/04/11 09:13:42 nyan Exp $");

#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/random.h>
#include <sys/selinfo.h>
#include <sys/stdint.h>
#include <sys/sysctl.h>

#if defined(__i386__) && !defined(PC98)
#include <machine/cpufunc.h>
#endif

#include <dev/random/randomdev.h>
#include <dev/random/randomdev_soft.h>
#include <dev/random/nehemiah.h>

void
random_ident_hardware(struct random_systat *systat)
{
#if defined(__i386__) && !defined(PC98)
	u_int regs[4];
#endif

	/* Set default to software */
	*systat = random_yarrow;

	/* Then go looking for hardware */
#if defined(__i386__) && !defined(PC98)
	do_cpuid(1, regs);
	if ((regs[0] & 0xf) >= 3) {
		do_cpuid(0xc0000000, regs);
		if (regs[0] == 0xc0000001) {
			do_cpuid(0xc0000001, regs);
			if ((regs[3] & 0x0c) == 0x0c)
				*systat = random_nehemiah;
		}
	}
#endif
}
