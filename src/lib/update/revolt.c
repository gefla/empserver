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
 *  revolt.c: Have disloyal populace revolt!
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1997-2000
 *     Markus Armbruster, 2004-2012
 */

#include <config.h>

#include "chance.h"
#include "land.h"
#include "lost.h"
#include "news.h"
#include "nsc.h"
#include "nuke.h"
#include "path.h"
#include "plane.h"
#include "update.h"

static void take_casualties(struct sctstr *, int);
static void lnd_dies_fighting_che(struct lndstr *);

void
revolt(struct sctstr *sp)
{
    int che_civ;
    int che_uw;
    int civ;
    int uw;
    int che;
    int n;

    che = sp->sct_che;
    if (che != 0 && (sp->sct_che_target != sp->sct_own || che >= CHE_MAX))
	return;
    civ = sp->sct_item[I_CIVIL];
    uw = sp->sct_item[I_UW];
    if (che > (civ + uw) * 3)
	return;
    che_uw = 0;
    /* che due to civilian unrest */
    n = 10 - roll0(20);
    che_civ = 3 + (civ * n / 500);
    if (che_civ < 0)
	che_civ = 0;
    else if (che_civ * 3 > civ)
	che_civ = civ / 3;
    if (che + che_civ > CHE_MAX)
	che_civ = CHE_MAX - che;
    che += che_civ;
    if (che < CHE_MAX) {
	/* che due to uw unrest */
	n = 9 + roll(30);
	che_uw = 5 + (uw * n / 500);
	if (che_uw > uw)
	    che_uw = uw;
	if (che + che_uw > CHE_MAX)
	    che_uw = CHE_MAX - che_uw;
	che += che_uw;
    }
    if (che_civ + che_uw > 0) {
	civ -= che_civ;
	uw -= che_uw;
	sp->sct_che_target = sp->sct_own;
	sp->sct_che = che;
	if (che_civ > 0)
	    sp->sct_item[I_CIVIL] = civ;
	if (che_uw > 0)
	    sp->sct_item[I_UW] = uw;
    }
}

/*
 * summary of effects.
 * if there are no military in the sector, che recruit from
 *   populace if pop loyalty is > 10.  They spread subversion otherwise,
 *   trying to lower pop loyalty.
 * if che outnumber military, they stay and shoot it out, kill the
 *   military.
 * if che are outnumbered by less than 5 to 1, they blow up stuff,
 *   killing innocent civilians (never uw's) and damaging commodities.
 * if che are outnumbered by more than 5 to 1, they try to leave the
 *   sector for a nearby sector with fewer military.
 *
 * if the military lose any attacks, the pop loyalty in the sector
 *   gets worse, representing military defeat.
 * military can "catch" che's after bombing attacks, or after they move.
 *   If military catch them, then they get to shoot it out with a portion
 *   of the che's depending on the # of mil in the sector.  Chance to contact
 *   is around 10% per every equal number of mil:che ratio in the sector.
 *   "contact" is by 20% of the military in the sector, and odds are equal.
 *
 * Without a doubt this routine should be broken up, if only for readabilty.
 */
