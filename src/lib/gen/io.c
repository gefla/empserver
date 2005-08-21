/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  io.c: Arrange for input and output on a file descriptor to be queued.
 * 
 *  Known contributors to this file:
 *      Doug Hay, 1998
 *      Steve McClure, 1998
 */

/*
 * Arrange for input and output on a file descriptor
 * to be queued.  Provide main loop -- a mechanism for
 * blocking across all registered file descriptors, and
 * reading or writing when appropriate.
 */

#include <errno.h>
#include <sys/types.h>
#if !defined(_WIN32)
#include <sys/uio.h>
#include <sys/file.h>
#include <unistd.h>		/* close read shutdown select */
#include <sys/socket.h>
#endif
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>		/* malloc calloc free */

#if defined(_WIN32)
#define WIN32
#include <winsock2.h>
#undef NS_ALL
#endif

#include "misc.h"
#include "queue.h"
#include "ioqueue.h"
#include "empio.h"
#include "gen.h"		/* getfdtablesize */
#include "server.h"

#include "empthread.h"

struct iop {
    int fd;
    struct ioqueue *input;
    struct ioqueue *output;
    int flags;
    s_char *assoc;
    int bufsize;
    int (*notify)(void);
};

void
io_init(void)
{
}

struct iop *
io_open(int fd, int flags, int bufsize, int (*notify)(void),
	s_char *assoc)
{
    struct iop *iop;

    flags = flags & (IO_READ | IO_WRITE | IO_NBLOCK | IO_NEWSOCK);
    if ((flags & (IO_READ | IO_WRITE)) == 0)
	return NULL;
    iop = malloc(sizeof(struct iop));
    if (!iop)
	return NULL;
    iop->fd = fd;
    iop->input = 0;
    iop->output = 0;
    iop->flags = 0;
    iop->bufsize = bufsize;
    if ((flags & IO_READ) && (flags & IO_NEWSOCK) == 0)
	iop->input = ioq_create(bufsize);
    if ((flags & IO_WRITE) && (flags & IO_NEWSOCK) == 0)
	iop->output = ioq_create(bufsize);
    if (flags & IO_NBLOCK)
	io_noblocking(iop, 1);
    iop->flags = flags;
    iop->assoc = assoc;
    iop->notify = notify;
    return iop;
}

void
io_close(struct iop *iop)
{

    if (iop->input != 0)
	ioq_destroy(iop->input);
    if (iop->output != 0)
	ioq_destroy(iop->output);
#if !defined(_WIN32)
    (void)close(iop->fd);
#else
    closesocket(iop->fd);
#endif
    free(iop);
}

