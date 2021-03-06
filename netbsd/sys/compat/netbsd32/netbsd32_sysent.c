/*	$NetBSD: netbsd32_sysent.c,v 1.9 1999/03/25 16:32:27 mrg Exp $	*/

/*
 * System call switch table.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	NetBSD: syscalls.master,v 1.7 1999/03/25 16:22:50 mrg Exp 
 */

#include "opt_ktrace.h"
#include "opt_nfsserver.h"
#include "opt_compat_netbsd.h"
#include "opt_ntp.h"
#include "opt_sysv.h"
#include "opt_compat_43.h"
#include "fs_lfs.h"
#include "fs_nfs.h"
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/mount.h>
#include <sys/syscallargs.h>
#include <compat/netbsd32/netbsd32.h>
#include <compat/netbsd32/netbsd32_syscallargs.h>

#ifdef COMPAT_43
#define compat_43(func) __CONCAT(compat_43_,func)
#else
#define compat_43(func) sys_nosys
#endif

#ifdef COMPAT_09
#define compat_09(func) __CONCAT(compat_09_,func)
#else
#define compat_09(func) sys_nosys
#endif

#ifdef COMPAT_10
#define compat_10(func) __CONCAT(compat_10_,func)
#else
#define compat_10(func) sys_nosys
#endif

#ifdef COMPAT_11
#define compat_11(func) __CONCAT(compat_11_,func)
#else
#define compat_11(func) sys_nosys
#endif

#ifdef COMPAT_12
#define compat_12(func) __CONCAT(compat_12_,func)
#else
#define compat_12(func) sys_nosys
#endif

#ifdef COMPAT_13
#define compat_13(func) __CONCAT(compat_13_,func)
#else
#define compat_13(func) sys_nosys
#endif

#define	s(type)	sizeof(type)

