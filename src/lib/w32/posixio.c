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
 *  posixio.c: POSIX IO emulation layer for WIN32
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 */

/*
 * POSIX has just one kind of file descriptors, while Windows has (at
 * least) two: one for sockets and one for files, with separate
 * functions to operate on them.  To present a more POSIX-like
 * interface to our application code, we provide a compatibility layer
 * that maps POSIX file descriptors to sockets and file handles behind
 * the scenes.  This actual mapping is done by the fdmap.  It doesn't
 * implement the finer points of POSIX correctly.  In particular, the
 * actual values of the file descriptors usually differ.
 */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "misc.h"
#include "sys/socket.h"
#include "sys/uio.h"
#include "unistd.h"

/*
 * FD_SETSIZE is the size for the maximum number of sockets.
 * The number of file descriptors is in a variable _nhandle
 * based on the assertion output.  In order the simplify the
 * code and skip dynamic allocation, used double the socket size.
 */
#define MAX_FDS (FD_SETSIZE * 2)

enum fdmap_io_type {
    FDMAP_IO_NOTUSED = 0,
    FDMAP_IO_FILE,
    FDMAP_IO_SOCKET,
    FDMAP_IO_ANY /* used for searching only (lookup_handle) */
};

struct fdmap {
    int handle;
    enum fdmap_io_type type;
};

static struct fdmap fdmap[MAX_FDS] = {
    {0, FDMAP_IO_FILE},
    {1, FDMAP_IO_FILE},
    {2, FDMAP_IO_FILE}
};
static int nfd = 3;

/*
 * Allocate a POSIX equivalent file descriptor.
 * Note once get_fd() is called either free_fd() or set_fd()
 * must be called before thread mutex is released as the
 * allocation/deallocation code is not thread safe.
 */
static int
get_fd(void)
{
    int fd;

    for (fd = 0; fd < nfd && fdmap[fd].type != FDMAP_IO_NOTUSED; fd++) ;
    if (fd == MAX_FDS) {
	errno = EMFILE;
	return -1;
    }
    if (fd == nfd) {
	fdmap[fd].type = FDMAP_IO_NOTUSED;
	nfd++;
    }
    return fd;
}

/*
 * Deallocate a POSIX equivalent file descriptor.
 */
static void
free_fd(int fd)
{
    fdmap[fd].type = FDMAP_IO_NOTUSED;
    for(; fdmap[nfd - 1].type == FDMAP_IO_NOTUSED; nfd--) ;
}

/*
 * Complete the allocation of the file descriptor.
 */
static void
set_fd(int fd, enum fdmap_io_type type, int handle)
{
    int i;

    fdmap[fd].handle = handle;
    fdmap[fd].type = type;

    /*
     * Garbage collection for posix_fileno(), currently not
     * replacing fclose() and fcloseall() so do not know when
     * a stream is closed.
     */
    for (i = 0; i < nfd; i++) {
	if (i != fd && type == fdmap[i].type && handle == fdmap[i].handle)
	    free_fd(i);
    }
}

/*
 * Find the windows handle (file or socket) for file descriptor.
 * Return windows handle and type of handle.
 * You can search for a specific type (FDMAP_IO_FILE or FDMAP_IO_SOCKET)
 * or for both search by using FDMAP_IO_ANY.
 * FDMAP_IO_NOTUSED is not valid type to search with.
 */
static int
lookup_handle(int fd, enum fdmap_io_type d_type, int error,
	enum fdmap_io_type *type_ptr, int *handle_ptr)
{

    if (fd < 0 || fd >= MAX_FDS) {
	if (error != 0)
	    errno = error;
	return 0;
    } else if ((fdmap[fd].type != d_type && d_type != FDMAP_IO_ANY) ||
	(fdmap[fd].type == FDMAP_IO_NOTUSED && d_type == FDMAP_IO_ANY)) {
	if (error != 0)
	    errno = error;
	return 0;
    }
    if (type_ptr != NULL)
	*type_ptr = fdmap[fd].type;
    if (handle_ptr != NULL)
	*handle_ptr = fdmap[fd].handle;
    return 1;
}

/*
 * Find and return the file descriptor associated with windows handle.
 * You can search for FDMAP_IO_FILE or FDMAP_IO_SOCKET.
 * FDMAP_IO_ANY or FDMAP_IO_NOTUSED is not considered valid search
 * criteria.
 */
static int
lookup_fd(int handle, enum fdmap_io_type d_type)
{
    int i;

    for (i = 0; i < nfd; i++)
	if (fdmap[i].handle == handle && fdmap[i].type == d_type)
	    return i;
    return -1;
}

