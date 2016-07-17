/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  prepare.c: Perform prelimiary updates of sectors
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1992
 *     Steve McClure, 1997
 *     Markus Armbruster, 2016
 */

#include <config.h>

#include "chance.h"
#include "item.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "update.h"

static void tax(struct sctstr *, int);
static void bank_income(struct sctstr *, int);

void
prepare_sects(int etu, struct bp *bp)
{
    struct sctstr *sp, scratch_sect;
    int n;

    if (!player->simulation)
	fallout(etu);

    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if (bp_skip_sect(bp, sp))
	    continue;
	bp_set_from_sect(bp, sp);
	if (sp->sct_type == SCT_WATER || sp->sct_type == SCT_SANCT)
	    continue;

	/*
	 * When running the test suite, reseed PRNG for each sector
	 * with its UID, to keep results stable even when the number
	 * of PRNs consumed changes.
	 */
	if (running_test_suite)
	    seed_prng(sp->sct_uid);

	if (player->simulation) {
	    /* work on a copy, which will be discarded */
	    scratch_sect = *sp;
	    sp = &scratch_sect;
	}

	if (!player->simulation) {
	    guerrilla(sp);
	    do_plague(sp, etu);
	    populace(sp, etu);
	}
	tax(sp, etu);
	if (sp->sct_type == SCT_BANK)
	    bank_income(sp, etu);
	bp_set_from_sect(bp, sp);
    }
}

static void
tax(struct sctstr *sp, int etu)
{
    struct budget *budget = &nat_budget[sp->sct_own];
    double civ_tax, uw_tax, mil_pay;

    civ_tax = sp->sct_item[I_CIVIL] * etu * money_civ * sp->sct_effic / 100;
    if (sp->sct_own == sp->sct_oldown)
	budget->oldowned_civs += sp->sct_item[I_CIVIL];
    else
	civ_tax /= 4;		/* captured civs pay less */
    budget->civ.count += sp->sct_item[I_CIVIL];
    budget->civ.money += civ_tax;
    budget->money += civ_tax;

    uw_tax = sp->sct_item[I_UW] * etu * money_uw * sp->sct_effic / 100;
    budget->uw.count += sp->sct_item[I_UW];
    budget->uw.money += uw_tax;
    budget->money += uw_tax;

    mil_pay = sp->sct_item[I_MILIT] * etu * money_mil;
    budget->mil.count += sp->sct_item[I_MILIT];
    budget->mil.money += mil_pay;
    budget->money += mil_pay;
}

static void
bank_income(struct sctstr *sp, int etu)
{
    double income;

    income = sp->sct_item[I_BAR] * etu * bankint * sp->sct_effic / 100;
    nat_budget[sp->sct_own].bars.count += sp->sct_item[I_BAR];
    nat_budget[sp->sct_own].bars.money += income;
    nat_budget[sp->sct_own].money += income;
}

void
pay_reserve(struct natstr *np, int etu)
{
    double pay = np->nat_reserve * money_res * etu;

    nat_budget[np->nat_cnum].mil.money += pay;
    nat_budget[np->nat_cnum].money += pay;
}
