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
 *  map.h: Definitions for making maps
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef _MAP_H_
#define _MAP_H_

#include "misc.h"
#include "xy.h"

#define	MAPWIDTH(persec) ((WORLD_X/2)*(persec + 1)+1)

extern s_char *routech[7][2];

extern void blankfill(s_char *, register struct range *, int);
/* src/lib/subs/border.c */
extern int map_set(natid, coord, coord, s_char, int);
extern void writebmap(natid);
extern void writemap(natid);
/* src/lib/subs/border.c */
extern void border(struct range *, s_char *, s_char *);

#define MAP_SHIP	bit(0)
#define MAP_LAND	bit(1)
#define MAP_PLANE	bit(2)
#define MAP_HIGH	bit(3)
#define MAP_ALL		MAP_SHIP | MAP_LAND | MAP_PLANE

#endif /* _MAP_H_ */
