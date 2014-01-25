/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  lndsub.c: Land unit subroutines
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2014
 */

#include <config.h>

#include <math.h>
#include <stdlib.h>
#include "chance.h"
#include "combat.h"
#include "damage.h"
#include "empobj.h"
#include "file.h"
#include "misc.h"
#include "mission.h"
#include "news.h"
#include "nsc.h"
#include "optlist.h"
#include "path.h"
#include "player.h"
#include "prototypes.h"
#include "unit.h"
#include "xy.h"

static void lnd_stays(natid, char *, struct ulist *);
static int lnd_hit_mine(struct lndstr *);
static int has_helpful_engineer(coord, coord, natid);

double
attack_val(int combat_mode, struct lndstr *lp)
{
    int men;
    double value;
    struct lchrstr *lcp;

    if (lp->lnd_effic < LAND_MINEFF) {
	putland(lp->lnd_uid, lp);
	return 0;
    }

    lcp = &lchr[(int)lp->lnd_type];

/* Spies always count as 1 during assaults.  If they are the only ones
   in the assault, they get to sneak on anyway. */

    if (lcp->l_flags & L_SPY && combat_mode == A_ASSAULT)
	return 1;

    men = lp->lnd_item[I_MILIT];
    value = men * lnd_att(lp) * lp->lnd_effic / 100.0;

    switch (combat_mode) {
    case A_ATTACK:
	return value;
    case A_ASSAULT:
	if (!(lcp->l_flags & L_MARINE))
	    return assault_penalty * value;
	break;
    case A_BOARD:
	if (!(lcp->l_flags & L_MARINE))
	    return assault_penalty * men;
    }

    return value;
}

double
defense_val(struct lndstr *lp)
{
    int men;
    double value;
    struct lchrstr *lcp;

    if (lp->lnd_effic < LAND_MINEFF) {
	putland(lp->lnd_uid, lp);
	return 0;
    }

    lcp = &lchr[(int)lp->lnd_type];

    men = lp->lnd_item[I_MILIT];

    if ((lp->lnd_ship >= 0 || lp->lnd_land >= 0) &&
	!(lcp->l_flags & L_MARINE))
	return men;

    value = men * lnd_def(lp) * lp->lnd_effic / 100.0;
    value *= ((double)land_mob_max + lp->lnd_harden) / land_mob_max;

    /* If there are military on the unit, you get at least a 1
       man defensive unit, except for spies */
    if (value < 1.0 && men > 0 && !(lcp->l_flags & L_SPY))
	return 1;

    return value;
}

int
lnd_reaction_range(struct lndstr *lp)
{
    struct sctstr sect;

    getsect(lp->lnd_x, lp->lnd_y, &sect);
    if (sect.sct_type == SCT_HEADQ && sect.sct_effic >= 60)
	return lchr[lp->lnd_type].l_rad + 1;
    return lchr[lp->lnd_type].l_rad;
}

void
lnd_print(natid actor, struct ulist *llp, char *s)
{
    if (actor == player->cnum)
	pr("%s %s\n", prland(&llp->unit.land), s);
    else
	wu(0, actor, "%s %s\n", prland(&llp->unit.land), s);
}

