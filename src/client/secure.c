/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  secure.c: Check redir etc. to protect against tampering deity
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include <assert.h>
#include <string.h>
#include "ringbuf.h"
#include "secure.h"

static struct ring recent_input;
static size_t saved_bytes;

/*
 * Remember line of input INP for a while.
 * It must end with a newline.
 * Return value is suitable for forget_input(): it makes it forget all
 * input up to and including this line.
 */
size_t
save_input(char *inp)
{
    size_t len = strlen(inp);
    int eol;

    assert(len && inp[len - 1] == '\n');

    while (ring_putm(&recent_input, inp, len) < 0) {
	eol = ring_search(&recent_input, "\n");
	assert(eol >= 0);
	ring_discard(&recent_input, eol + 1);
    }
    saved_bytes += len;
    return saved_bytes;
}

/*
 * Can you still remember a line of input that ends with TAIL?
 * It must end with a newline.
 * Return non-zero iff TAIL can be remembered.
 * Passing that value to forget_input() will forget all input up to
 * and including this line.
 */
size_t
seen_input(char *tail)
{
    size_t len = strlen(tail);
    size_t remembered = ring_len(&recent_input);
    int dist;

    assert(len && tail[len - 1] == '\n');

    dist = ring_search(&recent_input, tail);
    if (dist < 0)
	return 0;

    assert(dist + len <= remembered && remembered <= saved_bytes);
    return saved_bytes - remembered + dist + len;
}

/*
 * Forget remembered input up to SEEN.
 * SEEN should be obtained from save_input() or seen_input().
 */
void
forget_input(size_t seen)
{
    size_t forgotten = saved_bytes - ring_len(&recent_input);

    assert(seen);

    if (seen > forgotten) {
	assert(ring_peek(&recent_input, seen - forgotten - 1) == '\n');
	ring_discard(&recent_input, seen - forgotten);
    }
}
