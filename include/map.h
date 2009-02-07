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
 *  map.h: Definitions for making maps
 *
 *  Known contributors to this file:
 *
 */

#ifndef MAP_H
#define MAP_H

#include "types.h"

/*
 * Width of the body of a map using PERSEC characters per sector.
 *
 * One row shows WORLD_X/2 sectors, separated by one space.  Requires
 * WORLD_X/2 * (PERSEC+1) - 1 characters.
 *
 * Every other row is indented so that the center of the first sector
 * is aligned with the space separating the first two sectors in the
 * adjacent rows.  For odd PERSEC, that's (PERSEC+1)/2 additional
 * characters.  For even PERSEC, it's either PERSEC/2 or PERSEC/2 + 1,
 * depending on whether we align the character left or right of the
 * center with the space (the map will look rather odd either way).
 *
 * We need one more character for the terminating zero.
 */
#define MAPWIDTH(persec) (WORLD_X/2 * ((persec) + 1) + ((persec) + 2) / 2)

/* src/lib/subs/bigmap.c */
extern int map_set(natid, coord, coord, char, int);
extern void writebmap(natid);
extern void writemap(natid);
/* src/lib/subs/border.c */
extern void blankfill(char *, struct range *, int);
extern void border(struct range *, char *, char *);

#define MAP_PLANE	bit(0) /* this order must match */
#define MAP_SHIP	bit(1) /* priority of the map and */
#define MAP_LAND	bit(2) /* ef_unit_list */
#define MAP_NUKE	bit(3)
#define MAP_HIGH	bit(4)
#define MAP_ALL		MAP_SHIP | MAP_LAND | MAP_PLANE | MAP_NUKE

#endif