int
io_input(struct iop *iop, int waitforinput)
{
    s_char buf[IO_BUFSIZE];
    int cc;

    /* Not a read IOP */
    if ((iop->flags & IO_READ) == 0)
	return -1;
    /* IOP is markes as in error. */
    if (iop->flags & IO_ERROR)
	return -1;
    /* Wait for the file to have input. */
    if (waitforinput) {
	empth_select(iop->fd, EMPTH_FD_READ);
    }
#if !defined(_WIN32)
    /* Do the actual read. */
    cc = read(iop->fd, buf, sizeof(buf));
    if (cc < 0) {
	/* would block, so nothing to read. */
	if (errno == EWOULDBLOCK)
	    return 0;

	/* Some form of file error occurred... */
	iop->flags |= IO_ERROR;
	return -1;
    }
#else
    cc = recv(iop->fd, buf, sizeof(buf), 0);
    if (cc == SOCKET_ERROR) {
	int err = WSAGetLastError();
	/* Hmm, it would block.  file is opened noblock, soooooo.. */
	if (err == WSAEWOULDBLOCK)
	    return 0;

	/* Some form of file error occurred... */
	iop->flags |= IO_ERROR;
	return -1;
    }
#endif

    /* We eof'd */
    if (cc == 0) {
	iop->flags |= IO_EOF;
	return 0;
    }

    /* Append the input to the IOQ. */
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

int
io_output(struct iop *iop, int waitforoutput)
{
#if !defined(_WIN32)
    struct iovec iov[16];
#else
    s_char buf[IO_BUFSIZE];
#endif
    int cc;
    int n;
    int remain;

    /* If there is no output waiting. */
    if (!io_outputwaiting(iop))
	return 0;

    /* If the iop is not write enabled. */
    if ((iop->flags & IO_WRITE) == 0)
	return -1;

    /* If the io is marked as in error... */
    if (iop->flags & IO_ERROR)
	return -1;

    /* This is the same test as io_outputwaiting.... */
    if (ioq_qsize(iop->output) == 0)
	return 0;

#if !defined(_WIN32)
    /* make the iov point to the data in the queue. */
    /* I.E., each of the elements in the queue. */
    /* returns the number of elements in the iov. */
    n = ioq_makeiov(iop->output, iov, IO_BUFSIZE);
#else
    /* Make a buffer containing the output to write. */
    n = ioq_makebuf(iop->output, buf, sizeof(buf));
#endif

    if (n <= 0) {
	return 0;
    }

    /* wait for the file to be output ready. */
    if (waitforoutput != IO_NOWAIT) {
	/* This waits for the file to be ready for writing, */
	/* and lets other threads run. */
	empth_select(iop->fd, EMPTH_FD_WRITE);
    }

    /* Do the actual write. */
#if !defined(_WIN32)
    cc = writev(iop->fd, iov, n);

    /* if it failed.... */
    if (cc < 0) {
	/* Hmm, it would block.  file is opened noblock, soooooo.. */
	if (errno == EWOULDBLOCK) {
	    /* If there are remaining bytes, set the IO as remaining.. */
	    remain = ioq_qsize(iop->output);
	    return remain;
	}
	iop->flags |= IO_ERROR;
	return -1;
    }
#else
    cc = send(iop->fd, buf, n, 0);

    /* if it failed.... */
    if (cc == SOCKET_ERROR) {
	int err = WSAGetLastError();
	/* Hmm, it would block.  file is opened noblock, soooooo.. */
	if (err == WSAEWOULDBLOCK) {
	    /* If there are remaining bytes, set the IO as remaining.. */
	    remain = ioq_qsize(iop->output);
	    return remain;
	}
	iop->flags |= IO_ERROR;
	return -1;
    }
#endif

    /* If no bytes were written, something happened..  Like an EOF. */
#ifndef	hpux
    if (cc == 0) {
	iop->flags |= IO_EOF;
	return 0;
    }
#else
    if (cc == 0) {
	remain = ioq_qsize(iop->output);
	return remain;
    }
#endif /* hpux */

    /* Remove the number of written bytes from the queue. */
    ioq_dequeue(iop->output, cc);

    return cc;
}

int
io_peek(struct iop *iop, s_char *buf, int nbytes)
{
    if ((iop->flags & IO_READ) == 0)
	return -1;
    return ioq_peek(iop->input, buf, nbytes);
}

int
io_read(struct iop *iop, s_char *buf, int nbytes)
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
io_write(struct iop *iop, s_char *buf, int nbytes, int doWait)
{
    int len;

    if ((iop->flags & IO_WRITE) == 0)
	return -1;
    ioq_append(iop->output, buf, nbytes);
    len = ioq_qsize(iop->output);
    if (len > iop->bufsize) {
	if (doWait) {
	    io_output_all(iop);
	} else {
	    /* only try a write every BUFSIZE characters */
	    if (((len - nbytes) % iop->bufsize) < (len % iop->bufsize))
		io_output(iop, IO_NOWAIT);
	}
    }
    return nbytes;
}

int
io_output_all(struct iop *iop)
{
    int n;

    /*
     * Mustn't block a player thread while update is pending, or else
     * a malicous player could delay the update indefinitely
     */
    while (((n = io_output(iop, IO_NOWAIT)) > 0) && !update_pending) {
	empth_select(iop->fd, EMPTH_FD_WRITE);
    }
    return n;
}

int
io_gets(struct iop *iop, s_char *buf, int nbytes)
{
    if ((iop->flags & IO_READ) == 0)
	return -1;
    return ioq_gets(iop->input, buf, nbytes);
}

int
io_puts(struct iop *iop, s_char *buf)
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
    }
    return 0;
}

int
io_noblocking(struct iop *iop, int value)
{
#if !defined(_WIN32)
    int flags;

    flags = fcntl(iop->fd, F_GETFL, 0);
    if (flags < 0)
	return -1;
    if (value == 0)
	flags &= ~FNDELAY;
    else
	flags |= FNDELAY;
    if (fcntl(iop->fd, F_SETFL, flags) < 0)
	return -1;
#else
    u_long arg = value;
    ioctlsocket(iop->fd, FIONBIO, &arg);
#endif
    if (value == 0)
	iop->flags &= ~IO_NBLOCK;
    else
	iop->flags |= IO_NBLOCK;
    return 0;
}

int
io_conn(struct iop *iop)
{
    return iop->flags & IO_CONN;
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

int
io_fileno(struct iop *iop)
{
    return iop->fd;
}
