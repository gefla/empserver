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
 *  io.c: Arrange for input and output on a file descriptor to be queued.
 *
 *  Known contributors to this file:
 *     Doug Hay, 1998
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2012
 *     Ron Koenderink, 2009
 */

/*
 * Arrange for input and output on a file descriptor
 * to be queued.  Provide main loop -- a mechanism for
 * blocking across all registered file descriptors, and
 * reading or writing when appropriate.
 */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "empio.h"
#include "empthread.h"
#include "file.h"
#include "ioqueue.h"
#include "misc.h"
#include "queue.h"

struct iop {
    int fd;
    struct ioqueue *input;
    struct ioqueue *output;
    int flags;
    int bufsize;
    int last_out;
};

void
io_init(void)
{
}

struct iop *
io_open(int fd, int flags, int bufsize)
{
    int fdfl;
    struct iop *iop;

    flags = flags & (IO_READ | IO_WRITE);
    if ((flags & (IO_READ | IO_WRITE)) == 0)
	return NULL;

    fdfl = fcntl(fd, F_GETFL, 0);
    if (fdfl < 0)
	return NULL;
    fdfl |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, fdfl) < 0)
	return NULL;

    iop = malloc(sizeof(struct iop));
    if (!iop)
	return NULL;
    iop->fd = fd;
    iop->input = NULL;
    iop->output = NULL;
    iop->flags = flags;
    iop->last_out = 0;
    iop->bufsize = bufsize;
    if (flags & IO_READ)
	iop->input = ioq_create(bufsize);
    if (flags & IO_WRITE)
	iop->output = ioq_create(bufsize);
    return iop;
}

void
io_close(struct iop *iop, struct timeval *timeout)
{
    char buf[IO_BUFSIZE];
    int ret;

    while (io_output(iop, 1) > 0) ;
    shutdown(iop->fd, SHUT_WR);
    while (empth_select(iop->fd, EMPTH_FD_READ, timeout) > 0) {
	ret = read(iop->fd, buf, sizeof(buf));
	if (ret <= 0)
	    break;
    }
    if (iop->input)
	ioq_destroy(iop->input);
    if (iop->output)
	ioq_destroy(iop->output);
    (void)close(iop->fd);
    free(iop);
}

/*
 * Read input from IOP and enqueue it.
 * If TIMEOUT is non-null, wait at most that long for input to arrive.
 * Does not yield the processor when timeout is zero.
 * A wait for input can be cut short by empth_wakeup().
 * Return number of bytes read on success, -1 on error.
 * In particular, return zero on timeout, early wakeup or EOF.  Use
 * io_eof() to distinguish timeout and early wakeup from EOF.
 */
int
io_input(struct iop *iop, struct timeval *timeout)
{
    char buf[IO_BUFSIZE];
    int cc;
    int res;

    if ((iop->flags & IO_READ) == 0)
	return -1;
    if (iop->flags & IO_ERROR)
	return -1;
    if (iop->flags & IO_EOF)
	return 0;

    if (!timeout || timeout->tv_sec || timeout->tv_usec) {
	res = empth_select(iop->fd, EMPTH_FD_READ, timeout);
	if (res < 0) {
	    iop->flags |= IO_ERROR;
	    return -1;
	} else if (res == 0)
	    return 0;
    }

    cc = read(iop->fd, buf, sizeof(buf));
    if (cc < 0) {
	if (errno == EAGAIN || errno == EWOULDBLOCK)
	    return 0;
	iop->flags |= IO_ERROR;
	return -1;
    }
    if (cc == 0) {
	iop->flags |= IO_EOF;
	return 0;
    }

    ioq_append(iop->input, buf, cc);
    return cc;
}

int
io_inputwaiting(struct iop *iop)
{
    return ioq_qsize(iop->input);
}

int
io_outputwaiting(struct iop *iop)
{
    return ioq_qsize(iop->output);
}

