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
 *  lndsub.c: Land unit subroutines
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#include <math.h>
#include "misc.h"
#include "player.h"
#include "file.h"
#include "var.h"
#include "sect.h"
#include "path.h"
#include "news.h"
#include "treaty.h"
#include "nat.h"
#include "xy.h"
#include "land.h"
#include "ship.h"
#include "nsc.h"
#include "mission.h"
#include "plane.h"
#include "combat.h"
#include "damage.h"
#include "optlist.h"
#include "prototypes.h"

int
attack_val(int combat_mode, struct lndstr *lp)
{
    int men;
    int value;
    struct lchrstr *lcp;

    if (((int)lp->lnd_effic) < LAND_MINEFF) {
	makelost(EF_LAND, lp->lnd_own, lp->lnd_uid, lp->lnd_x, lp->lnd_y);
	lp->lnd_own = 0;
	putland(lp->lnd_uid, lp);
	return 0;
    }

    lcp = &lchr[(int)lp->lnd_type];

/* Spies always count as 1 during assaults.  If they are the only ones
   in the assault, they get to sneak on anyway. */

    if (lcp->l_flags & L_SPY && combat_mode == A_ASSAULT)
	return 1;

    men = total_mil(lp);

    value = ldround(((double)men * (double)lp->lnd_att), 1);

    value = (int)((double)value * ((double)lp->lnd_effic / 100.0));

    switch (combat_mode) {
    case A_ATTACK:
	return value;
    case A_ASSAULT:
	if (!(lcp->l_flags & L_MARINE))
	    return (int)(assault_penalty * value);
	break;
    case A_BOARD:
	if (!(lcp->l_flags & L_MARINE))
	    return (int)(assault_penalty * men);
    }

    return value;
}

int
defense_val(struct lndstr *lp)
{
    int men;
    double value;
    struct lchrstr *lcp;

    if (((int)lp->lnd_effic) < LAND_MINEFF) {
	makelost(EF_LAND, lp->lnd_own, lp->lnd_uid, lp->lnd_x, lp->lnd_y);
	lp->lnd_own = 0;
	putland(lp->lnd_uid, lp);
	return 0;
    }

    lcp = &lchr[(int)lp->lnd_type];

    men = total_mil(lp);

    if (men < 0)
	men = 0;
    if ((lp->lnd_ship >= 0 || lp->lnd_land >= 0) &&
	!(lcp->l_flags & L_MARINE))
	return men;

    value = men * lp->lnd_def;

    value *=
	((double)land_mob_max + lp->lnd_harden) / (double)land_mob_max;
    value = (int)((double)value * ((double)lp->lnd_effic / 100.0));
    value = (int)ldround(value, 1);

    /* If there are military on the unit, you get at least a 1
       man defensive unit, except for spies */
    if (value < 1.0 && men > 0 && !(lcp->l_flags & L_SPY))
	return 1;

    return (int)(value);
}

int
total_mil(struct lndstr *lp)
{
    struct lchrstr *lcp;
    double men;

    lcp = &lchr[(int)lp->lnd_type];

    men = lnd_getmil(lp);
/*	men *= ((double)lp->lnd_effic)/100.0;*/

    return ldround(men, 1);
}

void
lnd_print(struct llist *llp, s_char *s)
{
    if (llp->land.lnd_own == player->cnum)
	pr("%s %s\n", prland(&llp->land), s);
    else
	wu(0, llp->land.lnd_own, "%s %s\n", prland(&llp->land), s);
}

void
lnd_delete(struct llist *llp, s_char *s)
{
    if (s)
	lnd_print(llp, s);
    putland(llp->land.lnd_uid, &llp->land);
    emp_remque((struct emp_qelem *)llp);
    free((s_char *)llp);
}