void
guerrilla(struct sctstr *sp)
{
    struct sctstr *nsp;
    int recruit;
    int move;
    int ratio;
    int che;
    int mil;
    int cc, mc;
    double odds;
    int civ;
    int n;
    int uw;
    natid target;
    struct natstr *tnat;
    int convert;
    natid actor;
    natid victim;
    int tmp;
    int min_mil;
    int val;
    int oldmob;
    struct lndstr *lp;
    struct nstr_item ni;

    mc = cc = 0;
    recruit = 0;
    convert = 0;
    move = 0;
    if (!sp->sct_che)
	return;
    civ = sp->sct_item[I_CIVIL];
    uw = sp->sct_item[I_UW];
    victim = sp->sct_own;
    actor = sp->sct_oldown;
    che = sp->sct_che;
    mil = sp->sct_item[I_MILIT];

    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);

    while (NULL != (lp = nxtitemp(&ni))) {
	if (lp->lnd_own != sp->sct_own)
	    continue;
	if (lp->lnd_ship >= 0)
	    continue;

	mil += lp->lnd_item[I_MILIT];

	if (sp->sct_che_target != sp->sct_own)
	    continue;

	/* Security troops can now kill up to 1/5 their complement each
	   update, before doing anything else. */
	if (lchr[(int)lp->lnd_type].l_flags & L_SECURITY) {
	    int che_kill, r;

	    mil += lp->lnd_item[I_MILIT];
	    r = (lp->lnd_item[I_MILIT] * lp->lnd_effic) / 500;
	    che_kill = r < 1 ? 0 : roll(r);
	    if (che_kill > che)
		che_kill = che;
	    if (che_kill) {
		wu(0, sp->sct_own,
		   "%s kills %d guerrilla%s in raid at %s!\n",
		   prland(lp), che_kill, splur(che_kill), ownxy(sp));
		che -= che_kill;
	    }
	}
    }

    /* Security forces killed all the che */
    if (che <= 0) {
	sp->sct_che = 0;
	sp->sct_che_target = 0;
	return;
    }

    target = sp->sct_che_target;
    if (CANT_HAPPEN(target == 0))
	return;
    tnat = getnatp(target);
    if (tnat->nat_stat == STAT_UNUSED) {
	/* target nation has dissolved: che's retire.  */
	logerror("%d Che targeted at country %d retiring", che, target);
	sp->sct_che = 0;
	sp->sct_che_target = 0;
	sp->sct_item[I_CIVIL] = MIN(civ + che, ITEM_MAX);
	return;
    }

    if (sp->sct_own != target) {
	move++;
	goto domove;
    }

    ratio = mil / che;
    odds = (double)che / (mil + che);
    odds /= hap_fact(tnat, getnatp(sp->sct_oldown));
    if (mil == 0) {
	wu(0, sp->sct_own, "Revolutionary subversion reported in %s!\n",
	   ownxy(sp));
	recruit++;
	convert++;
    } else if (che > mil && mil > 0) {
	/*
	 * shoot it out with the military, and kill them off.
	 * If loyalty bad enough, then take the sector over,
	 * and enlist 5% of civ as military force.
	 */
	while (che > 0 && mil > 0) {
	    if (chance(odds)) {
		mc++;
		mil--;
	    } else {
		cc++;
		che--;
	    }
	}
	if (mil > 0) {
	    /* military won.  */
	    n = sp->sct_loyal - roll0(15);
	    if (n < 0)
		n = 0;
	    sp->sct_loyal = n;
	} else {
	    convert++;
	    recruit++;
	}
	take_casualties(sp, mc);
    } else if (ratio < 5) {
	/*
	 * guerrillas have to resort to blowing things up.
	 * Note this disrupts work in the sector.
	 */
	n = roll0(10) + roll0(che);
	if (n > 100)
	    n = 100;
	tmp = sp->sct_work - n;
	if (tmp < 0)
	    tmp = 0;
	sp->sct_work = tmp;
	wu(0, sp->sct_own,
	   "Production %s disrupted by terrorists in %s\n",
	   effadv(n), ownxy(sp));
	sect_damage(sp, n / 10);
	recruit++;
    } else {
	/* ratio >= 5 */
	move++;
    }
    if (mil > 0 && che > 0) {
	/*
	 * we only get here if we haven't had combat previously.
	 * Chance to catch them.
	 * 20% of mil involved in attacking the che's.
	 */
	if (chance(ratio * 0.10)) {
	    n = (mil / 5) + 1;
	    odds = (double)che / (n + che);
	    odds /= hap_fact(tnat, getnatp(sp->sct_oldown));
	    while (che > 0 && n > 0) {
		if (chance(odds)) {
		    mc++;
		    n--;
		} else {
		    cc++;
		    che--;
		}
	    }
	    take_casualties(sp, mc);
	    recruit = 0;
	}
    }
    if (convert && sp->sct_loyal >= 50) {
	int n;
	/* new owner gets to keep the mobility there */
	oldmob = sp->sct_mobil;
	/* che won, and sector converts. */
	if (sp->sct_own == sp->sct_oldown)
	    sp->sct_oldown = 0;
	else {
	    lost_and_found(EF_SECTOR, sp->sct_own, sp->sct_oldown,
			   0, sp->sct_x, sp->sct_y);
	    takeover(sp, sp->sct_oldown);
	}
	sp->sct_mobil = oldmob;
	civ += uw;
	uw = 0;
	n = civ / 20;
	civ -= n;
	if (civ > ITEM_MAX) {
	    uw = civ - ITEM_MAX;
	    civ = ITEM_MAX;
	}
	sp->sct_item[I_CIVIL] = civ;
	sp->sct_item[I_UW] = uw;
	sp->sct_item[I_MILIT] = n;
	move++;
	recruit = 0;
	if (sp->sct_own)
	    wu(0, sp->sct_own, "Sector %s has been retaken!\n",
	       xyas(sp->sct_x, sp->sct_y, sp->sct_own));
    }
    if (recruit && che > 0) {
	/* loyalty drops during recruitment efforts */
	n = sp->sct_loyal;
	if (n < 30)
	    n += roll(5);
	else if (n < 70)
	    n += roll(10) + 3;
	if (n > 127)
	    n = 127;
	sp->sct_loyal = n;
	if (sp->sct_oldown != sp->sct_own || n > 100) {
	    n = civ * roll0(3) / 200;
	    n /= hap_fact(tnat, getnatp(sp->sct_oldown));
	    if (n + che > CHE_MAX)
		n = CHE_MAX - che;
	    che += n;
	    civ -= n;
	    sp->sct_item[I_CIVIL] = civ;
	}
	n = uw * roll0(3) / 200;
	if (n + che > CHE_MAX)
	    n = CHE_MAX - che;
	che += n;
	uw -= n;
	sp->sct_item[I_UW] = uw;
    }
