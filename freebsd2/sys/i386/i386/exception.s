/*-
 * Copyright (c) 1990 The Regents of the University of California.
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
 *	$Id: exception.s,v 1.19.2.1 1998/08/16 23:05:57 bde Exp $
 */

#include "npx.h"				/* NNPX */
#include "assym.s"				/* system defines */
#include <sys/errno.h>				/* error return codes */
#include <machine/spl.h>			/* SWI_AST_MASK ... */
#include <machine/psl.h>			/* PSL_I */
#include <machine/trap.h>			/* trap codes */
#include <sys/syscall.h>			/* syscall numbers */
#include <machine/asmacros.h>			/* miscellaneous macros */
#include <sys/cdefs.h>				/* CPP macros */

#define	KDSEL		0x10			/* kernel data selector */
#define	SEL_RPL_MASK	0x0003
#define	TRAPF_CS_OFF	(13 * 4)

	.text

/*****************************************************************************/
/* Trap handling                                                             */
/*****************************************************************************/
/*
 * Trap and fault vector routines
 */
#define	IDTVEC(name)	ALIGN_TEXT; .globl __CONCAT(_X,name); __CONCAT(_X,name):
#define	TRAP(a)		pushl $(a) ; jmp _alltraps

/*
 * XXX - debugger traps are now interrupt gates so at least bdb doesn't lose
 * control.  The sti's give the standard losing behaviour for ddb and kgdb.
 */
#ifdef BDE_DEBUGGER
#define	BDBTRAP(name) \
	ss ; \
	cmpb	$0,_bdb_exists ; \
	je	1f ; \
	testb	$SEL_RPL_MASK,4(%esp) ; \
	jne	1f ; \
	ss ; \
	.globl	__CONCAT(__CONCAT(bdb_,name),_ljmp); \
__CONCAT(__CONCAT(bdb_,name),_ljmp): \
	ljmp	$0,$0 ; \
1:
#else
#define BDBTRAP(name)
#endif

#define BPTTRAP(a)	testl $PSL_I,4+8(%esp) ; je 1f ; sti ; 1: ; TRAP(a)

MCOUNT_LABEL(user)
MCOUNT_LABEL(btrap)

IDTVEC(div)
	pushl $0; TRAP(T_DIVIDE)
IDTVEC(dbg)
	BDBTRAP(dbg)
	pushl $0; BPTTRAP(T_TRCTRAP)
IDTVEC(nmi)
	pushl $0; TRAP(T_NMI)
IDTVEC(bpt)
	BDBTRAP(bpt)
	pushl $0; BPTTRAP(T_BPTFLT)
IDTVEC(ofl)
	pushl $0; TRAP(T_OFLOW)
IDTVEC(bnd)
	pushl $0; TRAP(T_BOUND)
IDTVEC(ill)
	pushl $0; TRAP(T_PRIVINFLT)
IDTVEC(dna)
	pushl $0; TRAP(T_DNA)
IDTVEC(fpusegm)
	pushl $0; TRAP(T_FPOPFLT)
IDTVEC(tss)
	TRAP(T_TSSFLT)
IDTVEC(missing)
	TRAP(T_SEGNPFLT)
IDTVEC(stk)
	TRAP(T_STKFLT)
IDTVEC(prot)
	TRAP(T_PROTFLT)
IDTVEC(page)
	TRAP(T_PAGEFLT)
IDTVEC(mchk)
	pushl $0; TRAP(T_MCHK)
IDTVEC(rsvd)
	pushl $0; TRAP(T_RESERVED)
IDTVEC(fpu)
#if NNPX > 0
	/*
	 * Handle like an interrupt (except for accounting) so that we can
	 * call npxintr to clear the error.  It would be better to handle
	 * npx interrupts as traps.  This used to be difficult for nested
	 * interrupts, but now it is fairly easy - mask nested ones the
	 * same as SWI_AST's.
	 */
	pushl	$0				/* dummy error code */
	pushl	$0				/* dummy trap type */
	pushal
	pushl	%ds
	pushl	%es				/* now the stack frame is a trap frame */
	movl	$KDSEL,%eax
	movl	%ax,%ds
	movl	%ax,%es
	FAKE_MCOUNT(12*4(%esp))
	movl	_cpl,%eax
	pushl	%eax
	pushl	$0				/* dummy unit to finish building intr frame */
	incl	_cnt+V_TRAP
	orl	$SWI_AST_MASK,%eax
	movl	%eax,_cpl
	call	_npxintr
	incb	_intr_nesting_level
	MEXITCOUNT
	jmp	_doreti