int
lnd_take_casualty(int combat_mode, struct llist *llp, int cas)
			/* attacking or assaulting or paratrooping? */
			/* number of casualties to take */
{
    int eff_eq;
    int n;
    int biggest;
    int civs;
    int nowned;
    coord ret_x, ret_y;
    coord bx, by;
    struct sctstr sect;
    int ret_chance;
    s_char buf[1024];
    int taken;
    int nowhere_to_go = 0;
    struct sctstr rsect;
    double mobcost;
    s_char orig;
    int mob;



    taken = lnd_getmil(&(llp->land));
    /* Spies always die */
    if (llp->lcp->l_flags & L_SPY) {
	eff_eq = 100;
	llp->land.lnd_effic = 0;
    } else {
	eff_eq =
	    ldround((((double)cas * 100.0) / (double)llp->lcp->l_mil), 1);
	llp->land.lnd_effic -= eff_eq;
	lnd_submil(&(llp->land), cas);
    }

    if (llp->land.lnd_effic < LAND_MINEFF) {
	sprintf(buf, "dies %s %s!",
		combat_mode ? att_mode[combat_mode] : (s_char *)
		"defending", xyas(llp->land.lnd_x, llp->land.lnd_y,
				  llp->land.lnd_own));
	lnd_delete(llp, buf);
	/* Since we killed the unit, we killed all the mil on it */
	return taken;
    } else {
	/* Ok, now, how many did we take off? (sould be the diff) */
	taken = taken - lnd_getmil(&(llp->land));
    }

    if (llp->land.lnd_effic >= llp->land.lnd_retreat)
	return taken;

    /* we're being boarded */
    if (llp->land.lnd_ship >= 0 && combat_mode == A_DEFEND)
	return taken;

    /* we're being boarded */
    if (llp->land.lnd_land >= 0 && combat_mode == A_DEFEND)
	return taken;

    /* Have to make a retreat check */

    ret_chance = llp->land.lnd_retreat - llp->land.lnd_effic;
    if (roll(100) < ret_chance) {
	pr("\n");
	lnd_print(llp, "fails morale check!");
	llp->land.lnd_mission = 0;
	llp->land.lnd_harden = 0;
	if (llp->land.lnd_ship >= 0 || llp->land.lnd_land >= 0)
	    nowhere_to_go = 1;
	else if (combat_mode == A_DEFEND) {
	    /*
	     * defending unit.. find a place to send it
	     * strategy: look for the most-populated 
	     * adjacent sector that is owned by the unit
	     * player->owner. Charge mob..
	     */
	    biggest = -1;
	    nowned = 0;
	    for (n = 1; n <= 6; ++n) {
		ret_x = llp->land.lnd_x + diroff[n][0];
		ret_y = llp->land.lnd_y + diroff[n][1];
		getsect(ret_x, ret_y, &sect);
		if (sect.sct_own != llp->land.lnd_own)
		    continue;
		if (sect.sct_type == SCT_MOUNT)
		    continue;
		++nowned;
		civs = sect.sct_item[I_CIVIL];
		if (civs > biggest) {
		    biggest = civs;
		    bx = sect.sct_x;
		    by = sect.sct_y;
		}
	    }
	    if (!nowned)
		nowhere_to_go = 1;
	    else {
		/* retreat to bx,by */
		llp->land.lnd_x = bx;
		llp->land.lnd_y = by;
		getsect(bx, by, &rsect);
		mobcost = lnd_mobcost(&llp->land, &rsect, MOB_ROAD);
		mob = llp->land.lnd_mobil - (int)mobcost;
		if (mob < -127)
		    mob = -127;
		orig = llp->land.lnd_mobil;
		llp->land.lnd_mobil = (s_char)mob;
		if (llp->land.lnd_mobil > orig)
		    llp->land.lnd_mobil = (-127);
		sprintf(buf, "retreats at %d%% efficiency to %s!",
			llp->land.lnd_effic,
			xyas(bx, by, llp->land.lnd_own));
		lnd_delete(llp, buf);
	    }
	} else {		/* attacking from a sector */
	    sprintf(buf, "leaves the battlefield at %d%% efficiency",
		    llp->land.lnd_effic);
	    if ((llp->land.lnd_mobil - (int)llp->mobil) < -127)
		llp->land.lnd_mobil = -127;
	    else
		llp->land.lnd_mobil -= (int)llp->mobil;
	    llp->mobil = 0.0;
	    lnd_delete(llp, buf);
	}
    }
    if (nowhere_to_go) {
	/* nowhere to go.. take more casualties */
	llp->land.lnd_effic -= 10;
	lnd_submil(&llp->land, llp->lcp->l_mil / 10);
	if (llp->land.lnd_effic < LAND_MINEFF)
	    lnd_delete(llp, "has nowhere to retreat, and dies!");
	else
	    lnd_print(llp,
		      "has nowhere to retreat and takes extra losses!");
    }

    return taken;
}

void
lnd_takemob(struct emp_qelem *list, double loss)
{
    struct emp_qelem *qp, *next;
    struct llist *llp;
    int new;
    int mcost = ldround(combat_mob * loss, 1);

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
/*
		if (chance(loss))
			use_supply(&llp->land);
		if (llp->land.lnd_mission == MI_RESERVE)
			new = llp->land.lnd_mobil - mcost/2;
		else
 */
	new = llp->land.lnd_mobil - mcost;
	if (new < -127)
	    new = -127;
	llp->land.lnd_mobil = (s_char)new;
    }
}
int
lnd_getmil(struct lndstr *lp)
{
    return lp->lnd_item[I_MILIT];
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
	return (lp->lnd_spy * (lp->lnd_effic / 100.0)) + 2;
    else
	return (lp->lnd_spy * (lp->lnd_effic / 100.0));
}

