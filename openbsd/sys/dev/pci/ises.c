/*	$OpenBSD: ises.c,v 1.4 2001/03/28 20:02:59 angelos Exp $	*/

/*
 * Copyright (c) 2000 H�kan Olsson (ho@crt.se)
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * PCC-ISES hardware crypto accelerator
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/timeout.h>
#include <vm/vm.h>
#include <vm/vm_extern.h>
#include <vm/pmap.h>
#include <machine/pmap.h>
#include <sys/device.h>
#include <sys/queue.h>

#include <crypto/crypto.h>
#include <crypto/cryptosoft.h>
#include <dev/rndvar.h>
#include <sys/md5k.h>
#include <crypto/sha1.h>
#include <crypto/rmd160.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include <dev/pci/isesreg.h>
#include <dev/pci/isesvar.h>

/*
 * Prototypes and count for the pci_device structure
 */
int	ises_match __P((struct device *, void *, void *));
void	ises_attach __P((struct device *, struct device *, void *));

void	ises_initstate __P((void *));
void	ises_hrng_init __P((struct ises_softc *));
void	ises_hrng __P((void *));
void	ises_process_oqueue __P((struct ises_softc *));
int	ises_queue_cmd __P((struct ises_softc *, u_int32_t, u_int32_t *));
u_int32_t ises_get_fwversion __P((struct ises_softc *));
int	ises_assert_cmd_mode __P((struct ises_softc *));

int	ises_intr __P((void *));
int	ises_newsession __P((u_int32_t *, struct cryptoini *));
int	ises_freesession __P((u_int64_t));
int	ises_process __P((struct cryptop *));
void	ises_callback __P((struct ises_q *));
int	ises_feed __P((struct ises_softc *));

/* XXX for now... */
void	ubsec_mcopy __P((struct mbuf *, struct mbuf *, int, int));

#define READ_REG(sc,r) \
    bus_space_read_4((sc)->sc_memt, (sc)->sc_memh,r)

#define WRITE_REG(sc,reg,val) \
    bus_space_write_4((sc)->sc_memt, (sc)->sc_memh, reg, val)

#define SWAP32(x) ((x) = swap32((x)))

#ifdef ISESDEBUG
#  define DPRINTF(x) printf x
#else
#  define DPRINTF(x)
#endif

/* For HRNG entropy collection, these values gather 1600 bytes/s */
#ifndef ISESRNGBITS
#define ISESRNGBITS	128		/* Bits per iteration (mult. of 32) */
#define ISESRNGIPS	100		/* Iterations per second */
#endif

struct cfattach ises_ca = {
	sizeof(struct ises_softc), ises_match, ises_attach,
};

struct cfdriver ises_cd = {
	0, "ises", DV_DULL
};

int
ises_match(struct device *parent, void *match, void *aux)
{
	struct pci_attach_args *pa = (struct pci_attach_args *)aux;

	if (PCI_VENDOR(pa->pa_id) == PCI_VENDOR_PIJNENBURG &&
	    PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_PIJNENBURG_PCC_ISES)
		return (1);

	return (0);
}

void
ises_attach(struct device *parent, struct device *self, void *aux)
{
	struct ises_softc *sc = (struct ises_softc *)self;
	struct pci_attach_args *pa = aux;
	pci_chipset_tag_t pc = pa->pa_pc;
	pci_intr_handle_t ih;
	const char *intrstr = NULL;
	bus_addr_t membase;
	bus_size_t memsize;
	u_int32_t cmd;

	bus_dma_segment_t seg;
	int nsegs, error, state;

	SIMPLEQ_INIT(&sc->sc_queue);
	SIMPLEQ_INIT(&sc->sc_qchip);
	state = 0;

	/* Verify PCI space */
	cmd = pci_conf_read(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG);
	cmd |= PCI_COMMAND_MEM_ENABLE | PCI_COMMAND_MASTER_ENABLE;
	pci_conf_write(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG, cmd);
	cmd = pci_conf_read(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG);

	if (!(cmd & PCI_COMMAND_MEM_ENABLE)) {
		printf(": failed to enable memory mapping\n");
		return;
	}

	if (!(cmd & PCI_COMMAND_MASTER_ENABLE)) {
		printf(": failed to enable bus mastering\n");
		return;
	}

	/* Map control/status registers. */
	if (pci_mapreg_map(pa, PCI_MAPREG_START,
	    PCI_MAPREG_TYPE_MEM | PCI_MAPREG_MEM_TYPE_32BIT, 0, &sc->sc_memt,
	    &sc->sc_memh, &membase, &memsize)) {
		printf(": can't find mem space\n");
		return;
	}
	state++;

	/* Map interrupt. */
	if (pci_intr_map(pc, pa->pa_intrtag, pa->pa_intrpin, pa->pa_intrline,
	    &ih)) {
		printf(": couldn't map interrupt\n");
		goto fail;
	}
	state++;

	intrstr = pci_intr_string(pc, ih);
	sc->sc_ih = pci_intr_establish(pc, ih, IPL_NET, ises_intr, sc,
	    self->dv_xname);
	if (sc->sc_ih == NULL) {
		printf(": couldn't establish interrupt\n");
		if (intrstr != NULL)
			printf(" at %s", intrstr);
		printf("\n");
		goto fail;
	}

	/* Initialize DMA map */
	sc->sc_dmat = pa->pa_dmat;
	error = bus_dmamap_create(sc->sc_dmat, 1 << PGSHIFT, 1, 1 << PGSHIFT,
	    0, BUS_DMA_NOWAIT | BUS_DMA_ALLOCNOW, &sc->sc_dmamap_xfer);
	if (error) {
		printf(": cannot create dma map (%d)\n", error);
		goto fail;
	}
	state++;

	/* Allocate in DMAable memory. */
	if (bus_dmamem_alloc(sc->sc_dmat, sizeof sc->sc_dmamap, 1, 0, &seg, 1,
	    &nsegs, BUS_DMA_NOWAIT)) {
		printf(": can't alloc dma buffer space\n");
		goto fail;
	}
	state++;

	sc->sc_dmamap_phys = seg.ds_addr;
	if (bus_dmamem_map(sc->sc_dmat, &seg, nsegs, sizeof sc->sc_dmamap,
	    (caddr_t *)&sc->sc_dmamap, 0)) {
		printf(": can't map dma buffer space\n");
		goto fail;
	}
	state++;

	printf(": %s\n", intrstr);

	sc->sc_cid = crypto_get_driverid();

	if (sc->sc_cid < 0)
		goto fail;

	/*
	 * Since none of the initialization steps generate interrupts
	 * for example, the hardware reset, we use a number of timeouts
	 * (or init states) to do the rest of the chip initialization.
	 */

	sc->sc_initstate = 0;
	timeout_set(&sc->sc_timeout, ises_initstate, sc);
	ises_initstate(sc);

	return;

 fail:
	switch (state) { /* Always fallthrough here. */
	case 4:
		bus_dmamem_unmap(sc->sc_dmat, (caddr_t)&sc->sc_dmamap,
		    sizeof sc->sc_dmamap);
	case 3:
		bus_dmamem_free(sc->sc_dmat, &seg, nsegs);
	case 2:
		bus_dmamap_destroy(sc->sc_dmat, sc->sc_dmamap_xfer);
	case 1:
		pci_intr_disestablish(pc, sc->sc_ih);
	default: /* 0 */
		bus_space_unmap(sc->sc_memt, sc->sc_memh, memsize);
	}
	return;
}

