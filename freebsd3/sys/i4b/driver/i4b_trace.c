/*
 * Copyright (c) 1997, 1998 Hellmuth Michaelis. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *---------------------------------------------------------------------------
 *
 *	i4btrc - device driver for trace data read device
 *	---------------------------------------------------
 *
 *	$Id: i4b_trace.c,v 1.1 1998/12/27 21:46:42 phk Exp $
 *
 *	last edit-date: [Sat Dec  5 18:01:53 1998]
 *
 *	NOTE: the code assumes that SPLI4B >= splimp !
 *
 *---------------------------------------------------------------------------*/

#include "i4btrc.h"

#if NI4BTRC > 0

#ifdef __FreeBSD__
#include "isic.h"	/* 'isic' is no pseudo-device on non FreeBSD -
			 * so we just can't count it at compile time,
			 * we're doing an attach-time check instead. */

#if NI4BTRC < NISIC
#error "number of trace devices != number of passive ISDN controllers !"
#error "change number of i4btrc to be equal to number of isic devices !"
#endif
#endif	/* __FreeBSD__ */

#include <sys/param.h>
#include <sys/systm.h>

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
#include <sys/ioccom.h>
#else
#include <sys/ioctl.h>
#endif

#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/proc.h>
#include <sys/tty.h>

#ifdef __FreeBSD__
#include "opt_devfs.h"
#endif

#ifdef DEVFS
#include <sys/devfsext.h>
#endif

#ifdef __FreeBSD__
#include <machine/i4b_trace.h>
#include <machine/i4b_ioctl.h>
#else
#include <i4b/i4b_trace.h>
#include <i4b/i4b_ioctl.h>
#endif

#ifndef __FreeBSD__
#define	memcpy(d,s,l)	bcopy(s,d,l)
#endif

#include <i4b/include/i4b_mbuf.h>
#include <i4b/include/i4b_global.h>
#include <i4b/include/i4b_l3l4.h>

static struct ifqueue trace_queue[NI4BTRC];
static int device_state[NI4BTRC];
#define ST_IDLE		0x00
#define ST_ISOPEN	0x01
#define ST_WAITDATA	0x02
#ifdef DEVFS
static void *devfs_token[NI4BTRC];
#endif

static int analyzemode = 0;
static int rxunit = -1;
static int txunit = -1;
static int outunit = -1;

#ifndef __FreeBSD__
#define	PDEVSTATIC	/* - not static - */
void i4btrcattach __P((void));
int i4btrcopen __P((dev_t dev, int flag, int fmt, struct proc *p));
int i4btrcclose __P((dev_t dev, int flag, int fmt, struct proc *p));
int i4btrcread __P((dev_t dev, struct uio * uio, int ioflag));
int i4btrcioctl __P((dev_t dev, int cmd, caddr_t data, int flag, struct proc *p));
#endif

#if BSD > 199306 && defined(__FreeBSD__)
#define	PDEVSTATIC static
static d_open_t	i4btrcopen;
static d_close_t i4btrcclose;
static d_read_t i4btrcread;
static d_ioctl_t i4btrcioctl;
#if defined(__FreeBSD_version) && __FreeBSD_version >= 300001
static d_poll_t i4btrcpoll;
#endif

#define CDEV_MAJOR 59
static struct cdevsw i4btrc_cdevsw = {
	i4btrcopen,	i4btrcclose,	i4btrcread,	nowrite,
  	i4btrcioctl,	nostop,		noreset,	nodevtotty,
#if defined(__FreeBSD_version) && __FreeBSD_version >= 300001
	i4btrcpoll,	nommap, 	NULL, "i4btrc", NULL, -1
#else
	noselect,	nommap, 	NULL, "i4btrc", NULL, -1
#endif
};

/*---------------------------------------------------------------------------*
 *	interface init routine
 *---------------------------------------------------------------------------*/
static
void i4btrcinit(void *unused)
{
    dev_t dev;
    
    dev = makedev(CDEV_MAJOR, 0);

    cdevsw_add(&dev, &i4btrc_cdevsw, NULL);
}

SYSINIT(i4btrcdev, SI_SUB_DRIVERS,
	SI_ORDER_MIDDLE+CDEV_MAJOR, &i4btrcinit, NULL);

static void i4btrcattach(void *);
PSEUDO_SET(i4btrcattach, i4b_trace);

#endif /* BSD > 199306 && defined(__FreeBSD__) */

int get_trace_data_from_l1(i4b_trace_hdr_t *hdr, int len, char *buf);

/*---------------------------------------------------------------------------*
 *	interface attach routine
 *---------------------------------------------------------------------------*/