int
intelligence_report(int destination, struct lndstr *lp, int spy,
		    s_char *mess)
{
    struct lchrstr *lcp;
    s_char buf1[80], buf2[80], buf3[80];
    double estimate = 0.0;	/* estimated defense value */

    if (destination == 0)
	return 0;

    if (lp->lnd_own == 0)
	return 0;

    lcp = &lchr[(int)lp->lnd_type];

    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));
    memset(buf3, 0, sizeof(buf3));
    if (chance((double)(spy + lp->lnd_vis) / 10.0)) {
	if (destination == player->cnum)
	    pr("%s %s", mess, prland(lp));
	else
	    sprintf(buf1, "%s %s", mess, prland(lp));

	estimate = lnd_getmil(lp);

	if (chance((double)(spy + lp->lnd_vis) / 20.0)) {

	    if (destination == player->cnum)
		pr(" (eff %d, mil %d", roundintby(lp->lnd_effic, 5),
		   roundintby(lnd_getmil(lp), 10));
	    else
		sprintf(buf2, " (eff %d, mil %d",
			roundintby(lp->lnd_effic, 5),
			roundintby(lnd_getmil(lp), 10));
	    estimate = lnd_getmil(lp) * lp->lnd_effic / 100.0;

	    if (chance((double)(spy + lp->lnd_vis) / 20.0)) {
		int t;
		t = lp->lnd_tech - 20 + roll(40);
		t = max(t, 0);
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

    if (lp->lnd_ship < 0 || lcp->l_flags & L_MARINE)
	estimate *= lp->lnd_def;

    return (int)estimate;
}

/* Used by the spy command to count land units in a sector.  If used
   for anything else, you may want to reconsider, because this doesn't
   always count spies. :) */
int
count_sect_units(struct sctstr *sp)
{
    int count = 0;
    struct nstr_item ni;
    struct lndstr land;

    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, (s_char *)&land)) {
	if (!land.lnd_own)
	    continue;
	if (land.lnd_x != sp->sct_x || land.lnd_y != sp->sct_y)
	    continue;
	/* Don't always see spies */
	if (lchr[(int)land.lnd_type].l_flags & L_SPY) {
	    if (!(chance(LND_SPY_DETECT_CHANCE(land.lnd_effic))))
		continue;
	}
	/* Got here, report it */
	++count;
    }

    return count;
}

void
count_units(struct shpstr *sp)
{
    struct nstr_item ni;
    struct lndstr land;
    int nland = 0;

    if (sp->shp_effic < SHIP_MINEFF)
	return;

    snxtitem_xy(&ni, EF_LAND, sp->shp_x, sp->shp_y);
    while (nxtitem(&ni, (s_char *)&land)) {
	if (land.lnd_own == 0)
	    continue;
	if (land.lnd_ship == sp->shp_uid)
	    nland++;
    }

    if (sp->shp_nland != nland) {
	sp->shp_nland = nland;
	putship(sp->shp_uid, sp);
    }
}

void
lnd_count_units(struct lndstr *lp)
{
    struct nstr_item ni;
    struct lndstr land;
    int nland = 0;

    if (lp->lnd_effic < LAND_MINEFF)
	return;

    snxtitem_xy(&ni, EF_LAND, lp->lnd_x, lp->lnd_y);
    while (nxtitem(&ni, (s_char *)&land)) {
	if (land.lnd_own == 0)
	    continue;
	if (land.lnd_land == lp->lnd_uid)
	    nland++;
    }

    if (lp->lnd_nland != nland) {
	lp->lnd_nland = nland;
	putland(lp->lnd_uid, lp);
    }
}

void
lnd_sel(struct nstr_item *ni, struct emp_qelem *list)


/*	int	wantflags;
	int	nowantflags;
*/
{
    struct lndstr land;
    struct lchrstr *lcp;
    struct llist *llp;

    emp_initque(list);
    while (nxtitem(ni, (s_char *)&land)) {
	if (!player->owner)
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(EF_LAND, (int *)&land)) {
		pr("unit #%d inelligible - it's for sale.\n",
		   land.lnd_uid);
		continue;
	    }
	}
	lcp = &lchr[(int)land.lnd_type];
/*		if (wantflags && (lcp->m_flags & wantflags) != wantflags)
			continue;
		if (nowantflags && lcp->m_flags & nowantflags)
			continue;
*/
	/* This abuse is better fixed by building a unit with the normal negative
	   mobility that everything else is built with */
/* Just so that the player can't build a bunch of land units, and them
   march them a few minutes later... */
/*
		if (opt_MOB_ACCESS) {
		  if (land.lnd_effic < 11 &&
		    land.lnd_mobil < etu_per_update) {
		    pr("Land unit #%d needs at least %d mob to march.\n",
			land.lnd_uid, etu_per_update);
		    continue;
		  }
		}
*/
	land.lnd_mission = 0;
	land.lnd_rflags = 0;
	land.lnd_harden = 0;
	memset(land.lnd_rpath, 0, sizeof(land.lnd_rpath));
	putland(land.lnd_uid, &land);
	llp = (struct llist *)malloc(sizeof(struct llist));
	llp->lcp = lcp;
	llp->land = land;
	llp->mobil = (double)land.lnd_mobil;
	emp_insque(&llp->queue, list);
    }
}

