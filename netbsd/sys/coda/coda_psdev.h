/*	$NetBSD: coda_psdev.h,v 1.1 1998/09/25 15:01:13 rvb Exp $	*/

/*
 * 
 *             Coda: an Experimental Distributed File System
 *                              Release 3.1
 * 
 *           Copyright (c) 1998 Carnegie Mellon University
 *                          All Rights Reserved
 * 
 * Permission  to  use, copy, modify and distribute this software and its
 * documentation is hereby granted,  provided  that  both  the  copyright
 * notice  and  this  permission  notice  appear  in  all  copies  of the
 * software, derivative works or  modified  versions,  and  any  portions
 * thereof, and that both notices appear in supporting documentation, and
 * that credit is given to Carnegie Mellon University  in  all  documents
 * and publicity pertaining to direct or indirect use of this code or its
 * derivatives.
 * 
 * CODA IS AN EXPERIMENTAL SOFTWARE SYSTEM AND IS  KNOWN  TO  HAVE  BUGS,
 * SOME  OF  WHICH MAY HAVE SERIOUS CONSEQUENCES.  CARNEGIE MELLON ALLOWS
 * FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION.   CARNEGIE  MELLON
 * DISCLAIMS  ANY  LIABILITY  OF  ANY  KIND  FOR  ANY  DAMAGES WHATSOEVER
 * RESULTING DIRECTLY OR INDIRECTLY FROM THE USE OF THIS SOFTWARE  OR  OF
 * ANY DERIVATIVE WORK.
 * 
 * Carnegie  Mellon  encourages  users  of  this  software  to return any
 * improvements or extensions that  they  make,  and  to  grant  Carnegie
 * Mellon the rights to redistribute these changes without encumbrance.
 * 
 * 	@(#) coda/coda_psdev.h,v 1.1 1998/08/29 21:26:45 rvb Exp $ 
 */


int vc_nb_open(dev_t dev, int flag, int mode, struct proc *p);
int vc_nb_close (dev_t dev, int flag, int mode, struct proc *p);
int vc_nb_read(dev_t dev, struct uio *uiop, int flag);
int vc_nb_write(dev_t dev, struct uio *uiop, int flag);
int vc_nb_ioctl(dev_t dev, u_long cmd, caddr_t addr, int flag, struct proc *p);
int vc_nb_poll(dev_t dev, int events, struct proc *p);
