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
 *  repa.c: repay loan from a country
 * 
 *  Known contributors to this file:
 *     
 */

#include <math.h>
#include "misc.h"
#include "player.h"
#include "nat.h"
#include "loan.h"
#include "news.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"

int
repa(void)
{
    struct lonstr loan;
    struct natstr *natp;
    struct natstr *loaner;
    int loan_num;
    long payment;
    long owe;
    long normaltime;
    long doubletime;
    double rate_per_sec, amt;
    s_char *cp;
    time_t now;
    s_char buf[1024];

    if (!opt_LOANS) {
	pr("Loans are not enabled.\n");
	return RET_FAIL;
    }
    natp = getnatp(player->cnum);
    cp = getstarg(player->argp[1], "Repay loan #? ", buf);
    if (cp == 0)
	return RET_SYN;
    loan_num = atoi(cp);
    if (loan_num < 0)
	return RET_SYN;
    if (!getloan(loan_num, &loan) || loan.l_lonee != player->cnum
	|| loan.l_status != LS_SIGNED) {
	pr("You don't owe anything on that loan.\n");
	return RET_FAIL;
    }
    (void)time(&now);
    /*
     * split duration now - l_lastpay into regular (up to l_duedate)
     * and extended (beyond l_duedate)
     */
    normaltime = loan.l_duedate - loan.l_lastpay;
    doubletime = now - loan.l_duedate;
    if (normaltime < 0) {
	doubletime += normaltime;
	normaltime = 0;
    }

    rate_per_sec = loan.l_irate /
	((double)loan.l_ldur * SECS_PER_DAY * 100.0);

    owe = (long)(loan.l_amtdue *
		 ((double)normaltime * rate_per_sec + 1.0 +
		  (double)doubletime * rate_per_sec * 2.0) + 0.5);
    amt = ((double)normaltime * rate_per_sec + 1.0 +
	   (double)doubletime * rate_per_sec * 2.0);
    if (((1 << 30) / amt) < loan.l_amtdue)
	owe = (1 << 30);
    else
	owe = (long)(loan.l_amtdue *
		     ((double)normaltime * rate_per_sec + 1.0 +
		      (double)doubletime * rate_per_sec * 2.0) + 0.5);
    if ((cp = getstarg(player->argp[2], "amount? ", buf)) == 0)
	return RET_SYN;
    if (!check_loan_ok(&loan))
	return RET_FAIL;
    payment = atoi(cp);
    if (payment <= 0)
	return RET_SYN;
    if (payment > owe) {
	pr("You don't owe that much.\n");
	return RET_FAIL;
    }
    if (natp->nat_money < payment) {
	pr("You only have $%ld.\n", natp->nat_money);
	return RET_FAIL;
    }
    player->dolcost += payment;
    loaner = getnatp(loan.l_loner);
    loaner->nat_money += payment;
    putnat(loaner);
    (void)time(&loan.l_lastpay);
    if (owe <= payment) {
	wu(0, loan.l_loner, "Country #%d paid off loan #%d with $%ld\n",
	   player->cnum, loan_num, payment);
	nreport(player->cnum, N_REPAY_LOAN, loan.l_loner, 1);
	loan.l_status = LS_FREE;
	loan.l_ldur = 0;
	pr("Congratulations, you've paid off the loan!\n");
    } else {
	wu(0, loan.l_loner,
	   "Country #%d paid $%.2f on loan %d\n", player->cnum,
	   (double)payment, loan_num);
	loan.l_amtdue = owe - payment;
	loan.l_amtpaid += payment;
    }
    if (!putloan(loan_num, &loan)) {
	pr("Can't save loan; get help!\n");
	return RET_SYS;
    }
    return RET_OK;
}
