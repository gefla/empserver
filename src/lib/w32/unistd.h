/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  unistd.h: POSIX emulation for WIN32
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 */

#ifndef UNISTD_H
#define UNISTD_H

#include <io.h>
#include <stdio.h>
#undef fileno
#include <direct.h>
#include "sys/socket.h"
#include <sys/stat.h>
#include <ws2tcpip.h>

#include "w32misc.h"

/*
 * getopt.c
 */
extern int getopt(int, char * const[], const char *);
extern char *optarg;
extern int optind, opterr, optopt;

/*
 * posixfile.c
 */
#define mkdir(dir, perm)    posix_mkdir((dir), (perm))

extern int posix_mkdir(const char *dirname, mode_t perm);

/*
 * posixio.c
 */
#ifndef S_IRUSR
#define S_IRUSR	    _S_IREAD
#define S_IWUSR	    _S_IWRITE
#define S_IXUSR	    _S_IEXEC
#define S_IRWXU	    S_IRUSR | S_IWUSR | S_IXUSR
#endif
#ifndef S_IRGRP
#define S_IRGRP	    0
#define S_IWGRP	    0
#define S_IXGRP	    0
#define S_IRWXG	    S_IRGRP | S_IWGRP | S_IXGRP
#endif
#ifndef S_IROTH
#define S_IROTH	    0
#define S_IWOTH	    0
#define S_IXOTH	    0
#define S_IRWXO	    S_IROTH | S_IWOTH | S_IXOTH
#endif

typedef int socklen_t;

#define accept(fd, addr, addrlen) \
    posix_accept((fd), (addr), (addrlen))
#define bind(fd, name, namelen) \
    posix_bind((fd), (name), (namelen))
#define listen(fd, backlog) \
    posix_listen((fd), (backlog))
#define setsockopt(fd, level, optname, optval, optlen) \
    posix_setsockopt((fd), (level), (optname), (optval), (optlen))
#define shutdown(fd, how) \
    posix_shutdown((fd), (how))
#define socket(domain, type, protocol) \
    posix_socket((domain), (type), (protocol))

#define close(fd) \
    posix_close((fd))
#define creat(fname, pmode) \
    posix_open((fname), _O_WRONLY | _O_CREAT |_O_TRUNC, (pmode))
#define fstat(fd, buffer) \
    posix_fstat((fd), (buffer))
#define lseek(fd, offset, origin) \
    posix_lseek((fd), (offset), (origin))
#define open(fname, oflag, ...) \
    posix_open((fname), (oflag), __VA_ARGS__)
#define read	posix_read
#define write(fd, buffer, count) \
    posix_write((fd), (buffer), (count))
#define fileno(stream) \
    posix_fileno((stream))
#define fsync(fd) \
    posix_fsync((fd))

#define O_NONBLOCK  1
#define F_RDLCK	    0
#define F_WRLCK	    1
#define F_GETFL	    1
#define F_SETFL	    2
#define F_SETLK	    3
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ENOTSOCK    WSAENOTSOCK

struct flock
{
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    /* intentionally missing: pid_t l_pid */
};

extern int posix_fd2socket(int fd);

extern int posix_accept(int fd, struct sockaddr *addr, socklen_t *addrlen);
extern int posix_bind(int fd, const struct sockaddr *name, socklen_t namelen);
extern int posix_listen(int fd, int backlog);
extern int posix_setsockopt(int fd, int level, int optname,
		      const void *optval, socklen_t optlen);
extern int posix_shutdown(int fd, int how);
extern int posix_socket(int domain, int type, int protocol);
extern const char *inet_ntop(int af, const void *src, char *dst,
			     socklen_t cnt);

extern int posix_close(int fd);
extern int posix_fstat(int fd, struct stat *buffer);
extern off_t posix_lseek(int fd, off_t offset, int origin);
extern int posix_open(const char *fname, int oflag, ...);
extern ssize_t posix_read(int fd, void *buffer, size_t count);
extern ssize_t posix_write(int fd, const void *buffer, size_t count);

extern int posix_fileno(FILE *stream);
extern int posix_fsync(int fd);
extern int fcntl(int fd, int cmd, ...);
#endif /* UNISTD_H */
