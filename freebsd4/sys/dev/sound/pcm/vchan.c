/*
 * Copyright (c) 2001 Cameron Grant <gandalf@vilnya.demon.co.uk>
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
 * $FreeBSD: src/sys/dev/sound/pcm/vchan.c,v 1.5.2.1 2001/08/01 03:41:03 cg Exp $
 */

#include <dev/sound/pcm/sound.h>
#include <dev/sound/pcm/vchan.h>
#include "feeder_if.h"

struct vchinfo {
	u_int32_t spd, fmt, blksz, bps, run;
	struct pcm_channel *channel, *parent;
	struct pcmchan_caps caps;
};

static u_int32_t vchan_fmt[] = {
	AFMT_S16_LE,
	AFMT_STEREO | AFMT_S16_LE,
	0
};


static int
vchan_mix_s16(int16_t *to, int16_t *tmp, unsigned int count)
{
	/*
	 * to is the output buffer, tmp is the input buffer
	 * count is the number of 16bit samples to mix
	 */
	int i;
	int x;

	for(i = 0; i < count; i++) {
		x = to[i];
		x += tmp[i];
		if (x < -32768) {
			/* printf("%d + %d = %d (u)\n", to[i], tmp[i], x); */
			x = -32768;
		}
		if (x > 32767) {
			/* printf("%d + %d = %d (o)\n", to[i], tmp[i], x); */
			x = 32767;
		}
		to[i] = x & 0x0000ffff;
	}
	return 0;
}

static int
feed_vchan_s16(struct pcm_feeder *f, struct pcm_channel *c, u_int8_t *b, u_int32_t count, void *source)
{
	/* we're going to abuse things a bit */
	struct snd_dbuf *src = source;
	struct pcmchan_children *cce;
	struct pcm_channel *ch;
	int16_t *tmp, *dst;
	unsigned int cnt;

	KASSERT(sndbuf_getsize(src) >= count, ("bad bufsize"));
	count &= ~1;
	bzero(b, count);

	/*
	 * we are going to use our source as a temporary buffer since it's
	 * got no other purpose.  we obtain our data by traversing the channel
	 * list of children and calling vchan_mix_* to mix count bytes from each
	 * into our destination buffer, b
	 */
	dst = (int16_t *)b;
	tmp = (int16_t *)sndbuf_getbuf(src);
	bzero(tmp, count);
	SLIST_FOREACH(cce, &c->children, link) {
		ch = cce->channel;
		if (ch->flags & CHN_F_TRIGGERED) {
			cnt = FEEDER_FEED(ch->feeder, ch, (u_int8_t *)tmp, count, ch->bufsoft);
			vchan_mix_s16(dst, tmp, cnt / 2);
		}
	}

	return count;
}

static struct pcm_feederdesc feeder_vchan_s16_desc[] = {
	{FEEDER_MIXER, AFMT_S16_LE, AFMT_S16_LE, 0},
	{FEEDER_MIXER, AFMT_S16_LE | AFMT_STEREO, AFMT_S16_LE | AFMT_STEREO, 0},
	{0},
};
static kobj_method_t feeder_vchan_s16_methods[] = {
    	KOBJMETHOD(feeder_feed,		feed_vchan_s16),
	{ 0, 0 }
};
FEEDER_DECLARE(feeder_vchan_s16, 2, NULL);

/************************************************************/

static void *
vchan_init(kobj_t obj, void *devinfo, struct snd_dbuf *b, struct pcm_channel *c, int dir)
{
	struct vchinfo *ch;
	struct pcm_channel *parent = devinfo;

	KASSERT(dir == PCMDIR_PLAY, ("vchan_init: bad direction"));
	ch = malloc(sizeof(*ch), M_DEVBUF, M_WAITOK | M_ZERO);
	ch->parent = parent;
	ch->channel = c;
	ch->fmt = AFMT_U8;
	ch->spd = DSP_DEFAULT_SPEED;
	ch->blksz = 2048;

	c->flags |= CHN_F_VIRTUAL;

	return ch;
}

static int
vchan_free(kobj_t obj, void *data)
{
	return 0;
}

