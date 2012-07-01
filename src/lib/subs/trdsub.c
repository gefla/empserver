/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  trdsub.c: Trade helper functions
 *
 *  Known contributors to this file:
 *     St Phil, 1989
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#include <config.h>

#include "empobj.h"
#include "file.h"
#include "item.h"
#include "land.h"
#include "loan.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "nuke.h"
#include "plane.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "trade.h"
#include "unit.h"
#include "xy.h"

int
trade_check_ok(struct trdstr *tp, struct empobj *tgp)
{
    return check_trade_ok(tp) && check_obj_ok(tgp);
}

char *
trade_nameof(struct trdstr *tp, struct empobj *tgp)
{
    switch (tp->trd_type) {
    case EF_NUKE:
	return nchr[tgp->type].n_name;
    case EF_PLANE:
	return plchr[tgp->type].pl_name;
    case EF_SHIP:
	return mchr[tgp->type].m_name;
    case EF_LAND:
	return lchr[tgp->type].l_name;
    }
    return "Bad trade type, get help";
}

/*
 * Describe an item up for sale.  "tgp" is a union containing
 * the details of the generic item.
 * Return 1 on success, 0 on error
 */
int
trade_desc(struct empobj *tgp)
{
    i_type it;
    struct sctstr sect;
    struct nukstr *np;
    struct shpstr *sp;
    struct plnstr *pp;
    struct lndstr *lp;
    struct nstr_item ni;
    struct plnstr plane;
    struct lndstr land;
    struct nukstr nuke;

    switch (tgp->ef_type) {
    case EF_NUKE:
	np = (struct nukstr *)tgp;
	pr("(%3d)  tech %d %d%% %s #%d",
	   np->nuk_own, np->nuk_tech, np->nuk_effic,
	   nchr[(int)np->nuk_type].n_name, np->nuk_uid);
	break;
    case EF_SHIP:
	sp = (struct shpstr *)tgp;
	pr("(%3d)  tech %d %d%% %s [",
	   sp->shp_own, sp->shp_tech, sp->shp_effic, prship(sp));

	for (it = I_NONE + 1; it <= I_MAX; ++it) {
	    if (sp->shp_item[it])
		pr("%c:%d ", ichr[it].i_mnem, sp->shp_item[it]);
	}
	pr("] #%d", sp->shp_uid);
	snxtitem_cargo(&ni, EF_PLANE, EF_SHIP, sp->shp_uid);
	while (nxtitem(&ni, &plane)) {
	    pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
	       plane.pln_tech,
	       plane.pln_effic,
	       plchr[(int)plane.pln_type].pl_name, plane.pln_uid);
	    if (getnuke(nuk_on_plane(&plane), &nuke))
		pr("(%s)", nchr[nuke.nuk_type].n_name);
	}
	snxtitem_cargo(&ni, EF_LAND, EF_SHIP, sp->shp_uid);
	while (nxtitem(&ni, &land)) {
	    pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
	       land.lnd_tech,
	       land.lnd_effic,
	       lchr[(int)land.lnd_type].l_name, land.lnd_uid);
	    if (pln_first_on_land(&land) >= 0) {
		snxtitem_cargo(&ni, EF_PLANE, EF_LAND, land.lnd_uid);
		while (nxtitem(&ni, &plane)) {
		    pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
		       plane.pln_tech,
		       plane.pln_effic,
		       plchr[(int)plane.pln_type].pl_name,
		       plane.pln_uid);
		    if (getnuke(nuk_on_plane(&plane), &nuke))
			pr("(%s)", nchr[nuke.nuk_type].n_name);
		}
	    }
	}
	getsect(sp->shp_x, sp->shp_y, &sect);
	if (sect.sct_type != SCT_WATER)
	    pr(" in a %s %s",
	       cname(sect.sct_own), dchr[sect.sct_type].d_name);
	else
	    pr(" at sea");
	break;
    case EF_LAND:
	lp = (struct lndstr *)tgp;
	pr("(%3d)  tech %d %d%% %s [",
	   lp->lnd_own,
	   lp->lnd_tech, lp->lnd_effic, lchr[(int)lp->lnd_type].l_name);
	for (it = I_NONE + 1; it <= I_MAX; ++it) {
	    if (lp->lnd_item[it])
		pr("%c:%d ", ichr[it].i_mnem, lp->lnd_item[it]);
	}
	pr("] #%d", lp->lnd_uid);
	break;
    case EF_PLANE:
	pp = (struct plnstr *)tgp;
	pr("(%3d)  tech %d %d%% %s #%d",
	   pp->pln_own,
	   pp->pln_tech,
	   pp->pln_effic,
	   plchr[(int)pp->pln_type].pl_name, pp->pln_uid);
	if (getnuke(nuk_on_plane(pp), &nuke))
	    pr("(%s)", nchr[nuke.nuk_type].n_name);
	break;
    default:
	pr("flaky unit type %d", tgp->uid);
	break;
    }
    return 1;
}

int
trade_has_unsalable_cargo(struct empobj *tgp, int noisy)
{
    int ret, i, type;
    short *item;
    struct nstr_item ni;
    union empobj_storage cargo;

    ret = 0;
    if (tgp->ef_type == EF_SHIP || tgp->ef_type == EF_LAND) {
	item = tgp->ef_type == EF_SHIP
	    ? ((struct shpstr *)tgp)->shp_item
	    : ((struct lndstr *)tgp)->lnd_item;
	for (i = I_NONE + 1; i <= I_MAX; i++) {
	    if (item[i] && !ichr[i].i_sell) {
		if (noisy)
		    pr("%s carries %s, which you can't sell.\n",
		       unit_nameof(tgp), ichr[i].i_name);
		ret = 1;
	    }
	}
    }

    for (type = EF_PLANE; type <= EF_NUKE; type++) {
	snxtitem_cargo(&ni, type, tgp->ef_type, tgp->uid);
	while (nxtitem(&ni, &cargo))
	    ret |= trade_has_unsalable_cargo(&cargo.gen, noisy);
    }

    return ret;
}

int
trade_getitem(struct trdstr *tp, union empobj_storage *tgp)
{
    if (!ef_read(tp->trd_type, tp->trd_unitid, tgp))
	return 0;
    return 1;
}

/*
 * Return amount due for LOAN at time PAYTIME.
 */
double
loan_owed(struct lonstr *loan, time_t paytime)
{
    time_t rtime;		/* regular interest time */
    time_t xtime;		/* double interest time */
    double rate;
    int dur;

    /*
     * Split interval paytime - l_lastpay into regular (up to
     * l_duedate) and extended (beyond l_duedate) time.
     */
    rtime = loan->l_duedate - loan->l_lastpay;
    xtime = paytime - loan->l_duedate;
    if (rtime < 0) {
	xtime += rtime;
	rtime = 0;
    }
    if (xtime < 0) {
	rtime += xtime;
	xtime = 0;
    }
    if (CANT_HAPPEN(rtime < 0))
	rtime = 0;

    dur = loan->l_ldur;
    if (CANT_HAPPEN(dur <= 0))
	dur = 1;
    rate = loan->l_irate / 100.0 / (dur * SECS_PER_DAY);

    return loan->l_amtdue * (1.0 + (rtime + xtime * 2) * rate);
}