void
ises_initstate(void *v)
{
	/*
	 * Step through chip initialization.
	 * sc->sc_initstate tells us what to do.
	 */
	extern int hz;
	struct ises_softc *sc = v;
	char *dv = sc->sc_dv.dv_xname;
	u_int32_t stat;
	int p, ticks;

	ticks = hz; 

#if 0 /* Too noisy */
	DPRINTF (("%s: entered initstate %d\n", dv, sc->sc_initstate));
#endif

	switch (sc->sc_initstate) {
	case 0:
		/* Power up the chip (clear powerdown bit) */
		stat = READ_REG(sc, ISES_BO_STAT);
		if (stat & ISES_BO_STAT_POWERDOWN) {
			stat &= ~ISES_BO_STAT_POWERDOWN;
			WRITE_REG(sc, ISES_BO_STAT, stat);
			/* Selftests will take 1 second. */
			break;
		}
		/* FALLTHROUGH (chip is already powered up) */
		sc->sc_initstate++;

	case 1:
		/* Perform a hardware reset */
		stat = 0;

		printf ("%s: initializing...\n", dv);

		/* Clear all possible bypass bits. */
		for (p = 0; p < 128; p++)
			WRITE_REG(sc, ISES_B_BDATAOUT, 0L);

		stat |= ISES_BO_STAT_HWRESET;
		WRITE_REG(sc, ISES_BO_STAT, stat);
		stat &= ~ISES_BO_STAT_HWRESET;
		WRITE_REG(sc, ISES_BO_STAT, stat);
		/* Again, selftests will take 1 second. */
		break;

	case 2:
		/* Set AConf to zero, i.e 32-bits access to A-int. */
		stat = READ_REG(sc, ISES_BO_STAT);
		stat &= ~ISES_BO_STAT_ACONF;
		WRITE_REG(sc, ISES_BO_STAT, stat);

		/* Is the firmware already loaded? */
		if (READ_REG(sc, ISES_A_STAT) & ISES_STAT_HW_DA) {
			/* Yes it is, jump ahead a bit */
			ticks = 1;
			sc->sc_initstate += 4; /* Next step --> 7 */
			break;
		}

		/*
		 * Download the Basic Functionality firmware.
		 * Prior to downloading we need to reset the NSRAM.
		 * Setting the tamper bit will erase the contents
		 * in 1 microsecond.
		 */
		stat = READ_REG(sc, ISES_BO_STAT);
		stat |= ISES_BO_STAT_TAMPER;
		WRITE_REG(sc, ISES_BO_STAT, stat);
		ticks = 1;
		break;

	case 3:
		/* After tamper bit has been set, powerdown chip. */
		stat = READ_REG(sc, ISES_BO_STAT);
		stat |= ISES_BO_STAT_POWERDOWN;
		WRITE_REG(sc, ISES_BO_STAT, stat);
		/* Wait one second for power to dissipate. */
		break;

	case 4:
		/* Clear tamper and powerdown bits. */
		stat = READ_REG(sc, ISES_BO_STAT);
		stat &= ~(ISES_BO_STAT_TAMPER | ISES_BO_STAT_POWERDOWN);
		WRITE_REG(sc, ISES_BO_STAT, stat);
		/* Again, wait one second for selftests. */
		break;

	case 5:
		/*
		 * We'll need some space in the input queue (IQF)
		 * and we need to be in the 'waiting for program
		 * length' IDP state (0x4).
		 */
		p = ISES_STAT_IDP_STATE(READ_REG(sc, ISES_A_STAT));
		if (READ_REG(sc, ISES_A_IQF) < 4 || p != 0x4) {
			printf("%s: cannot download firmware, "
			    "IDP state is \"%s\"\n", dv, ises_idp_state[p]);
			return;
		}

		/* Write firmware length */
		WRITE_REG(sc, ISES_A_IQD, ISES_BF_IDPLEN);

		/* Write firmware code */
		for (p = 0; p < sizeof(ises_bf_fw)/sizeof(u_int32_t); p++) {
			WRITE_REG(sc, ISES_A_IQD, ises_bf_fw[p]);
			if (READ_REG(sc, ISES_A_IQF) < 4)
				DELAY(10);
		}

		/* Write firmware CRC */
		WRITE_REG(sc, ISES_A_IQD, ISES_BF_IDPCRC);

		/* Wait 1s while chip resets and runs selftests */
		break;

	case 6:
		/* Did the download succed? */
		if (READ_REG(sc, ISES_A_STAT) & ISES_STAT_HW_DA) {
			ticks = 1;
			break;
		}

		/* We failed. We cannot do anything else. */
		printf ("%s: firmware download failed\n", dv);
		return;

	case 7:
		if (ises_assert_cmd_mode(sc) < 0)
			goto fail;

		/*
		 * Now that the basic functionality firmware should be
		 * up and running, try to get the firmware version.
		 */

		stat = ises_get_fwversion(sc);
		if (stat == 0)
			goto fail;

		printf("%s: firmware v%d.%d loaded (%d bytes)", dv,
		    stat & 0xffff, (stat >> 16) & 0xffff, ISES_BF_IDPLEN << 2);

		/* We can use firmware version 1.x & 2.x */
		switch (stat & 0xffff) {
		case 0:
			printf(" diagnostic, %s disabled\n", dv);
			goto fail;
		case 1: /* Basic Func "base" firmware */
		case 2: /* Basic Func "ipsec" firmware, no ADP code */
			break;
		default:
			printf(" unknown, %s disabled\n", dv);
			goto fail;
		}

		stat = READ_REG(sc, ISES_A_STAT);
		DPRINTF((", mode %s",
		    ises_sw_mode[ISES_STAT_SW_MODE(stat)]));

		/* Reuse the timeout for HRNG entropy collection. */
		timeout_del(&sc->sc_timeout);
		ises_hrng_init(sc);

		/* Set the interrupt mask */
		sc->sc_intrmask = ISES_STAT_BCHU_OAF | ISES_STAT_BCHU_ERR |
		    ISES_STAT_BCHU_OFHF | ISES_STAT_SW_OQSINC;
#if 0
		    ISES_STAT_BCHU_ERR | ISES_STAT_BCHU_OAF |
		    ISES_STAT_BCHU_IFE | ISES_STAT_BCHU_IFHE |
		    ISES_STAT_BCHU_OFHF | ISES_STAT_BCHU_OFF;
#endif

		WRITE_REG(sc, ISES_A_INTE, sc->sc_intrmask);

		/* We're done. */
		printf("\n");

		/* Register ourselves with crypto framework. */
#ifdef notyet
		crypto_register(sc->sc_cid, CRYPTO_3DES_CBC,
		    ises_newsession, ises_freesession, ises_process);
		crypto_register(sc->sc_cid, CRYPTO_DES_CBC, NULL, NULL, NULL);
		crypto_register(sc->sc_cid, CRYPTO_MD5_HMAC, NULL, NULL, NULL);
		crypto_register(sc->sc_cid, CRYPTO_SHA1_HMAC, NULL, NULL,
		    NULL);
		crypto_register(sc->sc_cid, CRYPTO_RIPEMD160_HMAC, NULL, NULL,
		    NULL);
#endif

		return;

	default:
		printf("%s: entered unknown initstate %d\n", dv,
		    sc->sc_initstate);
		goto fail;
	}

	/* Increment state counter and schedule next step in 'ticks' ticks. */
	sc->sc_initstate++;
	timeout_add(&sc->sc_timeout, ticks);
	return;

 fail:
	printf("%s: firmware failure\n", dv);
	timeout_del(&sc->sc_timeout);
	return;
}

