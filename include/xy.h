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
 *  xy.h: Constants having to do with world locations.
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#ifndef XY_H
#define XY_H

#include "types.h"

/* Return the number of sectors in the world */
#define WORLD_SZ() (WORLD_X * WORLD_Y / 2)

/* Fast version of sctoff() for normalized arguments */
#define XYOFFSET(x, y) (((y) * WORLD_X + (x)) / 2)

#define XNORM(x) \
    (((x) < 0) ? (WORLD_X - 1 - ((-(x) - 1) % WORLD_X)) : ((x) % WORLD_X))
#define YNORM(y) \
    (((y) < 0) ? (WORLD_Y - 1 - ((-(y) - 1) % WORLD_Y)) : ((y) % WORLD_Y))

struct range {
    coord lx, ly;		/* low-range x,y (inclusive) */
    coord hx, hy;		/* high-range x,y (inclusive) */
    int width, height;
};

extern char *xyas(coord x, coord y, natid country);
extern char *ownxy(struct sctstr *sp);
extern coord xrel(struct natstr *np, coord absx);
extern coord yrel(struct natstr *np, coord absy);
extern void xyrelrange(struct natstr *np, struct range *src,
		       struct range *dst);
extern void xyabsrange(struct natstr *np, struct range *src,
		       struct range *dst);
extern coord strtox(char *str, char **end);
extern coord strtoy(char *str, char **end);
extern coord xabs(struct natstr *np, coord relx);
extern coord yabs(struct natstr *np, coord rely);
extern coord xnorm(coord x);
extern coord ynorm(coord y);
extern int xyinrange(coord x, coord y, struct range *rp);

#endif