#else	/* NNPX > 0 */
	pushl $0; TRAP(T_ARITHTRAP)
#endif	/* NNPX > 0 */
IDTVEC(align)
	TRAP(T_ALIGNFLT)

	SUPERALIGN_TEXT
	.globl	_alltraps
_alltraps:
	pushal
	pushl	%ds
	pushl	%es
alltraps_with_regs_pushed:
	movl	$KDSEL,%eax
	movl	%ax,%ds
	movl	%ax,%es
	FAKE_MCOUNT(12*4(%esp))
calltrap:
	FAKE_MCOUNT(_btrap)			/* init "from" _btrap -> calltrap */
	incl	_cnt+V_TRAP
	movl	_cpl,%eax
	movl	%eax,%ebx			/* keep orig. cpl here during trap() */
	orl	$SWI_AST_MASK,%eax
	movl	%eax,_cpl
	call	_trap

	/*
	 * Return via _doreti to handle ASTs.  Have to change trap frame
	 * to interrupt frame.
	 */
	pushl	%ebx				/* cpl to restore */
	subl	$4,%esp
	incb	_intr_nesting_level
	MEXITCOUNT
	jmp	_doreti

/*
 * Call gate entry for syscall.
 * The intersegment call has been set up to specify one dummy parameter.
 * This leaves a place to put eflags so that the call frame can be
 * converted to a trap frame. Note that the eflags is (semi-)bogusly
 * pushed into (what will be) tf_err and then copied later into the
 * final spot. It has to be done this way because esp can't be just
 * temporarily altered for the pushfl - an interrupt might come in
 * and clobber the saved cs/eip.
 */
	SUPERALIGN_TEXT
IDTVEC(syscall)
	pushfl					/* save eflags in tf_err for now */
	subl	$4,%esp				/* skip over tf_trapno */
	pushal
	pushl	%ds
	pushl	%es
	movl	$KDSEL,%eax			/* switch to kernel segments */
	movl	%ax,%ds
	movl	%ax,%es
	movl	TF_ERR(%esp),%eax		/* copy saved eflags to final spot */
	movl	%eax,TF_EFLAGS(%esp)
	movl	$7,TF_ERR(%esp) 		/* sizeof "lcall 7,0" */
	FAKE_MCOUNT(12*4(%esp))
	incl	_cnt+V_SYSCALL
	movl	$SWI_AST_MASK,_cpl
	call	_syscall
	/*
	 * Return via _doreti to handle ASTs.
	 */
	pushl	$0				/* cpl to restore */
	subl	$4,%esp
	movb	$1,_intr_nesting_level
	MEXITCOUNT
	jmp	_doreti

/*
 * Call gate entry for Linux/NetBSD syscall (int 0x80)
 */
	SUPERALIGN_TEXT
IDTVEC(int0x80_syscall)
	subl	$8,%esp				/* skip over tf_trapno and tf_err */
	pushal
	pushl	%ds
	pushl	%es
	movl	$KDSEL,%eax			/* switch to kernel segments */
	movl	%ax,%ds
	movl	%ax,%es
	movl	$2,TF_ERR(%esp)			/* sizeof "int 0x80" */
	FAKE_MCOUNT(12*4(%esp))
	incl	_cnt+V_SYSCALL
	movl	$SWI_AST_MASK,_cpl
	call	_syscall
	/*
	 * Return via _doreti to handle ASTs.
	 */
	pushl	$0				/* cpl to restore */
	subl	$4,%esp
	movb	$1,_intr_nesting_level
	MEXITCOUNT
	jmp	_doreti

/*
 * Include what was once config+isa-dependent code.
 * XXX it should be in a stand-alone file.  It's still icu-dependent and
 * belongs in i386/isa.
 */
#include "i386/isa/vector.s"

/*
 * Include what was once icu-dependent code.
 * XXX it should be merged into this file (also move the definition of
 * imen to vector.s or isa.c).
 * Before including it, set up a normal asm environment so that vector.s
 * doesn't have to know that stuff is included after it.
 */
	.data
	ALIGN_DATA
	.text
	SUPERALIGN_TEXT
#include "i386/isa/icu.s"
