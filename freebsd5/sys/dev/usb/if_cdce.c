/*	$NetBSD: if_cdce.c,v 1.4 2004/10/24 12:50:54 augustss Exp $ */

/*
 * Copyright (c) 1997, 1998, 1999, 2000-2003 Bill Paul <wpaul@windriver.com>
 * Copyright (c) 2003-2005 Craig Boston
 * Copyright (c) 2004 Daniel Hartmeier
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
 *	This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul, THE VOICES IN HIS HEAD OR
 * THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * USB Communication Device Class (Ethernet Networking Control Model)
 * http://www.usb.org/developers/devclass_docs/usbcdc11.pdf
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/dev/usb/if_cdce.c,v 1.6.2.1 2005/04/01 12:46:26 sobomax Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/socket.h>
#include <sys/endian.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>

#include <net/bpf.h>

#include <sys/bus.h>
#include <machine/bus.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>
#include <dev/usb/usbdi_util.h>
#include <dev/usb/usbdivar.h>
#include <dev/usb/usb_ethersubr.h>

#include <dev/usb/usbcdc.h>
#include "usbdevs.h"
#include <dev/usb/if_cdcereg.h>

Static device_shutdown_t cdce_shutdown;
USB_DECLARE_DRIVER_INIT(cdce,
	DEVMETHOD(device_probe, cdce_match),
	DEVMETHOD(device_attach, cdce_attach),
	DEVMETHOD(device_detach, cdce_detach),
	DEVMETHOD(device_shutdown, cdce_shutdown)
	);
DRIVER_MODULE(cdce, uhub, cdce_driver, cdce_devclass, usbd_driver_load, 0);
MODULE_VERSION(cdce, 0);

Static int	 cdce_encap(struct cdce_softc *, struct mbuf *, int);
Static void	 cdce_rxeof(usbd_xfer_handle, usbd_private_handle, usbd_status);
Static void	 cdce_txeof(usbd_xfer_handle, usbd_private_handle, usbd_status);
Static void	 cdce_start(struct ifnet *);
Static int	 cdce_ioctl(struct ifnet *, u_long, caddr_t);
Static void	 cdce_init(void *);
Static void	 cdce_reset(struct cdce_softc *);
Static void	 cdce_stop(struct cdce_softc *);
Static void	 cdce_rxstart(struct ifnet *);

Static const struct cdce_type cdce_devs[] = {
  {{ USB_VENDOR_PROLIFIC, USB_PRODUCT_PROLIFIC_PL2501 }, CDCE_NO_UNION },
  {{ USB_VENDOR_SHARP, USB_PRODUCT_SHARP_SL5500 }, CDCE_ZAURUS },
  {{ USB_VENDOR_SHARP, USB_PRODUCT_SHARP_SLA300 }, CDCE_ZAURUS | CDCE_NO_UNION },
  {{ USB_VENDOR_SHARP, USB_PRODUCT_SHARP_SL5600 }, CDCE_ZAURUS | CDCE_NO_UNION },
  {{ USB_VENDOR_SHARP, USB_PRODUCT_SHARP_SLC700 }, CDCE_ZAURUS | CDCE_NO_UNION },
  {{ USB_VENDOR_SHARP, USB_PRODUCT_SHARP_SLC750 }, CDCE_ZAURUS | CDCE_NO_UNION },
  {{ USB_VENDOR_GMATE, USB_PRODUCT_GMATE_YP3X00 }, CDCE_NO_UNION },
  {{ USB_VENDOR_NETCHIP, USB_PRODUCT_NETCHIP_ETHERNETGADGET }, CDCE_NO_UNION },
};
#define cdce_lookup(v, p) ((const struct cdce_type *)usb_lookup(cdce_devs, v, p))

USB_MATCH(cdce)
{
	USB_MATCH_START(cdce, uaa);
	usb_interface_descriptor_t *id;

	if (uaa->iface == NULL)
		return (UMATCH_NONE);

	id = usbd_get_interface_descriptor(uaa->iface);
	if (id == NULL)
		return (UMATCH_NONE);

	if (cdce_lookup(uaa->vendor, uaa->product) != NULL)
		return (UMATCH_VENDOR_PRODUCT);

	if (id->bInterfaceClass == UICLASS_CDC && id->bInterfaceSubClass ==
	    UISUBCLASS_ETHERNET_NETWORKING_CONTROL_MODEL)
		return (UMATCH_IFACECLASS_GENERIC);

	return (UMATCH_NONE);
}

USB_ATTACH(cdce)
{
	USB_ATTACH_START(cdce, sc, uaa);
	struct ifnet			*ifp;
	usbd_device_handle		 dev = uaa->device;
	const struct cdce_type		*t;
	usb_interface_descriptor_t	*id;
	usb_endpoint_descriptor_t	*ed;
	const usb_cdc_union_descriptor_t *ud;
	usb_config_descriptor_t		*cd;
	int				 data_ifcno;
	int				 i, j, numalts;
	u_char				 eaddr[ETHER_ADDR_LEN];
	const usb_cdc_ethernet_descriptor_t *ue;
	char				 eaddr_str[USB_MAX_STRING_LEN];

	bzero(sc, sizeof(struct cdce_softc));
	sc->cdce_dev = self;
	usbd_devinfo(dev, 0, sc->devinfo);
	device_set_desc_copy(self, sc->devinfo);
	printf("%s: %s\n", USBDEVNAME(sc->cdce_dev), sc->devinfo);

	sc->cdce_udev = uaa->device;
	sc->cdce_unit = device_get_unit(self);

	t = cdce_lookup(uaa->vendor, uaa->product);
	if (t)
		sc->cdce_flags = t->cdce_flags;

	if (sc->cdce_flags & CDCE_NO_UNION)
		sc->cdce_data_iface = uaa->iface;
	else {
		ud = (const usb_cdc_union_descriptor_t *)usb_find_desc(sc->cdce_udev,
		    UDESC_CS_INTERFACE, UDESCSUB_CDC_UNION);
		if (ud == NULL) {
			printf("%s: no union descriptor\n",
			    USBDEVNAME(sc->cdce_dev));
			USB_ATTACH_ERROR_RETURN;
		}
		data_ifcno = ud->bSlaveInterface[0];

		for (i = 0; i < uaa->nifaces; i++) {
			if (uaa->ifaces[i] != NULL) {
				id = usbd_get_interface_descriptor(
				    uaa->ifaces[i]);
				if (id != NULL && id->bInterfaceNumber ==
				    data_ifcno) {
					sc->cdce_data_iface = uaa->ifaces[i];
					uaa->ifaces[i] = NULL;
				}
			}
		}
	}

	if (sc->cdce_data_iface == NULL) {
		printf("%s: no data interface\n", USBDEVNAME(sc->cdce_dev));
		USB_ATTACH_ERROR_RETURN;
	}

	/*
	 * <quote>
	 *  The Data Class interface of a networking device shall have a minimum
	 *  of two interface settings. The first setting (the default interface
	 *  setting) includes no endpoints and therefore no networking traffic is
	 *  exchanged whenever the default interface setting is selected. One or
	 *  more additional interface settings are used for normal operation, and
	 *  therefore each includes a pair of endpoints (one IN, and one OUT) to
	 *  exchange network traffic. Select an alternate interface setting to
	 *  initialize the network aspects of the device and to enable the
	 *  exchange of network traffic.
	 * </quote>
	 *
	 * Some devices, most notably cable modems, include interface settings
	 * that have no IN or OUT endpoint, therefore loop through the list of all
	 * available interface settings looking for one with both IN and OUT
	 * endpoints.
	 */
	id = usbd_get_interface_descriptor(sc->cdce_data_iface);
	cd = usbd_get_config_descriptor(sc->cdce_udev);
	numalts = usbd_get_no_alts(cd, id->bInterfaceNumber);

	for (j = 0; j < numalts; j++) {
		if (usbd_set_interface(sc->cdce_data_iface, j)) {
			printf("%s: setting alternate interface failed\n",
			    USBDEVNAME(sc->cdce_dev));
			USB_ATTACH_ERROR_RETURN;
		}
		/* Find endpoints. */
		id = usbd_get_interface_descriptor(sc->cdce_data_iface);
		sc->cdce_bulkin_no = sc->cdce_bulkout_no = -1;
		for (i = 0; i < id->bNumEndpoints; i++) {
			ed = usbd_interface2endpoint_descriptor(sc->cdce_data_iface, i);
			if (!ed) {
				printf("%s: could not read endpoint descriptor\n",
				    USBDEVNAME(sc->cdce_dev));
				USB_ATTACH_ERROR_RETURN;
			}
			if (UE_GET_DIR(ed->bEndpointAddress) == UE_DIR_IN &&
			    UE_GET_XFERTYPE(ed->bmAttributes) == UE_BULK) {
				sc->cdce_bulkin_no = ed->bEndpointAddress;
			} else if (UE_GET_DIR(ed->bEndpointAddress) == UE_DIR_OUT &&
			    UE_GET_XFERTYPE(ed->bmAttributes) == UE_BULK) {
				sc->cdce_bulkout_no = ed->bEndpointAddress;
			} else if (UE_GET_DIR(ed->bEndpointAddress) == UE_DIR_IN &&
			    UE_GET_XFERTYPE(ed->bmAttributes) == UE_INTERRUPT) {
				/* XXX: CDC spec defines an interrupt pipe, but it is not
				 * needed for simple host-to-host applications. */
			} else {
				printf("%s: unexpected endpoint\n",
				    USBDEVNAME(sc->cdce_dev));
			}
		}
		/* If we found something, try and use it... */
		if ((sc->cdce_bulkin_no != -1) && (sc->cdce_bulkout_no != -1))
			break;
	}

	if (sc->cdce_bulkin_no == -1) {
		printf("%s: could not find data bulk in\n",
		    USBDEVNAME(sc->cdce_dev));
		USB_ATTACH_ERROR_RETURN;
	}
	if (sc->cdce_bulkout_no == -1 ) {
		printf("%s: could not find data bulk out\n",
		    USBDEVNAME(sc->cdce_dev));
		USB_ATTACH_ERROR_RETURN;
	}

	mtx_init(&sc->cdce_mtx, USBDEVNAME(sc->cdce_dev), MTX_NETWORK_LOCK,
	    MTX_DEF | MTX_RECURSE);
	CDCE_LOCK(sc);

	ue = (const usb_cdc_ethernet_descriptor_t *)usb_find_desc(dev,
	    UDESC_INTERFACE, UDESCSUB_CDC_ENF);
	if (!ue || usbd_get_string(dev, ue->iMacAddress, eaddr_str)) {
		/* Fake MAC address */
		printf("%s: faking MAC address\n", USBDEVNAME(sc->cdce_dev));
		eaddr[0]= 0x2a;
		memcpy(&eaddr[1], &ticks, sizeof(u_int32_t));
		eaddr[5] = (u_int8_t)(sc->cdce_unit);
	} else {
		int i;

		memset(eaddr, 0, ETHER_ADDR_LEN);
		for (i = 0; i < ETHER_ADDR_LEN * 2; i++) {
			int c = eaddr_str[i];

			if ('0' <= c && c <= '9')
				c -= '0';
			else
				c -= 'A' - 10;
			c &= 0xf;
			if (c % 2 == 0)
				c <<= 4;
			eaddr[i / 2] |= c;
		}
	}

	bcopy(eaddr, (char *)&sc->arpcom.ac_enaddr, ETHER_ADDR_LEN);

	ifp = GET_IFP(sc);
	ifp->if_softc = sc;
	if_initname(ifp, "cdce", sc->cdce_unit);
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST |
		IFF_NEEDSGIANT;
	ifp->if_ioctl = cdce_ioctl;
	ifp->if_output = ether_output;
	ifp->if_start = cdce_start;
	ifp->if_init = cdce_init;
	ifp->if_baudrate = 11000000;
	ifp->if_snd.ifq_maxlen = IFQ_MAXLEN;

	sc->q.ifp = ifp;
	sc->q.if_rxstart = cdce_rxstart;

	ether_ifattach(ifp, eaddr);
	usb_register_netisr();

	CDCE_UNLOCK(sc);

	usbd_add_drv_event(USB_EVENT_DRIVER_ATTACH, sc->cdce_udev,
	    USBDEV(sc->cdce_dev));

	USB_ATTACH_SUCCESS_RETURN;
}

