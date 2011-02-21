/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  caploss.c: Lose your capital (kiss of death)
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     Markus Armbruster, 2007-2008
 */

#include <config.h>

#include "commodity.h"
#include "file.h"
#include "loan.h"
#include "nat.h"
#include "news.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"

void
caploss(struct sctstr *sp, natid coun, char *msg)
{
    struct natstr *natp;
    struct lonstr loan;
    struct comstr comm;
    long lose;
    long gain;
    char *verb;
    int loan_num, comm_num;

    CANT_HAPPEN(sp->sct_own && sp->sct_own != player->cnum);

    natp = getnatp(coun);
    if (natp->nat_stat != STAT_ACTIVE)
	return;
    if (sp->sct_x != natp->nat_xcap || sp->sct_y != natp->nat_ycap)
	return;
    if (coun == player->cnum)
	return;
    if (natp->nat_flags & NF_SACKED)
	return;			/* sacked capital, not yet reset */
    natp->nat_flags |= NF_SACKED; /* no more sacking until player resets */

    pr(msg, natp->nat_cnam);
    gain = lose = natp->nat_money / 2;
    if (lose < 3000)
	lose = 3000;
    natp->nat_money -= lose;
    putnat(natp);
    if (gain >= 0 && sp->sct_own) {
	gain = (0.2 + 0.8 * (sp->sct_effic / 100.0)) * gain;
	player->dolcost -= gain;
    } else
	gain = 0;
    verb = sp->sct_own ? "sacked" : "obliterated";
    wu(0, coun, "* %s just %s your capital! *\n",
       cname(player->cnum), verb);
    wu(0, coun, "You lost $%ld and they gained $%ld\n", lose, gain);
    wu(0, coun, "You need to use 'capital' to activate a new capital.\n");
    wu(0, 0, "%s just %s %s's capital and gained $%ld\n",
       cname(player->cnum), verb, cname(coun), gain);

    if (opt_LOANS && sp->sct_own) {
	for (loan_num = 0; getloan(loan_num, &loan); loan_num++) {
	    if (loan.l_status == LS_SIGNED && loan.l_loner == coun) {
		loan.l_loner = player->cnum;
		putloan(loan_num, &loan);
		pr("Loan %d has been transfered over to you\n", loan_num);
	    }
	}
    }
    if (opt_MARKET && sp->sct_own) {
	for (comm_num = 0; getcomm(comm_num, &comm); comm_num++) {
	    if (comm.com_owner == 0)
		continue;
	    if (comm.com_owner == coun) {
		if (comm.com_owner == comm.com_maxbidder)
		    comm.com_maxbidder = player->cnum;
		comm.com_owner = player->cnum;
		putcomm(comm_num, &comm);
		pr("You now own commodity #%d.\n", comm_num);
	    }
	}
    }
    nreport(player->cnum, N_SACK_CAP, coun, 1);
}
