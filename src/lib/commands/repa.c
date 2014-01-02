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
 *  repa.c: repay loan from a country
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include <math.h>
#include "commands.h"
#include "loan.h"
#include "news.h"
#include "optlist.h"

int
repa(void)
{
    struct lonstr loan;
    struct natstr *natp;
    struct natstr *loaner;
    int loan_num;
    int payment;
    int newdue;
    char *cp;
    time_t now;
    char buf[1024];

    if (!opt_LOANS) {
	pr("Loans are not enabled.\n");
	return RET_FAIL;
    }
    natp = getnatp(player->cnum);
    cp = getstarg(player->argp[1], "Repay loan #? ", buf);
    if (!cp)
	return RET_SYN;
    loan_num = atoi(cp);
    if (loan_num < 0)
	return RET_SYN;
    if (!getloan(loan_num, &loan) || loan.l_lonee != player->cnum
	|| loan.l_status != LS_SIGNED) {
	pr("You don't owe anything on that loan.\n");
	return RET_FAIL;
    }
    if (!(cp = getstarg(player->argp[2], "amount? ", buf)))
	return RET_SYN;
    if (!check_loan_ok(&loan))
	return RET_FAIL;
    payment = atoi(cp);
    if (payment <= 0)
	return RET_SYN;

    newdue = (int)ceil(loan_owed(&loan, time(&now)) - payment);
    if (newdue < 0) {
	pr("You don't owe that much.\n");
	return RET_FAIL;
    }
    if (natp->nat_money < payment) {
	pr("You only have $%d.\n", natp->nat_money);
	return RET_FAIL;
    }
    player->dolcost += payment;
    loaner = getnatp(loan.l_loner);
    loaner->nat_money += payment;
    putnat(loaner);
    loan.l_lastpay = now;
    if (newdue == 0) {
	wu(0, loan.l_loner, "Country #%d paid off loan #%d with $%d\n",
	   player->cnum, loan_num, payment);
	nreport(player->cnum, N_REPAY_LOAN, loan.l_loner, 1);
	loan.l_status = LS_FREE;
	loan.l_ldur = 0;
	pr("Congratulations, you've paid off the loan!\n");
    } else {
	wu(0, loan.l_loner,
	   "Country #%d paid $%d on loan %d\n",
	   player->cnum, payment, loan_num);
	loan.l_amtdue = newdue;
	loan.l_amtpaid += payment;
    }
    if (!putloan(loan_num, &loan)) {
	logerror("repa: can't write loan");
	pr("Can't save loan; get help!\n");
	return RET_FAIL;
    }
    return RET_OK;
}