USB_DETACH(cdce)
{
	USB_DETACH_START(cdce, sc);
	struct ifnet	*ifp;

	CDCE_LOCK(sc);
	sc->cdce_dying = 1;
	ifp = GET_IFP(sc);
	if (ifp->if_flags & IFF_RUNNING)
		cdce_shutdown(sc->cdce_dev);

	ether_ifdetach(ifp);
	CDCE_UNLOCK(sc);
	mtx_destroy(&sc->cdce_mtx);

	return (0);
}

Static void
cdce_start(struct ifnet *ifp)
{
	struct cdce_softc	*sc;
	struct mbuf		*m_head = NULL;

	sc = ifp->if_softc;
	CDCE_LOCK(sc);


	if (sc->cdce_dying ||
		ifp->if_flags & IFF_OACTIVE ||
		!(ifp->if_flags & IFF_RUNNING)) {
		CDCE_UNLOCK(sc);
		return;
	}

	IF_DEQUEUE(&ifp->if_snd, m_head);
	if (m_head == NULL) {
		CDCE_UNLOCK(sc);
		return;
	}

	if (cdce_encap(sc, m_head, 0)) {
		IF_PREPEND(&ifp->if_snd, m_head);
		ifp->if_flags |= IFF_OACTIVE;
		CDCE_UNLOCK(sc);
		return;
	}

	BPF_MTAP(ifp, m_head);

	ifp->if_flags |= IFF_OACTIVE;

	CDCE_UNLOCK(sc);

	return;
}