/*
 * Get the window socket handle for POSIX file descriptor.
 */
int
posix_fd2socket(int fd)
{
    int handle;
    enum fdmap_io_type type;

    if (!lookup_handle(fd, FDMAP_IO_SOCKET, WSAENOTSOCK,
	&type, &handle))
	return INVALID_SOCKET;
    return handle;
}

#define SOCKET_FUNCTION(expr)				\
    int result;						\
    int handle;						\
							\
    if (!lookup_handle(fd, FDMAP_IO_SOCKET,		\
	ENOTSOCK, NULL, &handle))			\
	return -1;					\
							\
    result = (expr);					\
    if (result == SOCKET_ERROR) {			\
	errno = WSAGetLastError();			\
	return -1;					\
    }							\
    return result;

/*
 * POSIX equivalent for accept().
 */
#undef accept
int
posix_accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    int new_fd;
    int handle, new_handle;

    if (!lookup_handle(fd, FDMAP_IO_SOCKET, ENOTSOCK, NULL, &handle))
	return -1;

    new_fd = get_fd();
    if (new_fd < 0)
	return -1;

    new_handle = accept(handle, addr, addrlen);
    if (new_handle == INVALID_SOCKET) {
	free_fd(new_fd);
	errno = WSAGetLastError();
	return -1;
    }
    set_fd(new_fd, FDMAP_IO_SOCKET, new_handle);
    return new_fd;
}

/*
 * POSIX equivalent for bind().
 */
#undef bind
int
posix_bind(int fd, const struct sockaddr *name, socklen_t namelen)
{
    SOCKET_FUNCTION(bind(handle, name, namelen))
}

/*
 * POSIX equivalent for listen().
 */
#undef listen
int
posix_listen(int fd, int backlog)
{
    SOCKET_FUNCTION(listen(handle, backlog))
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
     * SO_REUSEADDR requests from tcp_listen.c
     * to permit another bind even when the
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
	SOCKET_FUNCTION(setsockopt(handle, level, optname,
			optval, optlen))
    }
}

/*
 * POSIX equivalent for shutdown().
 */
#undef shutdown
int
posix_shutdown(int fd, int how)
{
    SOCKET_FUNCTION(shutdown(handle, how))
}

/*
 * POSIX equivalent for socket().
 */
#undef socket
int
posix_socket(int domain, int type, int protocol)
{
    int handle;
    int new_fd;
    
    if ((new_fd = get_fd()) < 0)
	return -1;

    handle = socket(domain, type, protocol);
    if (handle == INVALID_SOCKET) {
	free_fd(new_fd);
	errno = WSAGetLastError();
	return -1;
    }
    set_fd(new_fd, FDMAP_IO_SOCKET, handle);
    return new_fd;
}

#define FILE_FUNCTION(type, expr)				\
    int handle;							\
								\
    if (!lookup_handle(fd, (type), EBADF, NULL, &handle))	\
	return -1;						\
								\
    return (expr);

/*
 * POSIX equivalent for close().
 */
int
posix_close(int fd)
{
    int result;
    int handle;
    enum fdmap_io_type type;

    if (!lookup_handle(fd, FDMAP_IO_ANY, EBADF, &type, &handle))
	return -1;

    free_fd(fd);
    switch (type) {
    case FDMAP_IO_SOCKET:
	result = closesocket(handle);
	if (result == SOCKET_ERROR) {
	    errno = WSAGetLastError();
	    return -1;
	}
	return result;
    case FDMAP_IO_FILE:
       return _close(handle);
    }
    CANT_REACH();
    return -1;
}

/*
 * posix_fsync forces file sync with the disk.
 * In order for the power report report to accurate timestamp,
 * the _commit() is to force a sync with disk and therefore
 * an update for file time.
 */
int
posix_fsync(int fd)
{
    FILE_FUNCTION(FDMAP_IO_FILE, _commit(handle))
}

/*
 * POSIX equivalent for fstat().
 * fstat() is used instead of _fstat(),
 * otherwise problems with the 32/64 time definitions
 * in WIN32.
 */
#undef fstat
int
posix_fstat(int fd, struct stat *buffer)
{
    FILE_FUNCTION(FDMAP_IO_ANY, fstat(handle, buffer))
}

/*
 * POSIX equivalent for lseek().
 */
