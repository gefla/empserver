/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  coll.c: Collet on a loan
 * 
 *  Known contributors to this file:
 *     Pat Loney, 1992
 *     Steve McClure, 1996-2000
 */

#include <math.h>
#include "misc.h"
#include "player.h"
#include "file.h"
#include "sect.h"
#include "item.h"
#include "loan.h"
#include "news.h"
#include "nat.h"
#include "xy.h"
#include "commands.h"
#include "optlist.h"

int
coll(void)
{
    int arg;
    int i;
    int val;
    time_t now;
    char *p;
    struct lonstr loan;
    struct sctstr sect;
    coord x, y;
    double owed;
    double pay;
    char buf[1024];

    if (!opt_LOANS) {
	pr("Loans are not enabled.\n");
	return RET_FAIL;
    }
    if ((arg = onearg(player->argp[1], "Collect on loan #")) < 0)
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

    pr("You are owed $%.2f on that loan.\n", owed);
    if (!(p = getstarg(player->argp[2],
		       "What sector do you wish to confiscate? ", buf)))
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
    pay = dchr[sect.sct_type].d_value * ((float)sect.sct_effic + 100.0);
    for (i = 0; ichr[i].i_name; i++) {
	if (ichr[i].i_value == 0 || ichr[i].i_vtype == I_NONE)
	    continue;
	val = sect.sct_item[ichr[i].i_vtype];
	pay += val * ichr[i].i_value;
    }
    pr("That sector (and its contents) is valued at $%.2f\n", pay);
    if (pay > owed * 1.2) {
	pr("That is more than is owed!\n");
	return RET_FAIL;
    }
    if (sect.sct_type == SCT_CAPIT || sect.sct_type == SCT_MOUNT)
	caploss(&sect, sect.sct_own, "that was %s's capital!\n");
    sect.sct_item[I_MILIT] = 1;	/* FIXME now where did this guy come from? */

/* Consider modifying takeover to take a "no che" argument and
   putting using it here again. */
/*	(void) takeover(&sect, player->cnum);*/
    makelost(EF_SECTOR, sect.sct_own, 0, sect.sct_x, sect.sct_y);
    makenotlost(EF_SECTOR, player->cnum, 0, sect.sct_x, sect.sct_y);
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
	loan.l_amtdue = (long)owed;
	pay += loan.l_amtpaid;
	loan.l_amtpaid = pay;
	wu(0, loan.l_lonee,
	   "%s seized %s in partial payment of loan %d.\n",
	   cname(player->cnum),
	   xyas(sect.sct_x, sect.sct_y, loan.l_lonee), arg);
	pr("You are still owed $%.2f on loan %d.\n", owed, arg);
    }
    putloan(arg, &loan);
    return RET_OK;
}
