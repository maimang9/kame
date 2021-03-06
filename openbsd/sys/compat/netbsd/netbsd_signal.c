/*	$OpenBSD: netbsd_signal.c,v 1.4 1999/09/16 13:20:06 kstailey Exp $	*/

/*	$NetBSD: kern_sig.c,v 1.54 1996/04/22 01:38:32 christos Exp $	*/

/*
 * Copyright (c) 1997 Theo de Raadt. All rights reserved. 
 * Copyright (c) 1982, 1986, 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)kern_sig.c	8.7 (Berkeley) 4/18/94
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/signal.h>
#include <sys/systm.h>

#include <compat/netbsd/netbsd_types.h>
#include <compat/netbsd/netbsd_signal.h>
#include <compat/netbsd/netbsd_syscallargs.h>

static void netbsd_to_openbsd_sigaction __P((struct netbsd_sigaction *,
	struct sigaction *));

static void openbsd_to_netbsd_sigaction __P((struct sigaction *,
	struct netbsd_sigaction *));

static void netbsd_to_openbsd_sigaltstack __P((struct netbsd_sigaltstack *,
	struct sigaltstack *));

static void openbsd_to_netbsd_sigaltstack __P((struct sigaltstack *,
	struct netbsd_sigaltstack *));

static void
openbsd_to_netbsd_sigaction(obsa, nbsa)
	struct sigaction	*obsa;
	struct netbsd_sigaction	*nbsa;
{
	bzero(nbsa, sizeof(struct netbsd_sigaction));
	nbsa->netbsd_sa_handler = obsa->sa_handler;
	bcopy(&obsa->sa_mask, &nbsa->netbsd_sa_mask.__bits[0],
		sizeof(sigset_t));
	nbsa->netbsd_sa_flags = obsa->sa_flags;
}

static void
netbsd_to_openbsd_sigaction(nbsa, obsa)
	struct netbsd_sigaction	*nbsa;
	struct sigaction	*obsa;
{
	obsa->sa_handler = nbsa->netbsd_sa_handler;
	bcopy(&nbsa->netbsd_sa_mask.__bits[0], &obsa->sa_mask,
		sizeof(sigset_t));
	obsa->sa_flags = nbsa->netbsd_sa_flags;
}

static void
netbsd_to_openbsd_sigaltstack(nbss, obss)
	struct netbsd_sigaltstack *nbss;
	struct sigaltstack *obss;
{
	obss->ss_sp = nbss->netbsd_ss_sp;
	obss->ss_size = nbss->netbsd_ss_size; /* XXX may cause truncation */
	obss->ss_flags = nbss->netbsd_ss_flags;
}

static void
openbsd_to_netbsd_sigaltstack(obss, nbss)
	struct sigaltstack *obss;
	struct netbsd_sigaltstack *nbss;
{
	bzero(nbss, sizeof(nbss));
	nbss->netbsd_ss_sp = obss->ss_sp;
	nbss->netbsd_ss_size = (size_t)obss->ss_size;
	nbss->netbsd_ss_flags = obss->ss_flags;
}


/* ARGSUSED */
int
netbsd_sys___sigaction14(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct netbsd_sys___sigaction14_args /* {
		syscallarg(int) signum;
		syscallarg(struct netbsd_sigaction *) nsa;
		syscallarg(struct netbsd_sigaction *) osa;
	} */ *uap = v;
	struct sigaction vec;
	register struct sigaction *sa;
	struct netbsd_sigaction nbsa;
	register struct sigacts *ps = p->p_sigacts;
	register int signum;
	int bit, error;

	signum = SCARG(uap, signum);
	if (signum <= 0 || signum >= NSIG ||
	    (SCARG(uap, nsa) && (signum == SIGKILL || signum == SIGSTOP)))
		return (EINVAL);
	sa = &vec;
	if (SCARG(uap, osa)) {
		sa->sa_handler = ps->ps_sigact[signum];
		sa->sa_mask = ps->ps_catchmask[signum];
		bit = sigmask(signum);
		sa->sa_flags = 0;
		if ((ps->ps_sigonstack & bit) != 0)
			sa->sa_flags |= SA_ONSTACK;
		if ((ps->ps_sigintr & bit) == 0)
			sa->sa_flags |= SA_RESTART;
		if ((ps->ps_sigreset & bit) != 0)
			sa->sa_flags |= SA_RESETHAND;
		if ((ps->ps_siginfo & bit) != 0)
			sa->sa_flags |= SA_SIGINFO;
		if (signum == SIGCHLD) {
			if ((p->p_flag & P_NOCLDSTOP) != 0)
				sa->sa_flags |= SA_NOCLDSTOP;
			if ((p->p_flag & P_NOCLDWAIT) != 0)
				sa->sa_flags |= SA_NOCLDWAIT;
		}
		if ((sa->sa_mask & bit) == 0)
			sa->sa_flags |= SA_NODEFER;
		sa->sa_mask &= ~bit;
		openbsd_to_netbsd_sigaction(sa, &nbsa);
		error = copyout((caddr_t)&nbsa, (caddr_t)SCARG(uap, osa),
				sizeof (struct netbsd_sigaction));
		if (error)
			return (error);
	}
	if (SCARG(uap, nsa)) {
		error = copyin((caddr_t)SCARG(uap, nsa), (caddr_t)&nbsa,
			       sizeof (struct netbsd_sigaction));
		if (error)
			return (error);
		netbsd_to_openbsd_sigaction(&nbsa, sa);
		setsigvec(p, signum, sa);
	}
	return (0);
}

