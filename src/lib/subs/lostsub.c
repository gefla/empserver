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
 *  lostsub.c: Subroutines for lost items
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1997
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "file.h"
#include "land.h"
#include "ship.h"
#include "xy.h"
#include "nat.h"
#include "nsc.h"
#include "plane.h"
#include "lost.h"
#include "prototypes.h"

void
makelost(char type, natid owner, short int id, coord x, coord y)
{
    struct loststr lost;
    int n;

    n = findlost(type, owner, id, x, y, 1);
    if (n < 0) {
	ef_extend(EF_LOST, 25);
	n = findlost(type, owner, id, x, y, 1);
	if (n < 0)
	    return;
    }

    getlost(n, &lost);
    lost.lost_type = type;
    lost.lost_owner = owner;
    lost.lost_id = id;
    lost.lost_x = x;
    lost.lost_y = y;
    lost.lost_uid = n;
    time(&lost.lost_timestamp);
    putlost(n, &lost);
}

void
makenotlost(char type, natid owner, short int id, coord x, coord y)
{
    struct loststr lost;
    int n;

    n = findlost(type, owner, id, x, y, 0);
    if (n < 0)
	return;
    getlost(n, &lost);
    lost.lost_owner = 0;
    lost.lost_timestamp = 0;
    putlost(n, &lost);
}

int
findlost(char type, natid owner, short int id, coord x, coord y, int free)





	  /* Give me the item of a free slot */
{
    struct loststr lost;
    int n;
    int freeslot = -1;

/* Find a free slot, or find this item again */
    for (n = 0; getlost(n, &lost); n++) {
	if (!lost.lost_owner && freeslot == -1 && free == 1)
	    freeslot = n;
	if (!lost.lost_owner)
	    continue;
	if (lost.lost_owner == owner && type == lost.lost_type) {
	    if (type == EF_SECTOR && lost.lost_x == x && lost.lost_y == y) {
		freeslot = n;
		break;
	    } else if (lost.lost_id == id) {
		freeslot = n;
		break;
	    }
	}
    }
    return freeslot;
}
