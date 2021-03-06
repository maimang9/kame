/*	$NetBSD: amdpmreg.h,v 1.1 2002/06/02 02:44:28 enami Exp $	*/

/*-
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Enami Tsugutomo.
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
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define	AMDPM_CONFREG	0x40

/* 0x40: General Configuration 1 Register */
#define	AMDPM_RNGEN	0x00000080	/* random number generator enable */

/* 0x41: General Configuration 2 Register */
#define	AMDPM_PMIOEN	0x00008000	/* system management IO space enable */

/* 0x42: SCI Interrupt Configuration Register */
/* 0x43: Previous Power State Register */

#define	AMDPM_PMPTR	0x58		/* PMxx System Management IO space
					   Pointer */
#define	AMDPM_PMBASE(x)	((x) & 0xff00)	/* PMxx base address */
#define	AMDPM_PMSIZE	256		/* PMxx space size */

/* Registers in PMxx space */
#define	AMDPM_RNGDATA	0xf0		/* 32 bit random data register */
#define	AMDPM_RNGSTAT	0xf4		/* RNG status register */
#define	AMDPM_RNGDONE	0x00000001	/* Random number generation complete */
