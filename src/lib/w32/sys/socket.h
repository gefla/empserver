/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sys/socket.h: POSIX socket emulation for WIN32
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 */

#ifndef SYS_SOCKET_H
#define SYS_SOCKET_H

#include <winsock2.h>
#undef NS_ALL

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

extern int posix_accept(int fd, struct sockaddr *addr, socklen_t *addrlen);
extern int posix_bind(int fd, const struct sockaddr *name, socklen_t namelen);
extern int posix_listen(int fd, int backlog);
extern int posix_setsockopt(int fd, int level, int optname,
		      const void *optval, socklen_t optlen);
extern int posix_shutdown(int fd, int how);
extern int posix_socket(int domain, int type, int protocol);

/* Low-level stuff specific to the emulation */
extern SOCKET posix_fd2socket(int fd);
extern void w32_set_winsock_errno(void);
extern int w32_socket_init(void);

#endif /* SYS_SOCKET_H */