int
lnd_take_casualty(int combat_mode, struct ulist *llp, int cas)
			/* attacking or assaulting or paratrooping? */
			/* number of casualties to take */
{
    int eff_eq;
    int n;
    int biggest;
    int civs;
    coord ret_x, ret_y;
    coord bx, by;
    struct sctstr sect;
    int ret_chance;
    char buf[1024];
    int taken;
    int nowhere_to_go = 0;
    double mobcost, bmcost;
    signed char orig;
    int mob;

    taken = llp->unit.land.lnd_item[I_MILIT];
    /* Spies always die */
    if (((struct lchrstr *)llp->chrp)->l_flags & L_SPY)
	llp->unit.land.lnd_effic = 0;
    else {
	eff_eq = ldround(cas * 100.0 /
	    ((struct lchrstr *)llp->chrp)->l_item[I_MILIT], 1);
	llp->unit.land.lnd_effic -= eff_eq;
	lnd_submil(&llp->unit.land, cas);
    }

    if (llp->unit.land.lnd_effic < LAND_MINEFF) {
	sprintf(buf, "dies %s %s!",
		combat_mode ? att_mode[combat_mode] : "defending",
		xyas(llp->unit.land.lnd_x, llp->unit.land.lnd_y,
		     llp->unit.land.lnd_own));
	lnd_print(llp->unit.land.lnd_own, llp, buf);
	lnd_put_one(llp);
	/* Since we killed the unit, we killed all the mil on it */
	return taken;
    } else {
	/* Ok, now, how many did we take off? (sould be the diff) */
	taken = taken - llp->unit.land.lnd_item[I_MILIT];
    }

    if (llp->unit.land.lnd_effic >= llp->unit.land.lnd_retreat)
	return taken;

    /* we're being boarded */
    if (llp->unit.land.lnd_ship >= 0 && combat_mode == A_DEFEND)
	return taken;

    /* we're being boarded */
    if (llp->unit.land.lnd_land >= 0 && combat_mode == A_DEFEND)
	return taken;

    /* Have to make a retreat check */

    ret_chance = llp->unit.land.lnd_retreat - llp->unit.land.lnd_effic;
    if (pct_chance(ret_chance)) {
	pr("\n");
	lnd_print(llp->unit.land.lnd_own, llp, "fails morale check!");
	llp->unit.land.lnd_mission = 0;
	llp->unit.land.lnd_harden = 0;
	if (llp->unit.land.lnd_ship >= 0 || llp->unit.land.lnd_land >= 0)
	    nowhere_to_go = 1;
	else if (combat_mode == A_DEFEND) {
	    /*
	     * defending unit.. find a place to send it
	     * strategy: look for the most-populated
	     * adjacent sector that is owned by the unit
	     * owner. Charge mob..
	     */
	    biggest = -1;
	    for (n = 1; n <= 6; ++n) {
		ret_x = llp->unit.land.lnd_x + diroff[n][0];
		ret_y = llp->unit.land.lnd_y + diroff[n][1];
		getsect(ret_x, ret_y, &sect);
		if (sect.sct_own != llp->unit.land.lnd_own)
		    continue;
		if (sect.sct_type == SCT_MOUNT)
		    continue;
		mobcost = lnd_mobcost(&llp->unit.land, &sect);
		if (mobcost < 0)
		    continue;
		civs = sect.sct_item[I_CIVIL];
		if (civs > biggest) {
		    biggest = civs;
		    bx = sect.sct_x;
		    by = sect.sct_y;
		    bmcost = mobcost;
		}
	    }
	    if (biggest < 0)
		nowhere_to_go = 1;
	    else {
		/* retreat to bx,by */
		llp->unit.land.lnd_x = bx;
		llp->unit.land.lnd_y = by;
		/* FIXME landmines */
		mob = llp->unit.land.lnd_mobil - (int)bmcost;
		if (mob < -127)
		    mob = -127;
		orig = llp->unit.land.lnd_mobil;
		llp->unit.land.lnd_mobil = (signed char)mob;
		if (llp->unit.land.lnd_mobil > orig)
		    llp->unit.land.lnd_mobil = -127;
		sprintf(buf, "retreats at %d%% efficiency to %s!",
			llp->unit.land.lnd_effic,
			xyas(bx, by, llp->unit.land.lnd_own));
		lnd_print(llp->unit.land.lnd_own, llp, buf);
		lnd_put_one(llp);
	    }
	} else {		/* attacking from a sector */
	    sprintf(buf, "leaves the battlefield at %d%% efficiency",
		    llp->unit.land.lnd_effic);
	    if ((llp->unit.land.lnd_mobil - (int)llp->mobil) < -127)
		llp->unit.land.lnd_mobil = -127;
	    else
		llp->unit.land.lnd_mobil -= (int)llp->mobil;
	    llp->mobil = 0.0;
	    lnd_print(llp->unit.land.lnd_own, llp, buf);
	    lnd_put_one(llp);
	}
    }
    if (nowhere_to_go) {
	/* nowhere to go.. take more casualties */
	llp->unit.land.lnd_effic -= 10;
	lnd_submil(&llp->unit.land,
		   ((struct lchrstr *)llp->chrp)->l_item[I_MILIT] / 10);
	if (llp->unit.land.lnd_effic < LAND_MINEFF) {
	    lnd_print(llp->unit.land.lnd_own, llp,
		      "has nowhere to retreat, and dies!");
	    lnd_put_one(llp);
	} else
	    lnd_print(llp->unit.land.lnd_own, llp,
		      "has nowhere to retreat and takes extra losses!");
    }

    return taken;
}

void
lnd_takemob(struct emp_qelem *list, double loss)
{
    struct emp_qelem *qp, *next;
    struct ulist *llp;
    int new;
    int mcost = ldround(combat_mob * loss, 1);

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	llp = (struct ulist *)qp;
#if 0
	if (chance(loss))
	    use_supply(&llp->unit.land);
#endif
	new = llp->unit.land.lnd_mobil - mcost;
	if (new < -127)
	    new = -127;
	llp->unit.land.lnd_mobil = (signed char)new;
    }
}

void
lnd_submil(struct lndstr *lp, int num)
{
    int new = lp->lnd_item[I_MILIT] - num;
    lp->lnd_item[I_MILIT] = new < 0 ? 0 : new;
}

