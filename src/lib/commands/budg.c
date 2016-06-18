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
 *  budg.c: Calculate production levels, prioritize
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Ville Virrankoski, 1995
 *     Steve McClure, 1997-2000
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "item.h"
#include "optlist.h"
#include "product.h"
#include "update.h"

static struct budget *calc_all(int *taxes, int *Ncivs,
			       int *Nuws, int *bars, int *Nbars);
static char *dotsprintf(char *buf, char *format, int data);

int
budg(void)
{
    int i;
    int taxes, Ncivs, Nuws, bars, Nbars;
    struct budget *budget;
    int income, expenses;
    struct natstr *np;
    char buf[1024];
    char in[80];

    np = getnatp(player->cnum);

    player->simulation = 1;
    budget = calc_all(&taxes, &Ncivs, &Nuws, &bars, &Nbars);
    player->simulation = 0;

    income = taxes + bars;
    expenses = 0;
    pr("Sector Type\t\t\tProduction\t\t\t    Cost\n");
    for (i = 0; i <= SCT_TYPE_MAX; i++) {
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
	pr("\t\t%8d\n", -budget->prod[i].money);
	expenses -= budget->prod[i].money;
    }

    if (budget->bm[BUDG_SHP_BUILD].money) {
	snprintf(buf, sizeof(buf), "%d ship%s",
		 budget->bm[BUDG_SHP_BUILD].count,
		 splur(budget->bm[BUDG_SHP_BUILD].count));
	pr("Ship building\t\t\t%-16s\t\t%8d\n",
	   buf, -budget->bm[BUDG_SHP_BUILD].money);
	expenses -= budget->bm[BUDG_SHP_BUILD].money;
    }

    if (budget->bm[BUDG_SHP_MAINT].money) {
	snprintf(buf, sizeof(buf), "%d ship%s",
		 budget->bm[BUDG_SHP_MAINT].count,
		 splur(budget->bm[BUDG_SHP_MAINT].count));
	pr("Ship maintenance\t\t%-16s\t\t%8d\n",
	   buf, -budget->bm[BUDG_SHP_MAINT].money);
	expenses -= budget->bm[BUDG_SHP_MAINT].money;
    }

    if (budget->bm[BUDG_PLN_BUILD].money) {
	snprintf(buf, sizeof(buf), "%d plane%s",
		 budget->bm[BUDG_PLN_BUILD].count,
		 splur(budget->bm[BUDG_PLN_BUILD].count));
	pr("Plane building\t\t\t%-16s\t\t%8d\n",
	   buf, -budget->bm[BUDG_PLN_BUILD].money);
	expenses -= budget->bm[BUDG_PLN_BUILD].money;
    }

    if (budget->bm[BUDG_PLN_MAINT].money) {
	snprintf(buf, sizeof(buf), "%d plane%s",
		 budget->bm[BUDG_PLN_MAINT].count,
		 splur(budget->bm[BUDG_PLN_MAINT].count));
	pr("Plane maintenance\t\t%-16s\t\t%8d\n",
	   buf, -budget->bm[BUDG_PLN_MAINT].money);
	expenses -= budget->bm[BUDG_PLN_MAINT].money;
    }
    if (budget->bm[BUDG_LND_BUILD].money) {
	snprintf(buf, sizeof(buf), "%d unit%s",
		 budget->bm[BUDG_LND_BUILD].count,
		 splur(budget->bm[BUDG_LND_BUILD].count));
	pr("Unit building\t\t\t%-16s\t\t%8d\n",
	   buf, -budget->bm[BUDG_LND_BUILD].money);
	expenses -= budget->bm[BUDG_LND_BUILD].money;
    }

    if (budget->bm[BUDG_LND_MAINT].money) {
	snprintf(buf, sizeof(buf), "%d unit%s",
		 budget->bm[BUDG_LND_MAINT].count,
		 splur(budget->bm[BUDG_LND_MAINT].count));
	pr("Unit maintenance\t\t%-16s\t\t%8d\n",
	   buf, -budget->bm[BUDG_LND_MAINT].money);
	expenses -= budget->bm[BUDG_LND_MAINT].money;
    }

    if (budget->bm[BUDG_SCT_BUILD].money) {
	snprintf(buf, sizeof(buf), "%d sector%s",
		 budget->bm[BUDG_SCT_BUILD].count,
		 splur(budget->bm[BUDG_SCT_BUILD].count));
	pr("Sector building\t\t\t%-16s\t\t%8d\n",
	   buf, -budget->bm[BUDG_SCT_BUILD].money);
	expenses -= budget->bm[BUDG_SCT_BUILD].money;
    }
    if (budget->bm[BUDG_SCT_MAINT].count) {
	snprintf(buf, sizeof(buf), "%d sector%s",
		 budget->bm[BUDG_SCT_MAINT].count,
		 splur(budget->bm[BUDG_SCT_MAINT].count));
	pr("Sector maintenance\t\t%-16s\t\t%8d\n",
	   buf, -budget->bm[BUDG_SCT_MAINT].money);
	expenses -= budget->bm[BUDG_SCT_MAINT].money;
    }
    if (budget->mil.money) {
	snprintf(buf, sizeof(buf), "%d mil, %d res",
		 budget->mil.count, np->nat_reserve);
	pr("Military payroll\t\t%-32s%8d\n",
	   buf, -budget->mil.money);
	expenses -= budget->mil.money;
    }

    pr("Total expenses%s\n", dotsprintf(buf, "%58d", expenses));
    if (taxes) {
	sprintf(in, "%d civ%s, %d uw%s",
		Ncivs, splur(Ncivs), Nuws, splur(Nuws));
	pr("Income from taxes\t\t%-32s%+8d\n", in, taxes);
    }
    if (bars) {
	sprintf(in, "%d bar%s", Nbars, splur(Nbars));
	pr("Income from bars\t\t%-32s%+8d\n", in, bars);
    }
    pr("Total income%s\n", dotsprintf(buf, "%+60d", income));
    pr("Balance forward\t\t\t\t\t\t      %10d\n", np->nat_money);
    pr("Estimated delta\t\t\t\t\t\t      %+10d\n", income - expenses);
    pr("Estimated new treasury%s\n",
       dotsprintf(buf, "%50d", np->nat_money + income - expenses));
    if (np->nat_money + income - expenses < 0 && !player->god) {
	pr("After processsing sectors, you will be broke!\n");
	pr("Sectors will not produce, distribute, or deliver!\n\n");
    }

    return RET_OK;
}

