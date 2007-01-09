/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "trade.h"
#include "xy.h"

int
trade_check_ok(struct trdstr *tp, union empobj_storage *tgp)
{
    return check_trade_ok(tp) && trade_check_item_ok(tgp);
}

int
trade_check_item_ok(union empobj_storage *tgp)
{
    if (tgp->gen.ef_type == EF_LAND)
	return check_land_ok(&tgp->land);
    if (tgp->gen.ef_type == EF_PLANE)
	return check_plane_ok(&tgp->plane);
    if (tgp->gen.ef_type == EF_SHIP)
	return check_ship_ok(&tgp->ship);
    if (tgp->gen.ef_type == EF_NUKE)
	return check_nuke_ok(&tgp->nuke);
    CANT_REACH();
    pr("Trade lot went bad!\n");
    return 0;
}

char *
trade_nameof(struct trdstr *tp, union empobj_storage *tgp)
{
    switch (tp->trd_type) {
    case EF_NUKE:
	return nchr[(int)tgp->nuke.nuk_type].n_name;
    case EF_PLANE:
	return plchr[(int)tgp->plane.pln_type].pl_name;
    case EF_SHIP:
	return mchr[(int)tgp->ship.shp_type].m_name;
    case EF_LAND:
	return lchr[(int)tgp->land.lnd_type].l_name;
    }
    return "Bad trade type, get help";
}

/*
 * Describe an item up for sale.  "tgp" is a union containing
 * the details of the generic item.
 * Return 1 on success, 0 on error
 */
int
trade_desc(struct trdstr *tp, union empobj_storage *tgp)
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

    switch (tp->trd_type) {
    case EF_NUKE:
	np = &tgp->nuke;
	tp->trd_owner = np->nuk_own;
	pr("(%3d)  tech %d %d%% %s #%d",
	   tp->trd_owner, np->nuk_tech, np->nuk_effic,
	   nchr[(int)np->nuk_type].n_name, tp->trd_unitid);
	break;
    case EF_SHIP:
	sp = &tgp->ship;
	tp->trd_owner = sp->shp_own;
	pr("(%3d)  tech %d %d%% %s [",
	   tp->trd_owner, sp->shp_tech, sp->shp_effic, prship(sp));

	for (it = I_NONE + 1; it <= I_MAX; ++it) {
	    if (sp->shp_item[it])
		pr("%c:%d ", ichr[it].i_mnem, sp->shp_item[it]);
	}
	pr("] #%d", tp->trd_unitid);
	if (opt_SHOWPLANE) {
	    snxtitem_all(&ni, EF_PLANE);
	    while (nxtitem(&ni, &plane)) {
		if (plane.pln_ship == sp->shp_uid && plane.pln_own != 0) {
		    pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
		       plane.pln_tech,
		       plane.pln_effic,
		       plchr[(int)plane.pln_type].pl_name, plane.pln_uid);
		    if (plane.pln_nuketype != -1) {
			pr("(%s)", nchr[(int)plane.pln_nuketype].n_name);
		    }
		}
	    }
	    snxtitem_all(&ni, EF_LAND);
	    while (nxtitem(&ni, &land)) {
		if (land.lnd_ship == sp->shp_uid && land.lnd_own != 0) {
		    pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
		       land.lnd_tech,
		       land.lnd_effic,
		       lchr[(int)land.lnd_type].l_name, land.lnd_uid);
		    if (land.lnd_nxlight) {
			snxtitem_all(&ni, EF_PLANE);
			while (nxtitem(&ni, &plane)) {
			    if (plane.pln_land == land.lnd_uid) {
				pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
				   plane.pln_tech,
				   plane.pln_effic,
				   plchr[(int)plane.pln_type].pl_name,
				   plane.pln_uid);
				if (plane.pln_nuketype != -1) {
				    pr("(%s)",
				       nchr[(int)plane.pln_nuketype].n_name);
				}
			    }
			}
		    }
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
	lp = &tgp->land;
	tp->trd_owner = lp->lnd_own;
	pr("(%3d)  tech %d %d%% %s [",
	   tp->trd_owner,
	   lp->lnd_tech, lp->lnd_effic, lchr[(int)lp->lnd_type].l_name);
	for (it = I_NONE + 1; it <= I_MAX; ++it) {
	    if (lp->lnd_item[it])
		pr("%c:%d ", ichr[it].i_mnem, lp->lnd_item[it]);
	}
	pr("] #%d", tp->trd_unitid);
	if (opt_SHOWPLANE) {
	    snxtitem_all(&ni, EF_PLANE);
	    while (nxtitem(&ni, &plane)) {
		if (plane.pln_land == lp->lnd_uid && plane.pln_own != 0) {
		    pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
		       plane.pln_tech,
		       plane.pln_effic,
		       plchr[(int)plane.pln_type].pl_name, plane.pln_uid);
		    if (plane.pln_nuketype != -1) {
			pr("(%s)", nchr[(int)plane.pln_nuketype].n_name);
		    }
		}
	    }
	}
	getsect(lp->lnd_x, lp->lnd_y, &sect);
	break;
    case EF_PLANE:
	pp = &tgp->plane;
	tp->trd_owner = pp->pln_own;
	pr("(%3d)  tech %d %d%% %s #%d",
	   tp->trd_owner,
	   pp->pln_tech,
	   pp->pln_effic,
	   plchr[(int)pp->pln_type].pl_name, tp->trd_unitid);
	if (pp->pln_nuketype != -1) {
	    pr("(%s)", nchr[(int)pp->pln_nuketype].n_name);
	}
	break;
    default:
	pr("flaky unit type %d", tp->trd_type);
	break;
    }
    return 1;
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
