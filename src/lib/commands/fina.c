/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fina.c: Financial report on current status of loans to countries
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 */

#include <config.h>

#include <math.h>
#include "commands.h"
#include "loan.h"
#include "optlist.h"

/*
 * format: fina
 */
int
fina(void)
{
    struct lonstr loan;
    struct nstr_item ni;
    time_t now;

    if (!opt_LOANS) {
	pr("Loans are not enabled.\n");
	return RET_FAIL;
    }
    if (!snxtitem(&ni, EF_LOAN, "*", NULL))
	return RET_SYN;
    (void)time(&now);
    pr("\n");
    pr("             -= Empire Financial Status Report =-\n");
    pr("                  ");
    prdate();
    pr("Loan       From            To        Rate   Dur     Paid      Total\n");
    while (nxtitem(&ni, &loan)) {
	if (loan.l_status != LS_SIGNED)
	    continue;
	pr(" %-2d  (%3d) %-8.8s  (%3d) %-8.8s  ", ni.cur,
	   loan.l_loner, cname(loan.l_loner),
	   loan.l_lonee, cname(loan.l_lonee));
	pr("%3d%%   %3d    %5d    %7.0f",
	   loan.l_irate, loan.l_ldur, loan.l_amtpaid,
	   ceil(loan_owed(&loan, now)));
	if (now > loan.l_duedate)
	    pr(" (in arrears)\n");
	else
	    pr("\n");
    }
    pr("\n");
    return RET_OK;
}
