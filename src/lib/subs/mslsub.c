/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mslsub.c: Missile subroutine stuff
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1996-2000
 *     Markus Armbruster, 2004-2012
 */

#include <config.h>

#include <stdlib.h>
#include "chance.h"
#include "misc.h"
#include "nat.h"
#include "news.h"
#include "nsc.h"
#include "nuke.h"
#include "optlist.h"
#include "plague.h"
#include "plane.h"
#include "prototypes.h"
#include "queue.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

int
msl_launch(struct plnstr *pp, int type, char *what, coord x, coord y,
	   natid victim, int *sublaunchp)
{
    struct shpstr ship;
    struct nukstr nuke;
    int sublaunch = 0;
    char *base, *in_or_at, *from;

    mpr(pp->pln_own, "Preparing to launch %s at %s %s %s%s\n",
	prplane(pp),
	cname(victim),
	what,
	type != EF_SECTOR ? "in " : "",
	xyas(x, y, pp->pln_own));
    if (pp->pln_ship >= 0) {
	getship(pp->pln_ship, &ship);
	base = prship(&ship);
	in_or_at = " in ";
	if (mchr[(int)ship.shp_type].m_flags & M_SUB) {
	    sublaunch = 1;
	    from = "in hatch";
	} else
	    from = "on deck";
    } else {
	if (pp->pln_harden > 0) {
	    base = "missile silo";
	    in_or_at = " at ";
	    from = "in silo";
	} else {
	    base = in_or_at = "";
	    from = "on launch pad";
	}
    }
    mpr(pp->pln_own, "\tLaunching from %s%s%s\n",
	base, in_or_at, xyas(pp->pln_x, pp->pln_y, pp->pln_own));

    CANT_HAPPEN(pp->pln_flags & PLN_LAUNCHED);
    pp->pln_flags |= PLN_LAUNCHED;
    putplane(pp->pln_uid, pp);

    if (chance((0.05 + (100 - pp->pln_effic) / 100.0)
	       * (1 - techfact(pp->pln_tech, 1.0)))) {
	mpr(pp->pln_own, "KABOOOOM!  Missile explodes %s!\n", from);
	if (getnuke(nuk_on_plane(pp), &nuke)) {
	    mpr(pp->pln_own, "%s lost!\n", prnuke(&nuke));
	    nuke.nuk_effic = 0;
	    putnuke(nuke.nuk_uid, &nuke);
	}
#if 0
	/*
	 * Disabled for now, because it breaks callers that call
	 * msl_launch() for each member of a list of planes, created
	 * by msl_sel() or perform_mission().  Damage to the base can
	 * damage other planes.  Any copies of them in the list become
	 * stale.  When msl_launch() modifies and writes back such a
	 * stale copy, the damage gets wiped out, triggering a seqno
	 * oops.
	 */
	if (chance(0.33)) {
	    struct sctstr sect;
	    int dam;

	    dam = pln_damage(pp, 'p', NULL) / 2;
	    if (pp->pln_ship >= 0) {
		shipdamage(&ship, dam);
		putship(ship.shp_uid, &ship);
	    } else {
		mpr(pp->pln_own, "Explosion damages %s %d%%\n",
		    xyas(pp->pln_x, pp->pln_y, pp->pln_own), dam);
		getsect(pp->pln_x, pp->pln_y, &sect);
		sectdamage(&sect, dam);
		putsect(&sect);
	    }
	}
#endif
	return -1;
    }

    mpr(pp->pln_own, "\tSHWOOOOOSH!  Missile launched!\n");

    if (type != EF_PLANE)
	mpr(victim, "Incoming %s missile sighted at %s...\n",
	    sublaunch ? "sub-launched" : cname(pp->pln_own),
	    xyas(x, y, victim));

    if (type == EF_SECTOR || type == EF_LAND) {
	if (msl_abm_intercept(pp, x, y, sublaunch))
	    return -1;
    }
    if (type == EF_SHIP) {
	if (shp_missile_defense(x, y, pp->pln_own, pln_def(pp))) {
	    return -1;
	}
    }

    if (sublaunchp)
	*sublaunchp = sublaunch;
    return 0;
}