/* This function assumes that the list was created by lnd_sel */
void
lnd_mar(struct emp_qelem *list, double *minmobp, double *maxmobp,
	int *togetherp, natid actor)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct llist *llp;
    struct sctstr sect;
    struct lndstr land;
    coord allx;
    coord ally;
    int first = 1;
    s_char mess[128];
    int rel;

    *minmobp = 9876.0;
    *maxmobp = -9876.0;
    *togetherp = 1;
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct llist *)qp;
	getland(llp->land.lnd_uid, &land);
	if (land.lnd_own != actor) {
	    mpr(actor, "%s was disbanded at %s\n",
		prland(&land), xyas(land.lnd_x, land.lnd_y, land.lnd_own));
	    emp_remque((struct emp_qelem *)llp);
	    free((s_char *)llp);
	    continue;
	}
	if (land.lnd_ship >= 0) {
	    lnd_mess("is on a ship", llp);
	    continue;
	}
	if (land.lnd_land >= 0) {
	    lnd_mess("is on a unit", llp);
	    continue;
	}
	if (!getsect(land.lnd_x, land.lnd_y, &sect)) {
	    lnd_mess("was sucked into the sky by a strange looking spaceland", llp);	/* heh -KHS */
	    continue;
	}
	if (!(lchr[(int)llp->land.lnd_type].l_flags & L_SPY) &&
	    !(lchr[(int)llp->land.lnd_type].l_flags & L_TRAIN) &&
	    lnd_getmil(&llp->land) == 0) {
	    lnd_mess("has no mil on it to guide it", llp);
	    continue;
	}
	rel = getrel(getnatp(sect.sct_own), player->cnum);
	if (sect.sct_own != land.lnd_own && rel != ALLIED &&
	    !(lchr[(int)llp->land.lnd_type].l_flags & L_SPY) &&
	    sect.sct_own) {
	    sprintf(mess, "has been kidnapped by %s", cname(sect.sct_own));
	    lnd_mess(mess, llp);
	    continue;
	}
	if (first) {
	    allx = land.lnd_x;
	    ally = land.lnd_y;
	    first = 0;
	}
	if (land.lnd_x != allx || land.lnd_y != ally)
	    *togetherp = 0;
	if (land.lnd_mobil + 1 < (int)llp->mobil) {
	    llp->mobil = (double)land.lnd_mobil;
	}
	if (llp->mobil < *minmobp)
	    *minmobp = llp->mobil;
	if (llp->mobil > *maxmobp)
	    *maxmobp = llp->mobil;
	llp->land = land;
    }
}

void
lnd_put(struct emp_qelem *list, natid actor)
{
    register struct emp_qelem *qp;
    register struct emp_qelem *newqp;
    struct llist *llp;

    qp = list->q_back;
    while (qp != list) {
	llp = (struct llist *)qp;
	if (actor) {
	    mpr(actor, "%s stopped at %s\n", prland(&llp->land),
		xyas(llp->land.lnd_x, llp->land.lnd_y, llp->land.lnd_own));
	    if (llp->mobil < -127)
		llp->mobil = -127;
	    llp->land.lnd_mobil = llp->mobil;
	}
	putland(llp->land.lnd_uid, &llp->land);
	newqp = qp->q_back;
	emp_remque(qp);
	free((s_char *)qp);
	qp = newqp;
    }
}

void
lnd_sweep(struct emp_qelem *land_list, int verbose, int takemob,
	  natid actor)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct llist *llp;
    struct sctstr sect;
    int mines, m, max, sshells, lshells;
    double mobcost;

    for (qp = land_list->q_back; qp != land_list; qp = next) {
	next = qp->q_back;
	llp = (struct llist *)qp;
	if (!(llp->lcp->l_flags & L_ENGINEER)) {
	    if (verbose)
		mpr(actor, "%s is not an engineer!\n", prland(&llp->land));
	    continue;
	}
	if (takemob && llp->mobil < 0.0) {
	    if (verbose)
		lnd_mess("is out of mobility", llp);
	    continue;
	}
	getsect(llp->land.lnd_x, llp->land.lnd_y, &sect);
	if (sect.sct_oldown == llp->land.lnd_own) {
	    if (verbose)
		mpr(actor,
		    "%s is in a sector completely owned by you.  Don't bother digging up mines there!\n",
		    prland(&llp->land));
	    continue;
	}
	if (sect.sct_type == SCT_BSPAN) {
	    if (verbose)
		mpr(actor, "%s is on a bridge.  No mines there!\n",
		    prland(&llp->land));
	    continue;
	}
	if (takemob) {
/*			mobcost = llp->land.lnd_effic * 0.01 * llp->lcp->l_spd;*/
	    mobcost = llp->land.lnd_spd;
	    mobcost = 480.0 / (mobcost +
			       techfact(llp->land.lnd_tech, mobcost));
	    llp->mobil -= mobcost;
	    llp->land.lnd_mobil = (int)llp->mobil;
	    llp->land.lnd_harden = 0;
	}
	putland(llp->land.lnd_uid, &llp->land);
	if (!(mines = sect.sct_mines))
	    continue;
	max = vl_find(V_SHELL, llp->lcp->l_vtype,
		      llp->lcp->l_vamt, (int)llp->lcp->l_nv);
	lshells = llp->land.lnd_item[I_SHELL];
	sshells = sect.sct_item[I_SHELL];
	for (m = 0; mines > 0 && m < max * 2; m++) {
	    if (chance(0.5 * llp->lcp->l_att)) {
		mpr(actor, "Sweep...\n");
		mines--;
		if (lshells < max)
		    ++lshells;
		else if (sshells < ITEM_MAX)
		    ++sshells;
	    }
	}
	sect.sct_mines = mines;
	llp->land.lnd_item[I_SHELL] = lshells;
	sect.sct_item[I_SHELL] = sshells;
	putland(llp->land.lnd_uid, &llp->land);
	putsect(&sect);
    }
}

