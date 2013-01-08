/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  w32sockets.c: POSIX socket emulation layer for Windows
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007-2010
 *     Markus Armbruster, 2007-2010
 */

/*
 * POSIX sockets are file descriptors.  Windows sockets are something
 * else, with a separate set of functions to operate on them.  To
 * present a more POSIX-like interface to our application code, we
 * provide a compatibility layer that wraps file descriptors around
 * sockets.
 */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <io.h>
/*
 * Need to include winsock2.h before ws2tcpip.h.
 * Use sys/socket.h to ensure the #undef NS_ALL
 * is not missed after including winsock2.h.
 */
#include "sys/socket.h"
#include <sys/stat.h>
#include <ws2tcpip.h>
#include "unistd.h"

#define W32_FD_TO_SOCKET(fd) ((SOCKET)_get_osfhandle((fd)))
#define W32_SOCKET_TO_FD(fh) (_open_osfhandle((long)(fh), O_RDWR | O_BINARY))

SOCKET
w32_fd2socket(int fd)
{
    return W32_FD_TO_SOCKET(fd);
}

static int
fd_is_socket(int fd, SOCKET *sockp)
{
    SOCKET sock;
    BOOL val;
    int size = sizeof(val);

    sock = W32_FD_TO_SOCKET(fd);
    if (sockp)
	*sockp = sock;
    return getsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&val, &size)
	== 0;
}

void
w32_set_winsock_errno(void)
{
    int err = WSAGetLastError();
    WSASetLastError(0);

    /* Map some WSAE* errors to the runtime library's error codes.  */
    switch (err) {
    case WSA_INVALID_HANDLE:
	errno = EBADF;
	break;
    case WSA_NOT_ENOUGH_MEMORY:
	errno = ENOMEM;
	break;
    case WSA_INVALID_PARAMETER:
	errno = EINVAL;
	break;
    case WSAEWOULDBLOCK:
	errno = EAGAIN;
	break;
    case WSAENAMETOOLONG:
	errno = ENAMETOOLONG;
	break;
    case WSAENOTEMPTY:
	errno = ENOTEMPTY;
	break;
    default:
	errno = (err > 10000 && err < 10025) ? err - 10000 : err;
	break;
    }
}

#define SOCKET_FUNCTION(expr) do {		\
	SOCKET sock = W32_FD_TO_SOCKET(fd);	\
	int res = (expr);			\
	if (res == SOCKET_ERROR) {		\
	    w32_set_winsock_errno();		\
	    return -1;				\
	}					\
	return res;				\
    } while (0)

/*
 * POSIX equivalent for accept().
 */
#undef accept
int
w32_accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    SOCKET sock;

    sock = accept(W32_FD_TO_SOCKET(fd), addr, addrlen);
    if (sock == INVALID_SOCKET) {
	w32_set_winsock_errno();
	return -1;
    }

    return W32_SOCKET_TO_FD(sock);
}

/*
 * POSIX equivalent for bind().
 */
#undef bind
int
w32_bind(int fd, const struct sockaddr *name, socklen_t namelen)
{
    SOCKET_FUNCTION(bind(sock, name, namelen));
}

/*
 * POSIX compatible connect() replacement
 */
#undef connect
int
w32_connect(int sockfd, const struct sockaddr *addr, int addrlen)
{
    SOCKET sock = W32_FD_TO_SOCKET(sockfd);
    int result;

    result = connect(sock, addr, addrlen);
    if (result == SOCKET_ERROR) {
	/* FIXME map WSAEWOULDBLOCK to EINPROGRESS */
	w32_set_winsock_errno();
	return -1;
    }
    return result;
}

/*
 * POSIX equivalent for listen().
 */
#undef listen
int
w32_listen(int fd, int backlog)
{
    SOCKET_FUNCTION(listen(sock, backlog));
}

/*
 * POSIX equivalent for setsockopt().
 */
#undef setsockopt
int
w32_setsockopt(int fd, int level, int optname,
	       const void *optval, socklen_t optlen)
{
    /*
     * SO_REUSEADDR requests to permit another bind even when the
     * port is still in state TIME_WAIT.  Windows' SO_REUSEADDR is
     * broken: it makes bind() succeed no matter what, even if
     * there's another server running on the same port.  Luckily,
     * bind() seems to be broken as well: it seems to succeed while
     * the port is in state TIME_WAIT by default; thus we get the
     * behavior we want by not setting SO_REUSEADDR.
     */
    if (level == SOL_SOCKET && optname == SO_REUSEADDR)
	return 0;
    {
	SOCKET_FUNCTION(setsockopt(sock, level, optname, optval, optlen));
    }
}

