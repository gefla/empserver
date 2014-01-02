/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  move_sat.c: Move a satellite to the next point in it's orbit.
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include <math.h>
#include "nsc.h"
#include "plane.h"
#include "update.h"

#ifndef PI
#define PI	3.14159265358979323846
#endif

void
move_sat(struct plnstr *pp)
{
    coord x1, y1, x2, y2;
    coord dx, dy;
    float newtheta;
    struct sctstr sect;

    newtheta = pp->pln_theta + .05;

    if (newtheta >= 1.0) {
	newtheta -= 1.0;
    }

    x1 = (coord)(2 * pp->pln_theta * WORLD_X);
    x1 = xnorm(x1);
    y1 = (coord)(sin(6 * PI * pp->pln_theta) * (WORLD_Y / 4));
    x2 = (coord)(2 * newtheta * WORLD_X);
    x2 = xnorm(x2);
    y2 = (coord)(sin(6 * PI * newtheta) * (WORLD_Y / 4));
    dx = x1 - pp->pln_x;
    dy = y1 - pp->pln_y;
    x2 -= dx;
    y2 -= dy;

    if ((x2 + y2) & 1) {
	x2++;
    }

    pp->pln_x = xnorm(x2);
    pp->pln_y = ynorm(y2);
    pp->pln_theta = newtheta;
    getsect(pp->pln_x, pp->pln_y, &sect);
    if (sect.sct_own)
	if (pp->pln_own != sect.sct_own)
	    wu(0, sect.sct_own, "%s satellite spotted over %s\n",
	       cname(pp->pln_own), xyas(pp->pln_x, pp->pln_y,
					sect.sct_own));
    return;
}