static int
vchan_setformat(kobj_t obj, void *data, u_int32_t format)
{
	struct vchinfo *ch = data;
	struct pcm_channel *parent = ch->parent;

	ch->fmt = format;
	ch->bps = 1;
	ch->bps <<= (ch->fmt & AFMT_STEREO)? 1 : 0;
	ch->bps <<= (ch->fmt & AFMT_16BIT)? 1 : 0;
	ch->bps <<= (ch->fmt & AFMT_32BIT)? 2 : 0;
	chn_notify(parent, CHN_N_FORMAT);
	return 0;
}

static int
vchan_setspeed(kobj_t obj, void *data, u_int32_t speed)
{
	struct vchinfo *ch = data;
	struct pcm_channel *parent = ch->parent;

	ch->spd = speed;
	chn_notify(parent, CHN_N_RATE);
	return speed;
}

static int
vchan_setblocksize(kobj_t obj, void *data, u_int32_t blocksize)
{
	struct vchinfo *ch = data;
	struct pcm_channel *parent = ch->parent;
	int prate, crate;

	ch->blksz = blocksize;
	chn_notify(parent, CHN_N_BLOCKSIZE);

	crate = ch->spd * ch->bps;
	prate = sndbuf_getspd(parent->bufhard) * sndbuf_getbps(parent->bufhard);
	blocksize = sndbuf_getblksz(parent->bufhard);
	blocksize *= prate;
	blocksize /= crate;

	return blocksize;
}

static int
vchan_trigger(kobj_t obj, void *data, int go)
{
	struct vchinfo *ch = data;
	struct pcm_channel *parent = ch->parent;

	if (go == PCMTRIG_EMLDMAWR || go == PCMTRIG_EMLDMARD)
		return 0;

	ch->run = (go == PCMTRIG_START)? 1 : 0;
	chn_notify(parent, CHN_N_TRIGGER);

	return 0;
}

static struct pcmchan_caps *
vchan_getcaps(kobj_t obj, void *data)
{
	struct vchinfo *ch = data;

	ch->caps.minspeed = sndbuf_getspd(ch->parent->bufhard);
	ch->caps.maxspeed = ch->caps.minspeed;
	ch->caps.fmtlist = vchan_fmt;
	ch->caps.caps = 0;

	return &ch->caps;
}

static kobj_method_t vchan_methods[] = {
    	KOBJMETHOD(channel_init,		vchan_init),
    	KOBJMETHOD(channel_free,		vchan_free),
    	KOBJMETHOD(channel_setformat,		vchan_setformat),
    	KOBJMETHOD(channel_setspeed,		vchan_setspeed),
    	KOBJMETHOD(channel_setblocksize,	vchan_setblocksize),
    	KOBJMETHOD(channel_trigger,		vchan_trigger),
    	KOBJMETHOD(channel_getcaps,		vchan_getcaps),
	{ 0, 0 }
};
CHANNEL_DECLARE(vchan);

/* virtual channel interface */

int
vchan_create(struct pcm_channel *parent)
{
    	struct snddev_info *d = parent->parentsnddev;
	struct pcmchan_children *pce;
	struct pcm_channel *child;
	int err, first;

	CHN_LOCK(parent);
	if (!(parent->flags & CHN_F_BUSY)) {
		CHN_UNLOCK(parent);
		return EBUSY;
	}

	pce = malloc(sizeof(*pce), M_DEVBUF, M_WAITOK | M_ZERO);
	if (!pce) {
		CHN_UNLOCK(parent);
		return ENOMEM;
	}

	/* create a new playback channel */
	child = pcm_chn_create(d, parent, &vchan_class, PCMDIR_VIRTUAL, parent);
	if (!child) {
		free(pce, M_DEVBUF);
		CHN_UNLOCK(parent);
		return ENODEV;
	}

	first = SLIST_EMPTY(&parent->children);
	/* add us to our parent channel's children */
	pce->channel = child;
	SLIST_INSERT_HEAD(&parent->children, pce, link);
	CHN_UNLOCK(parent);

	/* add us to our grandparent's channel list */
	err = pcm_chn_add(d, child);
	if (err) {
		pcm_chn_destroy(child);
		free(pce, M_DEVBUF);
	}

	/* XXX gross ugly hack, kill murder death */
	if (first && !err) {
		err = chn_reset(parent, AFMT_STEREO | AFMT_S16_LE);
		if (err)
			printf("chn_reset: %d\n", err);
		err = chn_setspeed(parent, 44100);
		if (err)
			printf("chn_setspeed: %d\n", err);
	}

	return err;
}

