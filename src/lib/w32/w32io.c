/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  w32io.c: POSIX I/O emulation layer for Windows
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
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "sys/uio.h"

int (*w32_close_function)(int) = _close;
int (*w32_read_function)(int, void *, unsigned) = _read;
int (*w32_write_function)(int, const void *, unsigned) = _write;

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

    bytes_read = read(fd, buffer, total_bytes);
    if (bytes_read < 0) {
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

    bytes_written = write(fd, buffer, total_bytes);
    free(buffer);
    return bytes_written;
}
