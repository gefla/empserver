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
 *  ioqueue.c: Read and write i/o queues
 * 
 *  Known contributors to this file:
 *     
 */

/*
 * Read and write onto io queues.  Note that
 * the io queues don't actually do any writing;
 * that is left for a higher level.
 */

#include <stdio.h>
#include <stdlib.h>		/* malloc free */
#include <sys/types.h>
#if !defined(_WIN32)
#include <sys/uio.h>
#endif
#include "misc.h"
#include "queue.h"
#include "ioqueue.h"

static int ioqtocbuf(struct ioqueue *ioq, s_char *buf, int cc,
		     register int stopc);
#if !defined(_WIN32)
static int ioqtoiov(struct ioqueue *ioq, struct iovec *iov,
		    register int max);
#endif
static int ioqtobuf(struct ioqueue *ioq, s_char *buf, int cc);
static int appendcc(struct ioqueue *ioq, s_char *buf, int cc);
static int removecc(struct ioqueue *ioq, register int cc);

#if defined(_WIN32)
static void loc_StripDels(char *pBuf);
#endif

struct ioqueue *
ioq_create(int size)
{
    struct ioqueue *ioq;

    ioq = malloc(sizeof(struct ioqueue));
    emp_initque(&ioq->list.queue);
    ioq->list.nbytes = 0;
    ioq->list.offset = 0;
    ioq->list.size = 0;
    ioq->list.data = 0;
    ioq->bufsize = size;
    ioq->cc = 0;
    return ioq;
}

void
ioq_destroy(struct ioqueue *ioq)
{
    ioq_drain(ioq);
    free(ioq);
}

void
ioq_drain(struct ioqueue *ioq)
{
    struct emp_qelem *qp;
    struct io *io;

    while ((qp = ioq->list.queue.q_forw) != &ioq->list.queue) {
	io = (struct io *)qp;
	emp_remque(&io->queue);
	free(io->data);
	free(io);
    }

    ioq->cc = 0;
}

/*
 * copy batch of pointers into the passed
 * iovec, but don't actually dequeue the data.
 * return # of iovec initialized.
 */
#if !defined(_WIN32)
int
ioq_makeiov(struct ioqueue *ioq, struct iovec *iov, int cc)
{
    if (ioq->cc <= 0)
	return 0;
    return ioqtoiov(ioq, iov, cc);
}
#endif

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
    return removecc(ioq, cc);
}

void
ioq_append(struct ioqueue *ioq, s_char *buf, int cc)
{
    appendcc(ioq, buf, cc);
}

int
ioq_qsize(struct ioqueue *ioq)
{
    return ioq->cc;
}

/*
 * read a line of text up to (but not including)
 * the newline.  return -1 and read nothing if
 * no input is available
 */
int
ioq_gets(struct ioqueue *ioq, s_char *buf, int cc)
{
    int nbytes;
    int actual;

    nbytes = ioqtocbuf(ioq, buf, cc - 1, '\n');
    if (nbytes >= 0) {
	actual = nbytes;
	if (actual > cc - 1)
	    actual = cc - 1;
	/* telnet terminates lines with "\r\n", get rid of \r */
	if (actual > 0 && buf[actual-1] == '\r')
	    actual--;
	buf[actual] = '\0';
	/* remove the newline too */
	removecc(ioq, nbytes + 1);
    }
    return nbytes;
}