int
lnd_spyval(struct lndstr *lp)
{
    if (lchr[(int)lp->lnd_type].l_flags & L_RECON)
	return lchr[lp->lnd_type].l_spy * (lp->lnd_effic / 100.0) + 2;
    else
	return lchr[lp->lnd_type].l_spy * (lp->lnd_effic / 100.0);
}

void
intelligence_report(int destination, struct lndstr *lp, int spy,
		    char *mess)
{
    int vis = lnd_vis(lp);
    char buf1[80], buf2[80], buf3[80];

    if (destination == 0)
	return;

    if (lp->lnd_own == 0)
	return;

    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));
    memset(buf3, 0, sizeof(buf3));
    if (chance((spy + vis) / 10.0)) {
	if (destination == player->cnum)
	    pr("%s %s", mess, prland(lp));
	else
	    sprintf(buf1, "%s %s", mess, prland(lp));

	if (chance((spy + vis) / 20.0)) {
	    if (destination == player->cnum)
		pr(" (eff %d, mil %d",
		   roundintby(lp->lnd_effic, 5),
		   roundintby(lp->lnd_item[I_MILIT], 10));
	    else
		sprintf(buf2, " (eff %d, mil %d",
			roundintby(lp->lnd_effic, 5),
			roundintby(lp->lnd_item[I_MILIT], 10));

	    if (chance((spy + vis) / 20.0)) {
		int t;
		t = lp->lnd_tech - 20 + roll(40);
		t = MAX(t, 0);
		if (destination == player->cnum)
		    pr(", tech %d)\n", t);
		else
		    sprintf(buf3, ", tech %d)\n", t);
	    } else {
		if (destination == player->cnum)
		    pr(")\n");
		else
		    sprintf(buf3, ")\n");
	    }
	} else {
	    if (destination == player->cnum)
		pr("\n");
	    else
		sprintf(buf2, "\n");
	}
    }

    if (destination != player->cnum) {
	wu(0, destination, "%s%s%s", buf1, buf2, buf3);
    }
}

void
lnd_sel(struct nstr_item *ni, struct emp_qelem *list)
{
    struct lndstr land;
    int this_mot;
    int mobtype = MOB_MOVE;	/* indeterminate */

    emp_initque(list);
    while (nxtitem(ni, &land)) {
	/*
	 * It would be nice to let deities march foreign land units,
	 * but much of the code assumes that only the land unit's
	 * owner can march it.
	 */
	if (!land.lnd_own || land.lnd_own != player->cnum)
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(EF_LAND, &land)) {
		pr("unit #%d inelligible - it's for sale.\n",
		   land.lnd_uid);
		continue;
	    }
	}
	/*
	 * The marching code gets confused when trains and non-trains
	 * march together.  Disallow for now.
	 */
	this_mot = lnd_mobtype(&land);
	if (this_mot != mobtype) {
	    if (mobtype == MOB_MOVE)
		mobtype = this_mot;
	    else if (mobtype == MOB_MARCH) {
		pr("%s is a train and can't march with the leader.\n",
		   prland(&land));
		continue;
	    } else {
		pr("%s can't rail-march with the leading train.\n",
		   prland(&land));
		continue;
	    }
	}

	land.lnd_mission = 0;
	land.lnd_rflags = 0;
	land.lnd_harden = 0;
	memset(land.lnd_rpath, 0, sizeof(land.lnd_rpath));
	putland(land.lnd_uid, &land);
	lnd_insque(&land, list);
    }
}

/*
 * Append LP to LIST.
 * Return the new list link.
 */
struct ulist *
lnd_insque(struct lndstr *lp, struct emp_qelem *list)
{
    struct ulist *mlp = malloc(sizeof(struct ulist));

    mlp->chrp = (struct empobj_chr *)&lchr[lp->lnd_type];
    mlp->unit.land = *lp;
    mlp->mobil = lp->lnd_mobil;
    emp_insque(&mlp->queue, list);
    return mlp;
}