Static int
cdce_encap(struct cdce_softc *sc, struct mbuf *m, int idx)
{
	struct ue_chain	*c;
	usbd_status		 err;
	int			 extra = 0;

	c = &sc->cdce_cdata.ue_tx_chain[idx];

	m_copydata(m, 0, m->m_pkthdr.len, c->ue_buf);
	if (sc->cdce_flags & CDCE_ZAURUS) {
		/* Zaurus wants a 32-bit CRC appended to every frame */
		u_int32_t crc;

		crc = htole32(crc32(c->ue_buf, m->m_pkthdr.len));
		bcopy(&crc, c->ue_buf + m->m_pkthdr.len, 4);
		extra = 4;
	}
	c->ue_mbuf = m;

	usbd_setup_xfer(c->ue_xfer, sc->cdce_bulkout_pipe, c, c->ue_buf,
	    m->m_pkthdr.len + extra, 0, 10000, cdce_txeof);
	err = usbd_transfer(c->ue_xfer);
	if (err != USBD_IN_PROGRESS) {
		cdce_stop(sc);
		return (EIO);
	}

	sc->cdce_cdata.ue_tx_cnt++;

	return (0);
}

Static void
cdce_stop(struct cdce_softc *sc)
{
	usbd_status	 err;
	struct ifnet	*ifp;

	CDCE_LOCK(sc);

	cdce_reset(sc);

	ifp = GET_IFP(sc);
	ifp->if_timer = 0;

	if (sc->cdce_bulkin_pipe != NULL) {
		err = usbd_abort_pipe(sc->cdce_bulkin_pipe);
		if (err)
			printf("%s: abort rx pipe failed: %s\n",
			    USBDEVNAME(sc->cdce_dev), usbd_errstr(err));
		err = usbd_close_pipe(sc->cdce_bulkin_pipe);
		if (err)
			printf("%s: close rx pipe failed: %s\n",
			    USBDEVNAME(sc->cdce_dev), usbd_errstr(err));
		sc->cdce_bulkin_pipe = NULL;
	}

	if (sc->cdce_bulkout_pipe != NULL) {
		err = usbd_abort_pipe(sc->cdce_bulkout_pipe);
		if (err)
			printf("%s: abort tx pipe failed: %s\n",
			    USBDEVNAME(sc->cdce_dev), usbd_errstr(err));
		err = usbd_close_pipe(sc->cdce_bulkout_pipe);
		if (err)
			printf("%s: close tx pipe failed: %s\n",
			    USBDEVNAME(sc->cdce_dev), usbd_errstr(err));
		sc->cdce_bulkout_pipe = NULL;
	}

	usb_ether_rx_list_free(&sc->cdce_cdata);
	usb_ether_tx_list_free(&sc->cdce_cdata);

	ifp->if_flags &= ~(IFF_RUNNING | IFF_OACTIVE);
	CDCE_UNLOCK(sc);

	return;
}

