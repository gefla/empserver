/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ledg.c: Get a report of the current ledger
 *
 *  Known contributors to this file:
 *     Steve McClure, 1996
 */

#include <config.h>

#include "commands.h"
#include "loan.h"
#include "optlist.h"

int
ledg(void)
{
    struct nstr_item nstr;
    struct lonstr loan;
    int nloan;

    if (!opt_LOANS) {
	pr("Loans are not enabled.\n");
	return RET_FAIL;
    }
    if (!snxtitem(&nstr, EF_LOAN, player->argp[1], NULL))
	return RET_SYN;
    pr("\n... %s Ledger ...\n", cname(player->cnum));
    nloan = 0;
    while (nxtitem(&nstr, &loan)) {
	if (disloan(nstr.cur, &loan) > 0)
	    nloan++;
    }
    if (!nloan)
	pr("No loans found\n");
    else
	pr("%d loan%s outstanding.\n", nloan, splur(nloan));
    return RET_OK;
}