PDEVSTATIC void
#ifdef __FreeBSD__
i4btrcattach(void *dummy)
#else
i4btrcattach()
#endif
{
	int i;
	
#ifndef HACK_NO_PSEUDO_ATTACH_MSG
	printf("i4btrc: %d ISDN trace device(s) attached\n", NI4BTRC);
#endif
	
	for(i=0; i < NI4BTRC; i++)
	{
#ifdef DEVFS
	  	devfs_token[i]
		  = devfs_add_devswf(&i4btrc_cdevsw, i, DV_CHR,
				     UID_ROOT, GID_WHEEL, 0600,
				     "i4btrc%d", i);
#endif
		trace_queue[i].ifq_maxlen = IFQ_MAXLEN;
		device_state[i] = ST_IDLE;
	}
}

/*---------------------------------------------------------------------------*
 *	get_trace_data_from_l1()
 *	------------------------
 *	is called from layer 1, adds timestamp to trace data and puts
 *	it into a queue, from which it can be read from the i4btrc
 *	device. The unit number in the trace header selects the minor
 *	device's queue the data is put into.
 *---------------------------------------------------------------------------*/
int
get_trace_data_from_l1(i4b_trace_hdr_t *hdr, int len, char *buf)
{
	struct mbuf *m;
	int x;
	int unit;
	int trunc = 0;
	int totlen = len + sizeof(i4b_trace_hdr_t);

	/*
	 * for telephony (or better non-HDLC HSCX mode) we get 
	 * (MCLBYTE + sizeof(i4b_trace_hdr_t)) length packets
	 * to put into the queue to userland. because of this
	 * we detect this situation, strip the length to MCLBYTES
	 * max size, and infor the userland program of this fact
	 * by putting the no of truncated bytes into hdr->trunc.
	 */
	 
	if(totlen > MCLBYTES)
	{
		trunc = 1;
		hdr->trunc = totlen - MCLBYTES;
		totlen = MCLBYTES;
	}
	else
	{
		hdr->trunc = 0;
	}

	/* set length of trace record */
	
	hdr->length = totlen;
	
	/* check valid unit no */
	
	if((unit = hdr->unit) > NI4BTRC)
	{
		printf("i4b_trace: get_trace_data_from_l1 - unit > NI4BTRC!\n"); 
		return(0);
	}

	/* get mbuf */
	
	if(!(m = i4b_Bgetmbuf(totlen)))
	{
		printf("i4b_trace: get_trace_data_from_l1 - i4b_getmbuf() failed!\n");
		return(0);
	}

	/* check if we are in analyzemode */
	
	if(analyzemode && (unit == rxunit || unit == txunit))
	{
		if(unit == rxunit)
			hdr->dir = FROM_NT;
		else
			hdr->dir = FROM_TE;
		unit = outunit;			
	}

	if(IF_QFULL(&trace_queue[unit]))
	{
		struct mbuf *m1;

		x = SPLI4B();
		IF_DEQUEUE(&trace_queue[unit], m1);
		splx(x);		

		i4b_Bfreembuf(m1);
	}
	
	/* copy trace header */
	memcpy(m->m_data, hdr, sizeof(i4b_trace_hdr_t));

	/* copy trace data */
	if(trunc)
		memcpy(&m->m_data[sizeof(i4b_trace_hdr_t)], buf, totlen-sizeof(i4b_trace_hdr_t));
	else
		memcpy(&m->m_data[sizeof(i4b_trace_hdr_t)], buf, len);

	x = SPLI4B();
	
	IF_ENQUEUE(&trace_queue[unit], m);
	
	if(device_state[unit] & ST_WAITDATA)
	{
		device_state[unit] &= ~ST_WAITDATA;
		wakeup((caddr_t) &trace_queue[unit]);
	}

	splx(x);
	
	return(1);
}

/*---------------------------------------------------------------------------*
 *	open trace device
 *---------------------------------------------------------------------------*/
PDEVSTATIC int
i4btrcopen(dev_t dev, int flag, int fmt, struct proc *p)
{
	int x;
	int unit = minor(dev);

	if(unit > NI4BTRC)
		return(ENXIO);

	if(device_state[unit] & ST_ISOPEN)
		return(EBUSY);

	if(analyzemode && (unit == outunit || unit == rxunit || unit == txunit))
		return(EBUSY);

	x = SPLI4B();
	
	device_state[unit] = ST_ISOPEN;		

	splx(x);
	
	return(0);
}

/*---------------------------------------------------------------------------*
 *	close trace device
 *---------------------------------------------------------------------------*/
