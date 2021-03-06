/* $NetBSD: copyinout.S,v 1.2 2002/03/24 23:37:42 bjh21 Exp $ */

/*-
 * Copyright (c) 2000 Ben Harris
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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
 */
/*
 * copyinout.S - copy data between user and kernel space
 */

#include <machine/asm.h>

RCSID("$NetBSD: copyinout.S,v 1.2 2002/03/24 23:37:42 bjh21 Exp $")

#include <sys/errno.h>
#include "assym.h"

/*
 * int copyin(const void *ua, void *ka, size_t len);
 * int copyout(const void *ka, void *ua, size_t len);
 * int kcopy(const void *src, void *dst, size_t len);
 */

/*
 * memcpy isn't currently data-abort-safe (it uses R14).  This is much
 * slower, but safer.
 */

/* LINTSTUB: Func: int copyin(const void *uaddr, void *kaddr, size_t len) */
ENTRY(copyin)
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	adr	r3, Lcopyfault
	ldr	r4, Lcurproc
	ldr	r4, [r4]
	ldr	r4, [r4, #P_ADDR]
	str	r3, [r4, #(U_PCB + PCB_ONFAULT)]
	teq	r2, #0
	beq	Lcopyinskip
Lcopyinloop:
	ldrbt	r3, [r0], #1
	strb	r3, [r1], #1
	subs	r2, r2, #1
	bne	Lcopyinloop
Lcopyinskip:
	mov	r0, #0
	str	r0, [r4, #(U_PCB + PCB_ONFAULT)]
#ifdef __APCS_26__
	ldmdb	fp, {r4, fp, sp, pc}^
#else
	ldmdb	fp, {r4, fp, sp, pc}
#endif

/* LINTSTUB: Func: int copyout(const void *kaddr, void *uaddr, size_t len) */
ENTRY(copyout)
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	adr	r3, Lcopyfault
	ldr	r4, Lcurproc
	ldr	r4, [r4]
	ldr	r4, [r4, #P_ADDR]
	str	r3, [r4, #(U_PCB + PCB_ONFAULT)]
	teq	r2, #0
	beq	Lcopyoutskip
Lcopyoutloop:
	ldrb	r3, [r0], #1
	strbt	r3, [r1], #1
	subs	r2, r2, #1
	bne	Lcopyoutloop
Lcopyoutskip:
	mov	r0, #0
	str	r0, [r4, #(U_PCB + PCB_ONFAULT)]
#ifdef __APCS_26__
	ldmdb	fp, {r4, fp, sp, pc}^
#else
	ldmdb	fp, {r4, fp, sp, pc}
#endif

/* LINTSTUB: Func: int kcopy(const void *kfaddr, void *kdaddr, size_t len) */
ENTRY(kcopy)
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	adr	r3, Lcopyfault
	ldr	r4, Lcurproc
	ldr	r4, [r4]
	ldr	r4, [r4, #P_ADDR]
	str	r3, [r4, #(U_PCB + PCB_ONFAULT)]
	teq	r2, #0
	beq	Lkcopyskip
Lkcopyloop:
	ldrb	r3, [r0], #1
	strb	r3, [r1], #1
	subs	r2, r2, #1
	bne	Lkcopyloop
Lkcopyskip:
	mov	r0, #0
	str	r0, [r4, #(U_PCB + PCB_ONFAULT)]
#ifdef __APCS_26__
	ldmdb	fp, {r4, fp, sp, pc}^
#else
	ldmdb	fp, {r4, fp, sp, pc}
#endif

Lcopyfault:
	mov	r1, #0
	str	r1, [r4, #(U_PCB + PCB_ONFAULT)]
	/* Return value is provided by fault handler. */
#ifdef __APCS_26__
	ldmdb	fp, {r4, fp, sp, pc}^
#else
	ldmdb	fp, {r4, fp, sp, pc}
#endif

/* LINTSTUB: Func: int copyinstr(const void *uaddr, void *kaddr, size_t len, size_t *done) */
ENTRY(copyinstr)
	mov	ip, sp
	stmfd	sp!, {r4, r5, r6, fp, ip, lr, pc}
	sub	fp, ip, #4
	adr	r4, Lcopystrfault
	ldr	r5, Lcurproc
	ldr	r5, [r5]
	ldr	r5, [r5, #P_ADDR]
	str	r4, [r5, #(U_PCB + PCB_ONFAULT)]
	mov	r6, r1
Lcopyinstrloop:
	ldrbt	r4, [r0], #1
	strb	r4, [r1], #1
	subs	r2, r2, #1
	teqne	r4, #0
	bne	Lcopyinstrloop
	teq	r3, #0
	subne	r6, r1, r6
	strne	r6, [r3]
	mov	r0, #0
	str	r0, [r5, #(U_PCB + PCB_ONFAULT)]
	teq	r4, #0
	movne	r0, #ENAMETOOLONG
#ifdef __APCS_26__
	ldmdb	fp, {r4, r5, r6, fp, sp, pc}^
#else
	ldmdb	fp, {r4, r5, r6, fp, sp, pc}
#endif

/* LINTSTUB: Func: int copyoutstr(const void *kaddr, void *uaddr, size_t len, size_t *done) */
ENTRY(copyoutstr)
	mov	ip, sp
	stmfd	sp!, {r4, r5, r6, fp, ip, lr, pc}
	sub	fp, ip, #4
	adr	r4, Lcopystrfault
	ldr	r5, Lcurproc
	ldr	r5, [r5]
	ldr	r5, [r5, #P_ADDR]
	str	r4, [r5, #(U_PCB + PCB_ONFAULT)]
	mov	r6, r1
Lcopyoutstrloop:
	ldrb	r4, [r0], #1
	strbt	r4, [r1], #1
	subs	r2, r2, #1
	teqne	r4, #0
	bne	Lcopyoutstrloop
	teq	r3, #0
	subne	r6, r1, r6
	strne	r6, [r3]
	mov	r0, #0
	str	r0, [r5, #(U_PCB + PCB_ONFAULT)]
	teq	r4, #0
	movne	r0, #ENAMETOOLONG
#ifdef __APCS_26__
	ldmdb	fp, {r4, r5, r6, fp, sp, pc}^
#else
	ldmdb	fp, {r4, r5, r6, fp, sp, pc}
#endif

/* LINTSTUB: Func: int copystr(const void *kfaddr, void *kdaddr, size_t len, size_t *done) */
ENTRY(copystr)
	mov	ip, sp
	stmfd	sp!, {r4, r5, r6, fp, ip, lr, pc}
	sub	fp, ip, #4
	adr	r4, Lcopystrfault
	ldr	r5, Lcurproc
	ldr	r5, [r5]
	ldr	r5, [r5, #P_ADDR]
	str	r4, [r5, #(U_PCB + PCB_ONFAULT)]
	mov	r6, r1
Lcopystrloop:
	ldrb	r4, [r0], #1
	strb	r4, [r1], #1
	subs	r2, r2, #1
	teqne	r4, #0
	bne	Lcopystrloop
	teq	r3, #0
	subne	r6, r1, r6
	strne	r6, [r3]
	mov	r0, #0
	str	r0, [r5, #(U_PCB + PCB_ONFAULT)]
	teq	r4, #0
	movne	r0, #ENAMETOOLONG
	ldmdb	fp, {r4, r5, r6, fp, sp, pc}^

Lcopystrfault:
	teq	r3, #0
	subne	r6, r1, r6
	strne	r6, [r3]
	mov	r1, #0
	str	r1, [r5, #(U_PCB + PCB_ONFAULT)]
	/* Return value is provided by fault handler. */
#ifdef __APCS_26__
	ldmdb	fp, {r4, r5, r6, fp, sp, pc}^
#else
	ldmdb	fp, {r4, r5, r6, fp, sp, pc}
#endif

/* LINTSTUB: Func: int fubyte(void *base) */
ENTRY(fubyte)
	mov	ip, lr			/* data-abort safety */
	adr	r1, Lfusufault
	ldr	r2, Lcurproc
	ldr	r2, [r2]
	ldr	r2, [r2, #P_ADDR]
	str	r1, [r2, #(U_PCB + PCB_ONFAULT)]
/* XXX Warning: Rn and Rd must be different in LDRT */
	ldrbt	r0, [r0]
	mov	r1, #0
	str	r1, [r2, #(U_PCB + PCB_ONFAULT)]
#ifdef __APCS_26__
	movs	pc, ip
#else
	mov	pc, ip
#endif

/* LINTSTUB: Func: int fuword(void *base) */
ENTRY(fuword)
	mov	ip, lr
	adr	r1, Lfusufault
	ldr	r2, Lcurproc
	ldr	r2, [r2]
	ldr	r2, [r2, #P_ADDR]
	str	r1, [r2, #(U_PCB + PCB_ONFAULT)]
/* XXX Warning: Rn and Rd must be different in LDRT */
	ldrt	r0, [r0]
	mov	r1, #0
	str	r1, [r2, #(U_PCB + PCB_ONFAULT)]
#ifdef __APCS_26__
	movs	pc, ip
#else
	mov	pc, ip
#endif

/* LINTSTUB: Func: int subyte(void *base, int c) */
ENTRY(subyte)
	mov	ip, lr
	adr	r3, Lfusufault
	ldr	r2, Lcurproc
	ldr	r2, [r2]
	ldr	r2, [r2, #P_ADDR]
	str	r3, [r2, #(U_PCB + PCB_ONFAULT)]
	strbt	r1, [r0]
	mov	r0, #0
	str	r0, [r2, #(U_PCB + PCB_ONFAULT)]
#ifdef __APCS_26__
	movs	pc, ip
#else
	mov	pc, ip
#endif

/* LINTSTUB: Func: int suword(void *base, long c) */
ENTRY(suword)
	mov	ip, lr
	adr	r3, Lfusufault
	ldr	r2, Lcurproc
	ldr	r2, [r2]
	ldr	r2, [r2, #P_ADDR]
	str	r3, [r2, #(U_PCB + PCB_ONFAULT)]
	strt	r1, [r0]
	mov	r0, #0
	str	r0, [r2, #(U_PCB + PCB_ONFAULT)]
#ifdef __APCS_26__
	movs	pc, ip
#else
	mov	pc, ip
#endif

Lfusufault:
	mov	r0, #0
	str	r0, [r2, #(U_PCB + PCB_ONFAULT)]
	mvn	r0, #0
#ifdef __APCS_26__
	movs	pc, ip
#else
	mov	pc, ip
#endif

Lcurproc:
	.word	_C_LABEL(curproc)
