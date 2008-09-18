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
 *  sysdep_w32.c: system dependent functions for WIN32 environments
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 */

#ifdef _WIN32
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include "misc.h"
#include "linebuf.h"
#include "ringbuf.h"
#include "secure.h"

/*
 * Get user name in the WIN32 environment
 */
struct passwd *
w32_getpw(void)
{
    static char unamebuf[128];
    static struct passwd pwd;
    long unamesize;

    unamesize = sizeof(unamebuf);
    if (GetUserName(unamebuf, &unamesize)) {
	pwd.pw_name = unamebuf;
	if ((unamesize <= 0 ) || (strlen(unamebuf) <= 0))
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
sysdep_init(void)
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
w32_socket(int family, int sock_type, int protocol)
{
    SOCKET result;

    result = socket(family, sock_type, protocol);
    if (result == INVALID_SOCKET) {
	errno = WSAGetLastError();
	return -1;
    }
    return (int)result;
}

/*
 * POSIX compatible connect() replacement
 */
#undef connect
int
w32_connect(int sock, struct sockaddr *addr, int addrlen)
{
    int result;

    result = connect(sock,  addr, addrlen);
    if (result == SOCKET_ERROR) {
	errno = WSAGetLastError();
	return -1;
    }
    return result;
}

/*
 * POSIX compatible recv() replacement
 */
#undef recv
int
w32_recv(int socket, char *buffer, size_t buf_size, int flags)
{
    int result;

    result = recv(socket, buffer, buf_size, flags);
    if (result == SOCKET_ERROR) {
	errno = WSAGetLastError();
	return -1;
    }
    return result;
}

/*
 * POSIX compatible writev() replacement specialized to sockets
 * Modelled after the GNU's libc/sysdeps/posix/writev.c
 */
ssize_t
w32_writev_socket(int fd, const struct iovec *iov, int iovcnt)
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

    bytes_written = send(fd, buffer, total_bytes, 0);

    free(buffer);

    if (bytes_written == SOCKET_ERROR) {
	errno = WSAGetLastError();
	return -1;
    }
    return bytes_written;
}

/*
 * POSIX compatible send() replacement
 */
int
w32_send(int socket, char *buffer, size_t buf_size, int flags)
{
	int result;

	result = send(socket, buffer, buf_size, flags);
	if (result == SOCKET_ERROR)
		errno = WSAGetLastError();
	return result;
}

/*
 * POSIX compatible close() replacement specialized to sockets.
 */
int
w32_close_socket(int fd)
{
    int result;

    result = closesocket(fd);
    if (result == SOCKET_ERROR)
	errno = WSAGetLastError();
    return result;
}

/*
 * WIN32 equivalent for getpass
 */
char *
getpass(char *prompt)
{
    static char tmp[128];
    int len;
    char *cpass;
    DWORD mode;
    HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);

    if (GetConsoleMode(input_handle, &mode))
	SetConsoleMode(input_handle, mode & ~ENABLE_ECHO_INPUT);
    else
	printf("Note: This is echoed to the screen\n");
    printf("%s", prompt);
    fflush(stdout);
    cpass = fgets(tmp, sizeof(tmp), stdin);
    if (GetConsoleMode(input_handle, &mode))
	SetConsoleMode(input_handle, mode | ENABLE_ECHO_INPUT);
    if (cpass == NULL)
	return NULL;
    len = strlen(cpass);
    if (tmp[len - 1] == '\n')
	tmp[len - 1] = 0;
    return cpass;
}

/*
 * POSIX compatible open() replacement
 */
int
w32_openfd(const char *fname, int oflag, ...)
{
    va_list ap;
    int pmode = 0;
    int fd;
    int create_permission = 0;

    if (oflag & O_CREAT) {
	va_start(ap, oflag);
	pmode = va_arg(ap, int);
	va_end(ap);

	if (pmode & 0400)
	    create_permission |= _S_IREAD;
	if (pmode & 0200)
	    create_permission |= _S_IWRITE;
    }

    fd = _open(fname, oflag, create_permission);
    return fd;
}
/*
 * Open a file for reading, return its handle.
 * This can be used in place of open() when a handle is desired for
 * waiting on it with WaitForMultipleObjects() or similar.
 * Ensure the handle is not zero in order to prevent a problem
 * input_fd.
 */
int
w32_openhandle(const char *fname, int oflag)
{
    HANDLE handle;

    handle = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL,
	OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
	errno = GetLastError();
	return -1;
    }
    if (handle == 0) {
	HANDLE dup_handle;
	if (!DuplicateHandle(GetCurrentProcess(), handle,
			GetCurrentProcess(), &dup_handle,
			0, FALSE, DUPLICATE_SAME_ACCESS)) {
	    errno = GetLastError();
	    return -1;
	} else {
	    CloseHandle(handle);
	    handle = dup_handle;
	}
    }
    return (int)handle;
}

/*
 * POSIX compatible readv() replacement specialized to files.
 * Modelled after the GNU's libc/sysdeps/posix/readv.c
 */
ssize_t
w32_readv_handle(int fd, const struct iovec *iov, int iovcnt)
{
    int i;
    unsigned char *buffer, *buffer_location;
    size_t total_bytes = 0;
    DWORD bytes_read;
    size_t bytes_left;

    for (i = 0; i < iovcnt; i++) {
	total_bytes += iov[i].iov_len;
    }

    buffer = malloc(total_bytes);
    if (buffer == NULL && total_bytes != 0) {
	errno = ENOMEM;
	return -1;
    }

    if (!ReadFile((HANDLE)fd, buffer, total_bytes, &bytes_read, NULL)) {
	free(buffer);
	errno = GetLastError();
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
 * POSIX compatible close() replacement specialized to files.
 * Hack: expects a handle, cannot be used with a file descriptor.
 */
int
w32_close_handle(int fd)
{
    int result;

    result = CloseHandle((HANDLE)fd);

    if (!result)
	errno = GetLastError();
    return result;
}
#endif /* _WIN32 */
