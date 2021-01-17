/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  coll.c: Collect on a loan
 *
 *  Known contributors to this file:
 *     Pat Loney, 1992
 *     Steve McClure, 1996-2000
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "loan.h"
#include "news.h"
#include "optlist.h"

static double appraise_sect(struct sctstr *);

int
coll(void)
{
    int arg;
    time_t now;
    char *p;
    struct lonstr loan;
    struct sctstr sect;
    struct natstr *lonee_np;
    coord x, y;
    double owed;
    double pay;
    char buf[1024];

    if (!opt_LOANS) {
	pr("Loans are not enabled.\n");
	return RET_FAIL;
    }
    if ((arg = onearg(player->argp[1], "Collect on loan # ")) < 0)
	return RET_SYN;
    /* Check if it's a valid loan.  That means, is it a valid loan,
       owed to this player, with a valid duration and it's been signed. */
    if (!getloan(arg, &loan) || (loan.l_loner != player->cnum) ||
	(loan.l_ldur == 0) || (loan.l_status != LS_SIGNED)) {
	pr("You aren't owed anything on that loan...\n");
	return RET_FAIL;
    }
    /* If we got here, we check to see if it's been defaulted on.  We
       already know it's owed to this player. */
    owed = loan_owed(&loan, time(&now));
    if (now <= loan.l_duedate) {
	pr("There has been no default on loan %d\n", arg);
	return RET_FAIL;
    }

    lonee_np = getnatp(loan.l_lonee);
    pr("You are owed $%.2f on that loan.\n", owed);
    p = getstarg(player->argp[2],
		 "What sector do you wish to confiscate? ", buf);
    if (!p)
	return RET_SYN;
    if (!check_loan_ok(&loan))
	return RET_FAIL;
    if (!sarg_xy(p, &x, &y) || !getsect(x, y, &sect))
	return RET_SYN;
    if (!neigh(x, y, player->cnum)) {
	pr("You are not adjacent to %s\n", xyas(x, y, player->cnum));
	return RET_FAIL;
    }
    if (sect.sct_own != loan.l_lonee) {
	pr("%s is not owned by %s.\n",
	   xyas(x, y, player->cnum), cname(loan.l_lonee));
	return RET_FAIL;
    }
    pay = appraise_sect(&sect);
    if (pay > owed * 1.2) {
	pr("That sector (and its contents) is valued at more than %.2f.\n",
	   owed);
	return RET_FAIL;
    }
    if (!influx(lonee_np)
	&& sect.sct_x == lonee_np->nat_xcap
	&& sect.sct_y == lonee_np->nat_ycap) {
	pr("%s's capital cannot be confiscated.\n", cname(loan.l_lonee));
	return RET_FAIL;
    }
    pr("That sector (and its contents) is valued at $%.2f\n", pay);

    sect.sct_item[I_MILIT] = 1;	/* FIXME now where did this guy come from? */

    /*
     * Used to call takeover() here a long time ago, but that does
     * unwanted things, like generate che.
     */
    sect.sct_own = player->cnum;
    memset(sect.sct_dist, 0, sizeof(sect.sct_dist));
    memset(sect.sct_del, 0, sizeof(sect.sct_del));
    sect.sct_off = 1;
    sect.sct_dist_x = sect.sct_x;
    sect.sct_dist_y = sect.sct_y;
    putsect(&sect);
    nreport(player->cnum, N_SEIZE_SECT, loan.l_lonee, 1);
    owed = loan_owed(&loan, time(&now));
    if (pay >= owed) {
	loan.l_status = LS_FREE;
	loan.l_ldur = 0;
	nreport(loan.l_lonee, N_REPAY_LOAN, player->cnum, 1);
	wu(0, loan.l_lonee,
	   "%s seized %s to satisfy loan #%d\n",
	   cname(player->cnum),
	   xyas(sect.sct_x, sect.sct_y, loan.l_lonee), arg);
	pr("That loan is now considered repaid.\n");
    } else {
	(void)time(&loan.l_lastpay);
	owed -= pay;
	loan.l_amtdue = (int)owed;
	pay += loan.l_amtpaid;
	loan.l_amtpaid = (int)pay;
	wu(0, loan.l_lonee,
	   "%s seized %s in partial payment of loan %d.\n",
	   cname(player->cnum),
	   xyas(sect.sct_x, sect.sct_y, loan.l_lonee), arg);
	pr("You are still owed $%.2f on loan %d.\n", owed, arg);
    }
    putloan(arg, &loan);
    return RET_OK;
}

static double
appraise_items(short item[])
{
    double total, val;
    int i;

    total = 0.0;
    for (i = I_NONE + 1; i <= I_MAX; i++) {
	val = ichr[i].i_power / 10.0;
	if (i == I_MILIT || i == I_CIVIL || i == I_UW)
	    val /= 5.0;		/* collect-specific fudge factor */
	total += item[i] * val;
    }
    return total;
}

static double
appraise_sect(struct sctstr *sp)
{
    struct dchrstr *dcp = &dchr[sp->sct_type];
    double bld_val = appraise_items(dcp->d_mat) + dcp->d_cost;

    return bld_val * sp->sct_effic / 100.0 + dcp->d_maxpop
	+ appraise_items(sp->sct_item);
}