/* Put a command on the A-interface queue. */
int
ises_queue_cmd(struct ises_softc *sc, u_int32_t cmd, u_int32_t *data)
{
	int p, len, s;

	len = cmd >> 24;

	s = splimp();

	if (len > READ_REG(sc, ISES_A_IQF)) {
		splx(s);
		return (EAGAIN); /* XXX ENOMEM ? */
	}

	WRITE_REG(sc, ISES_A_IQD, cmd);

	for (p = 0; p < len; p++)
		WRITE_REG(sc, ISES_A_IQD, *(data + p));

	WRITE_REG(sc, ISES_A_IQS, 0);

	splx(s);
	return (0);
}

/* Process all completed responses in the output queue. */
void
ises_process_oqueue(struct ises_softc *sc)
{
#ifdef ISESDEBUG
	char *dv = sc->sc_dv.dv_xname;
#endif
	u_int32_t oqs, r, d;
	int cmd, len;

	/* OQS gives us the number of responses we have to process. */
	while ((oqs = READ_REG(sc, ISES_A_OQS)) > 0) {
		/* Read command response. [ len(8) | cmd(8) | rc(16) ] */
		r = READ_REG(sc, ISES_A_OQD);
		len = (r >> 24);
		cmd = (r >> 16) & 0xff;
		r   = r & 0xffff;

		if (r) {
			/* This command generated an error */
			DPRINTF(("%s: cmd %d error %d\n", dv, cmd,
			    (r & ISES_RC_MASK)));
		} else
			switch (cmd) {
			case ISES_CMD_HBITS:
				/* XXX How about increasing the pool size? */
				/* XXX Use add_entropy_words instead? */
				/* XXX ... at proper spl */
				/* Cmd generated by ises_rng() via timeouts */
				while (len--) {
					d = READ_REG(sc, ISES_A_OQD);
					add_true_randomness(d);
				}
				break;

			case ISES_CMD_BR_OMR:
				sc->sc_omr = READ_REG(sc, ISES_A_OQD);
				DPRINTF(("%s: read sc->sc_omr [%08x]\n", dv,
				    sc->sc_omr));
				break;

			default:
				/* All other are ok (no response data) */
			}

	/* This will drain any remaining data and ACK this reponse. */
	while (len-- > 0)
		d = READ_REG(sc, ISES_A_OQD);
	WRITE_REG(sc, ISES_A_OQS, 0);
	if (oqs > 1)
		DELAY(1); /* Wait for firmware to decrement OQS (8 clocks) */
	}
}

