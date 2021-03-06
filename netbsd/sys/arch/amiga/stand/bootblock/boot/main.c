/*
 * $NetBSD: main.c,v 1.10 1998/11/13 22:12:35 is Exp $
 *
 *
 * Copyright (c) 1996 Ignatios Souvatzis
 * Copyright (c) 1994 Michael L. Hitch
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
 *      This product includes software developed by Michael L. Hitch.
 * 4. The name of the authors may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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
 *
 */

#include <sys/cdefs.h>
#include <sys/reboot.h>
#include <sys/types.h>

#include <sys/exec_aout.h>

#include <amiga/cfdev.h>
#include <amiga/memlist.h>
#include <include/cpu.h>

#include <saerrno.h>
#include <stand.h>

#include "libstubs.h"
#include "samachdep.h"

#undef __LDPGSZ
#define __LDPGSZ 8192
#define __PGSZ 8192

#define DRACOREVISION	(*(u_int8_t *)0x02000009)
#define DRACOMMUMARGIN	0x200000
#define DRACOZ2OFFSET	0x3000000
#define DRACOZ2MAX	0x1000000

#define EXECMIN 36

void startit __P((void *, u_long, u_long, void *, u_long, u_long, int, void *,
	int, int, u_long, u_long, u_long, int));
void startit_end __P((void));
int get_cpuid __P((u_int32_t *));

/*
 * Kernel startup interface version
 *	1:      first version of loadbsd
 *	2:      needs esym location passed in a4
 *	3:      load kernel image into fastmem rather than chipmem
 *	MAX:    highest version with backward compatibility.
 */     

#define KERNEL_STARTUP_VERSION		3
#define KERNEL_STARTUP_VERSION_MAX	9

static long get_number(char **);

#define VERSION "2.1"

char default_command[] = "netbsd -ASn2";

