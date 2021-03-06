/*-
 * Copyright (c) 1999 Michael Smith
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
 *	$FreeBSD: src/sys/dev/mlx/mlxio.h,v 1.1 1999/10/07 02:20:32 msmith Exp $
 */

#include <sys/ioccom.h>

/*
 * System Disk ioctls
 */
struct mlxd_rebuild
{
    int		rb_channel;
    int		rb_target;
};

struct mlxd_rebuild_status
{
    int		rs_drive;
    int		rs_size;
    int		rs_remaining;
};

#define MLXD_STATUS		_IOR ('M', 100, int)
#define MLXD_REBUILDASYNC	_IOW ('M', 101, struct mlxd_rebuild)
#define MLXD_CHECKASYNC		_IOW ('M', 102, int)
#define MLXD_REBUILDSTAT	_IOR ('M', 103, struct mlxd_rebuild_status)

/*
 * System Disk status values
 */
#define MLX_SYSD_ONLINE		0x03
#define MLX_SYSD_CRITICAL	0x04
#define MLX_SYSD_REBUILD	0xfe
#define MLX_SYSD_OFFLINE	0xff

/*
 * Controller ioctls
 */
struct mlx_pause 
{
    int		mp_which;
#define MLX_PAUSE_ALL		0xff
#define MLX_PAUSE_CANCEL	0x00
    int		mp_when;
    int		mp_howlong;
};

struct mlx_usercommand
{
    /* data buffer */
    size_t	mu_datasize;	/* size of databuffer */
    void	*mu_buf;	/* address in userspace of databuffer */
    int		mu_bufptr;	/* offset into command mailbox to place databuffer address */

    /* command */
    u_int16_t	mu_status;	/* command status returned */
    u_int8_t	mu_command[16];	/* command mailbox contents */
};

#define MLX_NEXT_CHILD		_IOWR('M', 0, int)
#define MLX_RESCAN_DRIVES	_IO  ('M', 1)
#define MLX_DETACH_DRIVE	_IOW ('M', 2, int)
#define MLX_PAUSE_CHANNEL	_IOW ('M', 3, struct mlx_pause)
#define MLX_COMMAND		_IOWR('M', 4, struct mlx_usercommand)
