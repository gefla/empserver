/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ringbuf.h: Simple ring buffer
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2007-2017
 */

#ifndef RINGBUF_H
#define RINGBUF_H

#include <stddef.h>
#include <sys/uio.h>

#define RING_SIZE 4096

/* Ring buffer, consumer reads, producer writes */
struct ring {
    /* All members are private! */
    /*
     * Consumer reads from buf[cons % RING_SIZE], incrementing cons
     * Produces writes to buf[prod % RING_SIZE], incrementing prod
     * prod == cons: empty
     * prod == cons + RING_SIZE: full
     * invariant: prod - cons <= RING_SIZE
     */
    unsigned cons, prod;
    unsigned char buf[RING_SIZE];
};

extern void ring_init(struct ring *);
extern int ring_len(struct ring *);
extern int ring_space(struct ring *);
extern int ring_peek(struct ring *, int);
extern int ring_getc(struct ring *);
extern int ring_putc(struct ring *, unsigned char);
extern int ring_putm(struct ring *, void *, size_t);
extern void ring_discard(struct ring *, int);
extern int ring_search(struct ring *, char *, int);
extern int ring_from_file(struct ring *, int fd);
extern int ring_to_iovec(struct ring *, struct iovec[]);

#endif
