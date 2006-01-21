/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plague.c: Plague related functions
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998-2000
 */

#include <config.h>

#include "misc.h"
#include "plague.h"
#include "sect.h"
#include "nat.h"
#include "item.h"
#include "news.h"
#include "file.h"
#include "xy.h"
#include "optlist.h"
#include "update.h"
#include "common.h"
#include "subs.h"
#include "lost.h"
#include "gen.h"

static int infect_people(struct natstr *, struct sctstr *);

void
do_plague(struct sctstr *sp, struct natstr *np, int etu)
{
    int pstage, ptime;
    int n;

    if (opt_NO_PLAGUE)		/* no plague nothing to do */
	return;

    pstage = sp->sct_pstage;
    ptime = sp->sct_ptime;

    if (pstage == PLG_HEALTHY) {
	pstage = infect_people(np, sp);
	ptime = 0;
    } else {
	n = plague_people(np, sp->sct_item, &pstage, &ptime, etu);
	switch (n) {
	case PLG_DYING:
	    wu(0, sp->sct_own, "PLAGUE deaths reported in %s.\n",
	       ownxy(sp));
	    nreport(sp->sct_own, N_DIE_PLAGUE, 0, 1);
	    break;
	case PLG_INFECT:
	    wu(0, sp->sct_own, "%s battling PLAGUE\n", ownxy(sp));
	    break;
	case PLG_INCUBATE:
	    /* Are we still incubating? */
	    if (n == pstage) {
		/* Yes. Will it turn "infectious" next time? */
		if (ptime <= etu) {
		    /* Yes.  Report an outbreak. */
		    wu(0, sp->sct_own,
		       "Outbreak of PLAGUE in %s!\n", ownxy(sp));
		    nreport(sp->sct_own, N_OUT_PLAGUE, 0, 1);
		}
	    } else {
		/* It has already moved on to "infectious" */
		wu(0, sp->sct_own, "%s battling PLAGUE\n", ownxy(sp));
	    }
	    break;
	case PLG_EXPOSED:
	    /* Has the plague moved to "incubation" yet? */
	    if (n != pstage) {
		/* Yes. Will it turn "infectious" next time? */
		if (ptime <= etu) {
		    /* Yes.  Report an outbreak. */
		    wu(0, sp->sct_own,
		       "Outbreak of PLAGUE in %s!\n", ownxy(sp));
		    nreport(sp->sct_own, N_OUT_PLAGUE, 0, 1);
		}
	    }
	    break;
	default:
	    break;
	}
    }
    if (sp->sct_item[I_CIVIL] == 0 && sp->sct_item[I_MILIT] == 0
	&& !has_units(sp->sct_x, sp->sct_y, sp->sct_own, 0)) {
	makelost(EF_SECTOR, sp->sct_own, 0, sp->sct_x, sp->sct_y);
	sp->sct_own = 0;
	sp->sct_oldown = 0;
    }
    sp->sct_pstage = pstage;
    sp->sct_ptime = ptime;
}

/*ARGSUSED*/
static int
infect_people(struct natstr *np, struct sctstr *sp)
{
    double pop, pop_space, bad_stuff, pollution, cleanup;
    double plg_chance;

    if (opt_NO_PLAGUE)		/* no plague nothing to do */
	return PLG_HEALTHY;

    if (np->nat_level[NAT_TLEV] <= 10.0)
	return PLG_HEALTHY;

    /*
     * make plague where there was none before...
     */
    pop = sp->sct_item[I_CIVIL] + sp->sct_item[I_MILIT] + sp->sct_item[I_UW];
    pop_space = max_pop(np->nat_level[NAT_RLEV], sp);
    bad_stuff
	= sp->sct_item[I_IRON] + sp->sct_item[I_OIL] + sp->sct_item[I_RAD] * 2;
    pollution = bad_stuff / 10.0 + np->nat_level[NAT_TLEV] + 100.0;
    cleanup = sp->sct_effic + sp->sct_mobil + 100 + np->nat_level[NAT_RLEV];
    plg_chance = ((pop / pop_space) * (pollution / cleanup) - 1.0) * 0.01;
    if (chance(plg_chance))
	return PLG_EXPOSED;
    return PLG_HEALTHY;
}

/*
 * Given the fact that plague exists, kill off
 * people if in plague state DYING.  Increment
 * the plague time.  Return "current" plague
 * stage.  No reports generated here anymore.
 */
int
plague_people(struct natstr *np, short *vec,
	      int *pstage, int *ptime,
	      int etus)
{
    int stage;
    double plg_num;
    double plg_denom;
    double pct_left;

    if (opt_NO_PLAGUE)		/* no plague nothing to do */
	return PLG_HEALTHY;
    *ptime -= etus;
    stage = *pstage;
    switch (stage) {
    case PLG_DYING:
	plg_num = 100.0 * etus;
	plg_denom = (np->nat_level[NAT_RLEV] + 100.0) *
	    (*ptime + etus + 1.0);
	pct_left = 1.0 - (double)(plg_num / plg_denom);
	if (pct_left < 0.2)
	    pct_left = 0.2;
	vec[I_CIVIL] = vec[I_CIVIL] * pct_left;
	vec[I_MILIT] = vec[I_MILIT] * pct_left;
	vec[I_UW] = vec[I_UW] * pct_left;
	break;
    case PLG_INFECT:
    case PLG_INCUBATE:
	break;
    case PLG_EXPOSED:
	*ptime = 0;
	break;
    default:
	/* bad */
	logerror("plague_people: bad pstage %d", stage);
	*pstage = PLG_HEALTHY;
	*ptime = 0;
	return PLG_HEALTHY;
    }
    if (*ptime <= 0) {
	*pstage -= 1;
	*ptime = (etus / 2) + (random() % etus);
    }
    return stage;
}
