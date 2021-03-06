/*	$OpenBSD: vectors.s,v 1.4 1996/05/06 21:55:32 deraadt Exp $ */

| Copyright (c) 1995 Theo de Raadt
|
| Redistribution and use in source and binary forms, with or without
| modification, are permitted provided that the following conditions
| are met:
| 1. Redistributions of source code must retain the above copyright
|    notice, this list of conditions and the following disclaimer.
| 2. Redistributions in binary form must reproduce the above copyright
|    notice, this list of conditions and the following disclaimer in the
|    documentation and/or other materials provided with the distribution.
| 3. All advertising materials mentioning features or use of this software
|    must display the following acknowledgement:
|	This product includes software developed under OpenBSD by
|	Theo de Raadt for Willowglen Singapore.
| 4. The name of the author may not be used to endorse or promote products
|    derived from this software without specific prior written permission.
|
| THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
| OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
| WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
| ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
| DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
| DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
| OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
| HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
| LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
| OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
| SUCH DAMAGE.
|
| Copyright (c) 1988 University of Utah
| Copyright (c) 1990, 1993
|	The Regents of the University of California.  All rights reserved.
|
| Redistribution and use in source and binary forms, with or without
| modification, are permitted provided that the following conditions
| are met:
| 1. Redistributions of source code must retain the above copyright
|    notice, this list of conditions and the following disclaimer.
| 2. Redistributions in binary form must reproduce the above copyright
|    notice, this list of conditions and the following disclaimer in the
|    documentation and/or other materials provided with the distribution.
| 3. All advertising materials mentioning features or use of this software
|    must display the following acknowledgement:
|	This product includes software developed by the University of
|	California, Berkeley and its contributors.
| 4. Neither the name of the University nor the names of its contributors
|    may be used to endorse or promote products derived from this software
|    without specific prior written permission.
|
| THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
| ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
| IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
| ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
| FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
| DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
| OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
| HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
| LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
| OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
| SUCH DAMAGE.
|
|	@(#)vectors.s	8.2 (Berkeley) 1/21/94
|

	.data
	.globl	_buserr,_addrerr
	.globl	_illinst,_zerodiv,_chkinst,_trapvinst,_privinst,_trace
	.globl	_badtrap
	.globl	_spurintr
	.globl	_trap0,_trap1,_trap2,_trap15
	.globl	_fpfline, _fpunsupp
	.globl	_trap12

	.globl	_vectab
_vectab:
	.long	0x12345678	/* 0: jmp 0x7400:w (unused reset SSP) */
	.long	0		/* 1: NOT USED (reset PC) */
	.long	_buserr		/* 2: bus error */
	.long	_addrerr	/* 3: address error */
	.long	_illinst	/* 4: illegal instruction */
	.long	_zerodiv	/* 5: zero divide */
	.long	_chkinst	/* 6: CHK instruction */
	.long	_trapvinst	/* 7: TRAPV instruction */
	.long	_privinst	/* 8: privilege violation */
	.long	_trace		/* 9: trace */
	.long	_illinst	/* 10: line 1010 emulator */
	.long	_fpfline	/* 11: line 1111 emulator */
	.long	_badtrap	/* 12: unassigned, reserved */
	.long	_coperr		/* 13: coprocessor protocol violation */
	.long	_fmterr		/* 14: format error */
	.long	_badtrap	/* 15: uninitialized interrupt vector */
	.long	_badtrap	/* 16: unassigned, reserved */
	.long	_badtrap	/* 17: unassigned, reserved */
	.long	_badtrap	/* 18: unassigned, reserved */
	.long	_badtrap	/* 19: unassigned, reserved */
	.long	_badtrap	/* 20: unassigned, reserved */
	.long	_badtrap	/* 21: unassigned, reserved */
	.long	_badtrap	/* 22: unassigned, reserved */
	.long	_badtrap	/* 23: unassigned, reserved */
	.long	_spurintr	/* 24: spurious interrupt */
	.long	_badtrap	/* 25: level 1 interrupt autovector */
	.long	_badtrap	/* 26: level 2 interrupt autovector */
	.long	_badtrap	/* 27: level 3 interrupt autovector */
	.long	_badtrap	/* 28: level 4 interrupt autovector */
	.long	_badtrap	/* 29: level 5 interrupt autovector */
	.long	_badtrap	/* 30: level 6 interrupt autovector */
	.long	_badtrap	/* 31: level 7 interrupt autovector */
	.long	_trap0		/* 32: syscalls */
	.long	_trap1		/* 33: sigreturn syscall or breakpoint */
	.long	_trap2		/* 34: breakpoint or sigreturn syscall */
	.long	_illinst	/* 35: TRAP instruction vector */
	.long	_illinst	/* 36: TRAP instruction vector */
	.long	_illinst	/* 37: TRAP instruction vector */
	.long	_illinst	/* 38: TRAP instruction vector */
	.long	_illinst	/* 39: TRAP instruction vector */
	.long	_illinst	/* 40: TRAP instruction vector */
	.long	_illinst	/* 41: TRAP instruction vector */
	.long	_illinst	/* 42: TRAP instruction vector */
	.long	_illinst	/* 43: TRAP instruction vector */
	.long	_trap12		/* 44: TRAP instruction vector */
	.long	_illinst	/* 45: TRAP instruction vector */
	.long	_illinst	/* 46: TRAP instruction vector */
	.long	_trap15		/* 47: TRAP instruction vector */

	/*
	 * 68881/68882: _fpfault zone
	 */
	.globl	_fpvect_tab, _fpvect_end
_fpvect_tab:
	.globl	_fpfault
	.long	_fpfault	/* 48: FPCP branch/set on unordered cond */
	.long	_fpfault	/* 49: FPCP inexact result */
	.long	_fpfault	/* 50: FPCP divide by zero */
	.long	_fpfault	/* 51: FPCP underflow */
	.long	_fpfault	/* 52: FPCP operand error */
	.long	_fpfault	/* 53: FPCP overflow */
	.long	_fpfault	/* 54: FPCP signalling NAN */
_fpvect_end:

	.long	_fpunsupp	/* 55: FPCP unimplemented data type */
	.long	_badtrap	/* 56: unassigned, reserved */
	.long	_badtrap	/* 57: unassigned, reserved */
	.long	_badtrap	/* 58: unassigned, reserved */
	.long	_badtrap	/* 59: unassigned, reserved */
	.long	_badtrap	/* 60: unassigned, reserved */
	.long	_badtrap	/* 61: unassigned, reserved */
	.long	_badtrap	/* 62: unassigned, reserved */
	.long	_badtrap	/* 63: unassigned, reserved */

#define BADTRAP16	.long	_badtrap,_badtrap,_badtrap,_badtrap,\
				_badtrap,_badtrap,_badtrap,_badtrap,\
				_badtrap,_badtrap,_badtrap,_badtrap,\
				_badtrap,_badtrap,_badtrap,_badtrap

	BADTRAP16		/* 64-79: user interrupt vectors */
	BADTRAP16		/* 80-95: user interrupt vectors */
	BADTRAP16		/* 96-111: user interrupt vectors */
	BADTRAP16		/* 112-127: user interrupt vectors */
	BADTRAP16		/* 128-143: user interrupt vectors */
	BADTRAP16		/* 144-159: user interrupt vectors */
	BADTRAP16		/* 160-175: user interrupt vectors */
	BADTRAP16		/* 176-191: user interrupt vectors */
	BADTRAP16		/* 192-207: user interrupt vectors */
	BADTRAP16		/* 208-223: user interrupt vectors */
	BADTRAP16		/* 224-239: user interrupt vectors */
	BADTRAP16		/* 240-255: user interrupt vectors */


#ifdef FPSP
	/*
	 * 68040: this chunk of vectors is copied into the _fpfault zone
	 */
	.globl _fpsp_tab
_fpsp_tab:
	.globl	bsun, inex, dz, unfl, operr, ovfl, snan
	.long	bsun		/* 48: FPCP branch/set on unordered cond */
	.long	inex		/* 49: FPCP inexact result */
	.long	dz		/* 50: FPCP divide by zero */
	.long	unfl		/* 51: FPCP underflow */
	.long	operr		/* 52: FPCP operand error */
	.long	ovfl		/* 53: FPCP overflow */
	.long	snan		/* 54: FPCP signalling NAN */
#endif FPSP