int
ises_intr(void *arg)
{
	struct ises_softc *sc = arg;
	volatile u_int32_t ints;

	ints = READ_REG(sc, ISES_A_INTS);
	if (!(ints & sc->sc_intrmask))
		return (0); /* Not our interrupt. */

	WRITE_REG(sc, ISES_A_INTS, ints); /* Clear all set intr bits. */

#if 0
	/* Check it we've got room for more data. */
	if (READ_REG(sc, ISES_A_STAT) &
	    (ISES_STAT_BCHU_IFE | ISES_STAT_BCHU_IFHE))
		ises_feed(sc);
#endif

	if (ints & ISES_STAT_SW_OQSINC) {	/* A-intf output q has data */
		ises_process_oqueue(sc);
	}

	if (ints & ISES_STAT_BCHU_OAF) {	/* output data available */
		DPRINTF(("ises_intr: BCHU_OAF bit set\n"));
		/* ises_process_oqueue(sc); */
	}

	if (ints & ISES_STAT_BCHU_ERR) {	/* We got a BCHU error */
		DPRINTF(("ises_intr: BCHU error\n"));
		/* XXX Error handling */
	}

	if (ints & ISES_STAT_BCHU_OFHF) {	/* Output is half full */
		DPRINTF(("ises_intr: BCHU output FIFO half full\n"));
		/* XXX drain data? */
	}

#if 0 /* XXX Useful? */
	if (ints & ISES_STAT_BCHU_OFF) {	/* Output is full */
		/* XXX drain data / error handling? */
	}
#endif
	return (1);
}

int
ises_feed(struct ises_softc *sc)
{
	struct ises_q *q;

	while (!SIMPLEQ_EMPTY(&sc->sc_queue)) {
		if (READ_REG(sc, ISES_A_STAT) & ISES_STAT_BCHU_IFF)
			break;
		q = SIMPLEQ_FIRST(&sc->sc_queue);
#if 0
		WRITE_REG(sc, ISES_OFFSET_BCHU_DATA,
		    (u_int32_t)vtophys(&q->q_mcr));
		printf("feed: q->chip %08x %08x\n", q,
		    (u_int32_t)vtophys(&q->q_mcr));
#endif
		SIMPLEQ_REMOVE_HEAD(&sc->sc_queue, q, q_next);
		--sc->sc_nqueue;
		SIMPLEQ_INSERT_TAIL(&sc->sc_qchip, q, q_next);
	}
	return (0);
}

/*
 * Allocate a new 'session' and return an encoded session id.  'sidp'
 * contains our registration id, and should contain an encoded session
 * id on successful allocation.
 */
int
ises_newsession(u_int32_t *sidp, struct cryptoini *cri)
{
	struct cryptoini *c, *mac = NULL, *enc = NULL;
	struct ises_softc *sc = NULL;
	struct ises_session *ses = NULL;
	MD5_CTX	   md5ctx;
	SHA1_CTX   sha1ctx;
	RMD160_CTX rmd160ctx;
	int i, sesn;

	if (sidp == NULL || cri == NULL)
		return (EINVAL);

	for (i = 0; i < ises_cd.cd_ndevs; i++) {
		sc = ises_cd.cd_devs[i];
		if (sc == NULL || sc->sc_cid == (*sidp))
			break;
	}
	if (sc == NULL)
		return (EINVAL);

	for (c = cri; c != NULL; c = c->cri_next) {
		if (c->cri_alg == CRYPTO_MD5_HMAC ||
		    c->cri_alg == CRYPTO_SHA1_HMAC ||
		    c->cri_alg == CRYPTO_RIPEMD160_HMAC) {
			if (mac)
				return (EINVAL);
			mac = c;
		} else if (c->cri_alg == CRYPTO_DES_CBC ||
		    c->cri_alg == CRYPTO_3DES_CBC) {
			if (enc)
				return (EINVAL);
			enc = c;
		} else
			return (EINVAL);
	}
	if (mac == 0 && enc == 0)
		return (EINVAL);

	if (sc->sc_sessions == NULL) {
		ses = sc->sc_sessions = (struct ises_session *)
		    malloc(sizeof(struct ises_session), M_DEVBUF, M_NOWAIT);
		if (ses == NULL)
			return (ENOMEM);
		sesn = 0;
		sc->sc_nsessions = 1;
	} else {
		for (sesn = 0; sesn < sc->sc_nsessions; sesn++)
			if (sc->sc_sessions[sesn].ses_used == 0) {
				ses = &sc->sc_sessions[sesn];
				break;
			}

		if (ses == NULL)  {
			i = sc->sc_nsessions * sizeof(struct ises_session);
			ses = (struct ises_session *)
			    malloc(i + sizeof(struct ises_session), M_DEVBUF,
			    M_NOWAIT);
			if (ses == NULL)
				return (ENOMEM);

			memcpy(ses, sc->sc_sessions, i);
			memset(sc->sc_sessions, 0, i);
			free(sc->sc_sessions, M_DEVBUF);
			sc->sc_sessions = ses;
			ses = &sc->sc_sessions[sc->sc_nsessions];
			sc->sc_nsessions++;
		}
	}

	memset(ses, 0, sizeof(struct ises_session));
	ses->ses_used = 1;

	if (enc) {
		/* get an IV, network byte order */
		/* XXX switch to using builtin HRNG ! */
		get_random_bytes(ses->ses_iv, sizeof(ses->ses_iv));

		/* crypto key */
		if (c->cri_alg == CRYPTO_DES_CBC) {
			memcpy(&ses->ses_deskey[0], enc->cri_key, 8);
			memcpy(&ses->ses_deskey[2], enc->cri_key, 8);
			memcpy(&ses->ses_deskey[4], enc->cri_key, 8);
		} else
			memcpy(&ses->ses_deskey[0], enc->cri_key, 24);

		SWAP32(ses->ses_deskey[0]);
		SWAP32(ses->ses_deskey[1]);
		SWAP32(ses->ses_deskey[2]);
		SWAP32(ses->ses_deskey[3]);
		SWAP32(ses->ses_deskey[4]);
		SWAP32(ses->ses_deskey[5]);
	}

	if (mac) {
		for (i = 0; i < mac->cri_klen / 8; i++)
			mac->cri_key[i] ^= HMAC_IPAD_VAL;

		if (mac->cri_alg == CRYPTO_MD5_HMAC) {
			MD5Init(&md5ctx);
			MD5Update(&md5ctx, mac->cri_key, mac->cri_klen / 8);
			MD5Update(&md5ctx, hmac_ipad_buffer, HMAC_BLOCK_LEN -
			    (mac->cri_klen / 8));
			memcpy(ses->ses_hminner, md5ctx.state,
			    sizeof(md5ctx.state));
		} else if (mac->cri_alg == CRYPTO_SHA1_HMAC) {
			SHA1Init(&sha1ctx);
			SHA1Update(&sha1ctx, mac->cri_key, mac->cri_klen / 8);
			SHA1Update(&sha1ctx, hmac_ipad_buffer, HMAC_BLOCK_LEN -
			    (mac->cri_klen / 8));
			memcpy(ses->ses_hminner, sha1ctx.state,
			    sizeof(sha1ctx.state));
		} else {
			RMD160Init(&rmd160ctx);
			RMD160Update(&rmd160ctx, mac->cri_key,
			    mac->cri_klen / 8);
			RMD160Update(&rmd160ctx, hmac_ipad_buffer,
			    HMAC_BLOCK_LEN - (mac->cri_klen / 8));
			memcpy(ses->ses_hminner, rmd160ctx.state,
			    sizeof(rmd160ctx.state));
		}

		for (i = 0; i < mac->cri_klen / 8; i++)
			mac->cri_key[i] ^= (HMAC_IPAD_VAL ^ HMAC_OPAD_VAL);

		if (mac->cri_alg == CRYPTO_MD5_HMAC) {
			MD5Init(&md5ctx);
			MD5Update(&md5ctx, mac->cri_key, mac->cri_klen / 8);
			MD5Update(&md5ctx, hmac_ipad_buffer, HMAC_BLOCK_LEN -
			    (mac->cri_klen / 8));
			memcpy(ses->ses_hmouter, md5ctx.state,
			    sizeof(md5ctx.state));
		} else if (mac->cri_alg == CRYPTO_SHA1_HMAC) {
			SHA1Init(&sha1ctx);
			SHA1Update(&sha1ctx, mac->cri_key, mac->cri_klen / 8);
			SHA1Update(&sha1ctx, hmac_ipad_buffer, HMAC_BLOCK_LEN -
			    (mac->cri_klen / 8));
			memcpy(ses->ses_hmouter, sha1ctx.state,
			    sizeof(sha1ctx.state));
		} else {
			RMD160Init(&rmd160ctx);
			RMD160Update(&rmd160ctx, mac->cri_key,
			    mac->cri_klen / 8);
			RMD160Update(&rmd160ctx, hmac_ipad_buffer,
			    HMAC_BLOCK_LEN - (mac->cri_klen / 8));
			memcpy(ses->ses_hmouter, rmd160ctx.state,
			    sizeof(rmd160ctx.state));
		}

		for (i = 0; i < mac->cri_klen / 8; i++)
			mac->cri_key[i] ^= HMAC_OPAD_VAL;
	}

	*sidp = ISES_SID(sc->sc_dv.dv_unit, sesn);
	return (0);
}

