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
 *  repo.c: Report on various levels (tech, research) of other nations
 * 
 *  Known contributors to this file:
 *     Keith Muller, 1983
 *     Dave Pare, 1986 (rewrite)
 *     Steve McClure, 2000
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "file.h"
#include "xy.h"
#include "nsc.h"
#include <fcntl.h>
#include <ctype.h>
#include "commands.h"
#include "optlist.h"

struct stats {
    double res;
    double tech;
    double edu;
    double hap;
    int stat;
};

static void printdiff(int mystat, double ours, struct natstr *natp,
		      int what);
static void repo_header(void);
static void repo_list(struct stats *stat, natid cn, struct natstr *natp);

static int check(s_char *buf, double theirs, double min, double max,
		 int shift, int tolerance);

int
repo(void)
{
    struct natstr *natp;
    struct stats mystat;
    struct natstr nat;
    struct nstr_item ni;
    int first;

    if (!snxtitem(&ni, EF_NATION, player->argp[1]))
	return RET_SYN;
    prdate();
    natp = getnatp(player->cnum);
    memset(&mystat, 0, sizeof(struct stats));
    mystat.stat = natp->nat_stat;
    if (mystat.stat & STAT_NORM) {
	mystat.res = natp->nat_level[NAT_RLEV];
	mystat.tech = natp->nat_level[NAT_TLEV];
	mystat.edu = natp->nat_level[NAT_ELEV];
	mystat.hap = natp->nat_level[NAT_HLEV];
    }
    if (opt_HIDDEN) {
	repo_header();
	first = 0;
    } else {
	first = 1;
    }
    while (nxtitem(&ni, (s_char *)&nat)) {
	if (!(nat.nat_stat & STAT_INUSE))
	    continue;
	if (opt_HIDDEN) {
	    if (!player->god && !getcontact(getnatp(player->cnum), ni.cur))
		continue;
	}
	if (first) {
	    repo_header();
	    first = 0;
	}
	repo_list(&mystat, (natid)ni.cur, &nat);
    }
    return RET_OK;
}

static void
repo_header(void)
{
    pr(" #    name                tech      research   education   happiness ");
    if (player->god)
	pr("capital\n");
    else {
	if (opt_HIDDEN)
	    pr("\n");
	else
	    pr(" status\n");
    }
}

static void
repo_list(struct stats *stat, natid cn, struct natstr *natp)
{
    struct sctstr cap;

    if (player->god) {
	pr(" %-3d   %-14.14s ", cn, natp->nat_cnam);
	pr(" %7.2f    %7.2f      %7.2f     %7.2f",
	   natp->nat_level[NAT_TLEV],
	   natp->nat_level[NAT_RLEV],
	   natp->nat_level[NAT_ELEV], natp->nat_level[NAT_HLEV]);
	prxy("  %4d,%-4d\n", natp->nat_xcap, natp->nat_ycap, player->cnum);
	return;
    }
    switch (natp->
	    nat_stat & (STAT_NORM | STAT_GOD | STAT_NEW | STAT_SANCT)) {
    case STAT_NORM:
    case (STAT_NORM | STAT_SANCT):
	pr(" %-3d   %-14.14s ", cn, natp->nat_cnam);
	printdiff(stat->stat, stat->tech, natp, NAT_TLEV);
	printdiff(stat->stat, stat->res, natp, NAT_RLEV);
	printdiff(stat->stat, stat->edu, natp, NAT_ELEV);
	printdiff(stat->stat, stat->hap, natp, NAT_HLEV);
	getsect(natp->nat_xcap, natp->nat_ycap, &cap);
	if (opt_HIDDEN) {
	    pr("\n");
	} else {
	    if ((cap.sct_own != cn) ||
		(cap.sct_type != SCT_CAPIT &&
		 cap.sct_type != SCT_MOUNT) ||
		(natp->nat_flags & NF_SACKED))
		pr("In flux\n");
	    else if (natp->nat_money <= 0)
		pr("Broke\n");
	    else
		pr("Active\n");
	}
	break;
    case STAT_SANCT:
	break;
    case STAT_NEW:
    case 0:
	break;
    case STAT_SANCT | STAT_NORM | STAT_GOD:
    case STAT_NORM | STAT_GOD:
    case STAT_GOD:
	break;
    default:
	pr("????        ????        ????        ????\n");
	break;
    }
}

static void
printdiff(int mystat, double ours, struct natstr *natp, int what)
{
    double theirs;
    int shift;
    int tolerance;
    s_char buf[128];

    if (natp->nat_cnum == player->cnum) {
	pr(" %7.2f    ", ours);
	return;
    }
    if (ours && mystat & STAT_NORM && natp->nat_stat & STAT_NORM) {
	theirs = natp->nat_level[what];
	if ((shift = min((int)theirs, (int)ours) - 100) > 0) {
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
	if (check(buf, theirs, 2 * ours, -1.0, shift, tolerance))
	  ;
	else if (check(buf, theirs, 1.5 * ours, 2.0 * ours, shift, tolerance))
	  ;
	else if (check(buf, theirs, 1.2 * ours, 1.5 * ours, shift, tolerance))
	  ;
	else if (check(buf, theirs, 1.1 * ours, 1.2 * ours, shift, tolerance))
	  ;
	else if (check(buf, theirs, ours / 1.1, 1.1 * ours, shift, tolerance))
	  ;
	else if (check(buf, theirs, ours / 1.2, ours / 1.1, shift, tolerance))
	  ;
	else if (check(buf, theirs, ours / 1.5, ours / 1.2, shift, tolerance))
	  ;
	else if (check(buf, theirs, ours / 2.0, ours / 1.5, shift, tolerance))
	  ;
	else if (check(buf, theirs, -1.0, ours / 2.0, shift, tolerance)) ;
	else
	    sprintf(buf, "    n/a");
    } else
	sprintf(buf, "    n/a");

    pr("%-11s ", buf);
}

static int
check(s_char *buf, double theirs, double min, double max, int shift,
      int tolerance)
{
    double shove;

    if (min < 0) {
	if (theirs <= max) {
	    if (max < tolerance)
		max = tolerance;
	    sprintf(buf, "   0 - %d", (int)max + shift);
	    return 1;
	}
    } else if (max < 0) {
	if (theirs >= min) {
	    sprintf(buf, "    >= %d", (int)min + shift);
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
	sprintf(buf, "%4d - %d", (int)min + shift, (int)max + shift);
	return 1;
    }

    return 0;
}