Static int
cdce_shutdown(device_ptr_t dev)
{
	struct cdce_softc *sc;

	sc = device_get_softc(dev);
	cdce_stop(sc);

	return (0);
}

Static int
cdce_ioctl(struct ifnet *ifp, u_long command, caddr_t data)
{
	struct cdce_softc	*sc = ifp->if_softc;
	int			 error = 0;

	if (sc->cdce_dying)
		return (ENXIO);

	switch(command) {
	case SIOCSIFFLAGS:
		if (ifp->if_flags & IFF_UP) {
			if (!(ifp->if_flags & IFF_RUNNING))
				cdce_init(sc);
		} else {
			if (ifp->if_flags & IFF_RUNNING)
				cdce_stop(sc);
		}
		error = 0;
		break;

	default:
		error = ether_ioctl(ifp, command, data);
		break;
	}

	return (error);
}

Static void
cdce_reset(struct cdce_softc *sc)
{
	/* XXX Maybe reset the bulk pipes here? */
	return;
}

Static void
cdce_init(void *xsc)
{
	struct cdce_softc	*sc = xsc;
	struct ifnet		*ifp = GET_IFP(sc);
	struct ue_chain	*c;
	usbd_status		 err;
	int			 i;

	if (ifp->if_flags & IFF_RUNNING)
		return;

	CDCE_LOCK(sc);
	cdce_reset(sc);

	if (usb_ether_tx_list_init(sc, &sc->cdce_cdata,
	    sc->cdce_udev) == ENOBUFS) {
		printf("%s: tx list init failed\n", USBDEVNAME(sc->cdce_dev));
		CDCE_UNLOCK(sc);
		return;
	}

	if (usb_ether_rx_list_init(sc, &sc->cdce_cdata,
	    sc->cdce_udev) == ENOBUFS) {
		printf("%s: rx list init failed\n", USBDEVNAME(sc->cdce_dev));
		CDCE_UNLOCK(sc);
		return;
	}

	/* Maybe set multicast / broadcast here??? */

	err = usbd_open_pipe(sc->cdce_data_iface, sc->cdce_bulkin_no,
	    USBD_EXCLUSIVE_USE, &sc->cdce_bulkin_pipe);
	if (err) {
		printf("%s: open rx pipe failed: %s\n", USBDEVNAME(sc->cdce_dev),
		    usbd_errstr(err));
		CDCE_UNLOCK(sc);
		return;
	}

	err = usbd_open_pipe(sc->cdce_data_iface, sc->cdce_bulkout_no,
	    USBD_EXCLUSIVE_USE, &sc->cdce_bulkout_pipe);
	if (err) {
		printf("%s: open tx pipe failed: %s\n", USBDEVNAME(sc->cdce_dev),
		    usbd_errstr(err));
		CDCE_UNLOCK(sc);
		return;
	}

	for (i = 0; i < UE_RX_LIST_CNT; i++) {
		c = &sc->cdce_cdata.ue_rx_chain[i];
		usbd_setup_xfer(c->ue_xfer, sc->cdce_bulkin_pipe, c,
		    mtod(c->ue_mbuf, char *), UE_BUFSZ, USBD_SHORT_XFER_OK,
		    USBD_NO_TIMEOUT, cdce_rxeof);
		usbd_transfer(c->ue_xfer);
	}

	ifp->if_flags |= IFF_RUNNING;
	ifp->if_flags &= ~IFF_OACTIVE;

	CDCE_UNLOCK(sc);

	return;
}