/*
 * Deallocate a session.
 */
int
ises_freesession(u_int64_t tsid)
{
	struct ises_softc *sc;
	int card, sesn;
	u_int32_t sid = ((u_int32_t) tsid) & 0xffffffff;

	card = ISES_CARD(sid);
	if (card >= ises_cd.cd_ndevs || ises_cd.cd_devs[card] == NULL)
		return (EINVAL);

	sc = ises_cd.cd_devs[card];
	sesn = ISES_SESSION(sid);
	memset(&sc->sc_sessions[sesn], 0, sizeof(sc->sc_sessions[sesn]));

	return (0);
}

int
ises_process(struct cryptop *crp)
{
	int card, err;
	struct ises_softc *sc;
	struct ises_q *q;
	struct cryptodesc *maccrd, *enccrd, *crd;
	struct ises_session *ses;
#if 0
	int s, i, j;
#else
	int s;
#endif
	int encoffset = 0, macoffset = 0;
	int sskip, stheend, dtheend, cpskip, cpoffset, dskip, nicealign;
	int16_t coffset;

	if (crp == NULL || crp->crp_callback == NULL)
		return (EINVAL);

	card = ISES_CARD(crp->crp_sid);
	if (card >= ises_cd.cd_ndevs || ises_cd.cd_devs[card] == NULL) {
		err = EINVAL;
		goto errout;
	}

	sc = ises_cd.cd_devs[card];

	s = splnet();
	if (sc->sc_nqueue == ISES_MAX_NQUEUE) {
		splx(s);
		err = ENOMEM;
		goto errout;
	}
	splx(s);

	q = (struct ises_q *)malloc(sizeof(struct ises_q), M_DEVBUF, M_NOWAIT);
	if (q == NULL) {
		err = ENOMEM;
		goto errout;
	}
	memset(q, 0, sizeof(struct ises_q));

	q->q_sesn = ISES_SESSION(crp->crp_sid);
	ses = &sc->sc_sessions[q->q_sesn];

	/* XXX */

	q->q_sc = sc;
	q->q_crp = crp;

	if (crp->crp_flags & CRYPTO_F_IMBUF) {
		q->q_src_m = (struct mbuf *)crp->crp_buf;
		q->q_dst_m = (struct mbuf *)crp->crp_buf;
	} else {
		err = EINVAL;
		goto errout;
	}

	/*
	 * Check if the crypto descriptors are sane. We accept:
	 * - just one crd; either auth or crypto
	 * - two crds; must be one auth and one crypto, although now
	 *   for encryption we only want the first to be crypto, while
	 *   for decryption the second one should be crypto.
	 */
	maccrd = enccrd = NULL;
	err = EINVAL;
	for (crd = crp->crp_desc; crd; crd = crd->crd_next) {
		switch (crd->crd_alg) {
		case CRYPTO_MD5_HMAC:
		case CRYPTO_SHA1_HMAC:
		case CRYPTO_RIPEMD160_HMAC:
			if (maccrd || (enccrd &&
			    (enccrd->crd_flags & CRD_F_ENCRYPT) == 0))
				goto errout;
			maccrd = crd;
			break;
		case CRYPTO_DES_CBC:
		case CRYPTO_3DES_CBC:
			if (enccrd ||
			    (maccrd && (crd->crd_flags & CRD_F_ENCRYPT)))
				goto errout;
			enccrd = crd;
			break;
		default:
			goto errout;
		}
	}
	if (!maccrd && !enccrd)
		goto errout;
	err = 0;

	if (enccrd) {
		encoffset = enccrd->crd_skip;

		if (enccrd->crd_alg == CRYPTO_3DES_CBC)
			q->q_ctx.pc_omrflags |= ISES_SOMR_BOMR_3DES;
		else
			q->q_ctx.pc_omrflags |= ISES_SOMR_BOMR_DES;
		q->q_ctx.pc_omrflags |= ISES_SOMR_FMR_CBC;

		if (enccrd->crd_flags & CRD_F_ENCRYPT) {
			q->q_ctx.pc_omrflags |= ISES_SOMR_EDR; /* XXX */

			if (enccrd->crd_flags & CRD_F_IV_EXPLICIT)
				bcopy(enccrd->crd_iv, q->q_ctx.pc_iv, 8);
			else {
				q->q_ctx.pc_iv[0] = ses->ses_iv[0];
				q->q_ctx.pc_iv[1] = ses->ses_iv[1];
			}

			if ((enccrd->crd_flags & CRD_F_IV_PRESENT) == 0)
				m_copyback(q->q_src_m, enccrd->crd_inject,
				    8, (caddr_t)q->q_ctx.pc_iv);
		} else {
			q->q_ctx.pc_omrflags &= ~ISES_SOMR_EDR; /* XXX */

			if (enccrd->crd_flags & CRD_F_IV_EXPLICIT)
				bcopy(enccrd->crd_iv, q->q_ctx.pc_iv, 8);
			else
				m_copyback(q->q_src_m, enccrd->crd_inject,
				    8, (caddr_t)q->q_ctx.pc_iv);
		}

		q->q_ctx.pc_deskey[0] = ses->ses_deskey[0];
		q->q_ctx.pc_deskey[1] = ses->ses_deskey[1];
		q->q_ctx.pc_deskey[2] = ses->ses_deskey[2];
		q->q_ctx.pc_deskey[3] = ses->ses_deskey[3];
		q->q_ctx.pc_deskey[4] = ses->ses_deskey[4];
		q->q_ctx.pc_deskey[5] = ses->ses_deskey[5];

		SWAP32(q->q_ctx.pc_iv[0]);
		SWAP32(q->q_ctx.pc_iv[1]);
	}

	if (maccrd) {
		macoffset = maccrd->crd_skip;

		switch (crd->crd_alg) {
		case CRYPTO_MD5_HMAC:
			q->q_ctx.pc_omrflags |= ISES_HOMR_HFR_MD5;
			break;
		case CRYPTO_SHA1_HMAC:
			q->q_ctx.pc_omrflags |= ISES_HOMR_HFR_SHA1;
			break;
		case CRYPTO_RIPEMD160_HMAC:
		default:
			q->q_ctx.pc_omrflags |= ISES_HOMR_HFR_RMD160;
			break;
		}

		q->q_ctx.pc_hminner[0] = ses->ses_hminner[0];
		q->q_ctx.pc_hminner[1] = ses->ses_hminner[1];
		q->q_ctx.pc_hminner[2] = ses->ses_hminner[2];
		q->q_ctx.pc_hminner[3] = ses->ses_hminner[3];
		q->q_ctx.pc_hminner[4] = ses->ses_hminner[4];
		q->q_ctx.pc_hminner[5] = ses->ses_hminner[5];

		q->q_ctx.pc_hmouter[0] = ses->ses_hmouter[0];
		q->q_ctx.pc_hmouter[1] = ses->ses_hmouter[1];
		q->q_ctx.pc_hmouter[2] = ses->ses_hmouter[2];
		q->q_ctx.pc_hmouter[3] = ses->ses_hmouter[3];
		q->q_ctx.pc_hmouter[4] = ses->ses_hmouter[4];
		q->q_ctx.pc_hmouter[5] = ses->ses_hmouter[5];
	}

	if (enccrd && maccrd) {
		/* XXX Check if ises handles differing end of auth/enc etc */
		/* XXX For now, assume not (same as ubsec). */
		if (((encoffset + enccrd->crd_len) !=
		    (macoffset + maccrd->crd_len)) ||
		    (enccrd->crd_skip < maccrd->crd_skip)) {
			err = EINVAL;
			goto errout;
		}

		sskip = maccrd->crd_skip;
		cpskip = dskip = enccrd->crd_skip;
		stheend = maccrd->crd_len;
		dtheend = enccrd->crd_len;
		coffset = cpskip - sskip;
		cpoffset = cpskip + dtheend;
		/* XXX DEBUG ? */
	} else {
		cpskip = dskip = sskip = macoffset + encoffset;
		dtheend = enccrd ? enccrd->crd_len : maccrd->crd_len;
		stheend = dtheend;
		cpoffset = cpskip + dtheend;
		coffset = 0;
	}
	q->q_ctx.pc_offset = coffset >> 2;

	q->q_src_l = mbuf2pages(q->q_src_m, &q->q_src_npa, &q->q_src_packp,
	    &q->q_src_packl, 1, &nicealign);
	if (q->q_src_l == 0) {
		err = ENOMEM;
		goto errout;
	}

	/* XXX mcr stuff; q->q_mcr->mcr_pktlen = stheend; */

#if 0 /* XXX */
	for (i = j = 0; i < q->q_src_npa; i++) {
		struct ises_pktbuf *pb;

		/* XXX DEBUG? */

		if (sskip) {
			if (sskip >= q->q_src_packl) {
				sskip -= q->q_src_packl;
				continue;
			}
			q->q_src_packp += sskip;
			q->q_src_packl -= sskip;
			sskip = 0;
		}

		pb = NULL; /* XXX initial packet */

		pb->pb_addr = q->q_src_packp;
		if (stheend) {
			if (q->q_src_packl > stheend) {
				pb->pb_len = stheend;
				stheend = 0;
			} else {
				pb->pb_len = q->q_src_packl;
				stheend -= pb->pb_len;
			}
		} else
			pb->pb_len = q->q_src_packl;

		if ((i + 1) == q->q_src_npa)
			pb->pb_next = 0;
		else
			pb->pb_next = vtophys(&q->q_srcpkt);

		j++;
	}

#endif /* XXX */
	/* XXX DEBUG ? */

	if (enccrd == NULL && maccrd != NULL) {
		/* XXX mcr stuff */
	} else {
		if (!nicealign) {
			int totlen, len;
			struct mbuf *m, *top, **mp;

			totlen = q->q_dst_l = q->q_src_l;
			if (q->q_src_m->m_flags & M_PKTHDR) {
				MGETHDR(m, M_DONTWAIT, MT_DATA);
				M_DUP_PKTHDR(m, q->q_src_m);
				len = MHLEN;
			} else {
				MGET(m, M_DONTWAIT, MT_DATA);
				len = MLEN;
			}
			if (m == NULL) {
				err = ENOMEM;
				goto errout;
			}
			if (totlen >= MINCLSIZE) {
				MCLGET(m, M_DONTWAIT);
				if (m->m_flags & M_EXT)
					len = MCLBYTES;
			}
			m->m_len = len;
			top = NULL;
			mp = &top;

			while (totlen > 0) {
				if (top) {
					MGET(m, M_DONTWAIT, MT_DATA);
					if (m == NULL) {
						m_freem(top);
						err = ENOMEM;
						goto errout;
					}
					len = MLEN;
				}
				if (top && totlen >= MINCLSIZE) {
					MCLGET(m, M_DONTWAIT);
					if (m->m_flags & M_EXT)
						len = MCLBYTES;
				}
				m->m_len = len = min(totlen, len);
				totlen -= len;
				*mp = m;

				mp = &m->m_next;
			}
			q->q_dst_m = top;
			ubsec_mcopy(q->q_src_m, q->q_dst_m, cpskip, cpoffset);
		} else
			q->q_dst_m = q->q_src_m;

		q->q_dst_l = mbuf2pages(q->q_dst_m, &q->q_dst_npa,
		    &q->q_dst_packp, &q->q_dst_packl, 1, NULL);

#if 0
		for (i = j = 0; i < q->q_dst_npa; i++) {
			struct ises_pktbuf *pb;

			if (dskip) {
				if (dskip >= q->q_dst_packl[i]) {
					dskip -= q->q_dst_packl[i];
					continue;
				}
				q->q_dst_packp[i] += dskip;
				q->q_dst_packl[i] -= dskip;
				dskip = 0;
			}

			if (j == 0)
				pb = NULL; /* &q->q_mcr->mcr_opktbuf; */
			else
				pb = &q->q_dstpkt[j - 1];

			pb->pb_addr = q->q_dst_packp[i];

			if (dtheend) {
				if (q->q_dst_packl[i] > dtheend) {
					pb->pb_len = dtheend;
					dtheend = 0;
				} else {
					pb->pb_len = q->q_dst_packl[i];
					dtheend -= pb->pb_len;
				}
			} else
				pb->pb_len = q->q_dst_packl[i];

			if ((i + 1) == q->q_dst_npa) {
				if (maccrd)
					pb->pb_next = vtophys(q->q_macbuf);
				else
					pb->pb_next = 0;
			} else
				pb->pb_next = vtophys(&q->q_dstpkt[j]);
			j++;
		}
#endif
	}

	s = splnet();
	SIMPLEQ_INSERT_TAIL(&sc->sc_queue, q, q_next);
	sc->sc_nqueue++;
	ises_feed(sc);
	splx(s);

	return (0);

errout:
	if (q) {
		if (q->q_src_m != q->q_dst_m)
			m_freem(q->q_dst_m);
		free(q, M_DEVBUF);
	}
	crp->crp_etype = err;
	crp->crp_callback(crp);
	return (0);
}

