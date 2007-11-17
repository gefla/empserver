/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#include <string.h>
#include "ringbuf.h"
#include "secure.h"

struct ring recent_input;

void
save_input(char *inp)
{
    size_t len = strlen(inp);
    int left;

    left = ring_putm(&recent_input, inp, len);
    if (left < 0) {
	ring_discard(&recent_input, ring_search(&recent_input, "\n"));
	ring_putm(&recent_input, inp, len);
    }
}

int
seen_input(char *tail)
{
    int dist = ring_search(&recent_input, tail);

    if (dist < 0)
	return 0;

    ring_discard(&recent_input, dist + strlen(tail));
    return 1;
}