int
vchan_destroy(struct pcm_channel *c)
{
	struct pcm_channel *parent = c->parentchannel;
    	struct snddev_info *d = parent->parentsnddev;
	struct pcmchan_children *pce;
	int err;

	/* remove us from our grantparent's channel list */
	err = pcm_chn_remove(d, c);
	if (err)
		return err;

	CHN_LOCK(parent);
	if (!(parent->flags & CHN_F_BUSY)) {
		CHN_UNLOCK(parent);
		return EBUSY;
	}
	if (SLIST_EMPTY(&parent->children)) {
		CHN_UNLOCK(parent);
		return EINVAL;
	}

	/* remove us from our parent's children list */
	SLIST_FOREACH(pce, &parent->children, link) {
		if (pce->channel == c)
			goto gotch;
	}
	CHN_UNLOCK(parent);
	return EINVAL;
gotch:
	SLIST_REMOVE(&parent->children, pce, pcmchan_children, link);
	free(pce, M_DEVBUF);

	if (SLIST_EMPTY(&parent->children))
		parent->flags &= ~CHN_F_BUSY;
	CHN_UNLOCK(parent);
	/* destroy ourselves */
	err = pcm_chn_destroy(c);

	return err;
}

#ifdef SND_DYNSYSCTL
static int
sysctl_hw_snd_vchans(SYSCTL_HANDLER_ARGS)
{
	struct snddev_info *d;
    	struct snddev_channel *sce;
	struct pcm_channel *c;
	int err, oldcnt, newcnt, cnt;

	d = oidp->oid_arg1;

	snd_mtxlock(d->lock);
	cnt = 0;
	SLIST_FOREACH(sce, &d->channels, link) {
		c = sce->channel;
		if ((c->direction == PCMDIR_PLAY) && (c->flags & CHN_F_VIRTUAL))
			cnt++;
	}
	oldcnt = cnt;
	newcnt = cnt;

	err = sysctl_handle_int(oidp, &newcnt, sizeof(newcnt), req);
	if (err == 0 && req->newptr != NULL) {
		if (newcnt > cnt) {
			/* add new vchans - find a parent channel first */
			SLIST_FOREACH(sce, &d->channels, link) {
				c = sce->channel;
				/* not a candidate if not a play channel */
				if (c->direction != PCMDIR_PLAY)
					goto addskip;
				/* not a candidate if a virtual channel */
				if (c->flags & CHN_F_VIRTUAL)
					goto addskip;
				/* not a candidate if it's in use */
				if ((c->flags & CHN_F_BUSY) && (SLIST_EMPTY(&c->children)))
					goto addskip;
				/*
				 * if we get here we're a nonvirtual play channel, and either
				 * 1) not busy
				 * 2) busy with children, not directly open
				 *
				 * thus we can add children
				 */
				goto addok;
addskip:
			}
			snd_mtxunlock(d->lock);
			return EBUSY;
addok:
			c->flags |= CHN_F_BUSY;
			while (err == 0 && newcnt > cnt) {
				err = vchan_create(c);
				if (err == 0)
					cnt++;
			}
			if (SLIST_EMPTY(&c->children))
				c->flags &= ~CHN_F_BUSY;
		} else if (newcnt < cnt) {
			while (err == 0 && newcnt < cnt) {
				SLIST_FOREACH(sce, &d->channels, link) {
					c = sce->channel;
					if ((c->flags & (CHN_F_BUSY | CHN_F_VIRTUAL)) == CHN_F_VIRTUAL)
						goto remok;
				}
				snd_mtxunlock(d->lock);
				return EINVAL;
remok:
				err = vchan_destroy(c);
				if (err == 0)
					cnt--;
			}
		}
	}

	snd_mtxunlock(d->lock);
	return err;
}
#endif

int
vchan_initsys(struct snddev_info *d)
{
#ifdef SND_DYNSYSCTL
	SYSCTL_ADD_PROC(&d->sysctl_tree, SYSCTL_CHILDREN(d->sysctl_tree_top),
            OID_AUTO, "vchans", CTLTYPE_INT | CTLFLAG_RW, d, sizeof(d),
	    sysctl_hw_snd_vchans, "I", "")
#endif
	return 0;
}


