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
 *  plague.c: Plague related functions
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "chance.h"
#include "item.h"
#include "optlist.h"
#include "nat.h"
#include "news.h"
#include "plague.h"
#include "prototypes.h"
#include "sect.h"
#include "update.h"

static int infect_people(struct natstr *, struct sctstr *);

void
do_plague(struct sctstr *sp, int etu)
{
    struct natstr *np = getnatp(sp->sct_own);
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
	if (n != PLG_HEALTHY)
	    plague_report(sp->sct_own, n, pstage, ptime, etu,
			  "in", ownxy(sp));
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
 * people if in plague state PLG_DYING.  Increment
 * the plague time.  Return "current" plague
 * stage.  No reports generated here anymore.
 */
int
plague_people(struct natstr *np, short *vec,
	      int *pstage, int *ptime, int etus)
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
	pct_left = 1.0 - plg_num / plg_denom;
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
	*ptime = etus / 2 + roll0(etus);
    }
    return stage;
}

void
plague_report(natid victim, int new_pstage, int pstage, int ptime,
	      int etus, char *in_on, char *place)
{
    switch (new_pstage) {
    case PLG_DYING:
	wu(0, victim, "PLAGUE deaths reported %s %s\n", in_on, place);
	nreport(victim, N_DIE_PLAGUE, 0, 1);
	break;
    case PLG_INFECT:
	wu(0, victim, "%s battling PLAGUE\n", place);
	break;
    case PLG_INCUBATE:
	/* Are we still incubating? */
	if (new_pstage == pstage) {
	    /* Yes. Will it turn "infectious" next time? */
	    if (ptime <= etus) {
		/* Yes.  Report an outbreak. */
		wu(0, victim,
		   "Outbreak of PLAGUE %s %s!\n", in_on, place);
		nreport(victim, N_OUT_PLAGUE, 0, 1);
	    }
	} else {
	    /* It has already moved on to "infectious" */
	    wu(0, victim, "%s battling PLAGUE\n", place);
	}
	break;
    case PLG_EXPOSED:
	/* Has the plague moved to "incubation" yet? */
	if (new_pstage != pstage) {
	    /* Yes. Will it turn "infectious" next time? */
	    if (ptime <= etus) {
		/* Yes.  Report an outbreak. */
		wu(0, victim,
		   "Outbreak of PLAGUE %s %s!\n", in_on, place);
		nreport(victim, N_OUT_PLAGUE, 0, 1);
	    }
	}
	break;
    default:
	break;
    }
}
