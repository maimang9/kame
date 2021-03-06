/*-
 * Copyright (c) 2001 Matthew Dillon.  This code is distributed under
 * the BSD copyright, /usr/src/COPYRIGHT.
 *
 * $FreeBSD: src/sys/amd64/amd64/critical.c,v 1.9 2003/05/01 01:05:21 peter Exp $
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signalvar.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/pcpu.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <sys/ucontext.h>
#include <machine/clock.h>
#include <machine/critical.h>

/*
 * cpu_critical_fork_exit() - cleanup after fork
 */
void
cpu_critical_fork_exit(void)
{
	struct thread *td;

	td = curthread;
	td->td_critnest = 1;
	td->td_md.md_savecrit = read_rflags() | PSL_I;
}

/*
 * cpu_thread_link() - thread linkup, initialize machine-dependant fields
 */
void
cpu_thread_link(struct thread *td)
{

	td->td_md.md_savecrit = 0;
}
