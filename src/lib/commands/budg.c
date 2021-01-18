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
 *  budg.c: Calculate production levels, prioritize
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Ville Virrankoski, 1995
 *     Steve McClure, 1997-2000
 *     Markus Armbruster, 2004-2020
 */

#include <config.h>

#include <limits.h>
#include "commands.h"
#include "item.h"
#include "optlist.h"
#include "product.h"
#include "update.h"

static struct budget *calc_all(void);
static char *dotsprintf(char *buf, char *format, int data);

int
c_budget(void)
{
    static struct {
	char *activity;
	char *object;
    } bm_name[] = {
	{ "Ship building", "ship" },
	{ "Ship maintenance", "ship" },
	{ "Plane building", "plane" },
	{ "Plane maintenance", "plane" },
	{ "Unit building", "unit" },
	{ "Unit maintenance", "unit" },
	{ "Sector building", "sector" },
	{ "Sector maintenance", "sector" }
    };
    unsigned i;
    struct budget *budget;
    int income, expenses, taxes;
    struct natstr *np;
    char buf[1024];

    np = getnatp(player->cnum);

    player->simulation = 1;
    budget = calc_all();
    player->simulation = 0;

    income = expenses = 0;

    pr("Sector Type\t\t\tProduction\t\t\t    Cost\n");
    for (i = 0; i < ARRAY_SIZE(budget->prod); i++) {
	if (!budget->prod[i].money)
	    continue;
	pr("%-17s\t\t", dchr[i].d_name);
	if (i == SCT_ENLIST)
	    pr("%d mil    \t", budget->prod[i].count);
	else if (dchr[i].d_prd >= 0)
	    pr("%d %-7s\t", budget->prod[i].count,
	       pchr[dchr[i].d_prd].p_sname);
	else
	    pr("\t\t");
	pr("\t\t%8.0f\n", -budget->prod[i].money);
	expenses -= budget->prod[i].money;
    }

    for (i = 0; i <= BUDG_BLD_MAX; i++) {
	if (!budget->bm[i].money)
	    continue;
	snprintf(buf, sizeof(buf), "%d %s%s",
		 budget->bm[i].count, bm_name[i].object,
		 splur(budget->bm[i].count));
	pr("%-20s\t\t%-16s\t\t%8.0f\n",
	   bm_name[i].activity, buf, -budget->bm[i].money);
	expenses -= budget->bm[i].money;
    }

    if (budget->mil.money) {
	snprintf(buf, sizeof(buf), "%d mil, %d res",
		 budget->mil.count, np->nat_reserve);
	pr("Military payroll\t\t%-32s%8.0f\n",
	   buf, -budget->mil.money);
	expenses -= budget->mil.money;
    }

    pr("Total expenses%s\n", dotsprintf(buf, "%58d", expenses));
    taxes = budget->civ.money + budget->uw.money;
    if (taxes) {
	snprintf(buf, sizeof(buf), "%d civ%s, %d uw%s",
		 budget->civ.count, splur(budget->civ.count),
		 budget->uw.count, splur(budget->uw.count));
	pr("Income from taxes\t\t%-32s%+8d\n", buf, taxes);
	income += taxes;
    }
    if (budget->bars.money) {
	snprintf(buf, sizeof(buf), "%d bar%s",
		 budget->bars.count, splur(budget->bars.count));
	pr("Income from bars\t\t%-32s%+8.0f\n",
	   buf, budget->bars.money);
	income += budget->bars.money;
    }
    pr("Total income%s\n", dotsprintf(buf, "%+60d", income));
    pr("Balance forward\t\t\t\t\t\t      %10d\n", np->nat_money);
    pr("Estimated delta\t\t\t\t\t\t      %+10d\n", income - expenses);
    pr("Estimated new treasury%s\n",
       dotsprintf(buf, "%50d", np->nat_money + income - expenses));
    if (np->nat_money + income - expenses < 0 && !player->god) {
	pr("After processing sectors, you will be broke!\n");
	pr("Sectors will not produce, distribute, or deliver!\n\n");
    }

    return RET_OK;
}

static struct budget *
calc_all(void)
{
    struct budget *budget = &nat_budget[player->cnum];
    struct natstr *np;
    struct bp *bp;
    int i;
    int etu = etu_per_update;

    memset(nat_budget, 0, sizeof(nat_budget));
    np = getnatp(player->cnum);
    /* Take care not to disclose others going broke: */
    for (i = 0; i < MAXNOC; i++)
	nat_budget[i].start_money = nat_budget[i].money = INT_MAX;
    budget->start_money = budget->money = np->nat_money;
    bp = bp_alloc();

    prep_ships(etu, bp);
    prep_planes(etu, bp);
    prep_lands(etu, bp);
    prepare_sects(etu, bp);
    pay_reserve(np, etu);

    /* Maintain ships, planes and land units */
    prod_ship(etu, bp, 0);
    prod_plane(etu, bp, 0);
    prod_land(etu, bp, 0);

    /* Produce */
    produce_sect(etu, bp);

    /* Build ships, planes and land units */
    prod_ship(etu, bp, 1);
    prod_plane(etu, bp, 1);
    prod_land(etu, bp, 1);

    if (CANT_HAPPEN(np->nat_money != budget->start_money))
	np->nat_money = budget->start_money;

    free(bp);
    return budget;
}

static char *
dotsprintf(char *buf, char *format, int data)
{
    sprintf(buf, format, data);
    return memset(buf, '.', strspn(buf, " "));
}
