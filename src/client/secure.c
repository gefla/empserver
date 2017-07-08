/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2017, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  secure.c: Check redir etc. to protect against tampering deity
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2007-2017
 */

#include <config.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "ringbuf.h"
#include "secure.h"

static struct ring recent_input;

/*
 * Remember input @inp for a while.
 */
void
save_input(char inp)
{
    int eol;

    while (ring_putc(&recent_input, inp) < 0) {
	eol = ring_search(&recent_input, "\n", 0);
	assert(eol >= 0);
	ring_discard(&recent_input, eol + 1);
    }
}

/*
 * Can you still remember a line of input that ends with @tail?
 * It must end with a newline.
 */
int
seen_input(char *tail)
{
    size_t len = strlen(tail);

    assert(len && tail[len - 1] == '\n');
    return ring_search(&recent_input, tail, 0) >= 0;
}

/*
 * Can you still remember input that looks like an execute @arg?
 * @arg must end with a newline.
 */
int
seen_exec_input(char *arg)
{
    size_t len = strlen(arg);
    int n, i, j, ch;
    unsigned char buf[RING_SIZE + 1];

    assert(len && arg[len - 1] == '\n');

    n = 1;
    for (;;) {
	/* find next line ending with arg */
	n = ring_search(&recent_input, arg, n + 1);
	if (n <= 0)
	    return 0;

	/* extract command (same or previous line) */
	i = n - 1;
	if (ring_peek(&recent_input, i) == '\n')
	    i--;
	j = sizeof(buf);
	buf[--j] = 0;
	for (; i >= 0 && (ch = ring_peek(&recent_input, i)) != '\n'; i--)
	    buf[--j] = ch;

	/* compare command */
	for (; isspace(buf[j]); j++) ;
	for (i = j; buf[i] && !isspace(buf[i]); i++) ;
	if (i - j >= 2 && !strncmp("execute", (char *)buf + j, i - j))
	    return 1;
    }
}