static int
contains_engineer(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct llist *llp;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct llist *)qp;
	if (llp->lcp->l_flags & L_ENGINEER)
	    return 1;
    }
    return 0;
}

int
lnd_check_mines(struct emp_qelem *land_list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct llist *llp;
    struct sctstr sect;
    int stopping = 0;
    int with_eng = contains_engineer(land_list);

    for (qp = land_list->q_back; qp != land_list; qp = next) {
	next = qp->q_back;
	llp = (struct llist *)qp;
	getsect(llp->land.lnd_x, llp->land.lnd_y, &sect);
	if (sect.sct_oldown == llp->land.lnd_own)
	    continue;
	if (sect.sct_type == SCT_BSPAN)
	    continue;
	if (!sect.sct_mines)
	    continue;
	if (chance(DMINE_LHITCHANCE(sect.sct_mines) / (1 + 2 * with_eng))) {
	    lnd_hit_mine(&llp->land, llp->lcp);
	    sect.sct_mines--;
	    putsect(&sect);
	    putland(llp->land.lnd_uid, (s_char *)&llp->land);
	    if (!llp->land.lnd_own) {
		stopping = 1;
		emp_remque(qp);
		free((s_char *)qp);
	    }
	}
    }
    return stopping;
}

void
lnd_list(struct emp_qelem *land_list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct llist *llp;
    struct lndstr *lnd;
    int vec[I_MAX + 1];

    pr("lnd#     land type       x,y    a  eff  sh gun xl  mu tech retr fuel\n");

    for (qp = land_list->q_back; qp != land_list; qp = next) {
	next = qp->q_back;
	llp = (struct llist *)qp;
	lnd = &llp->land;
	pr("%4d ", lnd->lnd_uid);
	pr("%-16.16s ", llp->lcp->l_name);
	prxy("%4d,%-4d ", lnd->lnd_x, lnd->lnd_y, llp->land.lnd_own);
	pr("%1c", lnd->lnd_army);
	pr("%4d%%", lnd->lnd_effic);
	getvec(VT_ITEM, vec, (s_char *)lnd, EF_LAND);
	pr("%4d", vec[I_SHELL]);
	pr("%4d", vec[I_GUN]);
	count_land_planes(lnd);
	pr("%3d", lnd->lnd_nxlight);
	pr("%4d", lnd->lnd_mobil);
	pr("%4d", lnd->lnd_tech);
	pr("%4d%%", lnd->lnd_retreat);
	pr("%5d\n", lnd->lnd_fuel);
    }
}

void
lnd_mess(s_char *str, struct llist *llp)
{
    mpr(llp->land.lnd_own, "%s %s & stays in %s\n",
	prland(&llp->land),
	str, xyas(llp->land.lnd_x, llp->land.lnd_y, llp->land.lnd_own));
    if (llp->mobil < -127)
	llp->mobil = -127;
    llp->land.lnd_mobil = llp->mobil;
    putland(llp->land.lnd_uid, &llp->land);
    emp_remque((struct emp_qelem *)llp);
    free((s_char *)llp);
}

static int
lnd_count(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct llist *llp;
    int count = 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct llist *)qp;
	++count;
    }
    return count;
}

static int
lnd_damage(struct emp_qelem *list, int totdam)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct llist *llp;
    int dam;
    int count;

    if (!totdam || !(count = lnd_count(list)))
	return 0;
    dam = ldround(((double)totdam / (double)count), 1);
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct llist *)qp;
	/* have to get it again because of collateral damage */
	getland(llp->land.lnd_uid, &llp->land);
	landdamage(&llp->land, dam);
	putland(llp->land.lnd_uid, &llp->land);
	if (!llp->land.lnd_own) {
	    emp_remque(qp);
	    free((s_char *)qp);
	}
    }
    return dam;
}

