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
 *  fina.c: Financial report on current status of loans to countries
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 */

#include <math.h>
#include "misc.h"
#include "player.h"
#include "loan.h"
#include "file.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "commands.h"
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
    (void)time(&now);
    pr("\n");
    pr("             -= Empire Financial Status Report =- \n");
    pr("                  ");
    prdate();
    pr("Loan       From            To        Rate   Dur     Paid      Total\n");
    snxtitem(&ni, EF_LOAN, "*");
    while (nxtitem(&ni, (s_char *)&loan)) {
	if (loan.l_status != LS_SIGNED)
	    continue;
	pr(" %-2d  (%3d) %-8.8s  (%3d) %-8.8s  ", ni.cur,
	   loan.l_loner, cname(loan.l_loner),
	   loan.l_lonee, cname(loan.l_lonee));
	pr("%3d%%   %3d    %5ld    %7ld",
	   loan.l_irate, loan.l_ldur, loan.l_amtpaid,
	   (long)loan_owed(&loan, now));
	if (now > loan.l_duedate)
	    pr(" (in arrears)\n");
	else
	    pr("\n");
    }
    pr("\n");
    return RET_OK;
}
