/*
 * System call numbers.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	OpenBSD: syscalls.master,v 1.10 1998/03/23 07:12:40 millert Exp 
 */

#define	SUNOS_SYS_syscall	0
#define	SUNOS_SYS_exit	1
#define	SUNOS_SYS_fork	2
#define	SUNOS_SYS_read	3
#define	SUNOS_SYS_write	4
#define	SUNOS_SYS_open	5
#define	SUNOS_SYS_close	6
#define	SUNOS_SYS_wait4	7
#define	SUNOS_SYS_creat	8
#define	SUNOS_SYS_link	9
#define	SUNOS_SYS_unlink	10
#define	SUNOS_SYS_execv	11
#define	SUNOS_SYS_chdir	12
				/* 13 is obsolete time */
#define	SUNOS_SYS_mknod	14
#define	SUNOS_SYS_chmod	15
#define	SUNOS_SYS_lchown	16
#define	SUNOS_SYS_break	17
				/* 18 is obsolete stat */
#define	SUNOS_SYS_lseek	19
#define	SUNOS_SYS_getpid	20
				/* 21 is obsolete sunos_old_mount */
#define	SUNOS_SYS_setuid	23
#define	SUNOS_SYS_getuid	24
#define	SUNOS_SYS_ostime	25
#define	SUNOS_SYS_ptrace	26
#define	SUNOS_SYS_access	33
#define	SUNOS_SYS_sync	36
#define	SUNOS_SYS_kill	37
#define	SUNOS_SYS_stat	38
#define	SUNOS_SYS_lstat	40
#define	SUNOS_SYS_dup	41
#define	SUNOS_SYS_pipe	42
#define	SUNOS_SYS_otimes	43
#define	SUNOS_SYS_profil	44
#define	SUNOS_SYS_setgid	46
#define	SUNOS_SYS_getgid	47
#define	SUNOS_SYS_acct	51
#define	SUNOS_SYS_mctl	53
#define	SUNOS_SYS_ioctl	54
#define	SUNOS_SYS_reboot	55
				/* 56 is obsolete sunos_owait3 */
#define	SUNOS_SYS_symlink	57
#define	SUNOS_SYS_readlink	58
#define	SUNOS_SYS_execve	59
#define	SUNOS_SYS_umask	60
#define	SUNOS_SYS_chroot	61
#define	SUNOS_SYS_fstat	62
#define	SUNOS_SYS_getpagesize	64
#define	SUNOS_SYS_msync	65
#define	SUNOS_SYS_vfork	66
				/* 67 is obsolete vread */
				/* 68 is obsolete vwrite */
#define	SUNOS_SYS_sbrk	69
#define	SUNOS_SYS_sstk	70
#define	SUNOS_SYS_mmap	71
#define	SUNOS_SYS_vadvise	72
#define	SUNOS_SYS_munmap	73
#define	SUNOS_SYS_mprotect	74
#define	SUNOS_SYS_madvise	75
#define	SUNOS_SYS_vhangup	76
#define	SUNOS_SYS_mincore	78
#define	SUNOS_SYS_getgroups	79
#define	SUNOS_SYS_setgroups	80
#define	SUNOS_SYS_getpgrp	81
#define	SUNOS_SYS_setpgrp	82
#define	SUNOS_SYS_setitimer	83
#define	SUNOS_SYS_swapon	85
#define	SUNOS_SYS_getitimer	86
#define	SUNOS_SYS_gethostname	87
#define	SUNOS_SYS_sethostname	88
#define	SUNOS_SYS_getdtablesize	89
#define	SUNOS_SYS_dup2	90
#define	SUNOS_SYS_fcntl	92
#define	SUNOS_SYS_select	93
#define	SUNOS_SYS_fsync	95
#define	SUNOS_SYS_setpriority	96
#define	SUNOS_SYS_socket	97
#define	SUNOS_SYS_connect	98
#define	SUNOS_SYS_accept	99
#define	SUNOS_SYS_getpriority	100
#define	SUNOS_SYS_send	101
#define	SUNOS_SYS_recv	102
#define	SUNOS_SYS_bind	104
#define	SUNOS_SYS_setsockopt	105
#define	SUNOS_SYS_listen	106
#define	SUNOS_SYS_sigvec	108
#define	SUNOS_SYS_sigblock	109
#define	SUNOS_SYS_sigsetmask	110
#define	SUNOS_SYS_sigsuspend	111
#define	SUNOS_SYS_sigstack	112
#define	SUNOS_SYS_recvmsg	113
#define	SUNOS_SYS_sendmsg	114
				/* 115 is obsolete vtrace */
#define	SUNOS_SYS_gettimeofday	116
#define	SUNOS_SYS_getrusage	117
#define	SUNOS_SYS_getsockopt	118
#define	SUNOS_SYS_readv	120
#define	SUNOS_SYS_writev	121
#define	SUNOS_SYS_settimeofday	122
#define	SUNOS_SYS_fchown	123
#define	SUNOS_SYS_fchmod	124
#define	SUNOS_SYS_recvfrom	125
#define	SUNOS_SYS_setreuid	126
#define	SUNOS_SYS_setregid	127
#define	SUNOS_SYS_rename	128
#define	SUNOS_SYS_truncate	129
#define	SUNOS_SYS_ftruncate	130
#define	SUNOS_SYS_flock	131
#define	SUNOS_SYS_sendto	133
#define	SUNOS_SYS_shutdown	134
#define	SUNOS_SYS_socketpair	135
#define	SUNOS_SYS_mkdir	136
#define	SUNOS_SYS_rmdir	137
#define	SUNOS_SYS_utimes	138
#define	SUNOS_SYS_sigreturn	139
#define	SUNOS_SYS_adjtime	140
#define	SUNOS_SYS_getpeername	141
#define	SUNOS_SYS_gethostid	142
#define	SUNOS_SYS_getrlimit	144
#define	SUNOS_SYS_setrlimit	145
#define	SUNOS_SYS_killpg	146
#define	SUNOS_SYS_getsockname	150
#define	SUNOS_SYS_poll	153
#define	SUNOS_SYS_nfssvc	155
#define	SUNOS_SYS_getdirentries	156
#define	SUNOS_SYS_statfs	157
#define	SUNOS_SYS_fstatfs	158
#define	SUNOS_SYS_unmount	159
#define	SUNOS_SYS_async_daemon	160
#define	SUNOS_SYS_getfh	161
#define	SUNOS_SYS_getdomainname	162
#define	SUNOS_SYS_setdomainname	163
#define	SUNOS_SYS_quotactl	165
#define	SUNOS_SYS_exportfs	166
#define	SUNOS_SYS_mount	167
#define	SUNOS_SYS_ustat	168
#define	SUNOS_SYS_semsys	169
#define	SUNOS_SYS_msgsys	170
#define	SUNOS_SYS_shmsys	171
#define	SUNOS_SYS_auditsys	172
#define	SUNOS_SYS_getdents	174
#define	SUNOS_SYS_setsid	175
#define	SUNOS_SYS_fchdir	176
#define	SUNOS_SYS_fchroot	177
#define	SUNOS_SYS_sigpending	183
#define	SUNOS_SYS_setpgid	185
#define	SUNOS_SYS_pathconf	186
#define	SUNOS_SYS_fpathconf	187
#define	SUNOS_SYS_sysconf	188
#define	SUNOS_SYS_uname	189
#define	SUNOS_SYS_MAXSYSCALL	190