static struct budget *
calc_all(int *taxes, int *Ncivs, int *Nuws, int *bars, int *Nbars)
{
    struct budget *budget = &nat_budget[player->cnum];
    struct natstr *np;
    struct bp *bp;
    int pop = 0;
    int n, civ_tax, uw_tax;
    struct sctstr *sp;
    int etu = etu_per_update;

    memset(nat_budget, 0, sizeof(nat_budget));
    *taxes = *Ncivs = *Nuws = *bars = *Nbars = 0;

    np = getnatp(player->cnum);
    bp = bp_alloc();
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	bp_set_from_sect(bp, sp);
	if (sp->sct_own == player->cnum) {
	    sp->sct_updated = 0;
	    tax(sp, etu, &pop, &civ_tax, &uw_tax);
	    *Ncivs += sp->sct_item[I_CIVIL];
	    *Nuws += sp->sct_item[I_UW];
	    *taxes += civ_tax + uw_tax;
	    if (sp->sct_type == SCT_BANK) {
		*bars += bank_income(sp, etu);
		*Nbars += sp->sct_item[I_BAR];
	    }
	}
    }
    tpops[player->cnum] = pop;
    upd_slmilcosts(etu, player->cnum);
    pay_reserve(np, etu);

    /* Maintain ships, planes and land units */
    prod_ship(etu, player->cnum, bp, 0);
    prod_plane(etu, player->cnum, bp, 0);
    prod_land(etu, player->cnum, bp, 0);

    /* Produce */
    produce_sect(np, etu, bp);

    /* Build ships, planes and land units */
    prod_ship(etu, player->cnum, bp, 1);
    prod_plane(etu, player->cnum, bp, 1);
    prod_land(etu, player->cnum, bp, 1);

    free(bp);
    return budget;
}

static char *
dotsprintf(char *buf, char *format, int data)
{
    sprintf(buf, format, data);
    return memset(buf, '.', strspn(buf, " "));
}
