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
 *  rang.c: Edit range of plane/land unit
 * 
 *  Known contributors to this file:
 *     Jeff Bailey
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "plane.h"
#include "land.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "deity.h"
#include "commands.h"

int
range(void)
{
    struct nstr_item np;
    struct plnstr plane;
    int i;
    s_char *p;
    s_char buf[1024];

    if (!snxtitem(&np, EF_PLANE, player->argp[1]))
	return RET_SYN;
    while (nxtitem(&np, (s_char *)&plane)) {
	if (!player->owner || plane.pln_own == 0)
	    continue;
	p = getstarg(player->argp[2], "New range? ", buf);
	if (!check_plane_ok(&plane))
	    return RET_SYN;
	if (!p || (i = atoi(p)) < 0)
	    continue;
	plane.pln_range = (plane.pln_range_max < i) ?
	    plane.pln_range_max : i;
	pr("Plane %d range changed to %d\n", plane.pln_uid,
	   plane.pln_range);

	putplane(plane.pln_uid, &plane);
    }

    return RET_OK;
}

int
lrange(void)
{
    struct nstr_item np;
    struct lndstr land;
    struct lchrstr *lcp;
    int i;
    s_char *p;
    s_char prompt[128];
    s_char buf[128];

    if (!snxtitem(&np, EF_LAND, player->argp[1]))
	return RET_SYN;
    while (nxtitem(&np, (s_char *)&land)) {
	if (!player->owner || land.lnd_own == 0)
	    continue;
	lcp = &lchr[(int)land.lnd_type];
	sprintf(prompt, "New range for %s? ", prland(&land));
	if ((p = getstarg(player->argp[2], prompt, buf)) == 0)
	    return RET_SYN;
	if (!check_land_ok(&land))
	    return RET_SYN;
	if ((i = atoi(p)) < 0)
	    continue;
	land.lnd_rad_max = (i < land.lnd_rad) ? i : land.lnd_rad;
	pr("%s reaction radius changed to %d\n", prland(&land),
	   land.lnd_rad_max);
	putland(land.lnd_uid, &land);
    }

    return RET_OK;
}
