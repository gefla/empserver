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
 *  plague.c: Plague related functions
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998-2000
 */

#include <math.h>
#include "misc.h"
#include "var.h"
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

static int infect_people(struct natstr *, register int *, u_int, int,
			 struct sctstr *);

void
do_plague(struct sctstr *sp, struct natstr *np, int etu)
{
    int vec[I_MAX + 1];
    int cvec[I_MAX + 1];
    int n;

    if (opt_NO_PLAGUE)		/* no plague nothing to do */
	return;

    if (getvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR) <= 0)
	return;
    if (getvec(VT_COND, cvec, (s_char *)sp, EF_SECTOR) <= 0)
	memset(cvec, 0, sizeof(cvec));

    if (cvec[C_PSTAGE] == 0) {
	cvec[C_PSTAGE] = infect_people(np, vec, sp->sct_effic,
				       (int)sp->sct_mobil, sp);
	cvec[C_PTIME] = 0;
    } else {
	n = plague_people(np, vec, cvec, etu);
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
	    if (n == cvec[C_PSTAGE]) {
		/* Yes. Will it turn "infectious" next time? */
		if (cvec[C_PTIME] <= etu) {
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
	    if (n != cvec[C_PSTAGE]) {
		/* Yes. Will it turn "infectious" next time? */
		if (cvec[C_PTIME] <= etu) {
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
    if (vec[I_CIVIL] == 0 && vec[I_MILIT] == 0 &&
	!has_units(sp->sct_x, sp->sct_y, sp->sct_own, 0)) {
	makelost(EF_SECTOR, sp->sct_own, 0, sp->sct_x, sp->sct_y);
	sp->sct_own = 0;
	sp->sct_oldown = 0;
    }
    putvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR);
    putvec(VT_COND, cvec, (s_char *)sp, EF_SECTOR);
}

/*ARGSUSED*/
static int
infect_people(struct natstr *np, register int *vec, u_int eff, int mobil,
	      struct sctstr *sp)
{
    double plg_num;
    double plg_denom;
    double plg_chance;
    double civvies = 999.0;

    if (opt_NO_PLAGUE)		/* no plague nothing to do */
	return PLG_HEALTHY;

    if (np->nat_level[NAT_TLEV] <= 10.0)
	return PLG_HEALTHY;

    if (opt_BIG_CITY && (sp->sct_type == SCT_CAPIT))
	civvies = 9999.0;

    /*
     * make plague where there was none before...
     */
    plg_num = ((vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW]) / civvies) *
	((vec[I_IRON] + vec[I_OIL] + (vec[I_RAD] * 2)) / 10.0 +
	 np->nat_level[NAT_TLEV] + 100.0);
    plg_denom = eff + mobil + 100 + np->nat_level[NAT_RLEV];
    plg_chance = ((plg_num / plg_denom) - 1.0) * 0.01;
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
plague_people(struct natstr *np, register int *vec, register int *cvec,
	      int etus)
{
    int stage;
    double plg_num;
    double plg_denom;
    double pct_left;

    if (opt_NO_PLAGUE)		/* no plague nothing to do */
	return PLG_HEALTHY;
    cvec[C_PTIME] -= etus;
    stage = cvec[C_PSTAGE];
    switch (stage) {
    case PLG_DYING:
	plg_num = 100.0 * etus;
	plg_denom = (np->nat_level[NAT_RLEV] + 100.0) *
	    (vec[C_PTIME] + etus + 1.0);
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
	cvec[C_PTIME] = 0;
	break;
    default:
	/* bad */
	cvec[C_PTIME] = 0;
	break;
    }
    if (cvec[C_PTIME] <= 0) {
	cvec[C_PSTAGE]--;
	cvec[C_PTIME] = (etus / 2) + (random() % etus);
    }
    return stage;
}