int
pain(aio)
	void *aio;	
{
	long int io = 0;
	char linebuf[128];
	char	*kernel_name = default_command;
	char	*path = default_command;
	int	boothowto = RB_AUTOBOOT;
	u_int32_t cpuid = 0;
	int	amiga_flags = 0;
	u_int32_t I_flag = 0;
	int	k_flag = 0;
	int	p_flag = 0;
	int	m_value = 0;
	int	S_flag = 0;
	int	t_flag = 0;
	long stringsz;

	u_int32_t fmem = 0x0;
	int	fmemsz = 0x0;
	int	cmemsz = 0x0;
	int	eclock = SysBase->EClockFreq;
	/* int	skip_chipmem = 0; */

	void (*start_it)(void *, u_long, u_long, void *, u_long, u_long, int,
	    void *, int, int, u_long, u_long, u_long, int) = startit;

	caddr_t kp;
	u_int16_t *kvers;
	struct exec ehs;
	int	textsz, ksize;
	void	*esym = 0;
	int32_t *nkcd;
	struct cfdev *cd, *kcd;
	struct boot_memseg *kmemseg;
	struct boot_memseg *memseg;
	struct MemHead *mh;
	u_int32_t from, size, vfrom, vsize;
	int contflag, mapped1to1;

	int ncd, nseg;
	char c;

	extern u_int16_t timelimit;

	extern u_int32_t aio_base;

	xdinit(aio);

	if (consinit())
		return(1);

	/*
	 * we need V36 for: EClock, RDB Bootblocks, CacheClearU
	 */

	if (SysBase->LibNode.Version < EXECMIN) {
		printf("Exec V%ld, need V%ld\n",
		    (long)SysBase->LibNode.Version, (long)EXECMIN);
		goto out;
	}

	printf("\2337mNetBSD/Amiga bootblock " VERSION "\2330m\n%s :- ",
		kernel_name);

	timelimit = 3;
	gets(linebuf);

	if (*linebuf == 'q')
		return 1;

	if (*linebuf)
		path = linebuf;

	/*
	 * parse boot command for path name and process any options
	 */
	while ((c = *path)) {
		while (c == ' ')
			c = *++path;
		if (c == '-') {
			while ((c = *++path) && c != ' ') {
				switch (c) {
				case 'a':	/* multi-user state */
					boothowto &= ~RB_SINGLE;
					break;
				case 'b':	/* ask for root device */
					boothowto |= RB_ASKNAME;
					break;
				case 'c':	/* force machine model */
					cpuid = get_number(&path) << 16;
					break;
				case 'k':	/* Reserve first 4M fastmem */
					k_flag++;
					break;
				case 'm':	/* Force fastmem size */
					m_value = get_number(&path) * 1024;
					break;
				case 'n':	/* non-contiguous memory */
					amiga_flags |= 
					    (get_number(&path) & 3) << 1;
					break;
				case 'p':	/* Select fastmem by priority */
					p_flag++;
					break;
				case 's':	/* single-user state */
					boothowto |= RB_SINGLE;
					break;
				case 't':	/* test flag */
					t_flag = 1;
					break;
				case 'A':	/* enable AGA modes */
					amiga_flags |= 1;
					break;
				case 'D':	/* enter Debugger */
					boothowto |= RB_KDB;
					break;
				case 'I':	/* inhibit sync negotiation */
					I_flag = get_number(&path);
					break;
				case 'K':	/* remove 1st 4MB fastmem */
					break;
				case 'S':	/* include debug symbols */
					S_flag = 1;
					break;
				}
			}
		} else {
			kernel_name = path;
			while ((c = *++path) && c != ' ')
				;
			if (c)
				*path++ = 0;
		}
	}
	while ((c = *kernel_name) && c == ' ')
		++kernel_name;
	path = kernel_name;
	while ((c = *path) && c != ' ')
		++path;
	if (c)
		*path = 0;

	if (get_cpuid(&cpuid))
		goto out;

	ExpansionBase = OpenLibrary("expansion.library", 0);
	if (!ExpansionBase) {
		printf("can't open %s\n", "expansion.library");
		return 1;
	}

	for (ncd=0, cd=0; (cd = FindConfigDev(cd, -1, -1)); ncd++)
		/* nothing */;

	/* find memory list */

	memseg = (struct boot_memseg *)alloc(16*sizeof(struct boot_memseg));

	/* Forbid(); */

	nseg = 0;
	mh = SysBase->MemLst;
	vfrom = mh->Lower & -__PGSZ;
	vsize = (mh->Upper & -__PGSZ) - vfrom; 
	contflag = mapped1to1 = 0;

	do {
		size = vsize;

		if (SysBase->LibNode.Version > 36) {
			from = CachePreDMA(vfrom, &size, contflag);
			contflag = DMAF_Continue;
			mapped1to1 = (from == vfrom);
			vsize -= size;
			vfrom += size;
		} else {
			from = vfrom;
			mapped1to1 = 1;
			vsize = 0;
		}

#ifdef DEBUG_MEMORY_LIST
		printf("%lx %lx %lx %ld/%lx %lx\n",
			(long)from, (long)size, 
			(long)mh->Attribs, (long)mh->Pri,
			(long)vfrom, (long)vsize);
#endif
		/* Insert The Evergrowing Kludge List Here: */

		/* a) dont load kernel over DraCo MMU table */
			
		if (((cpuid >> 24) == 0x7D) &&
		    ((from & -DRACOMMUMARGIN) == 0x40000000) && 
		    (size >= DRACOMMUMARGIN)) {

			memseg[nseg].ms_start = from & -DRACOMMUMARGIN;
			memseg[nseg].ms_size = DRACOMMUMARGIN;
			memseg[nseg].ms_attrib = mh->Attribs;
			memseg[nseg].ms_pri = mh->Pri;

			size -= DRACOMMUMARGIN - (from & (DRACOMMUMARGIN - 1));
			from += DRACOMMUMARGIN - (from & (DRACOMMUMARGIN - 1));
			++nseg;
		}

		if ((mh->Attribs & (MEMF_CHIP|MEMF_FAST)) == MEMF_CHIP) {
			size += from;
			cmemsz = size;;
			from = 0;
		} else if ((fmemsz < size) && mapped1to1) {
			fmem = from;
			fmemsz = size;
		}

		memseg[nseg].ms_start = from;
		memseg[nseg].ms_size = size;
		memseg[nseg].ms_attrib = mh->Attribs;
		memseg[nseg].ms_pri = mh->Pri;

		if (vsize == 0) {
			mh = mh->next;
			contflag = 0;
			if (mh->next) {
				vfrom = mh->Lower & -__PGSZ;
				vsize = (mh->Upper & -__PGSZ) - vfrom; 
			}
		}
	} while ((++nseg <= 16) && vsize);

	/* Permit(); */

	if (k_flag) {
		fmem += 4*1024*1024;
		fmemsz -= 4*1024*1024;
	}
	if (m_value && m_value < fmemsz)
		fmemsz = m_value;

	printf("Loading %s: ", kernel_name);
	io = open(kernel_name, 0);
	if (io < 0)
		goto err;

	if (read(io, &ehs, sizeof(ehs)) != sizeof(ehs)) {
		errno = ENOEXEC;
		goto err;
	}

	if ((N_GETMAGIC(ehs) != NMAGIC) || (N_GETMID(ehs) != MID_M68K)) {
		errno = ENOEXEC;
		goto err;
	}
		
	textsz = (ehs.a_text + __LDPGSZ - 1) & (-__LDPGSZ);
	esym = 0;

	ksize = textsz + ehs.a_data + ehs.a_bss 
	    + sizeof(*nkcd) + ncd*sizeof(*cd)
	    + sizeof(*nkcd) + nseg * sizeof(struct boot_memseg);

	if (S_flag && ehs.a_syms) {
		if (lseek(io, ehs.a_text+ ehs.a_data+ ehs.a_syms, SEEK_CUR)
		    <= 0
		    || read(io, &stringsz, 4) != 4
		    || lseek(io, sizeof(ehs), SEEK_SET) < 0)
			goto err;
		ksize += ehs.a_syms + 4 + ((stringsz + 3) & ~3);
	}

	kp = alloc(ksize + 256 + ((u_char *)startit_end - (u_char *)startit));
	if (kp == 0) {
		errno = ENOMEM;
		goto err;
	}

	printf("%ld", ehs.a_text);
	if (read(io, kp, ehs.a_text) != ehs.a_text)
		goto err;

	printf("+%ld", ehs.a_data);
	if (read(io, kp + textsz, ehs.a_data) != ehs.a_data)
		goto err;

	printf("+%ld", ehs.a_bss);

	kvers = (u_short *)(kp + ehs.a_entry - 2);

	if (*kvers > KERNEL_STARTUP_VERSION_MAX && *kvers != 0x4e73) {
                printf("\nnewer bootblock required: %ld\n", (long)*kvers);
		goto freeall;
	}
	if (*kvers < KERNEL_STARTUP_VERSION || *kvers == 0x4e73) {
                printf("\nkernel too old for bootblock\n");
		goto freeall;
	}
#if 0
        if (*kvers > KERNEL_STARTUP_VERSION)
		printf("\nKernel V%ld newer than bootblock V%ld\n",
		    (long)*kvers, (long)KERNEL_STARTUP_VERSION);
#endif
        nkcd = (int *)(kp + textsz + ehs.a_data + ehs.a_bss);
        if (*kvers != 0x4e73 && *kvers > 1 && S_flag && ehs.a_syms) {
                *nkcd++ = ehs.a_syms;
		printf("+[%ld", ehs.a_syms);
                if (read(io, (char *)nkcd, ehs.a_syms) != ehs.a_syms)
			goto err;
                nkcd = (int *)((char *)nkcd + ehs.a_syms);
		printf("+%ld]", stringsz);
                if (read(io, (char *)nkcd, stringsz) != stringsz)
			goto err;
                nkcd = (int*)((char *)nkcd + ((stringsz + 3) & ~3));
                esym = (char *)(textsz + ehs.a_data + ehs.a_bss
                    + ehs.a_syms + 4 + ((stringsz + 3) & ~3));
        }
	putchar('\n');

	*nkcd = ncd;
	kcd = (struct cfdev *)(nkcd + 1);

	while ((cd = FindConfigDev(cd, -1, -1))) {
		*kcd = *cd;
		if (((cpuid >> 24) == 0x7D) &&
		    ((u_long)kcd->addr < 0x1000000)) {
			kcd->addr += 0x3000000;
		}
		++kcd;
	}

	nkcd = (u_int32_t *)kcd;
	*nkcd = nseg;

	kmemseg = (struct boot_memseg *)(nkcd + 1);

	while (nseg-- > 0)
		*kmemseg++ = *memseg++;

	/*
	 * Copy startup code to end of kernel image and set start_it.
	 */
	memcpy(kp + ksize + 256, (char *)startit,
	    (char *)startit_end - (char *)startit);
	CacheClearU();
	(caddr_t)start_it = kp + ksize + 256;
	printf("*** Loading from %08lx to Fastmem %08lx ***\n",
	    (u_long)kp, (u_long)fmem);
	/* sleep(2); */

#if 0
	printf("would start(kp=0x%lx, ksize=%ld, entry=0x%lx,\n"
		"fmem=0x%lx, fmemsz=%ld, cmemsz=%ld\n"
		"boothow=0x%lx, esym=0x%lx, cpuid=0x%lx, eclock=%ld\n"
		"amigaflags=0x%lx, I_flags=0x%lx, ok?\n",
	    (u_long)kp, (u_long)ksize, ehs.a_entry,
	    (u_long)fmem, (u_long)fmemsz, (u_long)cmemsz,
	    (u_long)boothowto, (u_long)esym, (u_long)cpuid, (u_long)eclock,
	    (u_long)amiga_flags, (u_long)I_flag);
#endif
#ifdef DEBUG_MEMORY_LIST
	timelimit = 0;
#else
	timelimit = 2;
#endif
	(void)getchar();

	start_it(kp, ksize, ehs.a_entry, (void *)fmem, fmemsz, cmemsz,
	    boothowto, esym, cpuid, eclock, amiga_flags, I_flag,
	    aio_base >> 9, 1);
	/*NOTREACHED*/

freeall:
	free(kp, ksize);
err:
	printf("\nError %ld\n", (long)errno);
	close(io);
out:
	timelimit = 10;
	(void)getchar();
	return 1;
}

