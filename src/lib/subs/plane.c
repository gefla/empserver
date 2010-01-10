/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plane.c: Plane post-read and pre-write data massage
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 *     Markus Armbruster, 2006-2008
 */

#include <config.h>

#include "file.h"
#include "lost.h"
#include "misc.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "unit.h"

void
pln_postread(int n, void *ptr)
{
    struct plnstr *pp = ptr;

    if (pp->pln_uid != n) {
	logerror("pln_postread: Error - %d != %d, zeroing.\n",
		 pp->pln_uid, n);
	memset(pp, 0, sizeof(struct plnstr));
    }
    player->owner = (player->god || pp->pln_own == player->cnum);
    if (opt_MOB_ACCESS)
	pln_do_upd_mob(pp);
}

void
pln_prewrite(int n, void *old, void *new)
{
    struct plnstr *oldpp = old;
    struct plnstr *pp = new;
    natid own = pp->pln_effic < PLANE_MINEFF ? 0 : pp->pln_own;

    if (!own) {
	pp->pln_effic = 0;
	pp->pln_ship = pp->pln_land = -1;
    }

    if (CANT_HAPPEN(pp->pln_ship >= 0 && pp->pln_land >= 0))
	pp->pln_land = -1;
    if (oldpp->pln_ship != pp->pln_ship)
	pln_carrier_change(pp, EF_SHIP, oldpp->pln_ship, pp->pln_ship);
    if (oldpp->pln_land != pp->pln_land)
	pln_carrier_change(pp, EF_LAND, oldpp->pln_land, pp->pln_land);

    /* We've avoided assigning to pp->pln_own, in case oldpp == pp */
    if (oldpp->pln_own != own)
	lost_and_found(EF_PLANE, oldpp->pln_own, own,
		       pp->pln_uid, pp->pln_x, pp->pln_y);

    pp->pln_own = own;
    if (!own || pp->pln_x != oldpp->pln_x || pp->pln_y != oldpp->pln_y)
	unit_update_cargo((struct empobj *)pp);
}

void
pln_oninit(void *ptr)
{
    struct plnstr *pp = ptr;

    pp->pln_ship = pp->pln_land = -1;
}

char *
prplane(struct plnstr *pp)
{
    return prbuf("%s #%d", plchr[(int)pp->pln_type].pl_name, pp->pln_uid);
}