PDEVSTATIC int
i4btrcclose(dev_t dev, int flag, int fmt, struct proc *p)
{
	int unit = minor(dev);
	int i, x, cno = -1;

	for(i = 0; i < nctrl; i++)
	{
		if(ctrl_desc[i].N_SET_TRACE)
		{
			cno = i;
			break;
		}
	}

	if(analyzemode && (unit == outunit))
	{
		analyzemode = 0;		
		outunit = -1;
		
		if(cno >= 0)
		{
			ctrl_desc[cno].N_SET_TRACE(rxunit, TRACE_OFF);
			ctrl_desc[cno].N_SET_TRACE(txunit, TRACE_OFF);
		}
		rxunit = -1;
		txunit = -1;
	}
	
	if(cno >= 0)
	{
		ctrl_desc[cno].N_SET_TRACE(unit, TRACE_OFF);
	}

	x = SPLI4B();
	device_state[unit] = ST_IDLE;
	splx(x);
	
	return(0);
}

/*---------------------------------------------------------------------------*
 *	read from trace device
 *---------------------------------------------------------------------------*/
PDEVSTATIC int
i4btrcread(dev_t dev, struct uio * uio, int ioflag)
{
	struct mbuf *m;
	int x;
	int error = 0;
	int unit = minor(dev);
	
	if(!(device_state[unit] & ST_ISOPEN))
		return(EIO);

	x = SPLI4B();
	
	while(IF_QEMPTY(&trace_queue[unit]) && (device_state[unit] & ST_ISOPEN))
	{
		device_state[unit] |= ST_WAITDATA;
		
		if((error = tsleep((caddr_t) &trace_queue[unit],
					TTIPRI | PCATCH,
					"bitrc", 0 )) != 0)
		{
			device_state[unit] &= ~ST_WAITDATA;
			splx(x);
			return(error);
		}
	}

	IF_DEQUEUE(&trace_queue[unit], m);

	if(m && m->m_len)
		error = uiomove(m->m_data, m->m_len, uio);
	else
		error = EIO;
		
	if(m)
		i4b_Bfreembuf(m);

	splx(x);
	
	return(error);
}

/*---------------------------------------------------------------------------*
 *	poll device
 *---------------------------------------------------------------------------*/
#if defined(__FreeBSD_version) && __FreeBSD_version >= 300001
PDEVSTATIC int
i4btrcpoll(dev_t dev, int events, struct proc *p)
{
	return(ENODEV);
}
#endif

/*---------------------------------------------------------------------------*
 *	device driver ioctl routine
 *---------------------------------------------------------------------------*/
PDEVSTATIC int
#if defined (__FreeBSD_version) && __FreeBSD_version >= 300003
i4btrcioctl(dev_t dev, u_long cmd, caddr_t data, int flag, struct proc *p)
#else
i4btrcioctl(dev_t dev, int cmd, caddr_t data, int flag, struct proc *p)
#endif
{
	int error = 0;
	int unit = minor(dev);
	i4b_trace_setupa_t *tsa;
	int i, cno = -1;

	/* find the first passive controller to get at the set/get function
	   pointers. Would be better if we had the controller class virtual
	   function table separate from the port registry... */

	for(i=0; i < nctrl; i++)
	{
		if(ctrl_desc[i].N_SET_TRACE)
		{
			cno = i;	/* one suitable controller, might not */
			break;		/* be related to the trace unit at all, but */
		}			/* has the right function pointers */
	}
	
	switch(cmd)
	{
		case I4B_TRC_GET:
			if(cno < 0)
				return ENOTTY;
			*(int *)data = ctrl_desc[cno].N_GET_TRACE(unit);
			break;
		
		case I4B_TRC_SET:
			if(cno < 0)
				return ENOTTY;
			ctrl_desc[cno].N_SET_TRACE(unit, *(int *)data);
			break;

		case I4B_TRC_SETA:
			tsa = (i4b_trace_setupa_t *)data;

			if(tsa->rxunit >= 0 && tsa->rxunit < NI4BTRC)
				rxunit = tsa->rxunit;
			else
				error = EINVAL;

			if(tsa->txunit >= 0 && tsa->txunit < NI4BTRC)
				txunit = tsa->txunit;
			else
				error = EINVAL;

			if(error)
			{
				outunit = -1;
				rxunit = -1;
				txunit = -1;
			}
			else
			{
				if(cno < 0)
					return ENOTTY;
					
				outunit = unit;
				analyzemode = 1;
				ctrl_desc[cno].N_SET_TRACE(rxunit, tsa->rxflags & (TRACE_I | TRACE_D_RX | TRACE_B_RX));
				ctrl_desc[cno].N_SET_TRACE(txunit, tsa->txflags & (TRACE_I | TRACE_D_RX | TRACE_B_RX));
			}
			break;

		case I4B_TRC_RESETA:
			analyzemode = 0;		
			outunit = -1;
			rxunit = -1;
			txunit = -1;
			break;
			
		default:
			error = ENOTTY;
			break;
	}
	return(error);
}

#endif /* NI4BTRC > 0 */
