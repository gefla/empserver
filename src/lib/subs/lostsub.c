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
 *  lostsub.c: Subroutines for lost items
 *
 *  Known contributors to this file:
 *     Steve McClure, 1997
 *     Markus Armbruster, 2008-2016
 */

#include <config.h>

#include "lost.h"
#include "misc.h"
#include "optlist.h"

static int findlost(int, natid, int, coord, coord, int);

/*
 * Record item @id of @type changed owner from @exown to @own at @x,@y.
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
makelost(int type, natid owner, int id, coord x, coord y)
{
    struct loststr lost;
    int n;

    if (CANT_HAPPEN(!owner))
	return;
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
makenotlost(int type, natid owner, int id, coord x, coord y)
{
    struct loststr lost;
    int n;

    if (CANT_HAPPEN(!owner))
	return;
    n = findlost(type, owner, id, x, y, 0);
    if (n < 0)
	return;
    getlost(n, &lost);
    lost.lost_owner = 0;
    putlost(n, &lost);
}


/*
 * Find a suitable slot in the lost file.
 * If a record for @type, @owner, @id, @x, @y exists, return its index.
 * Else if @free is non-zero, return the index of an unused record.
 * Else return -1.
 */
static int
findlost(int type, natid owner, int id, coord x, coord y, int free)
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
	    if (type != EF_SECTOR && lost.lost_id == id)
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
