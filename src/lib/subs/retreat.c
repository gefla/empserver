/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  retreat.c: Retreat subroutines
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     Ron Koenderink, 2005-2006
 *     Markus Armbruster, 2006-2015
 */

#include <config.h>

#include "file.h"
#include "nsc.h"
#include "path.h"
#include "player.h"
#include "prototypes.h"
#include "retreat.h"
#include "unit.h"

static void retreat_ship_sel(struct shpstr *, struct emp_qelem *, int);
static int retreat_ships_step(struct emp_qelem *, char, natid);
static void retreat_land_sel(struct lndstr *, struct emp_qelem *, int);
static int retreat_lands_step(struct emp_qelem *, char, natid);

static int
retreat_steps(char *rpath)
{
    int i;

    for (i = 0; i < MAX_RETREAT && rpath[i]; i++) {
	if (rpath[i] == 'h')
	    return i + 1;
    }
    return i;
}

static void
consume_step(char *rpath, int *rflags)
{
    memmove(rpath, rpath + 1, RET_LEN - 1);
    if (!rpath[0])
	*rflags = 0;
}

void
retreat_ship(struct shpstr *sp, char code)
{
    int n, i;
    natid own;
    struct emp_qelem list;
    struct nstr_item ni;
    struct shpstr ship;

    if (sp->shp_own == player->cnum || !sp->shp_rpath[0])
	return;

    n = retreat_steps(sp->shp_rpath);
    if (!n)
	return;

    /*
     * We're going to put a copy of *sp into list.  The movement loop
     * will use that copy, which may render *sp stale.  To avoid
     * leaving the caller with a stale *sp, we'll re-get it at the
     * end.  To make that work, we need to put it now.  However, that
     * resets sp->shp_own when the ship sinks, so save it first.
     */
    own = sp->shp_own;
    putship(sp->shp_uid, sp);

    emp_initque(&list);
    if (sp->shp_own)
	retreat_ship_sel(sp, &list, n);

    if (sp->shp_rflags & RET_GROUP) {
	snxtitem_xy(&ni, EF_SHIP, sp->shp_x, sp->shp_y);
	while (nxtitem(&ni, &ship)) {
	    if (ship.shp_own != own
		|| !(ship.shp_rflags & RET_GROUP)
		|| ship.shp_fleet != sp->shp_fleet
		|| ship.shp_uid == sp->shp_uid)
		continue;
	    if (strncmp(ship.shp_rpath, sp->shp_rpath, MAX_RETREAT + 1))
		continue;
	    retreat_ship_sel(&ship, &list, n);
	}
    }

    /* Loop similar to the one in unit_move().  Keep it that way!  */
    for (i = 0; i < n && !QEMPTY(&list); i++) {
	/*
	 * Invariant: shp_may_nav() true for all ships
	 * Implies all are in the same sector
	 */
	if (!retreat_ships_step(&list, sp->shp_rpath[i], own))
	    n = i;
	shp_nav_stay_behind(&list, own);
	unit_rad_map_set(&list);
    }

    if (!QEMPTY(&list))
	shp_nav_put(&list, own);
    getship(sp->shp_uid, sp);
}

static void
retreat_ship_sel(struct shpstr *sp, struct emp_qelem *list, int n)
{
    struct shpstr *flg = QEMPTY(list) ? NULL
	: &((struct ulist *)(list->q_back))->unit.ship;

    if (!shp_may_nav(sp, flg, ", and can't retreat!"))
	return;
    if (sp->shp_mobil <= 0) {
	mpr(sp->shp_own, "%s has no mobility, and can't retreat!\n",
	    prship(sp));
	return;
    }

    if (flg)
	mpr(sp->shp_own, "%s retreats with her\n", prship(sp));
    else
	mpr(sp->shp_own, "%s retreats along path %.*s\n",
	    prship(sp), n, sp->shp_rpath);
    shp_insque(sp, list);
}

