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
 *  sysdep_w32.c: system dependent functions for WIN32 environments
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 *     Markus Armbruster, 2009
 */

#ifdef _WIN32
#include <errno.h>
#include <fcntl.h>
#include "misc.h"

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

/*
 * Get user name in the WIN32 environment
 */
struct passwd *
w32_getpw(void)
{
    static char unamebuf[128];
    static struct passwd pwd;
    DWORD unamesize;

    unamesize = sizeof(unamebuf);
    if (GetUserName(unamebuf, &unamesize)) {
	pwd.pw_name = unamebuf;
	if (unamesize == 0 || strlen(unamebuf) == 0)
	    pwd.pw_name = "nobody";
    } else
	pwd.pw_name = "nobody";
    return &pwd;
}

/*
 * Initialize the WIN32 socket library and
 * set up stdout to work around bugs
 */
void
w32_sysdep_init(void)
{
    int err;
    WSADATA WsaData;
    /*
     * stdout is unbuffered under Windows if connected to a character
     * device, and putchar() screws up when printing multibyte strings
     * bytewise to an unbuffered stream.  Switch stdout to line-
     * buffered mode.  Unfortunately, ISO C allows implementations to
     * screw that up, and of course Windows does.  Manual flushing
     * after each prompt is required.
     */
    setvbuf(stdout, NULL, _IOLBF, 4096);
    err = WSAStartup(MAKEWORD(2, 0), &WsaData);
    if (err != 0) {
	printf("WSAStartup Failed, error code %d\n", err);
	exit(1);
    }
}

/*
 * POSIX compatible socket() replacement
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
 * POSIX compatible recv() replacement
 */
#undef recv
int
w32_recv(int sockfd, void *buffer, size_t buf_size, int flags)
{
    SOCKET socket = W32_FD_TO_SOCKET(sockfd);
    int result;

    result = recv(socket, buffer, buf_size, flags);
    if (result == SOCKET_ERROR) {
	w32_set_winsock_errno();
	return -1;
    }
    return result;
}

/*
 * POSIX compatible send() replacement
 */
int
w32_send(int sockfd, const void *buffer, size_t buf_size, int flags)
{
    SOCKET socket = W32_FD_TO_SOCKET(sockfd);
    int result;

    result = send(socket, buffer, buf_size, flags);
    if (result == SOCKET_ERROR)
	w32_set_winsock_errno();
    return result;
}

/*
 * POSIX compatible close() replacement
 */
int
w32_close(int fd)
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

#endif /* _WIN32 */
