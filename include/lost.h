/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  lost.h: Definitions for lost items
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1997
 */

#ifndef _LOST_H_
#define _LOST_H_
struct loststr {
    short ef_type;
    natid lost_owner;		/* Who lost it */
    int lost_uid;		/* lost ID */
    char lost_type;		/* Type of thing (ship, plane, nuke, land, sector) */
    short lost_id;		/* ID of lost thing */
    coord lost_x;
    coord lost_y;
    time_t lost_timestamp;	/* When it was lost */
};

#define getlost(n, p) ef_read(EF_LOST, n, p)
#define putlost(n, p) ef_write(EF_LOST, n, p)

/* src/lib/subs/lostsub.c */
extern int findlost(char, natid, short, coord, coord, int);
extern void makelost(char, natid, short, coord, coord);
extern void makenotlost(char, natid, short, coord, coord);
#endif /* _LOST_H_ */