static int
lnd_easiest_target(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct llist *llp;
    int hard;
    int easiest = 9876;		/* things start great for victim */
    int count = 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct llist *)qp;
	hard = lnd_hardtarget(&llp->land);
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
    int dam;
    struct emp_qelem msl_list, *qp, *newqp;

    msl_sel(&msl_list, newx, newy, victim, P_T, P_MAR, MI_INTERDICT);

    dam = msl_launch_mindam(&msl_list, newx, newy,
			    lnd_easiest_target(list), EF_LAND,
			    lnd_count(list) * 20, "troops", victim,
			    MI_INTERDICT);
    if (dam) {
	mpr(victim, "missile interdiction mission does %d damage!\n", dam);
	collateral_damage(newx, newy, dam, 0);
    }
    qp = msl_list.q_forw;
    while (qp != msl_list.q_forw) {
	newqp = qp->q_forw;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }
    return dam;
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
    int trange;
    double range, range2, guneff;
    int shell, gun;
    int dam;
    int totdam = 0;
    int i;

    snxtsct_dist(&ns, newx, newy, fort_max_interdiction_range);
    while (nxtsct(&ns, &fsect)) {
	if (fsect.sct_own == 0)
	    continue;
	if (fsect.sct_own == victim)
	    continue;
	if (fsect.sct_type != SCT_FORTR)
	    continue;
	if (getrel(getnatp(fsect.sct_own), victim) >= NEUTRAL)
	    continue;
	gun = fsect.sct_item[I_GUN];
	if (gun < 1)
	    continue;
	range = tfactfire(fsect.sct_own, (double)min(gun, 7));
	if (fsect.sct_effic > 59)
	    range++;
	range2 = roundrange(range);
	trange = mapdist(newx, newy, fsect.sct_x, fsect.sct_y);
	if (trange > range2)
	    continue;
	if (fsect.sct_item[I_MILIT] < 5)
	    continue;
	shell = fsect.sct_item[I_SHELL];
	if (shell < 1)
	    shell += supply_commod(fsect.sct_own,
				   fsect.sct_x, fsect.sct_y, I_SHELL, 1);
	if (shell < 1)
	    continue;
	shell--;
	fsect.sct_item[I_SHELL] = shell;
	putsect(&fsect);
	if (gun > 7)
	    gun = 7;
	guneff = landgun((int)fsect.sct_effic, gun);
	dam = (int)guneff;
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
	return lnd_damage(list, totdam);
    return 0;
}
#endif

int
lnd_interdict(struct emp_qelem *list, coord newx, coord newy, natid victim)
{
    int stopping = 0;

#if 0
    if (!opt_NO_FORT_FIRE)
/* Steve M. - commented out for now until abuse is decided upon */
	stopping |= lnd_fort_interdiction(list, newx, newy, victim);
#endif

    stopping |=
	lnd_damage(list,
		   unit_interdict(newx, newy, victim, "land units",
				  lnd_easiest_target(list), MI_INTERDICT));

    stopping |=
	lnd_damage(list,
		   lnd_missile_interdiction(list, newx, newy, victim));
    return stopping;
}

/* high value of hardtarget is harder to hit */
int
lnd_hardtarget(struct lndstr *lp)
{
    struct sctstr sect;

    getsect(lp->lnd_x, lp->lnd_y, &sect);
    return (int)(((double)lp->lnd_effic / 100.0) *
		 (10 + dchr[sect.sct_type].d_dstr * 2 +
		  (double)lp->lnd_spd / 2.0 - lp->lnd_vis));
}

int
lnd_hit_mine(struct lndstr *lp, struct lchrstr *lcp)
{
    double m;

    mpr(lp->lnd_own, "Blammo! Landmines detected in %s! ",
	xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));

    nreport(lp->lnd_own, N_LHIT_MINE, 0, 1);

    m = roll(20) + 10;
    if (lcp->l_flags & L_ENGINEER)
	m /= 2.0;

    landdamage(lp, ldround(m, 1));

    return (int)m;
}

double
lnd_mobcost(struct lndstr *lp, struct sctstr *sp, int mobtype)
{
    double mobcost;
    double smobcost;

    /* supply unit's speed depends on their eff, since
       that is their purpose */
    if (lchr[(int)lp->lnd_type].l_flags & L_SUPPLY)
	mobcost = lp->lnd_effic * 0.01 * lp->lnd_spd;
    else
	mobcost = lp->lnd_spd;
    if (mobcost < 0.01)
	mobcost = 0.01;

/* sector_mcost now takes 2 different arguments, a sector pointer, and
   whether or not to figure in the highway bonus, rail bonus or none.
   bridge heads, bridges and highways have built-in highways bonus
   because they are a 1, and this will discount that. */

    smobcost = (double)sector_mcost(sp, mobtype);
    if (smobcost < 0.01)
	smobcost = 0.01;

/* marching through 0 mobility conquered sectors takes lots of mobility,
   unless you are a train.  Capturing railways is a good thing. */

    if (sp->sct_own != sp->sct_oldown && sp->sct_mobil <= 0 &&
	smobcost < LND_MINMOBCOST && mobtype != MOB_RAIL)
	smobcost = LND_MINMOBCOST;

    mobcost = smobcost * 5.0 * 480.0 /
	(mobcost + techfact(lp->lnd_tech, mobcost));

    return mobcost;
}

