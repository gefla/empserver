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
 *  lost.h: Definitions for lost items
 *
 *  Known contributors to this file:
 *     Steve McClure, 1997
 */

#ifndef LOST_H
#define LOST_H

#include "file.h"
#include "types.h"

struct loststr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned lost_seqno: 12;
    unsigned lost_generation: 12;
    int lost_uid;
    time_t lost_timestamp;	/* When it was lost */
    natid lost_owner;		/* Who lost it */
    /* end of part matching struct empobj */
    short lost_type;		/* Type of thing (ship, plane, nuke, land, sector) */
    int lost_id;		/* UID of lost thing (0 for sector) */
    coord lost_x;
    coord lost_y;
};

#define getlost(n, p) ef_read(EF_LOST, (n), (p))
#define putlost(n, p) ef_write(EF_LOST, (n), (p))

/* src/lib/subs/lostsub.c */
extern void lost_and_found(int, natid, natid, int, coord, coord);
extern void makelost(int, natid, int, coord, coord);
extern void makenotlost(int, natid, int, coord, coord);
extern void delete_old_lostitems(void);

#endif
