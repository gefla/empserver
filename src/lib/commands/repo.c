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
 *  repo.c: Report on various levels (tech, research) of other nations
 *
 *  Known contributors to this file:
 *     Keith Muller, 1983
 *     Dave Pare, 1986 (rewrite)
 *     Steve McClure, 2000
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"

static void repo_header(void);
static void repo_list(struct natstr *, struct natstr *);
static void printdiff(struct natstr *, struct natstr *, int what);
static int tryprdiff(double, double, double, int, int);

int
repo(void)
{
    struct natstr *natp;
    struct natstr nat;
    struct nstr_item ni;

    if (!snxtitem(&ni, EF_NATION, player->argp[1], NULL))
	return RET_SYN;
    prdate();
    natp = getnatp(player->cnum);
    repo_header();
    while (nxtitem(&ni, &nat)) {
	if (nat.nat_stat == STAT_UNUSED)
	    continue;
	if (opt_HIDDEN) {
	    if (!player->god && !getcontact(natp, ni.cur))
		continue;
	}
	if (!player->god && nat.nat_stat != STAT_ACTIVE)
	    continue;
	repo_list(natp, &nat);
    }
    return RET_OK;
}

static void
repo_header(void)
{
    pr(" #    name                tech      research   education   happiness"
       " %s\n",
       player->god ? "capital" : " status");
}

static void
repo_list(struct natstr *plnatp, struct natstr *natp)
{
    pr(" %-3d   %-14.14s ", natp->nat_cnum, natp->nat_cnam);
    if (player->god || player->cnum == natp->nat_cnum) {
	pr(" %7.2f     %7.2f     %7.2f     %7.2f%s",
	   natp->nat_level[NAT_TLEV], natp->nat_level[NAT_RLEV],
	   natp->nat_level[NAT_ELEV], natp->nat_level[NAT_HLEV],
	   player->god ? "" : "    ");
    } else {
	printdiff(plnatp, natp, NAT_TLEV);
	printdiff(plnatp, natp, NAT_RLEV);
	printdiff(plnatp, natp, NAT_ELEV);
	printdiff(plnatp, natp, NAT_HLEV);
    }
    if (player->god) {
	prxy("  %4d,%-4d\n", natp->nat_xcap, natp->nat_ycap);
    } else {
	if (!opt_HIDDEN && influx(natp))
	    pr("In flux\n");
	else if (!opt_HIDDEN && natp->nat_money < 0)
	    pr("Broke\n");
	else
	    pr("Active\n");
    }
}

static void
printdiff(struct natstr *plnatp, struct natstr *natp, int what)
{
    double ours = plnatp->nat_level[what];
    double theirs;
    int shift;
    int tolerance;

    if (ours
	&& plnatp->nat_stat >= STAT_ACTIVE
	&& natp->nat_stat >= STAT_ACTIVE) {
	theirs = natp->nat_level[what];
	if ((shift = MIN((int)theirs, (int)ours) - 100) > 0) {
	    ours -= shift;
	    theirs -= shift;
	} else
	    shift = 0;
	switch (what) {
	case NAT_TLEV:
	    tolerance = 20;
	    break;
	case NAT_RLEV:
	    tolerance = 10;
	    break;
	default:
	    tolerance = 5;
	}
	if (tolerance > 2 * ours)
	    tolerance = (int)(2 * ours);
	if (tryprdiff(theirs, 2 * ours, -1.0, shift, tolerance))
	  ;
	else if (tryprdiff(theirs, 1.5 * ours, 2.0 * ours, shift, tolerance))
	  ;
	else if (tryprdiff(theirs, 1.2 * ours, 1.5 * ours, shift, tolerance))
	  ;
	else if (tryprdiff(theirs, 1.1 * ours, 1.2 * ours, shift, tolerance))
	  ;
	else if (tryprdiff(theirs, ours / 1.1, 1.1 * ours, shift, tolerance))
	  ;
	else if (tryprdiff(theirs, ours / 1.2, ours / 1.1, shift, tolerance))
	  ;
	else if (tryprdiff(theirs, ours / 1.5, ours / 1.2, shift, tolerance))
	  ;
	else if (tryprdiff(theirs, ours / 2.0, ours / 1.5, shift, tolerance))
	  ;
	else if (tryprdiff(theirs, -1.0, ours / 2.0, shift, tolerance)) ;
	else
	    pr("    n/a     ");
    } else
	pr("    n/a     ");
}

static int
tryprdiff(double theirs, double min, double max, int shift, int tolerance)
{
    double shove;

    if (min < 0) {
	if (theirs <= max) {
	    if (max < tolerance)
		max = tolerance;
	    pr("   0 - %-4d ", (int)max + shift);
	    return 1;
	}
    } else if (max < 0) {
	if (theirs >= min) {
	    pr("    >= %-4d ", (int)min + shift);
	    return 1;
	}
    } else if (theirs >= min && theirs <= max) {
	if (max - min < tolerance) {
	    shove = (tolerance - (max - min)) / 2;
	    if (min + shift - shove >= 0) {
		min -= shove;
		max += shove;
	    } else {
		min = 0;
		max = tolerance;
	    }
	}
	pr("%4d - %-4d ", (int)min + shift, (int)max + shift);
	return 1;
    }

    return 0;
}