int
msl_hit(struct plnstr *pp, int hardtarget, int type,
	int news_item, int snews_item, int sublaunch, natid victim)
{
    int hitchance, hit;

    if (nuk_on_plane(pp) >= 0) {
	mpr(pp->pln_own, "\tArming nuclear warheads...\n");
	hit = 1;
    } else {
	hitchance = pln_hitchance(pp, hardtarget, type);
	hit = pct_chance(hitchance);
	mpr(pp->pln_own, "\t%d%% hit chance...%s\n", hitchance,
	    hit ? "HIT!" : "miss");
    }

    if (type != EF_PLANE)
	mpr(victim, "...Incoming %s missile %s\n",
	    sublaunch ? "" : cname(pp->pln_own),
	    hit ? "HIT!\n" : "missed\n");
    if (hit && news_item) {
	if (sublaunch)
	    nreport(victim, snews_item, 0, 1);
	else
	    nreport(pp->pln_own, news_item, victim, 1);
    }
    return hit;
}

void
msl_sel(struct emp_qelem *list, coord x, coord y, natid victim,
	int wantflags, int nowantflags, int mission)
{
    struct plchrstr *pcp;
    struct plnstr plane;
    struct plist *irv;
    struct nstr_item ni;

    emp_initque(list);
    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &plane)) {
	if (!plane.pln_own)
	    continue;

	pcp = &plchr[(int)plane.pln_type];
	if (!(pcp->pl_flags & P_M))
	    continue;
	if (wantflags && (pcp->pl_flags & wantflags) != wantflags)
	    continue;
	if (nowantflags && pcp->pl_flags & nowantflags)
	    continue;
	if (mission && plane.pln_mission != mission)
	    continue;
	if (mission &&
	    plane.pln_radius < mapdist(x, y, plane.pln_opx, plane.pln_opy))
	    continue;
	if (relations_with(plane.pln_own, victim) >= NEUTRAL)
	    continue;
	/* missiles go one way, so we can use all the range */
	if (plane.pln_range < mapdist(x, y, plane.pln_x, plane.pln_y))
	    continue;
	if (plane.pln_mobil <= 0)
	    continue;
	if (plane.pln_effic < 100)
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, &plane))
		continue;
	}
	if (!pln_airbase_ok(&plane, 1, 0))
	    continue;
	/* got a valid interceptor */
	irv = malloc(sizeof(*irv));
	irv->load = 0;
	irv->pstage = PLG_HEALTHY;
	irv->pcp = &plchr[(int)plane.pln_type];
	irv->plane = plane;
	emp_insque(&irv->queue, list);
    }
}

