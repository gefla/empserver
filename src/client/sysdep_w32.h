/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sysdep_w32.h: system dependent support for WIN32 environments
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 */

#ifndef _SYSDEF_W32_H
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <signal.h>

extern int getopt(int, char * const[], const char *);
extern char *optarg;
extern int optind, opterr, optopt;

#ifdef _MSC_VER
typedef int __w64 ssize_t;
#endif

struct passwd {
    char *pw_name;
};

struct iovec {
    void *iov_base;
    size_t iov_len;
};

struct sigaction {
    int sa_flags;
    void (*sa_handler)(int sig);
};

extern int w32_recv(int socket, char *buffer,
			   size_t buf_size, int flags);
extern int w32_send(int socket, char *buffer,
			    size_t buf_size, int flags);
extern int w32_close_socket(int fd);
extern int w32_socket(int family, int sock_type, int protocol);
extern int w32_connect(int sock, struct sockaddr *addr, int addrlen);
extern int w32_close_handle(int fd);
extern ssize_t w32_readv_handle(int fd, const struct iovec *iov,
				int iovcnt);
extern ssize_t w32_writev_socket(int fd, const struct iovec *iov,
				 int iovcnt);
extern int w32_openfd(const char *fname, int oflag, ...);
extern int w32_openhandle(const char *fname, int oflag);

extern struct passwd *w32_getpw(void);
extern char *getpass(char *prompt);
extern void sysdep_init(void);

#define recv(sock, buffer, buf_size, flags) \
    w32_recv((sock), (buffer), (buf_size), (flags))
#define socket(family, sock_type, protocol) \
    w32_socket((family), (sock_type), (protocol))
#define connect(sock, addr, addrlen) \
    w32_connect((sock), (addr), (addrlen))

#define pclose _pclose
#define popen _popen
#define snprintf _snprintf
#define getsose() ((void)0)
#define putso() ((void)0)
#define putse() ((void)0)
#endif /* sysdef_w32.h */