static int
retreat_ships_step(struct emp_qelem *list, char step, natid actor)
{
    int dir = chkdir(step, DIR_STOP, DIR_LAST);
    struct emp_qelem *qp;
    struct ulist *mlp;
    struct shpstr *sp;

    if (dir != DIR_STOP && shp_nav_dir(list, dir, actor))
	return 0;		/* can't go there */

    for (qp = list->q_back; qp != list; qp = qp->q_back) {
	mlp = (struct ulist *)qp;
	sp = &mlp->unit.ship;
	consume_step(sp->shp_rpath, &sp->shp_rflags);
	if (dir != DIR_STOP)
	    sp->shp_mission = 0;
	putship(sp->shp_uid, sp);
    }

    return dir != DIR_STOP && !shp_nav_gauntlet(list, 0, actor);
}

void
retreat_land(struct lndstr *lp, char code)
{
    int n, i;
    natid own;
    struct emp_qelem list;
    struct nstr_item ni;
    struct lndstr land;

    if (lp->lnd_own == player->cnum || !lp->lnd_rpath[0])
	return;

    n = retreat_steps(lp->lnd_rpath);
    if (!n)
	return;

    /* See explanation in retreat_ship() */
    own = lp->lnd_own;
    putland(lp->lnd_uid, lp);

    emp_initque(&list);
    if (lp->lnd_own)
	retreat_land_sel(lp, &list, n);

    if (lp->lnd_rflags & RET_GROUP) {
	snxtitem_xy(&ni, EF_LAND, lp->lnd_x, lp->lnd_y);
	while (nxtitem(&ni, &land)) {
	    if (land.lnd_own != own
		|| !(land.lnd_rflags & RET_GROUP)
		|| land.lnd_army != lp->lnd_army
		|| land.lnd_uid == lp->lnd_uid)
		continue;
	    if (strncmp(land.lnd_rpath, lp->lnd_rpath, MAX_RETREAT + 1))
		continue;
	    retreat_land_sel(&land, &list, n);
	}
    }

    /* Loop similar to the one in unit_move().  Keep it that way!  */
    for (i = 0; i < n && !QEMPTY(&list); i++) {
	/*
	 * Invariant: lnd_may_nav() true for all land units
	 * Implies all are in the same sector
	 */
	if (!retreat_lands_step(&list, lp->lnd_rpath[i], own))
	    n = i;
	lnd_mar_stay_behind(&list, own);
	unit_rad_map_set(&list);
    }

    if (!QEMPTY(&list))
	lnd_mar_put(&list, own);
    getland(lp->lnd_uid, lp);
}

static void
retreat_land_sel(struct lndstr *lp, struct emp_qelem *list, int n)
{
    struct lndstr *ldr = QEMPTY(list)
	? NULL : &((struct ulist *)(list->q_back))->unit.land;

    if (!lnd_may_mar(lp, ldr, ", and can't retreat!"))
	return;
    if (lp->lnd_mobil <= 0) {
	mpr(lp->lnd_own, "%s has no mobility, and can't retreat!\n",
	    prland(lp));
	return;
    }

    if (ldr)
	mpr(lp->lnd_own, "%s retreats with them\n", prland(lp));
    else
	mpr(lp->lnd_own, "%s retreats along path %.*s\n",
	    prland(lp), n, lp->lnd_rpath);
    lnd_insque(lp, list);
}

static int
retreat_lands_step(struct emp_qelem *list, char step, natid actor)
{
    int dir = chkdir(step, DIR_STOP, DIR_LAST);
    struct emp_qelem *qp;
    struct ulist *llp;
    struct lndstr *lp;

    if (dir != DIR_STOP && lnd_mar_dir(list, dir, actor))
	return 0;		/* can't go there */

    for (qp = list->q_back; qp != list; qp = qp->q_back) {
	llp = (struct ulist *)qp;
	lp = &llp->unit.land;
	consume_step(lp->lnd_rpath, &lp->lnd_rflags);
	if (dir != DIR_STOP) {
	    lp->lnd_mission = 0;
	    lp->lnd_harden = 0;
	}
	putland(lp->lnd_uid, lp);
    }

    return dir != DIR_STOP && !lnd_mar_gauntlet(list, 0, actor);
}