/* This function assumes that the list was created by lnd_sel */
void
lnd_mar(struct emp_qelem *list, double *minmobp, double *maxmobp,
	int *togetherp, natid actor)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *llp;
    struct lndstr *lp;
    struct sctstr sect;
    coord allx;
    coord ally;
    int first = 1;
    char mess[128];

    *minmobp = 9876.0;
    *maxmobp = -9876.0;
    *togetherp = 1;
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct ulist *)qp;
	lp = &llp->unit.land;
	getland(lp->lnd_uid, lp);
	if (lp->lnd_own != actor) {
	    mpr(actor, "%s was disbanded at %s\n",
		prland(lp), xyas(lp->lnd_x, lp->lnd_y, actor));
	    emp_remque(&llp->queue);
	    free(llp);
	    continue;
	}
	if (lp->lnd_ship >= 0) {
	    lnd_stays(actor, "is on a ship", llp);
	    continue;
	}
	if (lp->lnd_land >= 0) {
	    lnd_stays(actor, "is on a unit", llp);
	    continue;
	}
	if (!getsect(lp->lnd_x, lp->lnd_y, &sect)) {
	    lnd_stays(actor, "was sucked into the sky by a strange looking spaceland", llp);	/* heh -KHS */
	    continue;
	}
	if (!(lchr[lp->lnd_type].l_flags & L_SPY) &&
	    !(lchr[lp->lnd_type].l_flags & L_TRAIN) &&
	    lp->lnd_item[I_MILIT] == 0) {
	    lnd_stays(actor, "has no mil on it to guide it", llp);
	    continue;
	}
	if (relations_with(sect.sct_own, actor) != ALLIED &&
	    !(lchr[lp->lnd_type].l_flags & L_SPY) &&
	    sect.sct_own) {
	    sprintf(mess, "has been kidnapped by %s", cname(sect.sct_own));
	    lnd_stays(actor, mess, llp);
	    continue;
	}
	if (first) {
	    allx = lp->lnd_x;
	    ally = lp->lnd_y;
	    first = 0;
	}
	if (lp->lnd_x != allx || lp->lnd_y != ally)
	    *togetherp = 0;
	if (lp->lnd_mobil + 1 < (int)llp->mobil) {
	    llp->mobil = lp->lnd_mobil;
	}
	if (llp->mobil < *minmobp)
	    *minmobp = llp->mobil;
	if (llp->mobil > *maxmobp)
	    *maxmobp = llp->mobil;
    }
}

static void
lnd_mar_put_one(struct ulist *llp)
{
    if (llp->mobil < -127)
	llp->mobil = -127;
    llp->unit.land.lnd_mobil = llp->mobil;
    lnd_put_one(llp);
}

void
lnd_mar_put(struct emp_qelem *list, natid actor)
{
    struct emp_qelem *qp, *next;
    struct ulist *llp;
    struct lndstr *lp;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct ulist *)qp;
	lp = &llp->unit.land;
	mpr(actor, "%s stopped at %s\n",
	    prland(lp), xyas(lp->lnd_x, lp->lnd_y, actor));
	lnd_mar_put_one(llp);
    }
}

void
lnd_put(struct emp_qelem *list)
{
    struct emp_qelem *qp, *next;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	lnd_put_one((struct ulist *)qp);
    }
}

void
lnd_put_one(struct ulist *llp)
{
    putland(llp->unit.land.lnd_uid, &llp->unit.land);
    emp_remque(&llp->queue);
    free(llp);
}

/*
 * Sweep landmines with engineers in LAND_LIST for ACTOR.
 * If EXPLICIT is non-zero, this is for an explicit sweep command from
 * a player.  Else it's an automatic "on the move" sweep.
 * If TAKEMOB is non-zero, require and charge mobility.
 */
void
lnd_sweep(struct emp_qelem *land_list, int explicit, int takemob,
	  natid actor)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *llp;
    struct sctstr sect;
    int mines, m, max, sshells, lshells;

    for (qp = land_list->q_back; qp != land_list; qp = next) {
	next = qp->q_back;
	llp = (struct ulist *)qp;
	if (!(((struct lchrstr *)llp->chrp)->l_flags & L_ENGINEER)) {
	    if (explicit)
		mpr(actor, "%s is not an engineer!\n",
		    prland(&llp->unit.land));
	    continue;
	}
	if (takemob && llp->mobil < 0.0) {
	    if (explicit)
		lnd_stays(actor, "is out of mobility", llp);
	    continue;
	}
	getsect(llp->unit.land.lnd_x, llp->unit.land.lnd_y, &sect);
	if (!explicit && relations_with(sect.sct_oldown, actor) == ALLIED)
	    continue;
	if (SCT_MINES_ARE_SEAMINES(&sect)) {
	    if (explicit)
		mpr(actor, "%s is in a %s sector.  No landmines there!\n",
		    prland(&llp->unit.land), dchr[sect.sct_type].d_name);
	    continue;
	}
	if (takemob) {
	    llp->mobil -= lnd_pathcost(&llp->unit.land, 0.2);
	    llp->unit.land.lnd_mobil = (int)llp->mobil;
	    llp->unit.land.lnd_harden = 0;
	}
	putland(llp->unit.land.lnd_uid, &llp->unit.land);
	if (!(mines = sect.sct_mines))
	    continue;
	max = ((struct lchrstr *)llp->chrp)->l_item[I_SHELL];
	lshells = llp->unit.land.lnd_item[I_SHELL];
	sshells = sect.sct_item[I_SHELL];
	for (m = 0; mines > 0 && m < max * 2; m++) {
	    if (chance(0.5 * ((struct lchrstr *)llp->chrp)->l_att)) {
		mpr(actor, "Sweep...\n");
		mines--;
		if (lshells < max)
		    ++lshells;
		else if (sshells < ITEM_MAX)
		    ++sshells;
	    }
	}
	sect.sct_mines = mines;
	llp->unit.land.lnd_item[I_SHELL] = lshells;
	sect.sct_item[I_SHELL] = sshells;
	putland(llp->unit.land.lnd_uid, &llp->unit.land);
	putsect(&sect);
    }
}