int
ioq_puts(struct ioqueue *ioq, s_char *buf)
{
    return appendcc(ioq, buf, strlen(buf));
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
ioqtobuf(struct ioqueue *ioq, s_char *buf, int cc)
{
    struct io *io;
    struct emp_qelem *qp;
    struct emp_qelem *head;
    register int nbytes;
    register int nleft;
    register s_char *offset;

    nleft = cc;
    offset = buf;
    head = &ioq->list.queue;
    for (qp = head->q_forw; qp != head && nleft > 0; qp = qp->q_forw) {
	io = (struct io *)qp;
	if ((nbytes = io->nbytes - io->offset) < 0) {
	    /* XXX log something here */
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
 * copy at most cc bytes from ioq to buf,
 * terminating on the stop character.
 */
static int
ioqtocbuf(struct ioqueue *ioq, s_char *buf, int cc, register int stopc)
{
    register int nbytes;
    register s_char *p;
    register int n;
    struct io *io;
    struct emp_qelem *qp;
    struct emp_qelem *head;
    int total;
    int found;

    head = &ioq->list.queue;
    found = 0;
    total = 0;
    for (qp = head->q_forw; qp != head; qp = qp->q_forw) {
	io = (struct io *)qp;
	if ((nbytes = io->nbytes - io->offset) <= 0)
	    continue;
	p = io->data + io->offset;
	for (n = 0; n < nbytes && p[n] != stopc; n++) ;
	total += n;
	if (n < nbytes) {
	    found++;
	    break;
	}
    }
    if (found == 0)
	return -1;
    ioqtobuf(ioq, buf, cc < total ? cc : total);
    return total;
}

/*
 * initialize an iovec to point at max bytes worth
 * of data from the ioqueue.
 */
#if !defined(_WIN32)
static int
ioqtoiov(struct ioqueue *ioq, struct iovec *iov, register int max)
{
    struct io *io;
    register int cc;
    register int niov;
    register int len;
    struct emp_qelem *qp;

    cc = max;
    niov = 0;
    qp = ioq->list.queue.q_forw;
    while (qp != &ioq->list.queue && cc > 0) {
	io = (struct io *)qp;
	len = io->nbytes - io->offset;
	if (len > cc)
	    len = cc;
	iov->iov_base = io->data + io->offset;
	iov->iov_len = len;
	cc -= len;
	niov++;
	iov++;
	qp = qp->q_forw;
	if (niov >= 16)
	    break;
    }
    return niov;
}
#endif

/*
 * append a buffer to the end of the ioq.
 */
static int
appendcc(struct ioqueue *ioq, s_char *buf, int cc)
{
    struct io *io;
    int len;
    s_char *ptr;
    int avail;

    /* determine if any space is left */
    io = (struct io *)ioq->list.queue.q_back;
    avail = io->size - io->nbytes;
    if (avail > 0) {
	/* append to existing buffer */
	len = cc > avail ? avail : cc;
	memcpy(io->data + io->nbytes, buf, len);
	io->nbytes += len;
	ioq->cc += len;
	if (avail < cc)
	    appendcc(ioq, buf + len, cc - len);
    } else {
	/* create a new buffer, minimum bufsize bytes */
	len = cc > ioq->bufsize ? cc : ioq->bufsize;
	ptr = malloc(len);
	memcpy(ptr, buf, cc);
	io = malloc(sizeof(struct io));
	io->nbytes = cc;
	io->size = len;
	io->offset = 0;
	io->data = ptr;
	emp_insque(&io->queue, ioq->list.queue.q_back);
	ioq->cc += cc;
    }
    return cc;
}

/*
 * remove cc bytes from ioqueue ioq
 * free memory, dequeue io elements
 * which are no longer used.
 */
static int
removecc(struct ioqueue *ioq, register int cc)
{
    struct io *io;
    struct emp_qelem *qp;
    register int nbytes;
    register int there;
    register int remain;

    nbytes = 0;
    remain = cc;
    while ((qp = ioq->list.queue.q_forw) != &ioq->list.queue) {
	io = (struct io *)qp;
	there = io->nbytes - io->offset;
	if (there < 0) {
	    /* error */
	    emp_remque(&io->queue);
	    free(io);
	    continue;
	}
	if (remain >= there) {
	    /* not enough or exact; free entry */
	    nbytes += there;
	    remain -= there;
	    emp_remque(&io->queue);
	    free(io->data);
	    free(io);
	} else {
	    /* too much; increment offset */
	    io->offset += remain;
	    nbytes += remain;
	    remain = 0;
	}
	if (remain <= 0)
	    break;
    }
    ioq->cc -= nbytes;
    return nbytes;
}

#if defined(_WIN32)
/*
 * Make an (output) buffer up to the
 * maximum size of the buffer.
 *
 * We don't free the bytes...
 */
int
ioq_makebuf(struct ioqueue *ioq, char *pBuf, int nBufLen)
{
    struct io *io;
    struct emp_qelem *qp;
    struct emp_qelem *head;
    int nbytes;
    int nleft;
    int ncopied;
    s_char *offset;

    ncopied = 0;
    nleft = nBufLen;
    offset = pBuf;
    head = &ioq->list.queue;

    for (qp = head->q_forw; (qp != head) && (nleft > 0); qp = qp->q_forw) {
	io = (struct io *)qp;
	nbytes = io->nbytes - io->offset;
	if (nbytes < 0) {
	    /* Paranoid check for bad buffer. */
	    continue;
	}

	/* too many bytes, wait till next time. */
	if (nbytes > nleft)
	    break;

	memcpy(offset, io->data + io->offset, nbytes);
	offset += nbytes;
	nleft -= nbytes;
	ncopied += nbytes;
    }
    return ncopied;
}
#endif /* _WIN32 */
