/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  unitsub.c: Common subroutines for multiple type of units
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 */

#include <config.h>

#include "empobj.h"
#include "file.h"
#include "player.h"
#include "prototypes.h"
#include "unit.h"

void
unit_list(struct emp_qelem *unit_list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *ulp;
    struct empobj *unit;
    struct lndstr *lnd;
    struct shpstr *shp;

    CANT_HAPPEN(QEMPTY(unit_list));

    qp = unit_list->q_back;
    ulp = (struct ulist *)qp;

    if (ulp->unit.ef_type == EF_LAND)
	pr("lnd#     land type       x,y    a  eff  sh gun xl  mu tech retr fuel\n");
    else
        pr("shp#     ship type       x,y   fl  eff mil  sh gun pn he xl ln mob tech\n");

    for (; qp != unit_list; qp = next) {
	next = qp->q_back;
	ulp = (struct ulist *)qp;
	lnd = &ulp->unit.land;
	shp = &ulp->unit.ship;
	unit = &ulp->unit.gen;
	pr("%4d ", unit->uid);
	pr("%-16.16s ", emp_obj_chr_name(unit));
	prxy("%4d,%-4d ", unit->x, unit->y, unit->own);
	pr("%1.1s", &unit->group);
	pr("%4d%%", unit->effic);
	if (unit->ef_type == EF_LAND) {
	    pr("%4d", lnd->lnd_item[I_SHELL]);
	    pr("%4d", lnd->lnd_item[I_GUN]);
	    count_land_planes(lnd);
	    pr("%3d", lnd->lnd_nxlight);
	} else {
	    pr("%4d", shp->shp_item[I_MILIT]);
	    pr("%4d", shp->shp_item[I_SHELL]);
	    pr("%4d", shp->shp_item[I_GUN]);
	    count_planes(shp);
	    pr("%3d", shp->shp_nplane);
	    pr("%3d", shp->shp_nchoppers);
	    pr("%3d", shp->shp_nxlight);
	    count_units(shp);
	    pr("%3d", shp->shp_nland);
	}
	pr("%4d", unit->mobil);
	pr("%4d", unit->tech);
	if (unit->ef_type == EF_LAND) {
	    pr("%4d%%", lnd->lnd_retreat);
	    pr("%5d", lnd->lnd_fuel);
	}
	pr("\n");
    }
}

void
unit_put(struct emp_qelem *list, natid actor)
{
    struct emp_qelem *qp;
    struct emp_qelem *newqp;
    struct ulist *ulp;
    struct empobj *unit;

    qp = list->q_back;
    while (qp != list) {
	ulp = (struct ulist *)qp;
	unit = &ulp->unit.gen;
	if (actor) {
	    mpr(actor, "%s stopped at %s\n", obj_nameof(unit),
		xyas(unit->x, unit->y, unit->own));
	    if (unit->ef_type == EF_LAND) {
		if (ulp->mobil < -127)
		    ulp->mobil = -127;
		unit->mobil = ulp->mobil;
	    }
	}
	if (unit->ef_type == EF_SHIP)
	    unit->mobil = (int)ulp->mobil;
	put_empobj(unit->ef_type, unit->uid, unit);
	newqp = qp->q_back;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }
}

char *
unit_path(int together, struct empobj *unit, char *buf)
{
    coord destx;
    coord desty;
    struct sctstr d_sect, sect;
    char *cp;
    double dummy;
    int mtype;

    if (!sarg_xy(buf, &destx, &desty))
	return 0;
    if (!together) {
	pr("Cannot go to a destination sector if not all starting in the same sector\n");
	return 0;
    }
    if (!getsect(destx, desty, &d_sect)) {
	pr("%d,%d is not a sector\n", destx, desty);
	return 0;
    }
    if (unit->ef_type == EF_SHIP) {
	cp = BestShipPath(buf, unit->x, unit->y,
			  d_sect.sct_x, d_sect.sct_y, player->cnum);
	if (!cp || unit->mobil <= 0) {
	    pr("Can't get to '%s' right now.\n",
		xyas(d_sect.sct_x, d_sect.sct_y, player->cnum));
	    return 0;
	}
    } else {
	getsect(unit->x, unit->y, &sect);
	mtype = lnd_mobtype((struct lndstr *)unit);
	cp = BestLandPath(buf, &sect, &d_sect, &dummy, mtype);
	if (!cp) {
	    pr("No owned %s from %s to %s!\n",
	       mtype == MOB_RAIL ? "railway" : "path",
	       xyas(unit->x, unit->y, player->cnum),
	       xyas(d_sect.sct_x, d_sect.sct_y, player->cnum));
	    return 0;
	}
	pr("Using path '%s'\n", cp);
    }
    return cp;
}

void
unit_view(struct emp_qelem *list)
{
    struct sctstr sect;
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *ulp;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	ulp = (struct ulist *)qp;
	getsect(ulp->unit.gen.x, ulp->unit.gen.y, &sect);
	if (ulp->unit.ef_type == EF_SHIP) {
	    if (((struct mchrstr *)ulp->chrp)->m_flags & M_FOOD)
		pr("[fert:%d] ", sect.sct_fertil);
	    if (((struct mchrstr *)ulp->chrp)->m_flags & M_OIL)
		pr("[oil:%d] ", sect.sct_oil);
	}
	pr("%s @ %s %d%% %s\n", obj_nameof(&ulp->unit.gen),
	    xyas(ulp->unit.gen.x, ulp->unit.gen.y, player->cnum),
	    sect.sct_effic, dchr[sect.sct_type].d_name);
    }
}
