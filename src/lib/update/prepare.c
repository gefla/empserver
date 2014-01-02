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
 *  prepare.c: Perform prelimiary updates of sectors
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1992
 *     Steve McClure, 1997
 */

#include <config.h>

#include "budg.h"
#include "item.h"
#include "land.h"
#include "player.h"
#include "ship.h"
#include "update.h"

void
prepare_sects(int etu, struct bp *bp)
{
    struct sctstr *sp;
    struct natstr *np;
    int n, civ_tax, uw_tax, mil_pay;

    memset(levels, 0, sizeof(levels));

/* Process all the fallout. */
    if (opt_FALLOUT) {
	if (!player->simulation) {
	    /* First, we determine which sectors to process fallout in */
	    for (n = 0; NULL != (sp = getsectid(n)); n++)
		sp->sct_updated = sp->sct_fallout != 0;
	    /* Next, we process the fallout there */
	    for (n = 0; NULL != (sp = getsectid(n)); n++)
		if (sp->sct_updated)
		    do_fallout(sp, etu);
	    /* Next, we spread the fallout */
	    for (n = 0; NULL != (sp = getsectid(n)); n++)
		if (sp->sct_updated)
		    spread_fallout(sp, etu);
	    /* Next, we decay the fallout */
	    for (n = 0; NULL != (sp = getsectid(n)); n++)
		if (sp->sct_fallout)
		    decay_fallout(sp, etu);
	}
    }
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	sp->sct_updated = 0;

	if (sp->sct_type == SCT_WATER)
	    continue;
	bp_set_from_sect(bp, sp);
	np = getnatp(sp->sct_own);

	if (np->nat_stat != STAT_SANCT) {
	    guerrilla(sp);
	    do_plague(sp, np, etu);
	    tax(sp, np, etu, &pops[sp->sct_own], &civ_tax, &uw_tax,
		&mil_pay);
	    np->nat_money += civ_tax + uw_tax + mil_pay;
	    if (sp->sct_type == SCT_BANK)
		np->nat_money += bank_income(sp, etu);
	}
    }
    for (n = 0; NULL != (np = getnatp(n)); n++) {
	np->nat_money += upd_slmilcosts(np->nat_cnum, etu);
    }
}

void
tax(struct sctstr *sp, struct natstr *np, int etu, int *pop, int *civ_tax,
    int *uw_tax, int *mil_pay)
{
    *civ_tax = 0;
    *uw_tax = 0;
    *mil_pay = 0;

    if (!player->simulation)
	populace(np, sp, etu);
    *civ_tax = (int)(0.5 + sp->sct_item[I_CIVIL] * sp->sct_effic *
		     etu * money_civ / 100);
    /*
     * captured civs only pay 1/4 taxes
     */
    if (sp->sct_own != sp->sct_oldown)
	*civ_tax = *civ_tax / 4;
    *uw_tax = (int)(0.5 + sp->sct_item[I_UW] * sp->sct_effic *
		    etu * money_uw / 100);
    *mil_pay = sp->sct_item[I_MILIT] * etu * money_mil;

    /*
     * only non-captured civs add to census for nation
     */
    if (sp->sct_oldown == sp->sct_own)
	*pop += sp->sct_item[I_CIVIL];
}

int
upd_slmilcosts(natid n, int etu)
{
    struct shpstr *sp;
    struct lndstr *lp;
    int mil = 0;
    int totalmil = 0;
    int mil_pay = 0;
    int i;

    for (i = 0; NULL != (sp = getshipp(i)); i++) {
	if (!sp->shp_own || sp->shp_own != n)
	    continue;
	if ((mil = sp->shp_item[I_MILIT]) > 0)
	    totalmil += mil;
    }
    for (i = 0; NULL != (lp = getlandp(i)); i++) {
	if (!lp->lnd_own || lp->lnd_own != n)
	    continue;
	if ((mil = lp->lnd_item[I_MILIT]) > 0)
	    totalmil += mil;
    }
    mil_pay = totalmil * etu * money_mil;
    return mil_pay;
}

int
bank_income(struct sctstr *sp, int etu)
{
    return (int)(sp->sct_item[I_BAR] * etu * bankint * sp->sct_effic / 100);
}
