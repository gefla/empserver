/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ioqueue.c: Manage an i/o queue
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef _WIN32
#include <sys/uio.h>
#include <unistd.h>
#endif
#include "misc.h"
#include "queue.h"
#include "ioqueue.h"

#ifdef _WIN32
typedef struct iovec {
    char *iov_base;
    int iov_len;
} iovec_t;
#endif


static int ioqtobuf(register struct ioqueue *ioq, s_char *buf, int cc);
static int ioqtoiov(register struct ioqueue *ioq, register struct iovec *iov, register int max);
static void enqueuecc(struct ioqueue *ioq, s_char *buf, int cc);
static int dequeuecc(register struct ioqueue *ioq, register int cc);

void insque(struct qelem *, struct qelem *);
void remque(struct qelem *);
void initque(struct qelem *p);
struct qelem *makeqt(int nelem);

void
ioq_init(struct ioqueue *ioq, int bsize)
{
    extern s_char num_teles[];

    initque(&ioq->queue);
    ioq->cc = 0;
    ioq->bsize = bsize;
    *num_teles = '\0';
}

/*
 * copy batch of pointers into the passed
 * iovec, but don't actually dequeue the data.
 * return # of iovec initialized.
 */
int
ioq_peekiov(struct ioqueue *ioq, struct iovec *iov, int max)
{
    if (ioq->cc <= 0)
	return 0;
    return ioqtoiov(ioq, iov, max);
}

/*
 * Copy the specified number of characters into the buffer
 * provided, without actually dequeueing the data.  Return
 * number of bytes actually found.
 */
int
ioq_peek(struct ioqueue *ioq, s_char *buf, int cc)
{
    return ioqtobuf(ioq, buf, cc);
}

int
ioq_dequeue(struct ioqueue *ioq, int cc)
{
    if (dequeuecc(ioq, cc) != cc)
	return 0;
    return cc;
}

int
ioq_read(struct ioqueue *ioq, s_char *buf, int cc)
{
    int n;

    n = ioqtobuf(ioq, buf, cc);
    if (n > 0)
	dequeuecc(ioq, n);
    return n;
}

void
ioq_write(struct ioqueue *ioq, s_char *buf, int cc)
{
    enqueuecc(ioq, buf, cc);
}

int
ioq_qsize(struct ioqueue *ioq)
{
    return ioq->cc;
}

void
ioq_drain(struct ioqueue *ioq)
{
    struct io *io;
    struct qelem *qp;

    while ((qp = ioq->queue.q_forw) != &ioq->queue) {
	io = (struct io *)qp;
	free(io->data);
	(void)remque(&io->queue);
	(void)free(io);
    }
    ioq->cc = 0;
}

s_char *
ioq_gets(struct ioqueue *ioq, s_char *buf, int cc)
{
    register s_char *p;
    register s_char *end;
    int nbytes;

    nbytes = ioqtobuf(ioq, buf, cc);
    if (nbytes < cc)
	cc = nbytes;
    end = &buf[cc];
    for (p = buf; p < end && *p; p++) {
	if (*p == '\n') {
	    *p = '\0';
	    dequeuecc(ioq, (p - buf) + 1);
	    return buf;
	}
    }
    return 0;
}

/*
 * all the rest are local to this module
 */


/*
 * copy cc bytes from ioq to buf.
 * this routine doesn't free memory; this is
 * left for a higher level.
 */
static int
ioqtobuf(register struct ioqueue *ioq, s_char *buf, int cc)
{
    register struct io *io;
    struct qelem *qp;
    s_char *offset;
    int nbytes;
    int nleft;

    nleft = cc;
    offset = buf;
    for (qp = ioq->queue.q_forw; qp != &ioq->queue; qp = qp->q_forw) {
	io = (struct io *)qp;
	if ((nbytes = io->nbytes - io->offset) < 0) {
	    fprintf(stderr, "ioqtobuf: offset %d nbytes %d\n",
		    io->offset, io->nbytes);
	    continue;
	}
	if (nbytes > 0) {
	    if (nleft < nbytes)
		nbytes = nleft;
	    memcpy(offset, io->data + io->offset, nbytes);
	    offset += nbytes;
	    nleft -= nbytes;
	}
    }
    return offset - buf;
}

/*
 * translate "around" max bytes to an iovec
 * array.  The limit max is only advisory,
 * and more may get buffered.  It is an attempt to limit
 * really silly sends -- like sending 40k on a socket
 * with one writev for example.  This makes the processing
 * of a full ioqueue still be quick.
 */
static int
ioqtoiov(register struct ioqueue *ioq, register struct iovec *iov, register int max)
{
    register struct io *io;
    register int cc;
    register int niov;
    struct qelem *qp;

    cc = 0;
    niov = 0;
    qp = ioq->queue.q_forw;
    for (qp = ioq->queue.q_forw; qp != &ioq->queue; qp = qp->q_forw) {
	io = (struct io *)qp;
	if (niov >= MAXIOV || cc >= max)
	    break;
	iov->iov_base = io->data + io->offset;
	iov->iov_len = io->nbytes - io->offset;
	cc += io->nbytes - io->offset;
	niov++;
	iov++;
    }
    return niov;
}

/*
 * append a buffer to the end of the ioq.
 */
static void
enqueuecc(struct ioqueue *ioq, s_char *buf, int cc)
{
    struct io *io;

    io = (struct io *)malloc(sizeof(*io));
    io->nbytes = cc;
    io->offset = 0;
    io->data = buf;
    insque(&io->queue, ioq->queue.q_back);
    ioq->cc += cc;
}

/*
 * remove cc bytes from ioqueue ioq
 * free memory, dequeue io elements
 * which are no longer used.
 */
static int
dequeuecc(register struct ioqueue *ioq, register int cc)
{
    register struct io *io;
    register struct qelem *qp;
    register int nbytes;
    register int there;

    nbytes = 0;
    while ((qp = ioq->queue.q_forw) != &ioq->queue) {
	io = (struct io *)qp;
	there = io->nbytes - io->offset;
	if (there < 0) {
	    fprintf(stderr, "dequeuecc: nbytes %d, offset %d\n",
		    io->nbytes, io->offset);
	    continue;
	}
	if (cc > there) {
	    cc -= there;
	    nbytes += there;
	    (void)remque(&io->queue);
	    free(io->data);
	} else {
	    io->offset += cc;
	    nbytes += cc;
	    break;
	}
    }
    ioq->cc -= nbytes;
    return nbytes;
}