static int
contains_engineer(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *llp;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct ulist *)qp;
	if (((struct lchrstr *)llp->chrp)->l_flags & L_ENGINEER)
	    return 1;
    }
    return 0;
}

int
lnd_check_mines(struct emp_qelem *land_list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *llp;
    struct sctstr sect;
    int stopping = 0;
    int with_eng = contains_engineer(land_list);

    for (qp = land_list->q_back; qp != land_list; qp = next) {
	next = qp->q_back;
	llp = (struct ulist *)qp;
	getsect(llp->unit.land.lnd_x, llp->unit.land.lnd_y, &sect);
	if (SCT_LANDMINES(&sect) == 0)
	    continue;
	if (relations_with(sect.sct_oldown, llp->unit.land.lnd_own)
	    == ALLIED)
	    continue;
	if (chance(DMINE_LHITCHANCE(sect.sct_mines) / (1 + 2 * with_eng))) {
	    lnd_hit_mine(&llp->unit.land);
	    sect.sct_mines--;
	    putsect(&sect);
	    putland(llp->unit.land.lnd_uid, &llp->unit.land);
	    if (!llp->unit.land.lnd_own) {
		stopping = 1;
		emp_remque(qp);
		free(qp);
	    }
	}
    }
    return stopping;
}

static void
lnd_stays(natid actor, char *str, struct ulist *llp)
{
    mpr(actor, "%s %s & stays in %s\n",
	prland(&llp->unit.land), str,
	xyas(llp->unit.land.lnd_x, llp->unit.land.lnd_y, actor));
    lnd_mar_put_one(llp);
}

static int
lnd_count(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    int count = 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	++count;
    }
    return count;
}

static int
lnd_damage(struct emp_qelem *list, int totdam)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *llp;
    int dam;
    int count;

    if (!totdam || !(count = lnd_count(list)))
	return 0;
    dam = ldround((double)totdam / count, 1);
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct ulist *)qp;
	/* land unit might have changed (launched SAMs, collateral dmg) */
	getland(llp->unit.land.lnd_uid, &llp->unit.land);
	landdamage(&llp->unit.land, dam);
	putland(llp->unit.land.lnd_uid, &llp->unit.land);
	if (!llp->unit.land.lnd_own) {
	    emp_remque(qp);
	    free(qp);
	}
    }
    return dam;
}

static int
lnd_easiest_target(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *llp;
    int hard;
    int easiest = 9876;		/* things start great for victim */
    int count = 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct ulist *)qp;
	hard = lnd_hardtarget(&llp->unit.land);
	if (hard < easiest)
	    easiest = hard;	/* things get worse for victim */
	++count;
    }
    return easiest - count;
}

static int
lnd_missile_interdiction(struct emp_qelem *list, coord newx, coord newy,
			 natid victim)
{
    int mindam = lnd_count(list) * 20;
    int hardtarget = lnd_easiest_target(list);
    int dam, newdam, sublaunch;
    int stopping = 0;
    struct plist *plp;
    struct emp_qelem msl_list, *qp, *newqp;

    msl_sel(&msl_list, newx, newy, victim, P_T, P_MAR, MI_INTERDICT);

    dam = 0;
    for (qp = msl_list.q_back; qp != &msl_list; qp = newqp) {
	newqp = qp->q_back;
	plp = (struct plist *)qp;

	if (dam < mindam && mission_pln_equip(plp, NULL, 'p') >= 0) {
	    if (msl_launch(&plp->plane, EF_LAND, "troops",
			   newx, newy, victim, &sublaunch) < 0)
		goto use_up_msl;
	    stopping = 1;
	    if (msl_hit(&plp->plane, hardtarget, EF_LAND,
			N_LND_MISS, N_LND_SMISS, sublaunch, victim)) {
		newdam = pln_damage(&plp->plane, 'p', 1);
		dam += newdam;
	    } else {
		newdam = pln_damage(&plp->plane, 'p', 0);
		collateral_damage(newx, newy, newdam);
	    }
	use_up_msl:
	    plp->plane.pln_effic = 0;
	    putplane(plp->plane.pln_uid, &plp->plane);
	}
	emp_remque(qp);
	free(qp);
    }

    if (dam) {
	mpr(victim, "missile interdiction mission does %d damage!\n", dam);
	collateral_damage(newx, newy, dam);
	lnd_damage(list, dam);
    }
    return stopping;
}

