/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  xy.h: Constants having to do with world locations.
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#ifndef _XY_H_
#define _XY_H_

#include "sect.h"
#include "nat.h"

/* Used to calculate an offset into an array.  Used for
   dynamically sizing the world. */
#define XYOFFSET(x, y) ((y * WORLD_X) + x)

#define XNORM(x) (((x)<0) ? ((WORLD_X-(-(x)%WORLD_X))%WORLD_X) : ((x)%WORLD_X))
#define YNORM(y) (((y)<0) ? ((WORLD_Y-(-(y)%WORLD_Y))%WORLD_Y) : ((y)%WORLD_Y))

struct range {
    coord lx;			/* low-range x,y */
    coord ly;
    coord hx;			/* high-range x,y */
    coord hy;
    int width;			/* range width, height */
    int height;
};

extern s_char *xyas(coord x, coord y, natid country);
extern s_char *ownxy(struct sctstr *sp);
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

#endif /* _XY_H_ */
