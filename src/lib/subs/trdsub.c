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
 *  trdsub.c: Trade helper functions
 * 
 *  Known contributors to this file:
 *     St Phil, 1989
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "item.h"
#include "ship.h"
#include "land.h"
#include "nuke.h"
#include "plane.h"
#include "trade.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "loan.h"
#include "prototypes.h"
#include "optlist.h"

int
trade_check_ok(int lot, struct trdstr *tp, union trdgenstr *tgp)
{
    union trdgenstr check;
    struct trdstr trade;
    int result = 0;

    trade_getitem(tp, &check);
    if (tp->trd_type == EF_LAND)
	result = memcmp(&(check.lnd), &(tgp->lnd), sizeof(struct lndstr));
    else if (tp->trd_type == EF_PLANE)
	result = memcmp(&(check.pln), &(tgp->pln), sizeof(struct plnstr));
    else if (tp->trd_type == EF_SHIP)
	result = memcmp(&(check.shp), &(tgp->shp), sizeof(struct shpstr));
    else
	result = memcmp(&(check.nuk), &(tgp->nuk), sizeof(struct nukstr));
    if (result) {
	pr("That item has changed!\n");
	return 0;
    }
    gettrade(lot, &trade);
    if (memcmp((s_char *)&trade, (s_char *)tp, sizeof(struct trdstr))) {
	pr("That item has changed!\n");
	return 0;
    }
    return 1;
}

s_char *
trade_nameof(struct trdstr *tp, union trdgenstr *tgp)
{
    switch (tp->trd_type) {
    case EF_NUKE:
	return "nuclear stockpile";
    case EF_PLANE:
	return plchr[(int)tgp->pln.pln_type].pl_name;
    case EF_SHIP:
	return mchr[(int)tgp->shp.shp_type].m_name;
    case EF_LAND:
	return lchr[(int)tgp->lnd.lnd_type].l_name;
    }
    return "Bad trade type, get help";
}

/*
 * Describe an item up for sale.  "tgp" is a union containing
 * the details of the generic item.
 * Return 1 on success, 0 on error
 */
