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
 *  ioqueue.h: stores and frees data associated with a file descriptor.
 *             uses writev to write, and read to read.
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef IOQUEUE_H
#define IOQUEUE_H

struct ioqueue {
    struct qelem queue;		/* queue fwd/back */
    int bsize;			/* basic block size */
    int cc;			/* character count */
};

struct io {
    struct qelem queue;		/* list of ioqueue elements */
    int nbytes;			/* number of data bytes present */
    int offset;			/* offset into current entry */
    char *data;			/* pointer to start */
};

void ioq_init(struct ioqueue *ioq, int bsize);
int ioq_peek(struct ioqueue *ioq, char *buf, int cc);
int ioq_dequeue(struct ioqueue *ioq, int cc);
int ioq_read(struct ioqueue *ioq, char *buf, int cc);
void ioq_write(struct ioqueue *ioq, char *buf, int cc);
int ioq_qsize(struct ioqueue *ioq);
void ioq_drain(struct ioqueue *ioq);
char *ioq_gets(struct ioqueue *ioq, char *buf, int cc, int *eol);

#endif
