/*	$OpenBSD: db_machdep.h,v 1.1 1998/12/05 17:38:34 mickey Exp $	*/

/*
 * Copyright (c) 1998 Michael Shalayeff
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
 *	This product includes software developed by Michael Shalayeff.
 * 4. The name of the author may not be used to endorse or promote products
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

#ifndef	_MACHINE_DB_MACHDEP_H_
#define	_MACHINE_DB_MACHDEP_H_

/* types the generic ddb module needs */
typedef	vm_offset_t db_addr_t;
typedef	long db_expr_t;

typedef struct trapframe db_regs_t;
extern db_regs_t	ddb_regs;
#define	DDB_REGS	(&ddb_regs)

#define	PC_REGS(regs)	((db_addr_t)(regs)->iioq_head)

/* Breakpoint related definitions */
#define	BKPT_INST	0x00000000	/* break 0,0 */
#define	BKPT_SIZE	sizeof(int)
#define	BKPT_SET(inst)	BKPT_INST

#define	IS_BREAKPOINT_TRAP(type, code) 1
#define	IS_WATCHPOINT_TRAP(type, code) 0

#define	FIXUP_PC_AFTER_BREAK(regs) ((regs)->tf_regs[FRAME_PC] -= sizeof(int))

#define DB_VALID_BREAKPOINT(addr) db_valid_breakpoint(addr)

static __inline int inst_call(u_int ins) {
	return (ins & 0xfc00e000) == 0xe8000000 ||
	       (ins & 0xfc00e000) == 0xe8004000 ||
	       (ins & 0xfc000000) == 0xe4000000;
}
static __inline int inst_branch(u_int ins) {
	return (ins & 0xf0000000) == 0xe0000000 ||
	       (ins & 0xf0000000) == 0xc0000000 ||
	       (ins & 0xf0000000) == 0xa0000000 ||
	       (ins & 0xf0000000) == 0x80000000;
}
static __inline int inst_load(u_int ins) {
	return (ins & 0xf0000000) == 0x40000000 ||
	       (ins & 0xf4000200) == 0x24000000 ||
	       (ins & 0xfc000200) == 0x0c000000 ||
	       (ins & 0xfc001fc0) != 0x0c0011c0;
}
static __inline int inst_store(u_int ins) {
	return (ins & 0xf0000000) == 0x60000000 ||
	       (ins & 0xf4000200) == 0x24000200 ||
	       (ins & 0xfc000200) == 0x0c000200;
}
static __inline int inst_return(u_int ins) {
	return (ins & 0xfc00e000) == 0xe800c000 ||
	       (ins & 0xfc000000) == 0xe0000000;
}
static __inline int inst_trap_return(u_int ins)	{
	return (ins & 0xfc001fc0) == 0x00000ca0;
}

#define db_clear_single_step(r)	((r)->flags |= 0)
#define db_set_single_step(r)	((r)->flags |= 0)

int db_valid_breakpoint __P((db_addr_t));
int kdb_trap __P((int, int, db_regs_t *));

#endif /* _MACHINE_DB_MACHDEP_H_ */