#if 0
/* Steve M. - commented out for now until abuse is decided upon */
/* risner: allow forts to interdict land units. */
static int
lnd_fort_interdiction(struct emp_qelem *list,
		      coord newx, coord newy, natid victim)
{
    struct nstr_sect ns;
    struct sctstr fsect;
    int trange, range;
    int dam;
    int stopping = 0;
    int totdam = 0;

    snxtsct_dist(&ns, newx, newy, fort_max_interdiction_range);
    while (nxtsct(&ns, &fsect)) {
	if (fsect.sct_own == 0)
	    continue;
	if (relations_with(fsect.sct_own, victim) >= NEUTRAL)
	    continue;
	range = roundrange(fortrange(&fsect));
	trange = mapdist(newx, newy, fsect.sct_x, fsect.sct_y);
	if (trange > range)
	    continue;
	dam = fort_fire(&fsect);
	putsect(&fsect);
	if (dam < 0)
	    continue;
	stopping = 1;
	totdam += dam;
	mpr(victim, "Incoming fire does %d damage!\n", dam);
	wu(0, fsect.sct_own,
	   "%s fires at %s land units in %s for %d!\n",
	   xyas(fsect.sct_x, fsect.sct_y,
		fsect.sct_own),
	   cname(victim), xyas(newx, newy, fsect.sct_own), dam);
	nreport(fsect.sct_own, N_SCT_SHELL, victim, 1);
    }
    if (totdam > 0)
	lnd_damage(list, totdam);
    return stopping;
}
#endif

static int
lnd_mission_interdiction(struct emp_qelem *list, coord x, coord y,
			 natid victim)
{
    int dam;

    dam = unit_interdict(x, y, victim, "land units",
			 lnd_easiest_target(list),
			 MI_INTERDICT);
    if (dam >= 0)
	lnd_damage(list, dam);
    return dam >= 0;
}

int
lnd_interdict(struct emp_qelem *list, coord newx, coord newy, natid victim)
{
    int stopping = 0;

#if 0
    if (!opt_NO_FORT_FIRE)
/* Steve M. - commented out for now until abuse is decided upon */
	stopping |= lnd_fort_interdiction(list, newx, newy, victim);
#endif

    stopping |= lnd_mission_interdiction(list, newx, newy, victim);
    stopping |= lnd_missile_interdiction(list, newx, newy, victim);
    return stopping;
}

/* high value of hardtarget is harder to hit */
int
lnd_hardtarget(struct lndstr *lp)
{
    struct sctstr sect;

    getsect(lp->lnd_x, lp->lnd_y, &sect);
    return (int)((lp->lnd_effic / 100.0) *
		 (10 + dchr[sect.sct_type].d_dstr * 2 + lnd_spd(lp) / 2.0
		  - lnd_vis(lp)));
}

static int
lnd_hit_mine(struct lndstr *lp)
{
    int m;

    mpr(lp->lnd_own, "Blammo! Landmines detected in %s!\n",
	xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));

    nreport(lp->lnd_own, N_LHIT_MINE, 0, 1);

    m = MINE_LDAMAGE();
    if (lchr[lp->lnd_type].l_flags & L_ENGINEER)
	m /= 2;

    landdamage(lp, m);
    return m;
}

double
lnd_pathcost(struct lndstr *lp, double pathcost)
{
    double effspd;

    effspd = lnd_spd(lp);
    if (lchr[(int)lp->lnd_type].l_flags & L_SUPPLY)
	effspd *= lp->lnd_effic * 0.01;

    /*
     * The return value must be PATHCOST times a factor that depends
     * only on the land unit.  Anything else breaks path finding.  In
     * particular, you can't add or enforce a minimum cost here.  Do
     * it in sector_mcost().
     */
    return pathcost * 5.0 * speed_factor(effspd, lp->lnd_tech);
}

int
lnd_mobtype(struct lndstr *lp)
{
    return (lchr[(int)lp->lnd_type].l_flags & L_TRAIN)
	? MOB_RAIL : MOB_MARCH;
}

double
lnd_mobcost(struct lndstr *lp, struct sctstr *sp)
{
    return lnd_pathcost(lp, sector_mcost(sp, lnd_mobtype(lp)));
}

