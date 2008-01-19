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
 *  linebuf.c: Simple line buffer
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include <assert.h>
#include <stdlib.h>
#include "linebuf.h"

/*
 * Initialize empty line buffer.
 * Not necessary if *LBUF is already zeroed.
 */
void
lbuf_init(struct lbuf *lbuf)
{
    lbuf->len = lbuf->full = 0;
}

/*
 * Return length of currently buffered line.
 * This includes the newline if present.
 */
int
lbuf_len(struct lbuf *lbuf)
{
    return lbuf->len;
}

/*
 * Is LBUF full (i.e. we got the newline)?
 */
int
lbuf_full(struct lbuf *lbuf)
{
    return lbuf->full;
}

/*
 * Return a pointer to the currently buffered line.
 * If you mess with the line in a way that changes the line length,
 * better call lbuf_init() next.
 */
char *
lbuf_line(struct lbuf *lbuf)
{
    assert(lbuf->len < sizeof(lbuf->line));
    lbuf->line[lbuf->len] = 0;
    return lbuf->line;
}

/*
 * Append CH to the line buffered in LBUF.
 * LBUF must not be full.
 * If CH is a newline, the buffer is now full.  Return the line
 * length, including the newline.
 * Else return 0 if there was space, and -1 if not.
 */
int
lbuf_putc(struct lbuf *lbuf, char ch)
{
    assert(!lbuf->full);

    if (ch == '\n') {
	assert(lbuf->len + 1 < sizeof(lbuf->line));
	lbuf->line[lbuf->len++] = ch;
	lbuf->line[lbuf->len] = 0;
	lbuf->full = 1;
	return lbuf->len;
    }

    if (lbuf->len + 2 >= sizeof(lbuf->line))
	return -1;		/* truncating long line */

    lbuf->line[lbuf->len++] = ch;
    return 0;
}
