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
 *  ringbuf.c: Simple ring buffer
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2007-2017
 */

#include <config.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ringbuf.h"

/*
 * Initialize empty ring buffer.
 * Not necessary if *@r is already zeroed.
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
 * @n must be between -RING_SIZE-1 and RING_SIZE.
 * If @n>=0, peek at the (@n+1)-th byte to be gotten.
 * If @n<0, peek at the -@n-th byte that has been put in.
 * Return the byte as unsigned char coverted to int, or EOF if there
 * aren't that many bytes in the ring buffer.
 */
int
ring_peek(struct ring *r, int n)
{
    unsigned idx;

    assert(-RING_SIZE - 1 <= n && n <= RING_SIZE);

    if (n >= 0) {
	idx = r->cons + n;
	if (idx >= r->prod)
	    return EOF;
    } else {
	/* Beware, r->prod - -n can wrap around, avoid that */
	if (r->prod < r->cons + -n)
	    return EOF;
	idx = r->prod - -n;
    }

    return r->buf[idx % RING_SIZE];
}

/*
 * Get and remove the oldest byte from the ring buffer.
 * Return it as unsigned char converted to int, or EOF if the buffer was
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
 * Attempt to put byte @c into the ring buffer.
 * Return EOF when the buffer is full, else @c.
 */
int
ring_putc(struct ring *r, unsigned char c)
{
    if (r->prod - r->cons == RING_SIZE)
	return EOF;
    return r->buf[r->prod++ % RING_SIZE] = c;
}

/*
 * Attempt to put @sz bytes from @buf into the ring buffer.
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
 * Discard the @n oldest bytes from the ring buffer.
 * It must hold at least that many.
 */
void
ring_discard(struct ring *r, int n)
{
    assert(0 <= n && n <= ring_len(r));
    r->cons += n;
}

/*
 * Search the ring buffer for zero-terminated string @s.
 * Start at the (@n+1)-th byte to be gotten.
 * If found, return the number of bytes in the buffer before @s.
 * Else return -1.
 */
int
ring_search(struct ring *r, char *s, int n)
{
    size_t len = strlen(s);
    size_t i, j;

    for (i = r->cons + n; i + len <= r->prod; i++) {
	for (j = 0; s[j] && s[j] == (char)r->buf[(i + j) % RING_SIZE]; j++)
	    ;
	if (!s[j])
	    return i - r->cons;
    }
    return -1;
}

/*
 * Fill ring buffer from file referred by file descriptor @fd.
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
 * Set up @iov[] to describe complete contents of ring buffer.
 * @iov[] must have at least two elements.
 * Return number of elements used (zero for an empty ring buffer).
 */
int
ring_to_iovec(struct ring *r, struct iovec iov[])
{
    unsigned cons = r->cons % RING_SIZE;
    unsigned prod = r->prod % RING_SIZE;

    if (r->cons == r->prod)
	return 0;

    iov[0].iov_base = r->buf + cons;
    if (prod > cons) {
	/* r->buf[cons..prod-1] */
	iov[0].iov_len = prod - cons;
	return 1;
    }
    /* r->buf[cons..] */
    iov[0].iov_len = RING_SIZE - cons;
    /* r->buf[..prod-1] */
    iov[1].iov_base = r->buf;
    iov[1].iov_len = prod;
    return 2;
}
