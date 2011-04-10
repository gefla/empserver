/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
/* src/lib/subs/maps.c */
extern int do_map(int bmap, int unit_type, char *arg1, char *arg2);
extern int display_region_map(int, int, coord, coord, char *, char *);
extern int bmaps_intersect(natid, natid);
extern int share_bmap(natid, natid, struct nstr_sect *, char, char *);

#endif
