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
 *  move_sat.c: Move a satellite to the next point in it's orbit.
 * 
 *  Known contributors to this file:
 *     
 */

#include <math.h>
#include "misc.h"
#include "var.h"
#include "plane.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "path.h"
#include "deity.h"
#include "file.h"
#include "update.h"
#include "subs.h"
#include "optlist.h"

#ifndef PI
#define	PI	3.14159265358979323846
#endif

void
move_sat(register struct plnstr *pp)
{
    coord	x1,y1,x2,y2;
    coord	dx,dy;
    float	newtheta;
    struct sctstr	sect;

    newtheta = pp->pln_theta + .05;

    if (newtheta >= 1.0)
    {
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

    if	((x2 + y2) & 1)
    {
	x2++;
    }

    pp->pln_x = xnorm(x2);
    pp->pln_y = ynorm(y2);
    pp->pln_theta = newtheta;
    getsect(pp->pln_x, pp->pln_y, &sect);
    if (sect.sct_own)
	    if (pp->pln_own != sect.sct_own)
		    wu(0, sect.sct_own, "%s satellite spotted over %s\n",
		       cname(pp->pln_own), xyas(pp->pln_x, pp->pln_y, sect.sct_own));
    return;
}