domove:
    if (move && che > 0) {
	struct sctstr *nicest_sp = NULL;
	if (convert)
	    min_mil = 999;
	else
	    min_mil = mil;
	/* search adjacent sectors for a nice one */
	for (n = 1; n <= 6; n++) {
	    nsp = getsectp(sp->sct_x + diroff[n][0],
			   sp->sct_y + diroff[n][1]);
	    if (dchr[nsp->sct_type].d_mob0 < 0)
		continue;
	    if (nsp->sct_own != target)
		continue;
	    if (nsp->sct_che > 0) {
		if (nsp->sct_che_target != target)
		    continue;
		if (nsp->sct_che + che > CHE_MAX)
		    continue;
	    }
	    val = nsp->sct_item[I_MILIT];
	    /* don't give che more precise info than spy */
	    val = roundintby(val, 10);
	    /* inject a modicum of indeterminism; also
	     * avoids che preferring certain directions */
	    val += roll(10) - 6;
	    if (val >= min_mil)
		continue;
	    nicest_sp = nsp;
	    min_mil = val;
	}
	/* if we found a nice sector, go there */
	if (nicest_sp) {
	    nicest_sp->sct_che += che;
	    nicest_sp->sct_che_target = target;
	    che = 0;
	}
    }
    if (che > 0) {
	sp->sct_che = che;
	sp->sct_che_target = target;
    } else {
	sp->sct_che = 0;
	sp->sct_che_target = 0;
    }
    if (mc > 0 || cc > 0) {
	wu(0, target,
	   "Guerrilla warfare in %s\n",
	   xyas(sp->sct_x, sp->sct_y, target));
	if (sp->sct_own == target)
	    wu(0, target, "  body count: troops: %d, rebels: %d\n", mc, cc);
	else
	    wu(0, target, "  rebels murder %d military\n", mc);
	nreport(actor, N_FREEDOM_FIGHT, victim, 1);
    }
    if (sp->sct_own != victim)
	wu(0, victim, "Partisans take over %s!\n",
	   xyas(sp->sct_x, sp->sct_y, victim));
}

