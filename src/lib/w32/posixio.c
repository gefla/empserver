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
 *  posixio.c: POSIX I/O emulation layer for WIN32
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 *     Markus Armbruster, 2007-2009
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
#include <share.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
/*
 * Need to include winsock2.h before ws2tcpip.h.
 * Use sys/socket.h to ensure the #undef NS_ALL
 * is not missed after including winsock2.h.
 */
#include "sys/socket.h"
#include <sys/stat.h>
#include <ws2tcpip.h>

#include "misc.h"
#include "sys/uio.h"
#include "unistd.h"

#define W32_FD_TO_SOCKET(fd) ((SOCKET)_get_osfhandle((fd)))
#define W32_SOCKET_TO_FD(fh) (_open_osfhandle((long)(fh), O_RDWR | O_BINARY))

SOCKET
posix_fd2socket(int fd)
{
    return W32_FD_TO_SOCKET(fd);
}

static int
fd_is_socket(int fd, SOCKET *sockp)
{
    SOCKET sock;
    WSANETWORKEVENTS ev;

    sock = W32_FD_TO_SOCKET(fd);
    if (sockp)
	*sockp = sock;
    return WSAEnumNetworkEvents(sock, NULL, &ev) == 0;
}

void
w32_set_winsock_errno(void)
{
  int err = WSAGetLastError();
  WSASetLastError(0);

  /* Map some WSAE* errors to the runtime library's error codes.  */
  switch (err)
    {
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
posix_accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
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
posix_bind(int fd, const struct sockaddr *name, socklen_t namelen)
{
    SOCKET_FUNCTION(bind(sock, name, namelen));
}

/*
 * POSIX equivalent for listen().
 */
#undef listen
int
posix_listen(int fd, int backlog)
{
    SOCKET_FUNCTION(listen(sock, backlog));
}

/*
 * POSIX equivalent for setsockopt().
 */
#undef setsockopt
int
posix_setsockopt(int fd, int level, int optname,
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
posix_shutdown(int fd, int how)
{
    SOCKET_FUNCTION(shutdown(sock, how));
}

/*
 * POSIX equivalent for socket().
 */
#undef socket
int
posix_socket(int domain, int type, int protocol)
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
 * POSIX equivalent for close().
 */
int
posix_close(int fd)
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
 * POSIX equivalent for open().
 * Implements file locks when opening files to provide equivalent
 * F_GETLK/F_SETLK.
 */
int
posix_open(const char *fname, int oflag, ...)
{
    va_list ap;
    int pmode = 0;

    if (oflag & O_CREAT) {
	va_start(ap, oflag);
	pmode = va_arg(ap, int);
	va_end(ap);
    }

    /*
     * We don't implement fcntl() for F_SETLK.  Instead, we lock *all*
     * files we open.  Not ideal, but it works for Empire.
     */
    return _sopen(fname, oflag,
		  oflag & O_RDONLY ? SH_DENYNO : SH_DENYWR,
		  pmode);
}

/*
 * POSIX equivalent for read().
 */
ssize_t
posix_read(int fd, void *buf, size_t sz)
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
 * POSIX equivalent for readv
 * Modelled after the GNU's libc/sysdeps/posix/readv.c
 */
ssize_t
readv(int fd, const struct iovec *iov, int iovcnt)
{
    int i;
    unsigned char *buffer, *buffer_location;
    size_t total_bytes = 0;
    int bytes_read;
    size_t bytes_left;

    for (i = 0; i < iovcnt; i++) {
	total_bytes += iov[i].iov_len;
    }

    buffer = malloc(total_bytes);
    if (buffer == NULL && total_bytes != 0) {
	errno = ENOMEM;
	return -1;
    }

    bytes_read = posix_read(fd, buffer, total_bytes);
    if (bytes_read <= 0) {
	free(buffer);
	return -1;
    }

    bytes_left = bytes_read;
    buffer_location = buffer;
    for (i = 0; i < iovcnt; i++) {
	size_t copy = MIN(iov[i].iov_len, bytes_left);

	memcpy(iov[i].iov_base, buffer_location, copy);

	buffer_location += copy;
	bytes_left -= copy;
	if (bytes_left == 0)
	    break;
    }

    free(buffer);

    return bytes_read;
}

/*
 * POSIX equivalent for write().
 */
ssize_t
posix_write(int fd, const void *buf, size_t sz)
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

/*
 * POSIX equivalent for writev
 * Modelled after the GNU's libc/sysdeps/posix/writev.c
 */
ssize_t
writev(int fd, const struct iovec *iov, int iovcnt)
{
    int i;
    unsigned char *buffer, *buffer_location;
    size_t total_bytes = 0;
    int bytes_written;

    for (i = 0; i < iovcnt; i++)
	total_bytes += iov[i].iov_len;

    buffer = malloc(total_bytes);
    if (buffer == NULL && total_bytes != 0) {
	errno = ENOMEM;
	return -1;
    }

    buffer_location = buffer;
    for (i = 0; i < iovcnt; i++) {
	memcpy(buffer_location, iov[i].iov_base, iov[i].iov_len);
	buffer_location += iov[i].iov_len;
    }

    bytes_written = posix_write(fd, buffer, total_bytes);

    free(buffer);

    if (bytes_written <= 0)
	return -1;
    return bytes_written;
}

/*
 * POSIX equivalent for fcntl().
 * Horrible hacks, just good enough support Empire's use of fcntl().
 * F_GETFL / F_SETFL support making a socket (non-)blocking by getting
 * flags, adding or removing O_NONBLOCK, and setting the result.
 * F_SETLK does nothing.  Instead, we lock in posix_open().
 */
int
fcntl(int fd, int cmd, ...)
{
    va_list ap;
    int value;
    unsigned long nonblocking;
    SOCKET sock;

    switch (cmd)
    {
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
    case F_SETLK:
	return 0;
    }
    errno = EINVAL;
    return -1;
}
