/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  whatitem.c: Return pointer to item chr struct given an item name
 * 
 *  Known contributors to this file:
 * 
 */

#include "misc.h"
#include "item.h"
#include "prototypes.h"

struct ichrstr *
whatitem(s_char *ptr, s_char *prompt)
{
    register s_char *p;
    register struct ichrstr *ip;
    s_char buf[1024];

    p = getstarg(ptr, prompt, buf);
    if (p == 0 || *p == 0)
	return 0;
    for (ip = &ichr[1]; ip->i_mnem != 0; ip++) {
	if (*p == ip->i_mnem)
	    return ip;
    }
    pr("Unrecognized item \"%c\"\n", *p);
    return 0;
}