off_t
posix_lseek(int fd, off_t offset, int origin)
{
    FILE_FUNCTION(FDMAP_IO_FILE, _lseek(handle, offset, origin))
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
    int pmode = 0, new_fd;
    int handle;

    if (oflag & O_CREAT) {
	va_start(ap, oflag);
	pmode = va_arg(ap, int);
	va_end(ap);
    }

    if ((new_fd = get_fd()) < 0)
	return -1;

    /*
     * We don't implement fcntl() for F_SETLK.  Instead, we lock *all*
     * files we open.  Not ideal, but it works for Empire.
     */
    handle = _sopen(fname, oflag,
	oflag & O_RDONLY ? SH_DENYNO : SH_DENYWR, pmode);
    if (handle == -1) {
	free_fd(new_fd);
	return -1;
    }
    set_fd(new_fd, FDMAP_IO_FILE, handle);
    return new_fd;
}

#define SHARED_FUNCTION(socket_expr, file_expr)			    \
    int result;							    \
    int handle;							    \
    enum fdmap_io_type type;					    \
								    \
    if (!lookup_handle(fd, FDMAP_IO_ANY, EBADF, &type, &handle))    \
	return -1;						    \
								    \
    switch (type) {						    \
    case FDMAP_IO_SOCKET:					    \
	result = (socket_expr);					    \
	if (result == SOCKET_ERROR) {				    \
	    errno = WSAGetLastError();				    \
	    return -1;						    \
	}							    \
	return result;						    \
    case FDMAP_IO_FILE:						    \
	return (file_expr);					    \
    }								    \
    CANT_REACH();						    \
    return -1;

/*
 * POSIX equivalent for read().
 */
ssize_t
posix_read(int fd, void *buffer, size_t count)
{
    SHARED_FUNCTION(recv(handle, buffer, count, 0),
	_read(handle, buffer, count))
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
    if (buffer == NULL)
	return -1;

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
posix_write(int fd, const void *buffer, size_t count)
{
    SHARED_FUNCTION(send(handle, buffer, count, 0),
	_write(handle, buffer, count))
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

    if (total_bytes == 0)
	return 0;

    buffer = malloc(total_bytes);
    if (buffer == NULL)
	return -1;

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
 * POSIX equivalent for fileno().
 * As fopen/fclose/fcloseall are not implemented as POSIX
 * equivalent functions, the mapping is done when required
 * by a call to fileno().  The garbage collection of the
 * file descriptors allocated is done in set_fd() when the
 * handle is reused.
 */
int
posix_fileno(FILE *stream)
{
    int fd;
    int handle;

    if (stream == NULL) {
	errno = EBADF;
	return -1;
    }

    handle = _fileno(stream);

    fd = lookup_fd(handle, FDMAP_IO_FILE);
    if (fd >= 0)
	return fd;

    if ((fd = get_fd()) < 0) {
	errno = EBADF;
	return -1;
    }

    set_fd(fd, FDMAP_IO_FILE, handle);
    return fd;
}

/*
 * POSIX equivalent for fcntl().
 * Currently supports only the F_GETFL/F_SETFL/O_NONBLOCK
 * Currently ignores F_GETLK/F_SETLK as the file locks are
 * implement in open()
 */
int
fcntl(int fd, int cmd, ...)
{
    va_list ap;
    int value;
    unsigned int nonblocking;
    int result;
    long bytes_returned;
    int handle;
    enum fdmap_io_type type;

    if (!lookup_handle(fd, FDMAP_IO_ANY, EBADF, &type, &handle))
        return -1;

    switch (cmd)
    {
    case F_GETFL:
	/*
	 * F_GETFL and F_SETFL only support O_NONBLOCK
	 * for sockets currently
	 */
	if (type == FDMAP_IO_SOCKET) {
	    result = WSAIoctl(handle, FIONBIO, NULL, 0,&nonblocking,
		sizeof (nonblocking), &bytes_returned, NULL, NULL);
    	
	    if(result < 0) {
		errno = WSAGetLastError();
		return -1;
	    }

	    if (nonblocking)
		return O_NONBLOCK;
	    else
		return 0;
	}
	break;
    case F_SETFL:
	if (type == FDMAP_IO_SOCKET) {
	    va_start(ap, cmd);
	    value = va_arg(ap, int);
	    va_end(ap);
	    if (value & O_NONBLOCK)
		nonblocking = 1;
	    else
		nonblocking = 0;

	    result = WSAIoctl(handle, FIONBIO, &nonblocking,
		sizeof (nonblocking), NULL, 0, &bytes_returned,
		NULL, NULL);

	    if(result < 0) {
		errno = WSAGetLastError();
		return -1;
	    }
	    return result;
	}
	break;
    case F_SETLK:
	/*
	 * The POSIX equivalent is not available in WIN32
	 * That implement the file locking in the file open
	 * by using sopen instead of open.
	 */
	return 0;
    }
    errno = EINVAL;
    return -1;
}