int
trade_desc(struct trdstr *tp, union trdgenstr *tgp)
{
    int i;
    struct sctstr sect;
    struct nukstr *np;
    struct shpstr *sp;
    struct plnstr *pp;
    struct lndstr *lp;
    struct natstr *natp;
    int needcomma;
    struct nstr_item ni;
    struct plnstr plane;
    struct lndstr land;

    switch (tp->trd_type) {
    case EF_NUKE:
	np = &tgp->nuk;
	if (!getsect(np->nuk_x, np->nuk_y, &sect))
	    return 0;
	tp->trd_owner = sect.sct_own;
	natp = getnatp(tp->trd_owner);
	pr("(%3d)  ", sect.sct_own);
	needcomma = 0;
	for (i = 0; i < N_MAXNUKE; i++) {
	    if (np->nuk_types[i]) {
		if (needcomma)
		    pr(",");
		pr("%dx%s", np->nuk_types[i], nchr[i].n_name);
		needcomma = 1;
	    }
	}
	break;
    case EF_SHIP:
	sp = &tgp->shp;
	tp->trd_owner = sp->shp_own;
	pr("(%3d)  tech %d %d%% %s [",
	   tp->trd_owner, sp->shp_tech, sp->shp_effic, prship(sp));

	for (i = 1; i <= I_MAX; ++i) {
	    if (sp->shp_item[i])
		pr("%c:%d ", ichr[i].i_mnem, sp->shp_item[i]);
	}
	pr("] #%d", tp->trd_unitid);
	if (opt_SHOWPLANE) {
	    snxtitem_all(&ni, EF_PLANE);
	    while (nxtitem(&ni, (s_char *)&plane)) {
		if (plane.pln_ship == sp->shp_uid && plane.pln_own != 0) {
		    pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
		       plane.pln_tech,
		       plane.pln_effic,
		       plchr[(int)plane.pln_type].pl_name, plane.pln_uid);
		    if (plane.pln_nuketype != (s_char)-1) {
			pr("(%s)", nchr[(int)plane.pln_nuketype].n_name);
		    }
		}
	    }
	    snxtitem_all(&ni, EF_LAND);
	    while (nxtitem(&ni, (s_char *)&land)) {
		if (land.lnd_ship == sp->shp_uid && land.lnd_own != 0) {
		    pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
		       land.lnd_tech,
		       land.lnd_effic,
		       lchr[(int)land.lnd_type].l_name, land.lnd_uid);
		    if (land.lnd_nxlight) {
			snxtitem_all(&ni, EF_PLANE);
			while (nxtitem(&ni, (s_char *)&plane)) {
			    if (plane.pln_land == land.lnd_uid) {
				pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
				   plane.pln_tech,
				   plane.pln_effic,
				   plchr[(int)plane.pln_type].pl_name,
				   plane.pln_uid);
				if (plane.pln_nuketype != (s_char)-1) {
				    pr("(%s)",
				       nchr[(int)plane.pln_nuketype].
				       n_name);
				}
			    }
			}
		    }
		}
	    }
	}
	getsect(sp->shp_x, sp->shp_y, &sect);
	if (sect.sct_type != SCT_WATER)
	    pr(" in a %s %s", cname(sect.sct_own),
	       dchr[sect.sct_type].d_name);
	else
	    pr(" at sea");
	break;
    case EF_LAND:
	lp = &tgp->lnd;
	tp->trd_owner = lp->lnd_own;
	pr("(%3d)  tech %d %d%% %s [",
	   tp->trd_owner,
	   lp->lnd_tech, lp->lnd_effic, lchr[(int)lp->lnd_type].l_name);
	for (i = 1; i <= I_MAX; ++i) {
	    if (lp->lnd_item[i])
		pr("%c:%d ", ichr[i].i_mnem, lp->lnd_item[i]);
	}
	pr("] #%d", tp->trd_unitid);
	if (opt_SHOWPLANE) {
	    snxtitem_all(&ni, EF_PLANE);
	    while (nxtitem(&ni, (s_char *)&plane)) {
		if (plane.pln_land == lp->lnd_uid && plane.pln_own != 0) {
		    pr("\n\t\t\t\t    tech %3d %3d%% %s #%d",
		       plane.pln_tech,
		       plane.pln_effic,
		       plchr[(int)plane.pln_type].pl_name, plane.pln_uid);
		    if (plane.pln_nuketype != (s_char)-1) {
			pr("(%s)", nchr[(int)plane.pln_nuketype].n_name);
		    }
		}
	    }
	}
	getsect(lp->lnd_x, lp->lnd_y, &sect);
	break;
    case EF_PLANE:
	pp = &tgp->pln;
	tp->trd_owner = pp->pln_own;
	pr("(%3d)  tech %d %d%% %s #%d",
	   tp->trd_owner,
	   pp->pln_tech,
	   pp->pln_effic,
	   plchr[(int)pp->pln_type].pl_name, tp->trd_unitid);
	if (pp->pln_nuketype != (s_char)-1) {
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
trade_getitem(struct trdstr *tp, union trdgenstr *tgp)
{
    if (!ef_read(tp->trd_type, tp->trd_unitid, (s_char *)tgp))
	return 0;
    return 1;
}

long
get_couval(int cnum)
{
    struct sctstr *sp;
    register int j, k, val;
    long secttot = 0;

    for (j = 0; NULL != (sp = getsectid(j)); j++) {
	if (sp->sct_own != cnum)
	    continue;
	secttot += (long)(dchr[sp->sct_type].d_value *
			  ((float)sp->sct_effic + 100.0));
	for (k = 0; ichr[k].i_name; k++) {
	    if (ichr[k].i_value == 0 || ichr[k].i_vtype == 0)
		continue;
	    val = sp->sct_item[ichr[k].i_vtype];
	    secttot += val * ichr[k].i_value;
	}
    }
    return secttot;
}

long
get_outstand(int cnum)
{
    struct lonstr loan;
    int j;
    long loantot = 0;

    for (j = 0; getloan(j, &loan); j++) {
	if (loan.l_status == LS_FREE)
	    continue;
	if (loan.l_lonee == cnum)
	    loantot += loan.l_amtdue;
    }
    return loantot;
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