void
ises_callback(struct ises_q *q)
{
	struct cryptop *crp = (struct cryptop *)q->q_crp;
	struct cryptodesc *crd;

	if ((crp->crp_flags & CRYPTO_F_IMBUF) && (q->q_src_m != q->q_dst_m)) {
		m_freem(q->q_src_m);
		crp->crp_buf = (caddr_t)q->q_dst_m;
	}

	for (crd = crp->crp_desc; crd; crd = crd->crd_next) {
		if (crd->crd_alg != CRYPTO_MD5_HMAC &&
		    crd->crd_alg != CRYPTO_SHA1_HMAC &&
		    crd->crd_alg != CRYPTO_RIPEMD160_HMAC)
			continue;
		m_copyback((struct mbuf *)crp->crp_buf,
		    crd->crd_inject, 12, (u_int8_t *)&q->q_macbuf[0]);
		break;
	}

	free(q, M_DEVBUF);
	crypto_done(crp);
}

/* Initilize the ISES hardware RNG, and set up timeouts. */
void
ises_hrng_init(struct ises_softc *sc)
{
	u_int32_t cmd, r;
	int i;
#ifdef ISESDEBUG
	struct timeval tv1, tv2;
#endif

	/* Asking for random data will seed LFSR and start the RBG */
	cmd = ISES_MKCMD(ISES_CMD_HBITS, 1);
	r   = 8; /* 8 * 32 = 256 bits */

	if (ises_queue_cmd(sc, cmd, &r))
		return;

	/* Wait until response arrives. */
	for (i = 1000; i && READ_REG(sc, ISES_A_OQS) == 0; i--)
		DELAY(10);

	if (!READ_REG(sc, ISES_A_OQS))
		return;

	/* Drain cmd response and 8*32 bits data */
	for (i = 0; i <= r; i++)
		(void)READ_REG(sc, ISES_A_OQD);

	/* ACK the response */
	WRITE_REG(sc, ISES_A_OQS, 0);
	DELAY(1);
	printf(", rng active", sc->sc_dv.dv_xname);

#ifdef ISESDEBUG
	/* Benchmark the HRNG. */

	/*
	 * XXX These values gets surprisingly large. Docs state the
	 * HNRG produces > 1 mbit/s of random data. The values I'm seeing
	 * are much higher, ca 2.7-2.8 mbit/s. AFAICT the algorithm is sound.
	 * Compiler optimization issues, perhaps?
	 */

#define ISES_WPR 250
#define ISES_ROUNDS 100
	cmd = ISES_MKCMD(ISES_CMD_HBITS, 1);
	r = ISES_WPR;

	/* Queue 100 cmds; each generate 250 32-bit words of rnd data. */
	microtime(&tv1);
	for (i = 0; i < ISES_ROUNDS; i++)
		ises_queue_cmd(sc, cmd, &r);
	for (i = 0; i < ISES_ROUNDS; i++) {
		while (READ_REG(sc, ISES_A_OQS) == 0) ; /* Wait for response */

		(void)READ_REG(sc, ISES_A_OQD);		/* read response */
		for (r = ISES_WPR; r--;)
			(void)READ_REG(sc, ISES_A_OQD);	/* read data */
		WRITE_REG(sc, ISES_A_OQS, 0);		/* ACK resp */
		DELAY(1); /* OQS needs 1us to decrement */
	}
	microtime(&tv2);

	timersub(&tv2, &tv1, &tv1);
	tv1.tv_usec += 1000000 * tv1.tv_sec;
	printf(", %dKb/sec",
	    ISES_WPR * ISES_ROUNDS * 32 / 1024 * 1000000 / tv1.tv_usec);
#endif

	printf("\n");
	timeout_set(&sc->sc_timeout, ises_hrng, sc);
	ises_hrng(sc); /* Call first update */
}

