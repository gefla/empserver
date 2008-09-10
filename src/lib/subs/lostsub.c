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
 *  lostsub.c: Subroutines for lost items
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1997
 *     Markus Armbruster, 2008
 */

#include <config.h>

#include "file.h"
#include "lost.h"
#include "misc.h"
#include "optlist.h"

static int findlost(short, natid, short, coord, coord, int);

/*
 * Record item ID of type TYPE changed owner from EXOWN to OWN at X, Y.
 */
void
lost_and_found(int type, natid exown, natid own, int id, coord x, coord y)
{
    if (exown == own)
	return;
    if (exown)
	makelost(type, exown, id, x, y);
    if (own)
	makenotlost(type, own, id, x, y);
}

void
makelost(short type, natid owner, short id, coord x, coord y)
{
    struct loststr lost;
    int n;

    n = findlost(type, owner, id, x, y, 1);
    ef_blank(EF_LOST, n, &lost);
    lost.lost_type = type;
    lost.lost_owner = owner;
    lost.lost_id = id;
    lost.lost_x = x;
    lost.lost_y = y;
    putlost(n, &lost);
}

void
makenotlost(short type, natid owner, short id, coord x, coord y)
{
    struct loststr lost;
    int n;

    n = findlost(type, owner, id, x, y, 0);
    if (n < 0)
	return;
    getlost(n, &lost);
    lost.lost_owner = 0;
    putlost(n, &lost);
}


/*
 * Find a suitable slot in the lost file.
 * If a record for TYPE, OWNER, ID, X, Y exists, return its index.
 * Else if FREE is non-zero, return the index of an unused record.
 * Else return -1.
 */
static int
findlost(short type, natid owner, short id, coord x, coord y, int free)
{
    struct loststr lost;
    int n;
    int freeslot = -1;

    for (n = 0; getlost(n, &lost); n++) {
	if (!lost.lost_owner && freeslot < 0)
	    freeslot = n;
	if (!lost.lost_owner)
	    continue;
	if (lost.lost_owner == owner && type == lost.lost_type) {
	    if (type == EF_SECTOR && lost.lost_x == x && lost.lost_y == y)
		return n;
	    else if (lost.lost_id == id)
		return n;
	}
    }

    if (free) {
	if (freeslot < 0)
	    freeslot = n;
	return freeslot;
    }

    return -1;
}

void
delete_old_lostitems(void)
{
    time_t expiry_time = time(NULL) - hours(lost_keep_hours);
    struct loststr lost;
    int i;

    for (i = 0; getlost(i, &lost); i++) {
	if (!lost.lost_owner)
	    continue;
	if (lost.lost_timestamp >= expiry_time)
	    continue;
	lost.lost_owner = 0;
	putlost(i, &lost);
    }
}