Static void
cdce_rxeof(usbd_xfer_handle xfer, usbd_private_handle priv, usbd_status status)
{
	struct ue_chain	*c = priv;
	struct cdce_softc	*sc = c->ue_sc;
	struct ifnet		*ifp;
	struct mbuf		*m;
	int			 total_len = 0;

	CDCE_LOCK(sc);
	ifp = GET_IFP(sc);

	if (sc->cdce_dying || !(ifp->if_flags & IFF_RUNNING)) {
		CDCE_UNLOCK(sc);
		return;
	}

	if (status != USBD_NORMAL_COMPLETION) {
		if (status == USBD_NOT_STARTED || status == USBD_CANCELLED) {
			CDCE_UNLOCK(sc);
			return;
		}
		if (sc->cdce_rxeof_errors == 0)
			printf("%s: usb error on rx: %s\n",
			    USBDEVNAME(sc->cdce_dev), usbd_errstr(status));
		if (status == USBD_STALLED)
			usbd_clear_endpoint_stall(sc->cdce_bulkin_pipe);
		DELAY(sc->cdce_rxeof_errors * 10000);
		sc->cdce_rxeof_errors++;
		goto done;
	}

	sc->cdce_rxeof_errors = 0;

	usbd_get_xfer_status(xfer, NULL, NULL, &total_len, NULL);

	if (sc->cdce_flags & CDCE_ZAURUS)
		total_len -= 4;	/* Strip off CRC added by Zaurus */

	m = c->ue_mbuf;

	if (total_len < sizeof(struct ether_header)) {
		ifp->if_ierrors++;
		goto done;
	}

	ifp->if_ipackets++;
	m->m_pkthdr.rcvif = (struct ifnet *)&sc->q;
	m->m_pkthdr.len = m->m_len = total_len;

	/* Put the packet on the special USB input queue. */
	usb_ether_input(m);
	CDCE_UNLOCK(sc);

	return;

done:
	/* Setup new transfer. */
	usbd_setup_xfer(c->ue_xfer, sc->cdce_bulkin_pipe, c,
	    mtod(c->ue_mbuf, char *),
	    UE_BUFSZ, USBD_SHORT_XFER_OK, USBD_NO_TIMEOUT,
	    cdce_rxeof);
	usbd_transfer(c->ue_xfer);
	CDCE_UNLOCK(sc);

	return;
}