int
lnd_mar_one_sector(struct emp_qelem *list, int dir, natid actor,
		   int together)
{
    struct sctstr sect, osect;
    struct emp_qelem *qp;
    struct emp_qelem *qp2;
    struct emp_qelem *next;
    struct ulist *llp;
    struct emp_qelem cur, done;
    coord dx;
    coord dy;
    coord newx;
    coord newy;
    int stopping = 0;
    int visible;
    int stop;
    char dp[80];
    int rel;
    int oldown;

    if (dir <= DIR_STOP || dir >= DIR_VIEW) {
	lnd_mar_put(list, actor);
	return 1;
    }
    dx = diroff[dir][0];
    dy = diroff[dir][1];
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct ulist *)qp;
	getsect(llp->unit.land.lnd_x, llp->unit.land.lnd_y, &osect);
	oldown = osect.sct_own;
	newx = xnorm(llp->unit.land.lnd_x + dx);
	newy = ynorm(llp->unit.land.lnd_y + dy);
	getsect(newx, newy, &sect);
	rel = relations_with(sect.sct_own, actor);
	if ((rel != ALLIED &&
	     !(lchr[(int)llp->unit.land.lnd_type].l_flags & L_SPY) &&
	     sect.sct_own) || (sect.sct_type == SCT_WATER ||
			       sect.sct_type == SCT_SANCT ||
			       sect.sct_type == SCT_WASTE)) {
	    if (together) {
		mpr(actor, "can't go to %s\n", xyas(newx, newy, actor));
		return 1;
	    } else {
		sprintf(dp, "can't go to %s", xyas(newx, newy, actor));
		lnd_stays(actor, dp, llp);
		continue;
	    }
	}
	if (!SCT_HAS_RAIL(&sect)
	    && lnd_mobtype(&llp->unit.land) == MOB_RAIL) {
	    if (together) {
		mpr(actor, "no rail system in %s\n",
		    xyas(newx, newy, actor));
		return 1;
	    } else {
		sprintf(dp, "has no rail system in %s",
			xyas(newx, newy, actor));
		lnd_stays(actor, dp, llp);
		continue;
	    }
	}
	/* Note we check would_abandon first because we don't want
	   to always have to do these checks */
	if (would_abandon(&osect, I_CIVIL, 0, &llp->unit.land)) {
	    stop = 0;
	    if (!want_to_abandon(&osect, I_CIVIL, 0, &llp->unit.land)) {
		stop = 1;
	    }
	    /* now check stuff */
	    if (!check_sect_ok(&sect))
		return 1;
	    if (!check_sect_ok(&osect))
		return 1;
	    for (qp2 = list->q_back; qp2 != list; qp2 = qp2->q_back) {
		if (!check_land_ok(&((struct ulist *)qp2)->unit.land))
		    return 1;
	    }
	    if (stop) {
		lnd_stays(actor, "stops", llp);
		continue;
	    }
	}
	if (llp->mobil <= 0.0) {
	    lnd_stays(actor, "is out of mobility", llp);
	    continue;
	}
	llp->unit.land.lnd_x = newx;
	llp->unit.land.lnd_y = newy;
	llp->mobil -= lnd_mobcost(&llp->unit.land, &sect);
	llp->unit.land.lnd_mobil = (int)llp->mobil;
	llp->unit.land.lnd_harden = 0;
	putland(llp->unit.land.lnd_uid, &llp->unit.land);
	putsect(&osect);
	getsect(osect.sct_x, osect.sct_y, &osect);
	if (osect.sct_own != oldown && oldown == actor) {
	    /* It was your sector, now it's not.  Simple :) */
	    mpr(actor, "You no longer own %s\n",
		xyas(osect.sct_x, osect.sct_y, actor));
	}
	if (rel != ALLIED && sect.sct_own) {	/* must be a spy */
	    /* Always a 10% chance of getting caught. */
	    if (chance(LND_SPY_DETECT_CHANCE(llp->unit.land.lnd_effic))) {
		if (rel == NEUTRAL || rel == FRIENDLY) {
		    wu(0, sect.sct_own,
		       "%s unit spotted in %s\n", cname(actor),
		       xyas(sect.sct_x, sect.sct_y, sect.sct_own));
		    setrel(sect.sct_own, llp->unit.land.lnd_own, HOSTILE);
		} else if (rel <= HOSTILE) {
		    wu(0, sect.sct_own,
		       "%s spy shot in %s\n", cname(actor),
		       xyas(sect.sct_x, sect.sct_y, sect.sct_own));
		    mpr(actor, "%s was shot and killed.\n",
			prland(&llp->unit.land));
		    llp->unit.land.lnd_effic = 0;
		    putland(llp->unit.land.lnd_uid, &llp->unit.land);
		    lnd_put_one(llp);
		}
	    }
	}
    }
    if (QEMPTY(list))
	return stopping;
    lnd_sweep(list, 0, 1, actor);
    stopping |= lnd_check_mines(list);
    if (QEMPTY(list))
	return stopping;

    /* interdict land units sector by sector */
    emp_initque(&cur);
    emp_initque(&done);
    while (!QEMPTY(list)) {
	llp = (struct ulist *)list->q_back;
	newx = llp->unit.land.lnd_x;
	newy = llp->unit.land.lnd_y;
	/* move units in NEWX,NEWY to cur */
	visible = 0;
	for (qp = list->q_back; qp != list; qp = next) {
	    next = qp->q_back;
	    llp = (struct ulist *)qp;
	    if (llp->unit.land.lnd_x == newx && llp->unit.land.lnd_y == newy) {
		emp_remque(qp);
		emp_insque(qp, &cur);
		if (!(lchr[(int)llp->unit.land.lnd_type].l_flags & L_SPY))
		    visible = 1;
	    }
	}
	/* interdict them */
	if (visible)
	    stopping |= lnd_interdict(&cur, newx, newy, actor);
	/* move survivors to done */
	for (qp = cur.q_back; qp != &cur; qp = next) {
	    next = qp->q_back;
	    emp_remque(qp);
	    emp_insque(qp, &done);
	}
    }
    /* assign surviving land units back to list */
    emp_insque(list, &done);
    emp_remque(&done);

    return stopping;
}

