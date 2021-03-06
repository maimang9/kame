/*	$NetBSD: vmparam.h,v 1.16 1999/01/16 20:39:03 chuck Exp $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
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
 *	@(#)vmparam.h	5.9 (Berkeley) 5/12/91
 */

#ifndef _NS532_VMPARAM_H_
#define _NS532_VMPARAM_H_

#if defined(_KERNEL) && !defined(_LKM)
#include "opt_pmap_new.h"
#endif

/*
 * Machine dependent constants for 532.
 */

/*
 * Virtual address space arrangement. On 532, both user and kernel
 * share the address space, not unlike the vax.
 * USRTEXT is the start of the user text/data space, while USRSTACK
 * is the top (end) of the user stack. Immediately above the user stack
 * resides the user structure, which is UPAGES long and contains the
 * kernel stack.
 *
 * Immediately after the user structure is the page table map, and then
 * kernal address space.
 */
#define	USRTEXT		CLBYTES			/* For NetBSD... */
#define	USRSTACK	VM_MAXUSER_ADDRESS

/*
 * INTSTACK is a temporary stack for the idle process and cpu_exit.
 */
#define INTSTACK	(0xffc00000 + NBPG - 4)

/*
 * Virtual memory related constants, all in bytes
 */
#define	MAXTSIZ		(16*1024*1024)		/* max text size */
#ifndef DFLDSIZ
#define	DFLDSIZ		(24*1024*1024)		/* initial data size limit */
#endif
#ifndef MAXDSIZ
#define	MAXDSIZ		(128*1024*1024)		/* max data size */
#endif
#ifndef	DFLSSIZ
#define	DFLSSIZ		(1*1024*1024)		/* initial stack size limit */
#endif
#ifndef	MAXSSIZ
#define	MAXSSIZ		(8*1024*1024)		/* max stack size */
#endif

/*
 * Size of shared memory map
 */
#ifndef SHMMAXPGS
#define SHMMAXPGS	1024
#endif

/*
 * PTEs for mapping user space into the kernel for phyio operations.
 * One page is enough to handle 4Mb of simultaneous raw IO operations.
 */
#ifndef USRIOSIZE
#define USRIOSIZE	(1 * NPTEPG)	/* 4mb */
#endif

/*
 * The time for a process to be blocked before being very swappable.
 * This is a number of seconds which the system takes as being a non-trivial
 * amount of real time.  You probably shouldn't change this;
 * it is used in subtle ways (fractions and multiples of it are, that is, like
 * half of a ``long time'', almost a long time, etc.)
 * It is related to human patience and other factors which don't really
 * change over time.
 */
#define	MAXSLP 		20

/*
 * A swapped in process is given a small amount of core without being bothered
 * by the page replacement algorithm.  Basically this says that if you are
 * swapped in you deserve some resources.  We protect the last SAFERSS
 * pages against paging and will just swap you out rather than paging you.
 * Note that each process has at least UPAGES+CLSIZE pages which are not
 * paged anyways (this is currently 8+2=10 pages or 5k bytes), so this
 * number just means a swapped in process is given around 25k bytes.
 * Just for fun: current memory prices are 4600$ a megabyte on VAX (4/22/81),
 * so we loan each swapped in process memory worth 100$, or just admit
 * that we don't consider it worthwhile and swap it out to disk which costs
 * $30/mb or about $0.75.
 * { wfj 6/16/89: Retail AT memory expansion $800/megabyte, loan of $17
 *   on disk costing $7/mb or $0.18 (in memory still 100:1 in cost!) }
 * Update: memory prices have changed recently (9/96). At the current    
 * value of $6 per megabyte, we lend each swapped in process memory worth
 * $0.15, or just admit that we don't consider it worthwhile and swap it out
 * to disk which costs $0.20/MB, or just under half a cent. 
 */
#define	SAFERSS		8		/* nominal ``small'' resident set size
					   protected against replacement */

/*
 * Mach derived constants
 */

/* user/kernel map constants */
#define VM_MIN_ADDRESS		((vaddr_t)0)
/* PTDPTDI << PDSHIFT */
#define VM_MAXUSER_ADDRESS	((vaddr_t)0xF7C00000)
/* PTDPTDI << PDSHIFT + PTDPTDI << PGSHIFT */
#define VM_MAX_ADDRESS		((vaddr_t)0xF7FDF000)
/* KPTDI << PDSHIFT */
#define VM_MIN_KERNEL_ADDRESS	((vaddr_t)0xF8000000)
/* APTDPTDI << PDSHIFT */
#define VM_MAX_KERNEL_ADDRESS	((vaddr_t)0xFF800000)

/* XXX max. amount of KVM to be used by buffers. */
#ifndef VM_MAX_KERNEL_BUF
#define VM_MAX_KERNEL_BUF \
	((VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS) * 7 / 10)
#endif

/* virtual sizes (bytes) for various kernel submaps */
#define VM_MBUF_SIZE		(NMBCLUSTERS*MCLBYTES)
#define VM_KMEM_SIZE		(NKMEMCLUSTERS*CLBYTES)
#define VM_PHYS_SIZE		(USRIOSIZE*CLBYTES)

#define VM_PHYSSEG_MAX		1	/* we have contiguous memory */
#define VM_PHYSSEG_STRAT	VM_PSTRAT_RANDOM
#define VM_PHYSSEG_NOADD		/* can't add RAM after vm_mem_init */

#define	VM_NFREELIST		1
#define	VM_FREELIST_DEFAULT	0

/*
 * pmap specific data stored in the vm_physmem[] array
 */
#if defined(PMAP_NEW)
struct pmap_physseg {
	struct pv_head *pvhead; 	/* pv_head array */
	short *attrs;			/* attrs array */
};
#else
struct pmap_physseg {
	struct pv_entry *pvent;		/* pv_entry array */
	short *attrs;			/* attrs array */
};
#endif

#endif
