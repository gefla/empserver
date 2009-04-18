/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ringbuf.c: Simple ring buffer
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2007-2009
 */

#include <config.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include "ringbuf.h"

/*
 * Initialize empty ring buffer.
 * Not necessary if *R is already zeroed.
 */
void
ring_init(struct ring *r)
{
    r->cons = r->prod = 0;
}

/*
 * Return number of bytes held in ring buffer.
 */
int
ring_len(struct ring *r)
{
    assert(r->prod - r->cons <= RING_SIZE);
    return r->prod - r->cons;
}

/*
 * Return how much space is left in ring buffer.
 */
int
ring_space(struct ring *r)
{
    return RING_SIZE - (r->prod - r->cons);
}

/*
 * Peek at ring buffer contents.
 * N must be between -RING_SIZE-1 and RING_SIZE.
 * If N>=0, peek at the (N+1)-th byte to be gotten.
 * If N<0, peek at the -N-th byte that has been put in.
 * Return the byte as unsigned char coverted to int, or EOF if there
 * aren't that many bytes in the ring buffer.
 */
int
ring_peek(struct ring *r, int n)
{
    unsigned idx;

    assert(-RING_SIZE - 1 <= n && n <= RING_SIZE);

    idx = n >= 0 ? r->cons + n : r->prod - -n;
    if (idx < r->cons && idx >= r->prod)
	return EOF;
    return r->buf[idx % RING_SIZE];
}

/*
 * Get and remove the oldest byte from the ring buffer.
 * Return it as unsigned char coverted to int, or EOF if the buffer was
 * empty.
 */
int
ring_getc(struct ring *r)
{
    if (r->cons == r->prod)
	return EOF;
    return r->buf[r->cons++ % RING_SIZE];
}

/*
 * Attempt to put byte C into the ring buffer.
 * Return EOF when the buffer is full, else C.
 */
int
ring_putc(struct ring *r, unsigned char c)
{
    if (r->prod - r->cons == RING_SIZE)
	return EOF;
    return r->buf[r->prod++ % RING_SIZE] = c;
}

/*
 * Attempt to put SZ bytes from BUF into the ring buffer.
 * Return space left in ring buffer when it fits, else don't change
 * the ring buffer and return how much space is missing times -1.
 */
int
ring_putm(struct ring *r, void *buf, size_t sz)
{
    char *p = buf;
    int left = ring_space(r) - sz;
    int res;
    size_t i;

    if (left >= 0) {
	res = 0;
	for (i = 0; i < sz; i++)
	    res = ring_putc(r, p[i]);
	assert(res != EOF);
    }

    return left;
}

/*
 * Discard the N oldest bytes from the ring buffer.
 * It must hold at least that many.
 */
void
ring_discard(struct ring *r, int n)
{
    assert(0 <= n && n <= ring_len(r));
    r->cons += n;
}

/*
 * Search the ring buffer for zero-terminated string S.
 * If found, return a non-negative value referring to the beginning of
 * S in the buffer when passed to ring_peek().  Else return -1.
 */
int
ring_search(struct ring *r, char *s)
{
    size_t len = strlen(s);
    size_t i, j;

    for (i = r->cons; i + len <= r->prod; i++) {
	for (j = 0; j < len && s[j] == r->buf[(i + j) % RING_SIZE]; j++) ;
	if (!s[j])
	    return i - r->cons;
    }
    return -1;
}

/*
 * Fill ring buffer from file referred by file descriptor FD.
 * If ring buffer is already full, do nothing and return 0.
 * Else attempt to read as many bytes as space permits, with readv(),
 * and return its value.
 */
int
ring_from_file(struct ring *r, int fd)
{
    unsigned cons = r->cons % RING_SIZE;
    unsigned prod = r->prod % RING_SIZE;
    struct iovec iov[2];
    int cnt;
    ssize_t res;

    if (r->prod == r->cons + RING_SIZE)
	return 0;

    iov[0].iov_base = r->buf + prod;
    if (cons <= prod) {
	/* r->buf[prod..] */
	iov[0].iov_len = RING_SIZE - prod;
	/* r->buf[..cons-1] */
	iov[1].iov_base = r->buf;
	iov[1].iov_len = cons;
	cnt = 2;
    } else {
	/* r->buf[prod..cons-1] */
	iov[0].iov_len = cons - prod;
	cnt = 1;
    }
    res = readv(fd, iov, cnt);
    if (res < 0)
	return res;
    r->prod += res;
    return res;
}

/*
 * Drain ring buffer to file referred by file descriptor FD.
 * If ring buffer is already empty, do nothing and return 0.
 * Else attempt to write complete contents with writev(), and return
 * its value.
 */
int
ring_to_file(struct ring *r, int fd)
{
    unsigned cons = r->cons % RING_SIZE;
    unsigned prod = r->prod % RING_SIZE;
    struct iovec iov[2];
    int cnt;
    ssize_t res;

    if (r->cons == r->prod)
	return 0;

    iov[0].iov_base = r->buf + cons;
    if (prod <= cons) {
	/* r->buf[cons..] */
	iov[0].iov_len = RING_SIZE - cons;
	/* r->buf[..prod-1] */
	iov[1].iov_base = r->buf;
	iov[1].iov_len = prod;
	cnt = 2;
    } else {
	/* r->buf[cons..prod-1] */
	iov[0].iov_len = prod - cons;
	cnt = 1;
    }
    res = writev(fd, iov, cnt);
    if (res < 0)
	return res;
    r->cons += res;
    return res;
}