/*
 * Fire land unit support against VICTIM for ATTACKER, at X,Y.
 * If DEFENDING, this is defensive support, else offensive support.
 * Return total damage.
 */
int
lnd_support(natid victim, natid attacker, coord x, coord y, int defending)
{
    struct nstr_item ni;
    struct lndstr land;
    int dam, dam2;
    int dist;
    int range;

    dam = 0;
    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, &land)) {
	if ((land.lnd_x == x) && (land.lnd_y == y))
	    continue;
	if (!feels_like_helping(land.lnd_own, attacker, victim))
	    continue;

	/* are we in range? */
	dist = mapdist(land.lnd_x, land.lnd_y, x, y);

	range = roundrange(lnd_fire_range(&land));
	if (dist > range)
	    continue;

	dam2 = lnd_fire(&land);
	putland(land.lnd_uid, &land);
	if (dam2 < 0)
	    continue;

	if (defending)
	    nreport(land.lnd_own, N_FIRE_BACK, victim, 1);
	else
	    nreport(land.lnd_own, N_FIRE_L_ATTACK, victim, 1);
	if (pct_chance(lnd_acc(&land) - 1))
	    dam2 /= 2;
	dam += dam2;
	if (land.lnd_own != attacker)
	    wu(0, land.lnd_own,
	       "%s supported %s at %s\n",
	       prland(&land), cname(attacker), xyas(x, y, land.lnd_own));
    }
    return dam;
}

int
lnd_can_attack(struct lndstr *lp)
{
    struct lchrstr *lcp = &lchr[(int)lp->lnd_type];

    if (lcp->l_flags & L_SUPPLY)
	return 0;

    return 1;
}

/*
 * Increase fortification value of LP.
 * Fortification costs mobility.  Use up to MOB mobility.
 * Return actual fortification increase.
 */
int
lnd_fortify(struct lndstr *lp, int mob)
{
    int hard_amt;
    double mob_used, mult;

    if (lp->lnd_ship >= 0 || lp->lnd_land >= 0)
	return 0;

    mob_used = MIN(lp->lnd_mobil, mob);
    if (mob_used < 0)
	return 0;

    mult = has_helpful_engineer(lp->lnd_x, lp->lnd_y, lp->lnd_own)
	? 1.5 : 1.0;

    hard_amt = (int)(mob_used * mult);
    if (lp->lnd_harden + hard_amt > land_mob_max) {
	hard_amt = land_mob_max - lp->lnd_harden;
	mob_used = ceil(hard_amt / mult);
    }

    lp->lnd_mobil -= (int)mob_used;
    lp->lnd_harden += hard_amt;
    lp->lnd_harden = MIN(lp->lnd_harden, land_mob_max);

    return hard_amt;
}

/*
 * Is there a engineer unit at X,Y that can help nation CN?
 */
static int
has_helpful_engineer(coord x, coord y, natid cn)
{
    struct nstr_item ni;
    struct lndstr land;

    snxtitem_xy(&ni, EF_LAND, x, y);
    while (nxtitem(&ni, &land)) {
	if (relations_with(land.lnd_own, cn) != ALLIED)
	    continue;
	if (lchr[(int)land.lnd_type].l_flags & L_ENGINEER)
	    return 1;
    }

    return 0;
}

/*
 * Set LP's tech to TLEV along with everything else that depends on it.
 */
void
lnd_set_tech(struct lndstr *lp, int tlev)
{
    struct lchrstr *lcp = lchr + lp->lnd_type;

    if (CANT_HAPPEN(tlev < lcp->l_tech))
	tlev = lcp->l_tech;

    lp->lnd_tech = tlev;
}