static void
take_casualties(struct sctstr *sp, int mc)
{
    int orig_mil;
    int cantake;
    int nunits = 0, each, deq;
    struct lndstr *lp;
    struct nstr_item ni;

    /* casualties come out of mil first */
    orig_mil = sp->sct_item[I_MILIT];

    if (mc <= orig_mil) {
	sp->sct_item[I_MILIT] = orig_mil - mc;
	return;
    }
    sp->sct_item[I_MILIT] = 0;

    /* remaining casualites */
    mc -= orig_mil;

    /*
     * Need to take total_casualties and divide
     * them amongst the land units in the sector
     * Do security troops first, then others.
     * Try not to kill any unit.
     */
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (NULL != (lp = nxtitemp(&ni))) {
	if (lp->lnd_own != sp->sct_own)
	    continue;
	if (lp->lnd_ship >= 0)
	    continue;
	nunits++;
	if (lchr[(int)lp->lnd_type].l_flags & L_SECURITY)
	    nunits++;
    }

    if (nunits == 0)
	return;

    each = (mc / nunits) + 2;

    /* kill some security troops */
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (NULL != (lp = nxtitemp(&ni))) {
	if (lp->lnd_own != sp->sct_own)
	    continue;
	if (lp->lnd_ship >= 0)
	    continue;
	if (!(lchr[(int)lp->lnd_type].l_flags & L_SECURITY))
	    continue;

	cantake = ((lp->lnd_effic - 40) / 100.0) * lp->lnd_item[I_MILIT];

	if (cantake >= each) {
	    deq = ((double)each / lp->lnd_item[I_MILIT]) * 100.0;
	    mc -= 2 * each;
	} else if (cantake > 0) {
	    deq = ((double)cantake / lp->lnd_item[I_MILIT]) * 100.0;
	    mc -= 2 * cantake;
	} else
	    deq = 0;

	lp->lnd_effic -= deq;
	lp->lnd_mobil -= deq / 2;
	deq = lchr[(int)lp->lnd_type].l_item[I_MILIT] * (deq / 100.0);
	lnd_submil(lp, deq);
	if (mc <= 0)
	    return;
    }

    /* kill some normal troops */
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (NULL != (lp = nxtitemp(&ni))) {
	if (lp->lnd_own != sp->sct_own)
	    continue;
	if (lp->lnd_ship >= 0)
	    continue;
	if (lchr[(int)lp->lnd_type].l_flags & L_SECURITY)
	    continue;

	cantake = ((lp->lnd_effic - 40) / 100.0) * lp->lnd_item[I_MILIT];

	if (cantake >= each) {
	    deq = ((double)each / lp->lnd_item[I_MILIT]) * 100.0;
	    mc -= each;
	} else if (cantake > 0) {
	    deq = ((double)cantake / lp->lnd_item[I_MILIT]) * 100.0;
	    mc -= cantake;
	} else
	    deq = 0;

	lp->lnd_effic -= deq;
	lp->lnd_mobil -= deq / 2;
	deq = lchr[(int)lp->lnd_type].l_item[I_MILIT] * (deq / 100.0);
	lnd_submil(lp, deq);
	if (mc <= 0)
	    return;
    }

    /* Hmm.. still some left.. kill off units now */
    /* kill some normal troops */
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (NULL != (lp = nxtitemp(&ni))) {
	if (lp->lnd_own != sp->sct_own)
	    continue;
	if (lp->lnd_ship >= 0)
	    continue;
	if (lchr[(int)lp->lnd_type].l_flags & L_SECURITY)
	    continue;

	mc -= (lp->lnd_effic / 100.0) * lp->lnd_item[I_MILIT];
	lnd_dies_fighting_che(lp);
	if (mc <= 0)
	    return;
    }

    /* Hmm.. still some left.. kill off units now */
    /* kill some security troops */
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (NULL != (lp = nxtitemp(&ni))) {
	if (lp->lnd_own != sp->sct_own)
	    continue;
	if (lp->lnd_ship >= 0)
	    continue;
	if (!(lchr[(int)lp->lnd_type].l_flags & L_SECURITY))
	    continue;

	mc -= (lp->lnd_effic / 100.0) * lp->lnd_item[I_MILIT] * 2.0;
	lnd_dies_fighting_che(lp);
	if (mc <= 0)
	    return;
    }

    /* Hmm.. everyone dead.. too bad */
}

static void
lnd_dies_fighting_che(struct lndstr *lp)
{
    int i, j;
    struct lndstr *clp;
    struct plnstr *cpp;
    struct nukstr *cnp;

    lp->lnd_effic = 0;
    lnd_submil(lp, 1000);	/* Remove 'em all */
    wu(0, lp->lnd_own, "%s dies fighting guerrillas in %s\n",
       prland(lp), xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));
    makelost(EF_LAND, lp->lnd_own, lp->lnd_uid, lp->lnd_x, lp->lnd_y);
    lp->lnd_own = 0;

    /* Take dead lp off its carrier */
    if (lp->lnd_land >= 0) {
	lnd_carrier_change(lp, EF_LAND, lp->lnd_land, -1);
	lp->lnd_land = -1;
    }

    /* Unload lp's land unit cargo */
    for (i = lnd_first_on_land(lp); i >= 0; i = lnd_next_on_unit(i)) {
	clp = getlandp(i);
	if (CANT_HAPPEN(!clp))
	    continue;
	lnd_carrier_change(clp, EF_LAND, clp->lnd_land, -1);
	clp->lnd_land = -1;
    }

    /* Destroy lp's plane cargo */
    for (i = pln_first_on_land(lp); i >= 0; i = pln_next_on_unit(i)) {
	cpp = getplanep(i);
	if (CANT_HAPPEN(!cpp))
	    continue;
	pln_carrier_change(cpp, EF_LAND, cpp->pln_land, -1);
	makelost(EF_PLANE, cpp->pln_own, i, cpp->pln_x, cpp->pln_y);
	wu(0, cpp->pln_own, "%s lost!\n", prplane(cpp));
	cpp->pln_own = 0;
	cpp->pln_effic = 0;
	cpp->pln_land = -1;

	j = nuk_on_plane(cpp);
	if (j >= 0) {
	    cnp = getnukep(j);
	    if (CANT_HAPPEN(!cnp))
		continue;
	    nuk_carrier_change(cnp, EF_PLANE, cnp->nuk_plane, -1);
	    makelost(EF_NUKE, cnp->nuk_own, j, cnp->nuk_x, cnp->nuk_y);
	    wu(0, cnp->nuk_own, "%s lost!\n", prnuke(cnp));
	    cnp->nuk_own = 0;
	    cnp->nuk_effic = 0;
	    cnp->nuk_plane = -1;
	}
    }
}