/*
 * POSIX equivalent for shutdown().
 */
#undef shutdown
int
w32_shutdown(int fd, int how)
{
    SOCKET_FUNCTION(shutdown(sock, how));
}

/*
 * POSIX equivalent for socket().
 */
#undef socket
int
w32_socket(int domain, int type, int protocol)
{
    SOCKET sock;

    /*
     * We have to use WSASocket() to create non-overlapped IO sockets.
     * Overlapped IO sockets cannot be used with read/write.
     */
    sock = WSASocket(domain, type, protocol, NULL, 0, 0);
    if (sock == INVALID_SOCKET) {
	w32_set_winsock_errno();
	return -1;
    }
    return W32_SOCKET_TO_FD(sock);
}

#ifdef HAVE_GETADDRINFO
const char *
inet_ntop(int af, const void *src, char *dst, socklen_t len)
{
    struct sockaddr *sa;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
    size_t salen;

    if (af == AF_INET) {
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = af;
	memcpy(&sin.sin_addr, src, sizeof(sin.sin_addr));
	sa = (struct sockaddr *)&sin;
	salen = sizeof(sin);
    } else if (af == AF_INET6) {
	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = af;
	memcpy(&sin6.sin6_addr, src, sizeof(sin6.sin6_addr));
	sa = (struct sockaddr *)&sin6;
	salen = sizeof(sin6);
    } else {
	WSASetLastError(WSAEAFNOSUPPORT);
	w32_set_winsock_errno();
	return NULL;
    }

    if (getnameinfo(sa, salen, dst, len, NULL, 0, NI_NUMERICHOST)) {
	WSASetLastError(WSAEAFNOSUPPORT);
	w32_set_winsock_errno();
	return NULL;
    }

    return dst;
}
#endif

/*
 * POSIX equivalent for fcntl().
 * Horrible hacks, just good enough support Empire's use of fcntl().
 * F_GETFL / F_SETFL support making a socket (non-)blocking by getting
 * flags, adding or removing O_NONBLOCK, and setting the result.
 */
int
fcntl(int fd, int cmd, ...)
{
    va_list ap;
    int value;
    unsigned long nonblocking;
    SOCKET sock;

    switch (cmd) {
    case F_GETFL:
	return 0;
    case F_SETFL:
	sock = W32_FD_TO_SOCKET(fd);
	va_start(ap, cmd);
	value = va_arg(ap, int);
	va_end(ap);
	nonblocking = (value & O_NONBLOCK) != 0;

	if (ioctlsocket(sock, FIONBIO, &nonblocking) == SOCKET_ERROR) {
	    w32_set_winsock_errno();
	    return -1;
	}
	return 0;
    }
    errno = EINVAL;
    return -1;
}

/*
 * close() replacement that does the right thing for sockets
 */
static int
w32_close_maybe_socket(int fd)
{
    SOCKET sock;

    if (fd_is_socket(fd, &sock)) {
	if (closesocket(sock)) {
	    w32_set_winsock_errno();
	    return -1;
	}
	/*
	 * This always fails because the underlying handle is already
	 * gone, but it closes the fd just fine.
	 */
	_close(fd);
	return 0;
    }
    return _close(fd);
}

/*
 * read() replacement that does the right thing for sockets
 */
static int
w32_read_maybe_socket(int fd, void *buf, unsigned sz)
{
    SOCKET sock;
    ssize_t res;

    if (fd_is_socket(fd, &sock)) {
	res = recv(sock, buf, sz, 0);
	if (res < 0)
	    w32_set_winsock_errno();
	return res;
    }
    return _read(fd, buf, sz);
}

/*
 * write() replacement that does the right thing for sockets
 */
static int
w32_write_maybe_socket(int fd, const void *buf, unsigned sz)
{
    SOCKET sock;
    ssize_t res;

    if (fd_is_socket(fd, &sock)) {
	res = send(sock, buf, sz, 0);
	if (res < 0)
	    w32_set_winsock_errno();
	return res;
    }
    return _write(fd, buf, sz);
}

int
w32_socket_init(void)
{
    int rc;
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 0);
    rc = WSAStartup(wVersionRequested, &wsaData);
    if (rc != 0)
	return -1;

    w32_close_function = w32_close_maybe_socket;
    w32_read_function = w32_read_maybe_socket;
    w32_write_function = w32_write_maybe_socket;
    return 0;
}