Static void
cdce_txeof(usbd_xfer_handle xfer, usbd_private_handle priv, usbd_status status)
{
	struct ue_chain	*c = priv;
	struct cdce_softc	*sc = c->ue_sc;
	struct ifnet		*ifp;
	usbd_status		 err;

	CDCE_LOCK(sc);
	ifp = GET_IFP(sc);

	if (sc->cdce_dying ||
		!(ifp->if_flags & IFF_RUNNING)) {
		CDCE_UNLOCK(sc);
		return;
	}

	if (status != USBD_NORMAL_COMPLETION) {
		if (status == USBD_NOT_STARTED || status == USBD_CANCELLED) {
			CDCE_UNLOCK(sc);
			return;
		}
		ifp->if_oerrors++;
		printf("%s: usb error on tx: %s\n", USBDEVNAME(sc->cdce_dev),
		    usbd_errstr(status));
		if (status == USBD_STALLED)
			usbd_clear_endpoint_stall(sc->cdce_bulkout_pipe);
		CDCE_UNLOCK(sc);
		return;
	}

	ifp->if_flags &= ~IFF_OACTIVE;
	usbd_get_xfer_status(c->ue_xfer, NULL, NULL, NULL, &err);

	if (c->ue_mbuf != NULL) {
		c->ue_mbuf->m_pkthdr.rcvif = ifp;
		usb_tx_done(c->ue_mbuf);
		c->ue_mbuf = NULL;
	}

	if (err)
		ifp->if_oerrors++;
	else
		ifp->if_opackets++;

	CDCE_UNLOCK(sc);

	return;
}

Static void
cdce_rxstart(struct ifnet *ifp)
{
	struct cdce_softc   *sc;
	struct ue_chain   *c;

	sc = ifp->if_softc;
	CDCE_LOCK(sc);

	if (sc->cdce_dying || !(ifp->if_flags & IFF_RUNNING)) {
		CDCE_UNLOCK(sc);
		return;
	}

	c = &sc->cdce_cdata.ue_rx_chain[sc->cdce_cdata.ue_rx_prod];

	c->ue_mbuf = usb_ether_newbuf();
	if (c->ue_mbuf == NULL) {
		printf("%s: no memory for rx list "
		    "-- packet dropped!\n", USBDEVNAME(sc->cdce_dev));
		ifp->if_ierrors++;
		CDCE_UNLOCK(sc);
		return;
	}

	usbd_setup_xfer(c->ue_xfer, sc->cdce_bulkin_pipe, c,
	    mtod(c->ue_mbuf, char *), UE_BUFSZ, USBD_SHORT_XFER_OK,
	    USBD_NO_TIMEOUT, cdce_rxeof);
	usbd_transfer(c->ue_xfer);

	CDCE_UNLOCK(sc);
	return;
}