struct sysent netbsd32_sysent[] = {
	{ 0, 0,
	    sys_nosys },			/* 0 = syscall (indir) */
	{ 1, s(struct compat_netbsd32_exit_args),
	    compat_netbsd32_exit },		/* 1 = compat_netbsd32_exit */
	{ 0, 0,
	    sys_fork },				/* 2 = fork */
	{ 3, s(struct compat_netbsd32_read_args),
	    compat_netbsd32_read },		/* 3 = compat_netbsd32_read */
	{ 3, s(struct compat_netbsd32_write_args),
	    compat_netbsd32_write },		/* 4 = compat_netbsd32_write */
	{ 3, s(struct compat_netbsd32_open_args),
	    compat_netbsd32_open },		/* 5 = compat_netbsd32_open */
	{ 1, s(struct compat_netbsd32_close_args),
	    compat_netbsd32_close },		/* 6 = compat_netbsd32_close */
	{ 4, s(struct compat_netbsd32_wait4_args),
	    compat_netbsd32_wait4 },		/* 7 = compat_netbsd32_wait4 */
	{ 2, s(struct compat_43_compat_netbsd32_ocreat_args),
	    compat_43(compat_netbsd32_ocreat) },/* 8 = compat_43 compat_netbsd32_ocreat */
	{ 2, s(struct compat_netbsd32_link_args),
	    compat_netbsd32_link },		/* 9 = compat_netbsd32_link */
	{ 1, s(struct compat_netbsd32_unlink_args),
	    compat_netbsd32_unlink },		/* 10 = compat_netbsd32_unlink */
	{ 0, 0,
	    sys_nosys },			/* 11 = obsolete execv */
	{ 1, s(struct compat_netbsd32_chdir_args),
	    compat_netbsd32_chdir },		/* 12 = compat_netbsd32_chdir */
	{ 1, s(struct compat_netbsd32_fchdir_args),
	    compat_netbsd32_fchdir },		/* 13 = compat_netbsd32_fchdir */
	{ 3, s(struct compat_netbsd32_mknod_args),
	    compat_netbsd32_mknod },		/* 14 = compat_netbsd32_mknod */
	{ 2, s(struct compat_netbsd32_chmod_args),
	    compat_netbsd32_chmod },		/* 15 = compat_netbsd32_chmod */
	{ 3, s(struct compat_netbsd32_chown_args),
	    compat_netbsd32_chown },		/* 16 = compat_netbsd32_chown */
	{ 1, s(struct compat_netbsd32_break_args),
	    compat_netbsd32_break },		/* 17 = compat_netbsd32_break */
	{ 3, s(struct compat_netbsd32_getfsstat_args),
	    compat_netbsd32_getfsstat },	/* 18 = compat_netbsd32_getfsstat */
	{ 3, s(struct compat_43_compat_netbsd32_olseek_args),
	    compat_43(compat_netbsd32_olseek) },/* 19 = compat_43 compat_netbsd32_olseek */
	{ 0, 0,
	    sys_getpid },			/* 20 = getpid */
	{ 4, s(struct compat_netbsd32_mount_args),
	    compat_netbsd32_mount },		/* 21 = compat_netbsd32_mount */
	{ 2, s(struct compat_netbsd32_unmount_args),
	    compat_netbsd32_unmount },		/* 22 = compat_netbsd32_unmount */
	{ 1, s(struct compat_netbsd32_setuid_args),
	    compat_netbsd32_setuid },		/* 23 = compat_netbsd32_setuid */
	{ 0, 0,
	    sys_getuid },			/* 24 = getuid */
	{ 0, 0,
	    sys_geteuid },			/* 25 = geteuid */
	{ 4, s(struct compat_netbsd32_ptrace_args),
	    compat_netbsd32_ptrace },		/* 26 = compat_netbsd32_ptrace */
	{ 3, s(struct compat_netbsd32_recvmsg_args),
	    compat_netbsd32_recvmsg },		/* 27 = compat_netbsd32_recvmsg */
	{ 3, s(struct compat_netbsd32_sendmsg_args),
	    compat_netbsd32_sendmsg },		/* 28 = compat_netbsd32_sendmsg */
	{ 6, s(struct compat_netbsd32_recvfrom_args),
	    compat_netbsd32_recvfrom },		/* 29 = compat_netbsd32_recvfrom */
	{ 3, s(struct compat_netbsd32_accept_args),
	    compat_netbsd32_accept },		/* 30 = compat_netbsd32_accept */
	{ 3, s(struct compat_netbsd32_getpeername_args),
	    compat_netbsd32_getpeername },	/* 31 = compat_netbsd32_getpeername */
	{ 3, s(struct compat_netbsd32_getsockname_args),
	    compat_netbsd32_getsockname },	/* 32 = compat_netbsd32_getsockname */
	{ 2, s(struct compat_netbsd32_access_args),
	    compat_netbsd32_access },		/* 33 = compat_netbsd32_access */
	{ 2, s(struct compat_netbsd32_chflags_args),
	    compat_netbsd32_chflags },		/* 34 = compat_netbsd32_chflags */
	{ 2, s(struct compat_netbsd32_fchflags_args),
	    compat_netbsd32_fchflags },		/* 35 = compat_netbsd32_fchflags */
	{ 0, 0,
	    sys_sync },				/* 36 = sync */
	{ 2, s(struct compat_netbsd32_kill_args),
	    compat_netbsd32_kill },		/* 37 = compat_netbsd32_kill */
	{ 2, s(struct compat_43_compat_netbsd32_stat43_args),
	    compat_43(compat_netbsd32_stat43) },/* 38 = compat_43 compat_netbsd32_stat43 */
	{ 0, 0,
	    sys_getppid },			/* 39 = getppid */
	{ 2, s(struct compat_43_compat_netbsd32_lstat43_args),
	    compat_43(compat_netbsd32_lstat43) },/* 40 = compat_43 compat_netbsd32_lstat43 */
	{ 1, s(struct compat_netbsd32_dup_args),
	    compat_netbsd32_dup },		/* 41 = compat_netbsd32_dup */
	{ 0, 0,
	    sys_pipe },				/* 42 = pipe */
	{ 0, 0,
	    sys_getegid },			/* 43 = getegid */
	{ 4, s(struct compat_netbsd32_profil_args),
	    compat_netbsd32_profil },		/* 44 = compat_netbsd32_profil */
#ifdef KTRACE
	{ 4, s(struct compat_netbsd32_ktrace_args),
	    compat_netbsd32_ktrace },		/* 45 = compat_netbsd32_ktrace */
#else
	{ 0, 0,
	    sys_nosys },			/* 45 = unimplemented compat_netbsd32_ktrace */
#endif
	{ 3, s(struct compat_netbsd32_sigaction_args),
	    compat_netbsd32_sigaction },	/* 46 = compat_netbsd32_sigaction */
	{ 0, 0,
	    sys_getgid },			/* 47 = getgid */
	{ 2, s(struct compat_13_compat_netbsd32_sigprocmask_args),
	    compat_13(compat_netbsd32_sigprocmask) },/* 48 = compat_13 sigprocmask13 */
	{ 2, s(struct compat_netbsd32___getlogin_args),
	    compat_netbsd32___getlogin },	/* 49 = compat_netbsd32___getlogin */
	{ 1, s(struct compat_netbsd32_setlogin_args),
	    compat_netbsd32_setlogin },		/* 50 = compat_netbsd32_setlogin */
	{ 1, s(struct compat_netbsd32_acct_args),
	    compat_netbsd32_acct },		/* 51 = compat_netbsd32_acct */
	{ 0, 0,
	    compat_13(sys_sigpending) },	/* 52 = compat_13 sigpending13 */
	{ 2, s(struct compat_13_compat_netbsd32_sigaltstack13_args),
	    compat_13(compat_netbsd32_sigaltstack13) },/* 53 = compat_13 compat_netbsd32_sigaltstack13 */
	{ 3, s(struct compat_netbsd32_ioctl_args),
	    compat_netbsd32_ioctl },		/* 54 = compat_netbsd32_ioctl */
#ifdef COMPAT_12
	{ 1, s(struct compat_12_compat_netbsd32_reboot_args),
	    compat_12(compat_netbsd32_reboot) },/* 55 = compat_12 compat_netbsd32_reboot */
#else
	{ 0, 0,
	    sys_nosys },			/* 55 = obsolete oreboot */
#endif
	{ 1, s(struct compat_netbsd32_revoke_args),
	    compat_netbsd32_revoke },		/* 56 = compat_netbsd32_revoke */
	{ 2, s(struct compat_netbsd32_symlink_args),
	    compat_netbsd32_symlink },		/* 57 = compat_netbsd32_symlink */
	{ 3, s(struct compat_netbsd32_readlink_args),
	    compat_netbsd32_readlink },		/* 58 = compat_netbsd32_readlink */
	{ 3, s(struct compat_netbsd32_execve_args),
	    compat_netbsd32_execve },		/* 59 = compat_netbsd32_execve */
	{ 1, s(struct compat_netbsd32_umask_args),
	    compat_netbsd32_umask },		/* 60 = compat_netbsd32_umask */
	{ 1, s(struct compat_netbsd32_chroot_args),
	    compat_netbsd32_chroot },		/* 61 = compat_netbsd32_chroot */
	{ 2, s(struct compat_43_compat_netbsd32_fstat43_args),
	    compat_43(compat_netbsd32_fstat43) },/* 62 = compat_43 compat_netbsd32_fstat43 */
	{ 4, s(struct compat_43_compat_netbsd32_ogetkerninfo_args),
	    compat_43(compat_netbsd32_ogetkerninfo) },/* 63 = compat_43 compat_netbsd32_ogetkerninfo */
	{ 0, 0,
	    compat_43(sys_getpagesize) },	/* 64 = compat_43 ogetpagesize */
	{ 2, s(struct compat_12_compat_netbsd32_msync_args),
	    compat_12(compat_netbsd32_msync) },	/* 65 = compat_12 compat_netbsd32_msync */
	{ 0, 0,
	    sys_vfork },			/* 66 = vfork */
	{ 0, 0,
	    sys_nosys },			/* 67 = obsolete vread */
	{ 0, 0,
	    sys_nosys },			/* 68 = obsolete vwrite */
	{ 1, s(struct compat_netbsd32_sbrk_args),
	    compat_netbsd32_sbrk },		/* 69 = compat_netbsd32_sbrk */
	{ 1, s(struct compat_netbsd32_sstk_args),
	    compat_netbsd32_sstk },		/* 70 = compat_netbsd32_sstk */
	{ 6, s(struct compat_43_compat_netbsd32_ommap_args),
	    compat_43(compat_netbsd32_ommap) },	/* 71 = compat_43 compat_netbsd32_ommap */
	{ 1, s(struct compat_netbsd32_ovadvise_args),
	    compat_netbsd32_ovadvise },		/* 72 = vadvise */
	{ 2, s(struct compat_netbsd32_munmap_args),
	    compat_netbsd32_munmap },		/* 73 = compat_netbsd32_munmap */
	{ 3, s(struct compat_netbsd32_mprotect_args),
	    compat_netbsd32_mprotect },		/* 74 = compat_netbsd32_mprotect */
	{ 3, s(struct compat_netbsd32_madvise_args),
	    compat_netbsd32_madvise },		/* 75 = compat_netbsd32_madvise */
	{ 0, 0,
	    sys_nosys },			/* 76 = obsolete vhangup */
	{ 0, 0,
	    sys_nosys },			/* 77 = obsolete vlimit */
	{ 3, s(struct compat_netbsd32_mincore_args),
	    compat_netbsd32_mincore },		/* 78 = compat_netbsd32_mincore */
	{ 2, s(struct compat_netbsd32_getgroups_args),
	    compat_netbsd32_getgroups },	/* 79 = compat_netbsd32_getgroups */
	{ 2, s(struct compat_netbsd32_setgroups_args),
	    compat_netbsd32_setgroups },	/* 80 = compat_netbsd32_setgroups */
	{ 0, 0,
	    sys_getpgrp },			/* 81 = getpgrp */
	{ 2, s(struct compat_netbsd32_setpgid_args),
	    compat_netbsd32_setpgid },		/* 82 = compat_netbsd32_setpgid */
	{ 3, s(struct compat_netbsd32_setitimer_args),
	    compat_netbsd32_setitimer },	/* 83 = compat_netbsd32_setitimer */
	{ 0, 0,
	    compat_43(sys_wait) },		/* 84 = compat_43 owait */
	{ 1, s(struct compat_12_compat_netbsd32_oswapon_args),
	    compat_12(compat_netbsd32_oswapon) },/* 85 = compat_12 compat_netbsd32_oswapon */
	{ 2, s(struct compat_netbsd32_getitimer_args),
	    compat_netbsd32_getitimer },	/* 86 = compat_netbsd32_getitimer */
	{ 2, s(struct compat_43_compat_netbsd32_ogethostname_args),
	    compat_43(compat_netbsd32_ogethostname) },/* 87 = compat_43 compat_netbsd32_ogethostname */
	{ 2, s(struct compat_43_compat_netbsd32_osethostname_args),
	    compat_43(compat_netbsd32_osethostname) },/* 88 = compat_43 compat_netbsd32_osethostname */
	{ 0, 0,
	    compat_43(sys_getdtablesize) },	/* 89 = compat_43 ogetdtablesize */
	{ 2, s(struct compat_netbsd32_dup2_args),
	    compat_netbsd32_dup2 },		/* 90 = compat_netbsd32_dup2 */
	{ 0, 0,
	    sys_nosys },			/* 91 = unimplemented getdopt */
	{ 3, s(struct compat_netbsd32_fcntl_args),
	    compat_netbsd32_fcntl },		/* 92 = compat_netbsd32_fcntl */
	{ 5, s(struct compat_netbsd32_select_args),
	    compat_netbsd32_select },		/* 93 = compat_netbsd32_select */
	{ 0, 0,
	    sys_nosys },			/* 94 = unimplemented setdopt */
	{ 1, s(struct compat_netbsd32_fsync_args),
	    compat_netbsd32_fsync },		/* 95 = compat_netbsd32_fsync */
	{ 3, s(struct compat_netbsd32_setpriority_args),
	    compat_netbsd32_setpriority },	/* 96 = compat_netbsd32_setpriority */
	{ 3, s(struct compat_netbsd32_socket_args),
	    compat_netbsd32_socket },		/* 97 = compat_netbsd32_socket */
	{ 3, s(struct compat_netbsd32_connect_args),
	    compat_netbsd32_connect },		/* 98 = compat_netbsd32_connect */
	{ 3, s(struct compat_43_compat_netbsd32_oaccept_args),
	    compat_43(compat_netbsd32_oaccept) },/* 99 = compat_43 compat_netbsd32_oaccept */
	{ 2, s(struct compat_netbsd32_getpriority_args),
	    compat_netbsd32_getpriority },	/* 100 = compat_netbsd32_getpriority */
	{ 4, s(struct compat_43_compat_netbsd32_osend_args),
	    compat_43(compat_netbsd32_osend) },	/* 101 = compat_43 compat_netbsd32_osend */
	{ 4, s(struct compat_43_compat_netbsd32_orecv_args),
	    compat_43(compat_netbsd32_orecv) },	/* 102 = compat_43 compat_netbsd32_orecv */
	{ 1, s(struct compat_netbsd32_sigreturn_args),
	    compat_netbsd32_sigreturn },	/* 103 = compat_netbsd32_sigreturn */
	{ 3, s(struct compat_netbsd32_bind_args),
	    compat_netbsd32_bind },		/* 104 = compat_netbsd32_bind */
	{ 5, s(struct compat_netbsd32_setsockopt_args),
	    compat_netbsd32_setsockopt },	/* 105 = compat_netbsd32_setsockopt */
	{ 2, s(struct compat_netbsd32_listen_args),
	    compat_netbsd32_listen },		/* 106 = compat_netbsd32_listen */
	{ 0, 0,
	    sys_nosys },			/* 107 = obsolete vtimes */
	{ 3, s(struct compat_43_compat_netbsd32_osigvec_args),
	    compat_43(compat_netbsd32_osigvec) },/* 108 = compat_43 compat_netbsd32_osigvec */
#ifdef COMPAT_43
	{ 1, s(struct compat_43_compat_netbsd32_sigblock_args),
	    compat_43(compat_netbsd32_sigblock) },/* 109 = compat_43 compat_netbsd32_sigblock */
	{ 1, s(struct compat_43_compat_netbsd32_sigsetmask_args),
	    compat_43(compat_netbsd32_sigsetmask) },/* 110 = compat_43 compat_netbsd32_sigsetmask */
#else
	{ 0, 0,
	    sys_nosys },			/* 109 = obsolete sigblock */
	{ 0, 0,
	    sys_nosys },			/* 110 = obsolete sigsetmask */
#endif
	{ 1, s(struct compat_13_compat_netbsd32_sigsuspend_args),
	    compat_13(compat_netbsd32_sigsuspend) },/* 111 = compat_13 sigsuspend13 */
	{ 2, s(struct compat_43_compat_netbsd32_osigstack_args),
	    compat_43(compat_netbsd32_osigstack) },/* 112 = compat_43 compat_netbsd32_osigstack */
	{ 3, s(struct compat_43_compat_netbsd32_orecvmsg_args),
	    compat_43(compat_netbsd32_orecvmsg) },/* 113 = compat_43 compat_netbsd32_orecvmsg */
	{ 3, s(struct compat_43_compat_netbsd32_osendmsg_args),
	    compat_43(compat_netbsd32_osendmsg) },/* 114 = compat_43 compat_netbsd32_osendmsg */
#ifdef TRACE
	{ 2, s(struct compat_netbsd32_vtrace_args),
	    compat_netbsd32_vtrace },		/* 115 = compat_netbsd32_vtrace */
#else
	{ 0, 0,
	    sys_nosys },			/* 115 = obsolete vtrace */
#endif
	{ 2, s(struct compat_netbsd32_gettimeofday_args),
	    compat_netbsd32_gettimeofday },	/* 116 = compat_netbsd32_gettimeofday */
	{ 2, s(struct compat_netbsd32_getrusage_args),
	    compat_netbsd32_getrusage },	/* 117 = compat_netbsd32_getrusage */
	{ 5, s(struct compat_netbsd32_getsockopt_args),
	    compat_netbsd32_getsockopt },	/* 118 = compat_netbsd32_getsockopt */
	{ 0, 0,
	    sys_nosys },			/* 119 = obsolete resuba */
	{ 3, s(struct compat_netbsd32_readv_args),
	    compat_netbsd32_readv },		/* 120 = compat_netbsd32_readv */
	{ 3, s(struct compat_netbsd32_writev_args),
	    compat_netbsd32_writev },		/* 121 = compat_netbsd32_writev */
	{ 2, s(struct compat_netbsd32_settimeofday_args),
	    compat_netbsd32_settimeofday },	/* 122 = compat_netbsd32_settimeofday */
	{ 3, s(struct compat_netbsd32_fchown_args),
	    compat_netbsd32_fchown },		/* 123 = compat_netbsd32_fchown */
	{ 2, s(struct compat_netbsd32_fchmod_args),
	    compat_netbsd32_fchmod },		/* 124 = compat_netbsd32_fchmod */
	{ 6, s(struct compat_43_compat_netbsd32_orecvfrom_args),
	    compat_43(compat_netbsd32_orecvfrom) },/* 125 = compat_43 compat_netbsd32_orecvfrom */
	{ 2, s(struct compat_netbsd32_setreuid_args),
	    compat_netbsd32_setreuid },		/* 126 = compat_netbsd32_setreuid */
	{ 2, s(struct compat_netbsd32_setregid_args),
	    compat_netbsd32_setregid },		/* 127 = compat_netbsd32_setregid */
	{ 2, s(struct compat_netbsd32_rename_args),
	    compat_netbsd32_rename },		/* 128 = compat_netbsd32_rename */
	{ 2, s(struct compat_43_compat_netbsd32_otruncate_args),
	    compat_43(compat_netbsd32_otruncate) },/* 129 = compat_43 compat_netbsd32_otruncate */
	{ 2, s(struct compat_43_compat_netbsd32_oftruncate_args),
	    compat_43(compat_netbsd32_oftruncate) },/* 130 = compat_43 compat_netbsd32_oftruncate */
	{ 2, s(struct compat_netbsd32_flock_args),
	    compat_netbsd32_flock },		/* 131 = compat_netbsd32_flock */
	{ 2, s(struct compat_netbsd32_mkfifo_args),
	    compat_netbsd32_mkfifo },		/* 132 = compat_netbsd32_mkfifo */
	{ 6, s(struct compat_netbsd32_sendto_args),
	    compat_netbsd32_sendto },		/* 133 = compat_netbsd32_sendto */
	{ 2, s(struct compat_netbsd32_shutdown_args),
	    compat_netbsd32_shutdown },		/* 134 = compat_netbsd32_shutdown */
	{ 4, s(struct compat_netbsd32_socketpair_args),
	    compat_netbsd32_socketpair },	/* 135 = compat_netbsd32_socketpair */
	{ 2, s(struct compat_netbsd32_mkdir_args),
	    compat_netbsd32_mkdir },		/* 136 = compat_netbsd32_mkdir */
	{ 1, s(struct compat_netbsd32_rmdir_args),
	    compat_netbsd32_rmdir },		/* 137 = compat_netbsd32_rmdir */
	{ 2, s(struct compat_netbsd32_utimes_args),
	    compat_netbsd32_utimes },		/* 138 = compat_netbsd32_utimes */
	{ 0, 0,
	    sys_nosys },			/* 139 = obsolete 4.2 sigreturn */
	{ 2, s(struct compat_netbsd32_adjtime_args),
	    compat_netbsd32_adjtime },		/* 140 = compat_netbsd32_adjtime */
	{ 3, s(struct compat_43_compat_netbsd32_ogetpeername_args),
	    compat_43(compat_netbsd32_ogetpeername) },/* 141 = compat_43 compat_netbsd32_ogetpeername */
	{ 0, 0,
	    compat_43(sys_gethostid) },		/* 142 = compat_43 ogethostid */
#ifdef COMPAT_43
	{ 1, s(struct compat_43_compat_netbsd32_sethostid_args),
	    compat_43(compat_netbsd32_sethostid) },/* 143 = compat_43 compat_netbsd32_sethostid */
#else
	{ 0, 0,
	    sys_nosys },			/* 143 = obsolete sethostid */
#endif
	{ 2, s(struct compat_43_compat_netbsd32_ogetrlimit_args),
	    compat_43(compat_netbsd32_ogetrlimit) },/* 144 = compat_43 compat_netbsd32_ogetrlimit */
	{ 2, s(struct compat_43_compat_netbsd32_osetrlimit_args),
	    compat_43(compat_netbsd32_osetrlimit) },/* 145 = compat_43 compat_netbsd32_osetrlimit */
#ifdef COMPAT_43
	{ 2, s(struct compat_43_compat_netbsd32_killpg_args),
	    compat_43(compat_netbsd32_killpg) },/* 146 = compat_43 compat_netbsd32_killpg */
#else
	{ 0, 0,
	    sys_nosys },			/* 146 = obsolete killpg */
#endif
	{ 0, 0,
	    sys_setsid },			/* 147 = setsid */
	{ 4, s(struct compat_netbsd32_quotactl_args),
	    compat_netbsd32_quotactl },		/* 148 = compat_netbsd32_quotactl */
	{ 0, 0,
	    compat_43(sys_quota) },		/* 149 = compat_43 oquota */
	{ 3, s(struct compat_43_compat_netbsd32_ogetsockname_args),
	    compat_43(compat_netbsd32_ogetsockname) },/* 150 = compat_43 compat_netbsd32_ogetsockname */
	{ 0, 0,
	    sys_nosys },			/* 151 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 152 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 153 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 154 = unimplemented */
#if defined(NFS) || defined(NFSSERVER)
	{ 2, s(struct compat_netbsd32_nfssvc_args),
	    compat_netbsd32_nfssvc },		/* 155 = compat_netbsd32_nfssvc */
#else
	{ 0, 0,
	    sys_nosys },			/* 155 = unimplemented */
#endif
	{ 4, s(struct compat_43_compat_netbsd32_ogetdirentries_args),
	    compat_43(compat_netbsd32_ogetdirentries) },/* 156 = compat_43 compat_netbsd32_ogetdirentries */
	{ 2, s(struct compat_netbsd32_statfs_args),
	    compat_netbsd32_statfs },		/* 157 = compat_netbsd32_statfs */
	{ 2, s(struct compat_netbsd32_fstatfs_args),
	    compat_netbsd32_fstatfs },		/* 158 = compat_netbsd32_fstatfs */
	{ 0, 0,
	    sys_nosys },			/* 159 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 160 = unimplemented */
#if defined(NFS) || defined(NFSSERVER)
	{ 2, s(struct compat_netbsd32_getfh_args),
	    compat_netbsd32_getfh },		/* 161 = compat_netbsd32_getfh */
#else
	{ 0, 0,
	    sys_nosys },			/* 161 = unimplemented compat_netbsd32_getfh */
#endif
	{ 2, s(struct compat_09_compat_netbsd32_ogetdomainname_args),
	    compat_09(compat_netbsd32_ogetdomainname) },/* 162 = compat_09 compat_netbsd32_ogetdomainname */
	{ 2, s(struct compat_09_compat_netbsd32_osetdomainname_args),
	    compat_09(compat_netbsd32_osetdomainname) },/* 163 = compat_09 compat_netbsd32_osetdomainname */
	{ 1, s(struct compat_09_compat_netbsd32_uname_args),
	    compat_09(compat_netbsd32_uname) },	/* 164 = compat_09 compat_netbsd32_uname */
	{ 2, s(struct compat_netbsd32_sysarch_args),
	    compat_netbsd32_sysarch },		/* 165 = compat_netbsd32_sysarch */
	{ 0, 0,
	    sys_nosys },			/* 166 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 167 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 168 = unimplemented */
#if defined(SYSVSEM) && !defined(alpha) && defined(COMPAT_10)
	{ 5, s(struct compat_netbsd32_compat_10_sys_semsys_args),
	    compat_netbsd32_compat_10_sys_semsys },/* 169 = osemsys */
#else
	{ 0, 0,
	    sys_nosys },			/* 169 = unimplemented 1.0 semsys */
#endif
#if defined(SYSVMSG) && !defined(alpha) && defined(COMPAT_10)
	{ 6, s(struct compat_netbsd32_compat_10_sys_msgsys_args),
	    compat_netbsd32_compat_10_sys_msgsys },/* 170 = omsgsys */
#else
	{ 0, 0,
	    sys_nosys },			/* 170 = unimplemented 1.0 msgsys */
#endif
#if defined(SYSVSHM) && !defined(alpha) && defined(COMPAT_10)
	{ 4, s(struct compat_netbsd32_compat_10_sys_shmsys_args),
	    compat_netbsd32_compat_10_sys_shmsys },/* 171 = oshmsys */
#else
	{ 0, 0,
	    sys_nosys },			/* 171 = unimplemented 1.0 shmsys */
#endif
	{ 0, 0,
	    sys_nosys },			/* 172 = unimplemented */
	{ 5, s(struct compat_netbsd32_pread_args),
	    compat_netbsd32_pread },		/* 173 = compat_netbsd32_pread */
	{ 5, s(struct compat_netbsd32_pwrite_args),
	    compat_netbsd32_pwrite },		/* 174 = compat_netbsd32_pwrite */
#ifdef NTP
	{ 1, s(struct compat_netbsd32_ntp_gettime_args),
	    compat_netbsd32_ntp_gettime },	/* 175 = compat_netbsd32_ntp_gettime */
	{ 1, s(struct compat_netbsd32_ntp_adjtime_args),
	    compat_netbsd32_ntp_adjtime },	/* 176 = compat_netbsd32_ntp_adjtime */
#else
	{ 0, 0,
	    sys_nosys },			/* 175 = unimplemented compat_netbsd32_ntp_gettime */
	{ 0, 0,
	    sys_nosys },			/* 176 = unimplemented compat_netbsd32_ntp_adjtime */
#endif
	{ 0, 0,
	    sys_nosys },			/* 177 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 178 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 179 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 180 = unimplemented */
	{ 1, s(struct compat_netbsd32_setgid_args),
	    compat_netbsd32_setgid },		/* 181 = compat_netbsd32_setgid */
	{ 1, s(struct compat_netbsd32_setegid_args),
	    compat_netbsd32_setegid },		/* 182 = compat_netbsd32_setegid */
	{ 1, s(struct compat_netbsd32_seteuid_args),
	    compat_netbsd32_seteuid },		/* 183 = compat_netbsd32_seteuid */
#ifdef LFS
	{ 3, s(struct compat_netbsd32_lfs_bmapv_args),
	    compat_netbsd32_lfs_bmapv },	/* 184 = compat_netbsd32_lfs_bmapv */
	{ 3, s(struct compat_netbsd32_lfs_markv_args),
	    compat_netbsd32_lfs_markv },	/* 185 = compat_netbsd32_lfs_markv */
	{ 2, s(struct compat_netbsd32_lfs_segclean_args),
	    compat_netbsd32_lfs_segclean },	/* 186 = compat_netbsd32_lfs_segclean */
	{ 2, s(struct compat_netbsd32_lfs_segwait_args),
	    compat_netbsd32_lfs_segwait },	/* 187 = compat_netbsd32_lfs_segwait */
#else
	{ 0, 0,
	    sys_nosys },			/* 184 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 185 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 186 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 187 = unimplemented */
#endif
	{ 2, s(struct compat_12_compat_netbsd32_stat12_args),
	    compat_12(compat_netbsd32_stat12) },/* 188 = compat_12 compat_netbsd32_stat12 */
	{ 2, s(struct compat_12_compat_netbsd32_fstat12_args),
	    compat_12(compat_netbsd32_fstat12) },/* 189 = compat_12 compat_netbsd32_fstat12 */
	{ 2, s(struct compat_12_compat_netbsd32_lstat12_args),
	    compat_12(compat_netbsd32_lstat12) },/* 190 = compat_12 compat_netbsd32_lstat12 */
	{ 2, s(struct compat_netbsd32_pathconf_args),
	    compat_netbsd32_pathconf },		/* 191 = compat_netbsd32_pathconf */
	{ 2, s(struct compat_netbsd32_fpathconf_args),
	    compat_netbsd32_fpathconf },	/* 192 = compat_netbsd32_fpathconf */
	{ 0, 0,
	    sys_nosys },			/* 193 = unimplemented */
	{ 2, s(struct compat_netbsd32_getrlimit_args),
	    compat_netbsd32_getrlimit },	/* 194 = compat_netbsd32_getrlimit */
	{ 2, s(struct compat_netbsd32_setrlimit_args),
	    compat_netbsd32_setrlimit },	/* 195 = compat_netbsd32_setrlimit */
	{ 4, s(struct compat_12_compat_netbsd32_getdirentries_args),
	    compat_12(compat_netbsd32_getdirentries) },/* 196 = compat_12 compat_netbsd32_getdirentries */
	{ 7, s(struct compat_netbsd32_mmap_args),
	    compat_netbsd32_mmap },		/* 197 = compat_netbsd32_mmap */
	{ 0, 0,
	    sys_nosys },			/* 198 = __syscall (indir) */
	{ 4, s(struct compat_netbsd32_lseek_args),
	    compat_netbsd32_lseek },		/* 199 = compat_netbsd32_lseek */
	{ 3, s(struct compat_netbsd32_truncate_args),
	    compat_netbsd32_truncate },		/* 200 = compat_netbsd32_truncate */
	{ 3, s(struct compat_netbsd32_ftruncate_args),
	    compat_netbsd32_ftruncate },	/* 201 = compat_netbsd32_ftruncate */
	{ 6, s(struct compat_netbsd32___sysctl_args),
	    compat_netbsd32___sysctl },		/* 202 = compat_netbsd32___sysctl */
	{ 2, s(struct compat_netbsd32_mlock_args),
	    compat_netbsd32_mlock },		/* 203 = compat_netbsd32_mlock */
	{ 2, s(struct compat_netbsd32_munlock_args),
	    compat_netbsd32_munlock },		/* 204 = compat_netbsd32_munlock */
	{ 1, s(struct compat_netbsd32_undelete_args),
	    compat_netbsd32_undelete },		/* 205 = compat_netbsd32_undelete */
	{ 2, s(struct compat_netbsd32_futimes_args),
	    compat_netbsd32_futimes },		/* 206 = compat_netbsd32_futimes */
	{ 1, s(struct compat_netbsd32_getpgid_args),
	    compat_netbsd32_getpgid },		/* 207 = compat_netbsd32_getpgid */
	{ 2, s(struct compat_netbsd32_reboot_args),
	    compat_netbsd32_reboot },		/* 208 = compat_netbsd32_reboot */
	{ 3, s(struct compat_netbsd32_poll_args),
	    compat_netbsd32_poll },		/* 209 = compat_netbsd32_poll */
#ifdef LKM
	{ 0, 0,
	    sys_lkmnosys },			/* 210 = lkmnosys */
	{ 0, 0,
	    sys_lkmnosys },			/* 211 = lkmnosys */
	{ 0, 0,
	    sys_lkmnosys },			/* 212 = lkmnosys */
	{ 0, 0,
	    sys_lkmnosys },			/* 213 = lkmnosys */
	{ 0, 0,
	    sys_lkmnosys },			/* 214 = lkmnosys */
	{ 0, 0,
	    sys_lkmnosys },			/* 215 = lkmnosys */
	{ 0, 0,
	    sys_lkmnosys },			/* 216 = lkmnosys */
	{ 0, 0,
	    sys_lkmnosys },			/* 217 = lkmnosys */
	{ 0, 0,
	    sys_lkmnosys },			/* 218 = lkmnosys */
	{ 0, 0,
	    sys_lkmnosys },			/* 219 = lkmnosys */
#else	/* !LKM */
	{ 0, 0,
	    sys_nosys },			/* 210 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 211 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 212 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 213 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 214 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 215 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 216 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 217 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 218 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 219 = unimplemented */
#endif	/* !LKM */
#ifdef SYSVSEM
	{ 4, s(struct compat_netbsd32___semctl_args),
	    compat_netbsd32___semctl },		/* 220 = compat_netbsd32___semctl */
	{ 3, s(struct compat_netbsd32_semget_args),
	    compat_netbsd32_semget },		/* 221 = compat_netbsd32_semget */
	{ 3, s(struct compat_netbsd32_semop_args),
	    compat_netbsd32_semop },		/* 222 = compat_netbsd32_semop */
	{ 1, s(struct compat_netbsd32_semconfig_args),
	    compat_netbsd32_semconfig },	/* 223 = compat_netbsd32_semconfig */
#else
	{ 0, 0,
	    sys_nosys },			/* 220 = unimplemented compat_netbsd32_semctl */
	{ 0, 0,
	    sys_nosys },			/* 221 = unimplemented compat_netbsd32_semget */
	{ 0, 0,
	    sys_nosys },			/* 222 = unimplemented compat_netbsd32_semop */
	{ 0, 0,
	    sys_nosys },			/* 223 = unimplemented compat_netbsd32_semconfig */
#endif
#ifdef SYSVMSG
	{ 3, s(struct compat_netbsd32_msgctl_args),
	    compat_netbsd32_msgctl },		/* 224 = compat_netbsd32_msgctl */
	{ 2, s(struct compat_netbsd32_msgget_args),
	    compat_netbsd32_msgget },		/* 225 = compat_netbsd32_msgget */
	{ 4, s(struct compat_netbsd32_msgsnd_args),
	    compat_netbsd32_msgsnd },		/* 226 = compat_netbsd32_msgsnd */
	{ 5, s(struct compat_netbsd32_msgrcv_args),
	    compat_netbsd32_msgrcv },		/* 227 = compat_netbsd32_msgrcv */
#else
	{ 0, 0,
	    sys_nosys },			/* 224 = unimplemented compat_netbsd32_msgctl */
	{ 0, 0,
	    sys_nosys },			/* 225 = unimplemented compat_netbsd32_msgget */
	{ 0, 0,
	    sys_nosys },			/* 226 = unimplemented compat_netbsd32_msgsnd */
	{ 0, 0,
	    sys_nosys },			/* 227 = unimplemented compat_netbsd32_msgrcv */
#endif
#ifdef SYSVSHM
	{ 3, s(struct compat_netbsd32_shmat_args),
	    compat_netbsd32_shmat },		/* 228 = compat_netbsd32_shmat */
	{ 3, s(struct compat_netbsd32_shmctl_args),
	    compat_netbsd32_shmctl },		/* 229 = compat_netbsd32_shmctl */
	{ 1, s(struct compat_netbsd32_shmdt_args),
	    compat_netbsd32_shmdt },		/* 230 = compat_netbsd32_shmdt */
	{ 3, s(struct compat_netbsd32_shmget_args),
	    compat_netbsd32_shmget },		/* 231 = compat_netbsd32_shmget */
#else
	{ 0, 0,
	    sys_nosys },			/* 228 = unimplemented compat_netbsd32_shmat */
	{ 0, 0,
	    sys_nosys },			/* 229 = unimplemented compat_netbsd32_shmctl */
	{ 0, 0,
	    sys_nosys },			/* 230 = unimplemented compat_netbsd32_shmdt */
	{ 0, 0,
	    sys_nosys },			/* 231 = unimplemented compat_netbsd32_shmget */
#endif
	{ 2, s(struct compat_netbsd32_clock_gettime_args),
	    compat_netbsd32_clock_gettime },	/* 232 = compat_netbsd32_clock_gettime */
	{ 2, s(struct compat_netbsd32_clock_settime_args),
	    compat_netbsd32_clock_settime },	/* 233 = compat_netbsd32_clock_settime */
	{ 2, s(struct compat_netbsd32_clock_getres_args),
	    compat_netbsd32_clock_getres },	/* 234 = compat_netbsd32_clock_getres */
	{ 0, 0,
	    sys_nosys },			/* 235 = unimplemented timer_create */
	{ 0, 0,
	    sys_nosys },			/* 236 = unimplemented timer_delete */
	{ 0, 0,
	    sys_nosys },			/* 237 = unimplemented timer_settime */
	{ 0, 0,
	    sys_nosys },			/* 238 = unimplemented timer_gettime */
	{ 0, 0,
	    sys_nosys },			/* 239 = unimplemented timer_getoverrun */
	{ 2, s(struct compat_netbsd32_nanosleep_args),
	    compat_netbsd32_nanosleep },	/* 240 = compat_netbsd32_nanosleep */
	{ 1, s(struct compat_netbsd32_fdatasync_args),
	    compat_netbsd32_fdatasync },	/* 241 = compat_netbsd32_fdatasync */
	{ 0, 0,
	    sys_nosys },			/* 242 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 243 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 244 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 245 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 246 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 247 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 248 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 249 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 250 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 251 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 252 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 253 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 254 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 255 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 256 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 257 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 258 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 259 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 260 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 261 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 262 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 263 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 264 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 265 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 266 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 267 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 268 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 269 = unimplemented */
	{ 2, s(struct compat_netbsd32___posix_rename_args),
	    compat_netbsd32___posix_rename },	/* 270 = compat_netbsd32___posix_rename */
	{ 3, s(struct compat_netbsd32_swapctl_args),
	    compat_netbsd32_swapctl },		/* 271 = compat_netbsd32_swapctl */
	{ 3, s(struct compat_netbsd32_getdents_args),
	    compat_netbsd32_getdents },		/* 272 = compat_netbsd32_getdents */
	{ 3, s(struct compat_netbsd32_minherit_args),
	    compat_netbsd32_minherit },		/* 273 = compat_netbsd32_minherit */
	{ 2, s(struct compat_netbsd32_lchmod_args),
	    compat_netbsd32_lchmod },		/* 274 = compat_netbsd32_lchmod */
	{ 3, s(struct compat_netbsd32_lchown_args),
	    compat_netbsd32_lchown },		/* 275 = compat_netbsd32_lchown */
	{ 2, s(struct compat_netbsd32_lutimes_args),
	    compat_netbsd32_lutimes },		/* 276 = compat_netbsd32_lutimes */
	{ 3, s(struct compat_netbsd32___msync13_args),
	    compat_netbsd32___msync13 },	/* 277 = compat_netbsd32___msync13 */
	{ 2, s(struct compat_netbsd32___stat13_args),
	    compat_netbsd32___stat13 },		/* 278 = compat_netbsd32___stat13 */
	{ 2, s(struct compat_netbsd32___fstat13_args),
	    compat_netbsd32___fstat13 },	/* 279 = compat_netbsd32___fstat13 */
	{ 2, s(struct compat_netbsd32___lstat13_args),
	    compat_netbsd32___lstat13 },	/* 280 = compat_netbsd32___lstat13 */
	{ 2, s(struct compat_netbsd32___sigaltstack14_args),
	    compat_netbsd32___sigaltstack14 },	/* 281 = compat_netbsd32___sigaltstack14 */
	{ 0, 0,
	    sys___vfork14 },			/* 282 = __vfork14 */
	{ 3, s(struct compat_netbsd32___posix_chown_args),
	    compat_netbsd32___posix_chown },	/* 283 = compat_netbsd32___posix_chown */
	{ 3, s(struct compat_netbsd32___posix_fchown_args),
	    compat_netbsd32___posix_fchown },	/* 284 = compat_netbsd32___posix_fchown */
	{ 3, s(struct compat_netbsd32___posix_lchown_args),
	    compat_netbsd32___posix_lchown },	/* 285 = compat_netbsd32___posix_lchown */
	{ 1, s(struct compat_netbsd32_getsid_args),
	    compat_netbsd32_getsid },		/* 286 = compat_netbsd32_getsid */
	{ 0, 0,
	    sys_nosys },			/* 287 = unimplemented */
#ifdef KTRACE
	{ 4, s(struct compat_netbsd32_fktrace_args),
	    compat_netbsd32_fktrace },		/* 288 = compat_netbsd32_fktrace */
#else
	{ 0, 0,
	    sys_nosys },			/* 288 = unimplemented */
#endif
	{ 5, s(struct compat_netbsd32_preadv_args),
	    compat_netbsd32_preadv },		/* 289 = compat_netbsd32_preadv */
	{ 5, s(struct compat_netbsd32_pwritev_args),
	    compat_netbsd32_pwritev },		/* 290 = compat_netbsd32_pwritev */
};