int
lnd_mar_one_sector(struct emp_qelem *list, int dir, natid actor,
		   int together)
{
    struct sctstr sect, osect;
    struct emp_qelem *qp;
    struct emp_qelem *qp2;
    struct emp_qelem *next;
    struct llist *llp;
    coord dx;
    coord dy;
    coord newx;
    coord newy;
    int stopping = 0;
    int visible = 0;
    int stop;
    s_char dp[80];
    int rel;
    int oldown;

    if (dir <= DIR_STOP || dir >= DIR_VIEW) {
	lnd_put(list, actor);
	return 1;
    }
    dx = diroff[dir][0];
    dy = diroff[dir][1];
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	llp = (struct llist *)qp;
	getsect(llp->land.lnd_x, llp->land.lnd_y, &osect);
	oldown = osect.sct_own;
	newx = xnorm(llp->land.lnd_x + dx);
	newy = ynorm(llp->land.lnd_y + dy);
	getsect(newx, newy, &sect);
	rel = getrel(getnatp(sect.sct_own), player->cnum);
	if ((sect.sct_own != actor && rel != ALLIED &&
	     !(lchr[(int)llp->land.lnd_type].l_flags & L_SPY) &&
	     sect.sct_own) || (sect.sct_type == SCT_WATER ||
			       sect.sct_type == SCT_SANCT ||
			       sect.sct_type == SCT_WASTE)) {
	    if (together) {
		pr("can't go to %s\n", xyas(newx, newy, actor));
		return 1;
	    } else {
		sprintf(dp, "can't go to %s", xyas(newx, newy, actor));
		lnd_mess(dp, llp);
		continue;
	    }
	}

	if (!(lchr[(int)llp->land.lnd_type].l_flags & L_SPY))
	    visible = 1;
	if (sect.sct_rail == 0 &&
	    lchr[(int)llp->land.lnd_type].l_flags & L_TRAIN) {
	    if (together) {
		pr("no rail system in %s\n", xyas(newx, newy, actor));
		return 1;
	    } else {
		sprintf(dp, "has no rail system in %s",
			xyas(newx, newy, actor));
		lnd_mess(dp, llp);
		continue;
	    }
	}
	/* Note we check would_abandon first because we don't want
	   to always have to do these checks */
	if (would_abandon(&osect, V_CIVIL, 0, &(llp->land))) {
	    stop = 0;
	    if (!want_to_abandon(&osect, V_CIVIL, 0, &(llp->land))) {
		stop = 1;
	    }
	    /* now check stuff */
	    if (!check_sect_ok(&sect))
		return 1;
	    if (!check_sect_ok(&osect))
		return 1;
	    for (qp2 = list->q_back; qp2 != list; qp2 = qp2->q_back) {
		if (!check_land_ok(&(((struct llist *)qp2)->land)))
		    return 1;
	    }
	    if (stop) {
		lnd_mess("stops", llp);
		continue;
	    }
	}
	if (llp->mobil <= 0.0) {
	    lnd_mess("is out of mobility", llp);
	    continue;
	}
	llp->land.lnd_x = newx;
	llp->land.lnd_y = newy;
	if (lchr[(int)llp->land.lnd_type].l_flags & L_TRAIN) {
	    llp->mobil -= lnd_mobcost(&llp->land, &sect, MOB_RAIL);
	} else {
	    llp->mobil -= lnd_mobcost(&llp->land, &sect, MOB_ROAD);
	}
	llp->land.lnd_mobil = (int)llp->mobil;
	llp->land.lnd_harden = 0;
	putland(llp->land.lnd_uid, &llp->land);
	putsect(&osect);
	getsect(osect.sct_x, osect.sct_y, &osect);
	if (osect.sct_own != oldown && oldown == player->cnum) {
	    /* It was your sector, now it's not.  Simple :) */
	    pr("You no longer own %s\n",
	       xyas(osect.sct_x, osect.sct_y, player->cnum));
	}
	if (rel != ALLIED && sect.sct_own != actor && sect.sct_own) {	/* must be a spy */
	    /* Always a 10% chance of getting caught. */
	    if (chance(LND_SPY_DETECT_CHANCE(llp->land.lnd_effic))) {
		if (rel == NEUTRAL || rel == FRIENDLY) {
		    wu(0, sect.sct_own,
		       "%s unit spotted in %s\n", cname(player->cnum),
		       xyas(sect.sct_x, sect.sct_y, sect.sct_own));
		    setrel(sect.sct_own, llp->land.lnd_own, HOSTILE);
		} else if (rel == HOSTILE || rel == AT_WAR ||
			   rel == SITZKRIEG || rel == MOBILIZATION) {
		    wu(0, sect.sct_own,
		       "%s spy shot in %s\n", cname(player->cnum),
		       xyas(sect.sct_x, sect.sct_y, sect.sct_own));
		    pr("%s was shot and killed.\n", prland(&llp->land));
		    llp->land.lnd_effic = 0;
		    putland(llp->land.lnd_uid, &llp->land);
		    lnd_delete(llp, 0);
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
    if (visible)
	stopping |= lnd_interdict(list, newx, newy, actor);
    return stopping;
}

/*
 * find all artillery units belonging
 * to the attacker or defender that can fire.
 * Each arty unit adds +1%/damage point
 *
 */
int
lnd_support(natid victim, natid attacker, coord x, coord y)
{
    struct nstr_item ni;
    struct lndstr land;
    int rel, rel2;
    double dam = 0.0;
    int dist;
    int shell;
    int gun;
    double range, range2;

    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, (s_char *)&land)) {
	if (land.lnd_frg == 0)
	    continue;
	if ((land.lnd_x == x) && (land.lnd_y == y))
	    continue;
	if (land.lnd_ship >= 0)
	    continue;
	if (land.lnd_land >= 0)
	    continue;
	if (land.lnd_mission > 0)
	    continue;
	if (land.lnd_effic < LAND_MINFIREEFF)
	    continue;
	/* Do we have mil? */
	if (land.lnd_item[I_MILIT] <= 0)
	    continue;
	rel = getrel(getnatp(land.lnd_own), attacker);
	rel2 = getrel(getnatp(land.lnd_own), victim);
	if ((land.lnd_own != attacker) &&
	    ((rel != ALLIED) || (rel2 != AT_WAR)))
	    continue;

	/* do we have supplies? */
	if (!has_supply(&land))
	    continue;

	/* are we in range? */
	dist = mapdist(land.lnd_x, land.lnd_y, x, y);

	range = techfact((int)land.lnd_tech, (double)land.lnd_frg / 2.0);

	range2 = roundrange(range);
	if (dist > range2)
	    continue;

	shell = land.lnd_item[I_SHELL];
	gun = land.lnd_item[I_GUN];

	if (shell == 0 || gun == 0)
	    continue;

	use_supply(&land);
	nreport(land.lnd_own, N_FIRE_L_ATTACK, victim, 1);
	if (roll(100) < land.lnd_acc) {
	    dam += landunitgun(land.lnd_effic, land.lnd_dam, gun,
			       land.lnd_ammo, shell) / 2;
	} else {
	    dam += landunitgun(land.lnd_effic, land.lnd_dam, gun,
			       land.lnd_ammo, shell);
	}
	if (land.lnd_own != attacker)
	    wu(0, land.lnd_own,
	       "%s supported %s at %s\n",
	       prland(&land), cname(attacker), xyas(x, y, land.lnd_own));
    }
    return (int)dam;
}

s_char *
lnd_path(int together, struct lndstr *lp, s_char *buf)
{
    coord destx;
    coord desty;
    struct sctstr d_sect, sect;
    s_char *cp;
    double dummy;

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
    getsect(lp->lnd_x, lp->lnd_y, &sect);
    if (lchr[(int)lp->lnd_type].l_flags & L_TRAIN)
	cp = (s_char *)BestLandPath(buf, &sect, &d_sect, &dummy, MOB_RAIL);
    else
	cp = (s_char *)BestLandPath(buf, &sect, &d_sect, &dummy, MOB_ROAD);
    if (!cp) {
	pr("No owned path from %s to %s!\n",
	   xyas(lp->lnd_x, lp->lnd_y, player->cnum),
	   xyas(d_sect.sct_x, d_sect.sct_y, player->cnum));
	return 0;
    }
    pr("Using path '%s'\n", cp);
    return cp;
}

int
lnd_can_attack(struct lndstr *lp)
{
    struct lchrstr *lcp = &lchr[(int)lp->lnd_type];

/*	if (lcp->l_flags & L_SUPPLY ||
	    lcp->l_flags & L_SECURITY ||
	    lcp->l_flags & L_FLAK ||
	    lcp->l_frg)
		return 0; */
    if (lcp->l_flags & L_SUPPLY)
	return 0;

    return 1;
}

/*
 * Increase fortification value of LP.
 * Fortification costs mobility.  Use up to HARD_AMT mobility.
 * Return actual fortification increase.
 */
int
lnd_fortify (struct lndstr *lp, int hard_amt)
{
    int mob_used;
    int eng;

    if ((lp->lnd_ship >= 0) || lp->lnd_land >= 0)
	return 0;

    hard_amt = min(lp->lnd_mobil, hard_amt);

    if ((lp->lnd_harden + hard_amt) > land_mob_max)
	hard_amt = land_mob_max - lp->lnd_harden;

    eng = has_helpful_engineer(lp->lnd_x, lp->lnd_y, lp->lnd_own);

    if (eng)
	hard_amt = ((float)hard_amt * 1.5);

    if ((lp->lnd_harden + hard_amt) > land_mob_max)
	hard_amt = land_mob_max - lp->lnd_harden;

    /* Ok, set the mobility used */
    mob_used = hard_amt;

    /* Now, if an engineer helped, it's really only 2/3rds of
       that */
    if (eng)
	mob_used = (int)((float)mob_used / 1.5);

    /* If we increased it, but not much, we gotta take at least 1
       mob point. */
    if (mob_used <= 0 && hard_amt > 0)
	mob_used = 1;

    lp->lnd_mobil -= mob_used;
    if (lp->lnd_mobil < 0)
	lp->lnd_mobil = 0;

    lp->lnd_harden += hard_amt;
    lp->lnd_harden = min(lp->lnd_harden, land_mob_max);

    return hard_amt;
}