static
long get_number(ptr)
char **ptr;
{
	long value = 0;
	int base = 10;
	char *p = *ptr;
	char c;
	char sign = 0;

	c = *++p;
	while (c == ' ')
		c = *++p;
	if (c == '-') {
		sign = -1;
		c = *++p;
	}
	if (c == '$') {
		base = 16;
		c = *++p;
	} else if (c == '0') {
		c = *++p;
		if ((c & 0xdf) == 'X') {
			base = 16;
			c = *++p;
		}
	}
	while (c) {
		if (c >= '0' && c <= '9')
			c -= '0';
		else {
			c = (c & 0xdf) - 'A' + 10;
			if (base != 16 || c < 10 || c > 15)
				break;
		}
		value = value * base + c;
		c = *++p;
	}
	*ptr = p - 1;
#ifdef TEST
	fprintf(stderr, "get_number: got %c0x%x", 
	    sign ? '-' : '+', value);
#endif	
	return (sign ? -value : value);
}

/*
 * Try to determine the machine ID by searching the resident module list
 * for modules only present on specific machines.  (Thanks, Bill!)
 */

int
get_cpuid(cpuid)
	u_int32_t *cpuid;
{
	*cpuid |= SysBase->AttnFlags;	/* get FPU and CPU flags */
	if (*cpuid & 0xffff0000) {
		if ((*cpuid >> 24) == 0x7D)
			return 0;

		switch (*cpuid >> 16) {
		case 500:
		case 600:
		case 1000:
		case 1200:
		case 2000:
		case 3000:
		case 4000:
			return 0;
		default:
			printf("Amiga %ld ???\n",
			    (long)(*cpuid >> 16));
			return(1);
		}
	}
	if (FindResident("A4000 Bonus") || FindResident("A4000 bonus")
	    || FindResident("A1000 Bonus"))
		*cpuid |= 4000 << 16;
	else if (FindResident("A3000 Bonus") || FindResident("A3000 bonus")
	    || (SysBase->LibNode.Version == 36))
		*cpuid |= 3000 << 16;
	else if (OpenResource("card.resource")) {
		/* Test for AGA? */
		*cpuid |= 1200 << 16;
	} else if (OpenResource("draco.resource")) {
		*cpuid |= (32000 | DRACOREVISION) << 16;
	}
	/*
	 * Nothing found, it's probably an A2000 or A500
	 */
	if ((*cpuid >> 16) == 0)
		*cpuid |= 2000 << 16;

	return 0;
}