/* Called by timeout (and once by ises_init_hrng()). */
void
ises_hrng(void *v)
{
	/*
	 * Throw a HRNG read random bits command on the command queue.
	 * The normal loop will manage the result and add it to the pool.
	 */
	struct ises_softc *sc = v;
	u_int32_t cmd, n;
	extern int hz; /* from param.c */

	timeout_add(&sc->sc_timeout, hz / ISESRNGIPS);

	if (ises_assert_cmd_mode(sc) != 0)
		return;

	cmd = ISES_MKCMD(ISES_CMD_HBITS, 1);
	n   = (ISESRNGBITS >> 5) & 0xff; /* ask for N 32 bit words */

	ises_queue_cmd(sc, cmd, &n);
}

u_int32_t
ises_get_fwversion(struct ises_softc *sc)
{
	u_int32_t r;
	int i;

	r = ISES_MKCMD(ISES_CMD_CHIP_ID, 0);
	WRITE_REG(sc, ISES_A_IQD, r);
	WRITE_REG(sc, ISES_A_IQS, 0);

	for (i = 100; i > 0 && READ_REG(sc, ISES_A_OQS) == 0; i--)
		DELAY(1);

	if (i < 1)
		return (0); /* No response */

	r = READ_REG(sc, ISES_A_OQD);

	/* Check validity. On error drain reponse data. */
	if (((r >> 16) & 0xff) != ISES_CMD_CHIP_ID ||
	    ((r >> 24) & 0xff) != 3 || (r & ISES_RC_MASK) != ISES_RC_SUCCESS) {
		if ((r & ISES_RC_MASK) == ISES_RC_SUCCESS)
			for (i = ((r >> 24) & 0xff); i; i--)
				(void)READ_REG(sc, ISES_A_OQD);
		r = 0;
		goto out;
	}

	r = READ_REG(sc, ISES_A_OQD); /* read version */
	(void)READ_REG(sc, ISES_A_OQD); /* Discard 64bit "chip-id" */
	(void)READ_REG(sc, ISES_A_OQD);
 out:
	WRITE_REG(sc, ISES_A_OQS, 0); /* Ack the response */
	DELAY(1);
	return (r);
}