/* ARGSUSED */
int
netbsd_sys___sigaltstack14(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct netbsd_sys___sigaltstack14_args /* {
		syscallarg(struct netbsd_sigaltstack *) nss;
		syscallarg(struct netbsd_sigaltstack *) oss;
	} */ *uap = v;
	struct sigacts *psp;
	struct sigaltstack ss;
	struct netbsd_sigaltstack nbss;
	int error;

	psp = p->p_sigacts;
	if ((psp->ps_flags & SAS_ALTSTACK) == 0)
		psp->ps_sigstk.ss_flags |= SS_DISABLE;
	if (SCARG(uap, oss)) {
		openbsd_to_netbsd_sigaltstack(&psp->ps_sigstk, &nbss);
		if ((error = copyout((caddr_t)&nbss, (caddr_t)SCARG(uap, oss),
			sizeof (struct netbsd_sigaltstack))))
		return (error);
	}
	if (SCARG(uap, nss) == 0)
		return (0);
	error = copyin((caddr_t)SCARG(uap, nss), (caddr_t)&nbss, sizeof(nbss));
	if (error)
		return (error);
	netbsd_to_openbsd_sigaltstack(&nbss, &ss);
	if (ss.ss_flags & SS_DISABLE) {
		if (psp->ps_sigstk.ss_flags & SS_ONSTACK)
			return (EINVAL);
		psp->ps_flags &= ~SAS_ALTSTACK;
		psp->ps_sigstk.ss_flags = ss.ss_flags;
		return (0);
	}
	if (ss.ss_size < MINSIGSTKSZ)
		return (ENOMEM);
	psp->ps_flags |= SAS_ALTSTACK;
	psp->ps_sigstk= ss;
	return (0);
}

/* ARGSUSED */
int
netbsd_sys___sigpending14(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct netbsd_sys___sigpending14_args /* {
		netbsd_sigset_t *set;
	} */ *uap = v;
	netbsd_sigset_t nss;

	bcopy(&p->p_siglist, &nss.__bits[0], sizeof(sigset_t));
	return (copyout((caddr_t)&nss, (caddr_t)SCARG(uap, set), sizeof(nss)));
}

int
netbsd_sys___sigprocmask14(p, v, retval)
	register struct proc *p;
	void *v;
	register_t *retval;
{
	struct netbsd_sys___sigprocmask14_args /* {
		syscallarg(int) how;
		syscallarg(netbsd_sigset_t *) set;
		syscallarg(netbsd_sigset_t *) oset;
	} */ *uap = v;
	netbsd_sigset_t nss, oss;
	sigset_t obnss;
	int error = 0;

	if (SCARG(uap, set)) {
		error = copyin(SCARG(uap, set), &nss, sizeof(nss));
		if (error)
			return (error);
	}
	if (SCARG(uap, oset)) {
		bzero(&oss, sizeof(netbsd_sigset_t));
		bcopy(&p->p_sigmask, &oss.__bits[0], sizeof(sigset_t));
		error = copyout((caddr_t)&oss, (caddr_t)SCARG(uap, oset),
			sizeof(netbsd_sigset_t));
		if (error)
			return (error);
	}
	if (SCARG(uap, set)) {
		bcopy(&nss.__bits[0], &obnss, sizeof(sigset_t));
		(void)splhigh();
		switch (SCARG(uap, how)) {
		case SIG_BLOCK:
			p->p_sigmask |= obnss &~ sigcantmask;
			break;
		case SIG_UNBLOCK:
			p->p_sigmask &= ~obnss;
			break;
		case SIG_SETMASK:
			p->p_sigmask = obnss &~ sigcantmask;
			break;
		default:
			error = EINVAL;
			break;
		}
		(void) spl0();
	}
	return (error);
}

int
netbsd_sys___sigsuspend14(p, v, retval)
	register struct proc *p;
	void *v;
	register_t *retval;
{
	struct netbsd_sys___sigsuspend14_args /* {
		syscallarg(netbsd_sigset_t *) set;
	} */ *uap = v;
	register struct sigacts *ps = p->p_sigacts;
	netbsd_sigset_t nbset;
	sigset_t obset;

	copyin(SCARG(uap, set), &nbset, sizeof(netbsd_sigset_t));
	bcopy(&nbset.__bits[0], &obset, sizeof(sigset_t));
	/*
	 * When returning from sigpause, we want
	 * the old mask to be restored after the
	 * signal handler has finished.  Thus, we
	 * save it here and mark the sigacts structure
	 * to indicate this.
	 */
	ps->ps_oldmask = p->p_sigmask;
	ps->ps_flags |= SAS_OLDMASK;
	p->p_sigmask = obset &~ sigcantmask;
	while (tsleep((caddr_t) ps, PPAUSE|PCATCH, "pause", 0) == 0)
		/* void */;
	/* always return EINTR rather than ERESTART... */
	return (EINTR);
}

