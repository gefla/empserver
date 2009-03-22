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
 *  whatitem.c: Return pointer to item chr struct given an item name
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "prototypes.h"

/*
 * Get item type argument.
 * If INPUT is not empty, use it, else prompt for more input using PROMPT.
 * Return item characteristics on success, else NULL.
 */
struct ichrstr *
whatitem(char *input, char *prompt)
{
    char *p;
    struct ichrstr *ip;
    char buf[1024];

    p = getstarg(input, prompt, buf);
    if (p == 0 || *p == 0)
	return NULL;
    ip = item_by_name(p);
    if (!ip)
	pr("Unrecognized item \"%c\"\n", *p);
    return ip;
}

/*
 * Map item type name STR to item characteristics, NULL if not found.
 */
struct ichrstr *
item_by_name(char *str)
{
    struct ichrstr *ip;

    for (ip = ichr; ip->i_mnem != 0; ip++) {
	/* FIXME check i_name if str[2]? */
	if (*str == ip->i_mnem)
	    return ip;
    }
    return NULL;
}
