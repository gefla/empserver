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
 *     Markus Armbruster, 2008
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
    int type, npln, nch, nxl;
    struct empobj *unit;
    struct lndstr *lnd;
    struct shpstr *shp;

    if (CANT_HAPPEN(QEMPTY(unit_list)))
	return;
    qp = unit_list->q_back;
    ulp = (struct ulist *)qp;
    type = ulp->unit.ef_type;
    if (CANT_HAPPEN(type != EF_LAND && type != EF_SHIP))
	return;

    if (type == EF_LAND)
	pr("lnd#     land type       x,y    a  eff  sh gun xl  mu tech retr\n");
    else
        pr("shp#     ship type       x,y   fl  eff mil  sh gun pn he xl ln mob tech\n");

    for (; qp != unit_list; qp = next) {
	next = qp->q_back;
	ulp = (struct ulist *)qp;
	lnd = &ulp->unit.land;
	shp = &ulp->unit.ship;
	unit = &ulp->unit.gen;
	if (CANT_HAPPEN(type != unit->ef_type))
	    continue;
	pr("%4d ", unit->uid);
	pr("%-16.16s ", empobj_chr_name(unit));
	prxy("%4d,%-4d ", unit->x, unit->y, unit->own);
	pr("%1.1s", &unit->group);
	pr("%4d%%", unit->effic);
	if (type == EF_LAND) {
	    pr("%4d", lnd->lnd_item[I_SHELL]);
	    pr("%4d", lnd->lnd_item[I_GUN]);
	    pr("%3d", lnd_nxlight(lnd));
	} else {
	    pr("%4d", shp->shp_item[I_MILIT]);
	    pr("%4d", shp->shp_item[I_SHELL]);
	    pr("%4d", shp->shp_item[I_GUN]);
	    npln = shp_nplane(shp, &nch, &nxl, NULL);
	    pr("%3d%3d%3d", npln - nch - nxl, nch, nxl);
	    pr("%3d", shp_nland(shp));
	}
	pr("%4d", unit->mobil);
	pr("%4d", unit->tech);
	if (type == EF_LAND) {
	    pr("%4d%%", lnd->lnd_retreat);
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
	if (CANT_HAPPEN(unit->ef_type != EF_LAND
			&& unit->ef_type != EF_SHIP))
	    continue;
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

    if (CANT_HAPPEN(unit->ef_type != EF_LAND && unit->ef_type != EF_SHIP))
	return NULL;

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
	if (CANT_HAPPEN(!(ef_flags(ulp->unit.ef_type) & EFF_XY)))
	    continue;
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

/*
 * Update cargo of CARRIER for movement or destruction.
 * If the carrier is destroyed, destroy its cargo (planes, land units,
 * nukes).
 * Else update their location to the carrier's.  Any op sectors equal
 * to location get updated, too.
 */
void
unit_update_cargo(struct empobj *carrier)
{
    int cargo_type;
    struct nstr_item ni;
    union empobj_storage obj;

    for (cargo_type = EF_PLANE; cargo_type <= EF_NUKE; cargo_type++) {
	snxtitem_cargo(&ni, cargo_type, carrier->ef_type, carrier->uid);
	while (nxtitem(&ni, &obj)) {
	    if (!carrier->own) {
		mpr(obj.gen.own, "%s lost!\n", obj_nameof(&obj.gen));
		obj.gen.effic = 0;
	    } else {
		/* mission op-area centered on the obj travels with it */
		if (obj.gen.opx == obj.gen.x && obj.gen.opy == obj.gen.y) {
		    obj.gen.opx = carrier->x;
		    obj.gen.opy = carrier->y;
		}
		obj.gen.x = carrier->x;
		obj.gen.y = carrier->y;
	    }
	    put_empobj(cargo_type, obj.gen.uid, &obj);
	}
    }
}
