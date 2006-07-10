/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ioqueue.h: Stores and frees data associated with a file descriptor.
 *             uses writev to write, and read to read
 *  Known contributors to this file:
 * 
 */

#ifndef IOQUEUE_H
#define IOQUEUE_H

#ifndef _WIN32
#include <sys/uio.h>
#endif
#include "queue.h"

struct io {
    struct emp_qelem queue;
    int size;
    int nbytes;
    int offset;
    char *data;
};

struct ioqueue {
    struct io list;
    int bufsize;
    int cc;
};

extern struct ioqueue *ioq_create(int size);
extern void ioq_destroy(struct ioqueue *ioq);
extern void ioq_drain(struct ioqueue *ioq);
#if defined (_WIN32)
extern int ioq_makebuf(struct ioqueue *ioq, char *pBuf, int nBufLen);
#else
extern int ioq_makeiov(struct ioqueue *ioq, struct iovec *iov, int cc);
#endif
extern int ioq_peek(struct ioqueue *ioq, char *buf, int cc);
extern int ioq_dequeue(struct ioqueue *ioq, int cc);
extern void ioq_append(struct ioqueue *ioq, char *buf, int cc);
extern int ioq_qsize(struct ioqueue *ioq);
extern int ioq_gets(struct ioqueue *ioq, char *buf, int cc);
extern int ioq_puts(struct ioqueue *ioq, char *buf);

#endif