/*
 * ises_assert_cmd_mode() returns
 *   -1 for failure to go to cmd
 *    0 if mode already was cmd
 *   >0 if mode was other (WFC/WFR) but now is cmd (this has reset the queues)
 */
int
ises_assert_cmd_mode(struct ises_softc *sc)
{
	switch (ISES_STAT_SW_MODE(READ_REG(sc, ISES_A_STAT))) {
	case 0x0: /* Selftest. XXX This is a transient state. */
		DELAY(1000000);
		if (ISES_STAT_SW_MODE(READ_REG(sc, ISES_A_STAT)) == 0)
			return (-1);
		return (ises_assert_cmd_mode(sc));
	case 0x1: /* Command mode */
		return (0);
	case 0x2: /* Waiting For Continue / WFC */
		bus_space_write_2(sc->sc_memt, sc->sc_memh, ISES_A_CTRL,
		    ISES_A_CTRL_CONTINUE);
		DELAY(1);
		return ((ISES_STAT_SW_MODE(READ_REG(sc, ISES_A_STAT)) == 0) ?
		    1 : -1);
	case 0x3: /* Waiting For Reset / WFR */
		bus_space_write_2(sc->sc_memt, sc->sc_memh, ISES_A_CTRL,
		    ISES_A_CTRL_RESET);
		DELAY(1000000);
		return ((ISES_STAT_SW_MODE(READ_REG(sc, ISES_A_STAT)) == 0) ?
		    2 : -1);
	default:
		return (-1); /* Unknown mode */
	}
}
