/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    w32_accept((fd), (addr), (addrlen))
#define bind(fd, name, namelen) \
    w32_bind((fd), (name), (namelen))
#define connect(fd, addr, addrlen) \
    w32_connect((fd), (addr), (addrlen))
#define listen(fd, backlog) \
    w32_listen((fd), (backlog))
#define setsockopt(fd, level, optname, optval, optlen) \
    w32_setsockopt((fd), (level), (optname), (optval), (optlen))
#define shutdown(fd, how) \
    w32_shutdown((fd), (how))
#define socket(domain, type, protocol) \
    w32_socket((domain), (type), (protocol))

extern int w32_accept(int fd, struct sockaddr *addr, socklen_t *addrlen);
extern int w32_bind(int fd, const struct sockaddr *name, socklen_t namelen);
extern int w32_connect(int fd, const struct sockaddr *addr, socklen_t addrlen);
extern int w32_listen(int fd, int backlog);
extern int w32_setsockopt(int fd, int level, int optname,
			  const void *optval, socklen_t optlen);
extern int w32_shutdown(int fd, int how);
extern int w32_socket(int domain, int type, int protocol);

/* Low-level stuff specific to the emulation */
extern SOCKET w32_fd2socket(int fd);
extern void w32_set_winsock_errno(void);
extern int w32_socket_init(void);

#endif /* SYS_SOCKET_H */