/*
 * Write output queued in IOP.
 * If WAIT, writing may put the thread to sleep.
 * Return number of bytes written on success, -1 on error.
 * In particular, return zero when nothing was written because the
 * queue was empty, or because the write slept and got woken up (only
 * if WAIT), or because the write refused to sleep (only if !WAIT).
 */
int
io_output(struct iop *iop, int wait)
{
    struct iovec iov[16];
    int n, res, cc;

    if (wait)
	ef_make_stale();

    if ((iop->flags & IO_WRITE) == 0)
	return -1;

    if (iop->flags & IO_ERROR)
	return -1;

    if (!ioq_qsize(iop->output))
	return 0;

    if (wait) {
	res = empth_select(iop->fd, EMPTH_FD_WRITE, NULL);
	if (res == 0)
	    return 0;
	if (res < 0) {
	    iop->flags |= IO_ERROR;
	    return -1;
	}
    }

    n = ioq_makeiov(iop->output, iov, IO_BUFSIZE);
    cc = writev(iop->fd, iov, n);
    if (cc < 0) {
	if (errno == EAGAIN || errno == EWOULDBLOCK)
	    return 0;
	iop->flags |= IO_ERROR;
	return -1;
    }

    ioq_dequeue(iop->output, cc);
    iop->last_out = ioq_qsize(iop->output);
    return cc;
}

/*
 * Write output queued in IOP if enough have been enqueued.
 * Write if at least one buffer has been filled since the last write.
 * If WAIT, writing may put the thread to sleep.
 * Return number of bytes written on success, -1 on error.
 * In particular, return zero when nothing was written because the
 * queue was not long, or the write slept and got woken up (only if
 * WAIT), or the write refused to sleep (only if !WAIT).
 */
int
io_output_if_queue_long(struct iop *iop, int wait)
{
    int len = ioq_qsize(iop->output);

    if (CANT_HAPPEN(iop->last_out > len))
	iop->last_out = 0;
    if (len - iop->last_out < iop->bufsize) {
	if (wait)
	    ef_make_stale();
	return 0;
    }
    return io_output(iop, wait);
}

int
io_peek(struct iop *iop, char *buf, int nbytes)
{
    if ((iop->flags & IO_READ) == 0)
	return -1;
    return ioq_peek(iop->input, buf, nbytes);
}

int
io_read(struct iop *iop, char *buf, int nbytes)
{
    int cc;

    if ((iop->flags & IO_READ) == 0)
	return -1;
    cc = ioq_peek(iop->input, buf, nbytes);
    if (cc > 0)
	ioq_dequeue(iop->input, cc);
    return cc;
}

int
io_write(struct iop *iop, char *buf, int nbytes)
{
    if ((iop->flags & IO_WRITE) == 0)
	return -1;
    ioq_append(iop->output, buf, nbytes);
    return nbytes;
}

int
io_gets(struct iop *iop, char *buf, int nbytes)
{
    if ((iop->flags & IO_READ) == 0)
	return -1;
    return ioq_gets(iop->input, buf, nbytes);
}

int
io_puts(struct iop *iop, char *buf)
{
    if ((iop->flags & IO_WRITE) == 0)
	return -1;
    return ioq_puts(iop->output, buf);
}

int
io_shutdown(struct iop *iop, int flags)
{
    flags &= (IO_READ | IO_WRITE);
    if ((iop->flags & flags) != flags)
	return -1;
    if (flags & IO_READ) {
	shutdown(iop->fd, 0);
	ioq_drain(iop->input);
    }
    if (flags & IO_WRITE) {
	shutdown(iop->fd, 1);
	ioq_drain(iop->output);
	iop->last_out = 0;
    }
    return 0;
}

int
io_error(struct iop *iop)
{
    return iop->flags & IO_ERROR;
}

int
io_eof(struct iop *iop)
{
    return iop->flags & IO_EOF;
}

/*
 * Discard IOP's buffered input and set its EOF flag.
 * No more input can be read from IOP.
 */
void
io_set_eof(struct iop *iop)
{
    ioq_drain(iop->input);
    iop->flags |= IO_EOF;
}

int
io_fileno(struct iop *iop)
{
    return iop->fd;
}