static int
msl_intercept(struct plnstr *msl, struct sctstr *sp, int sublaunch,
	      struct emp_qelem *irvlist, char *att_name, char *def_name,
	      int news_item)
{
    struct plnstr *pp;
    struct emp_qelem *intlist;
    struct emp_qelem intfoo;
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct plist *ip;
    int icount = 0;
    short destroyed;
    char *who = sublaunch ? "" : cname(msl->pln_own);

    intlist = &intfoo;
    emp_initque(intlist);
    /* First choose interceptors belonging to the target sector */
    /* only allow two defense missiles per missile attack */
    for (qp = irvlist->q_forw; qp != irvlist && icount < 2; qp = next) {
	next = qp->q_forw;
	ip = (struct plist *)qp;
	pp = &ip->plane;
	if (pp->pln_own != sp->sct_own)
	    continue;
	if (mission_pln_equip(ip, NULL, 'i') < 0) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}
	/* got one interceptor, delete from irv_list and
	 * add to  int_list.
	 */
	emp_remque(qp);
	emp_insque(qp, intlist);
	putplane(pp->pln_uid, pp);
	icount++;
    }
    /* only allow two defense missiles per missile attack */
    for (qp = irvlist->q_forw; qp != irvlist && icount < 2; qp = next) {
	next = qp->q_forw;
	ip = (struct plist *)qp;
	pp = &ip->plane;
	if (mission_pln_equip(ip, NULL, 'i') < 0) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}
	/* got one interceptor, delete from irv_list and
	 * add to  int_list.
	 */
	emp_remque(qp);
	emp_insque(qp, intlist);
	putplane(pp->pln_uid, pp);
	icount++;
    }
    /* Now, clean out the queue */
    while (!QEMPTY(irvlist)) {
	qp = irvlist->q_forw;
	emp_remque(qp);
	free(qp);
    }
    if (icount == 0) {
	mpr(sp->sct_own, "No %ss launched to intercept.\n", def_name);
	return 0;
    }

    /* attempt to destroy incoming missile */

    destroyed = 0;
    while (!destroyed && !QEMPTY(intlist)) {
	qp = intlist->q_forw;
	ip = (struct plist *)qp;
	pp = &ip->plane;

	mpr(msl->pln_own, "%s %s launched in defense!\n",
	    cname(pp->pln_own), def_name);
	if (sp->sct_own == pp->pln_own) {
	    mpr(sp->sct_own, "%s launched to intercept %s %s!\n",
		def_name, who, att_name);
	} else {
	    mpr(sp->sct_own,
		"%s launched an %s to intercept the %s %s!\n",
		cname(pp->pln_own), def_name, who, att_name);
	    mpr(pp->pln_own,
		"%s launched to intercept %s %s arcing towards %s territory!\n",
		def_name, who, att_name, cname(sp->sct_own));
	}

	if (msl_launch(pp, EF_PLANE, att_name, sp->sct_x, sp->sct_y,
		       msl->pln_own, NULL) >= 0
	    && msl_hit(pp, pln_def(msl), EF_PLANE, 0, 0, 0, msl->pln_own)) {
	    mpr(msl->pln_own, "%s destroyed by %s %s!\n",
		att_name, cname(pp->pln_own), def_name);
	    mpr(sp->sct_own, "%s %s intercepted!\n", who, att_name);
	    if (sp->sct_own != pp->pln_own)
		mpr(pp->pln_own, "%s %s intercepted!\n", who, att_name);
	    if (sublaunch)
		nreport(pp->pln_own, news_item, 0, 1);
	    else
		nreport(pp->pln_own, news_item, msl->pln_own, 1);
	    destroyed = 1;
	}
	/* zap the missile */
	pp->pln_effic = 0;
	putplane(pp->pln_uid, pp);
	emp_remque(qp);
	free(qp);
    }
    /* Clean out what is left in the list */
    while (!QEMPTY(intlist)) {
	qp = intlist->q_forw;
	emp_remque(qp);
	free(qp);
    }
    if (destroyed)
	return 1;
    if (icount) {
	mpr(msl->pln_own, "%s made it through %s defenses!\n",
	    att_name, def_name);
	mpr(sp->sct_own, "%s made it through %s defenses!\n",
	    att_name, def_name);
    }
    return 0;
}

int
msl_abm_intercept(struct plnstr *msl, coord x, coord y, int sublaunch)
{
    struct sctstr sect;
    struct emp_qelem irvlist;

    getsect(x, y, &sect);
    msl_sel(&irvlist, x, y, msl->pln_own, P_N, P_O, 0);
    return msl_intercept(msl, &sect, sublaunch,
			 &irvlist, "warhead", "abm",
			 sublaunch ? N_NUKE_SSTOP : N_NUKE_STOP);
}

int
msl_asat_intercept(struct plnstr *msl, coord x, coord y)
{
    struct sctstr sect;
    struct emp_qelem irvlist;

    getsect(x, y, &sect);
    mpr(sect.sct_own, "%s has positioned a satellite over %s\n",
	cname(msl->pln_own), xyas(x, y, sect.sct_own));
    msl_sel(&irvlist, x, y, msl->pln_own, P_O, 0, 0);
    return msl_intercept(msl, &sect, 0,
			 &irvlist, "satellite", "a-sat missile",
			 N_SAT_KILL);
}
