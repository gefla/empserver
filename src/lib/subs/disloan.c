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
 *  disloan.c: Display a loan
 * 
 *  Known contributors to this file:
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */
/*
 * Display a loan; there's a lot of stuff going
 * on here to figure out how much people owe.
 * It would be nice to change loans to work on
 * some kind of payment plan...like house payments
 * and such, where the bucks just get paid up each
 * update or so.
 *
 * I'd have made this more like treaty if I weren't
 * so disgusted with how it works.
 */

#include "misc.h"
#include "player.h"
#include "loan.h"
#include "nat.h"
#include "file.h"
#include "prototypes.h"

int
disloan(int n, struct lonstr *loan)
{
    time_t now;
    time_t accept;

    if (loan->l_status == LS_FREE)
	return 0;
    if (loan->l_ldur == 0)
	return 0;
    if (loan->l_loner != player->cnum && loan->l_lonee != player->cnum)
	return 0;
    (void)time(&now);
    pr("\nLoan #%d from %s to", n, cname(loan->l_loner));
    pr(" %s\n", cname(loan->l_lonee));
    if (loan->l_status == LS_PROPOSED) {
	pr("(proposed) principal=$%ld interest rate=%d%%",
	   loan->l_amtdue, loan->l_irate);
	pr(" duration(days)=%d\n", loan->l_ldur);
	if (loan->l_duedate < now) {
	    loan->l_status = LS_FREE;
	    putloan(n, loan);
	    pr("This offer has expired\n");
	    return 0;
	}
	accept = loan->l_lastpay + loan->l_ldur * SECS_PER_DAY;
	pr("Loan must be accepted by %s", ctime(&accept));
	return 1;
    }

    pr("Amount paid to date $%ld\n", loan->l_amtpaid);
    pr("Amount due (if paid now) $%.2f", loan_owed(loan, now));
    if (now <= loan->l_duedate) {
	pr(" (if paid on due date) $%.2f\n",
	   loan_owed(loan, loan->l_duedate));
	pr("Due date is %s", ctime(&loan->l_duedate));
    } else
	pr(" ** In Arrears **\n");
    return 1;
}
