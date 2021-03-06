/* 
 * Mach Operating System
 * Copyright (c) 1993-1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes.
 */

#include <sys/types.h>
#include <machine/board.h>
#include <sys/param.h>
#include <machine/m882xx.h>		/* CMMU stuff */
#include <vm/vm.h>
#include <vm/vm_kern.h>			/* vm/vm_kern.h */
#include <machine/pmap_table.h>		/* pmap_table.h*/

#define R VM_PROT_READ
#define RW VM_PROT_READ|VM_PROT_WRITE
#define C CACHE_DFL
#define CW CACHE_WT
#define CI CACHE_INH
#define CG CACHE_GLOBAL
#define PAGE M88K_PGBYTES
#define SEG M88K_SGBYTES

#define M188_UTILITY		U(0xFF000000)
#define M188_UTILITY_SIZE	U(0x01000000)
#if 0 
#undef VEQR_ADDR
#define VEQR_ADDR 0
#endif 

/*  phys_start, virt_start, size, prot, cacheability */
#ifdef MVME187
static pmap_table_entry m187_board_table[] = {
   { BUGROM_START, BUGROM_START, BUGROM_SIZE, RW, CI},
   { SRAM_START  , SRAM_START  , SRAM_SIZE  , RW, CG},
   { OBIO_START  , OBIO_START  , OBIO_SIZE  , RW, CI},
   { 0           , 0           , 0xffffffff , 0 , 0},
};
#endif 

#ifdef MVME188
static pmap_table_entry m188_board_table[] = {
   { MVME188_UTILITY, MVME188_UTILITY, MVME188_UTILITY_SIZE, RW, CI},
   { 0           , VEQR_ADDR   , 0/*filled in later*/, RW, CG},
   { 0           , 0           , 0/*filled in later*/, RW, CG},
   { 0           , 0           , 0xffffffff , 0,  0},
};
#endif 

#ifdef MVME197
static pmap_table_entry m197_board_table[] = {
   { BUGROM_START, BUGROM_START, BUGROM_SIZE, RW, CI},
   { SRAM_START  , SRAM_START  , SRAM_SIZE  , RW, CG},
   { OBIO_START  , OBIO_START  , OBIO_SIZE  , RW, CG},
   { 0           , 0           , 0xffffffff , 0 , 0},
};
#endif 

pmap_table_t pmap_table_build(unsigned memory_size)
{
   extern int kernelstart;
   unsigned int i;
   pmap_table_t bt, pbt;

   switch (cputyp) {
#ifdef MVME187
      case CPU_187:
         bt = m187_board_table;
         break;
#endif 
#ifdef MVME188
      case CPU_188:
         bt = m188_board_table;
         /* fill in the veqr map entry */
         m188_board_table[1].size = memory_size;
         m188_board_table[2].size = (unsigned)&kernelstart;
         break;
#endif 
#ifdef MVME197
      case CPU_197:
         bt = m197_board_table;
         break;
#endif 
   default:
      panic("pmap_table_build: Unknown CPU type.");
      /* NOT REACHED */
   }
   
   /* round off all entries to nearest segment */
   pbt = bt;
   for (i = 0; pbt->size != 0xffffffff; i++){
      if (pbt->size>0)
         pbt->size = (pbt->size + M88K_PGBYTES-1) & ~(M88K_PGBYTES-1);
      pbt++;
   }

   return bt;
}
