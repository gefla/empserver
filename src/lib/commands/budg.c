/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 */

#include <config.h>

#include <string.h>
#include "misc.h"
#include "player.h"
#include "xy.h"
#include "nsc.h"
#include "sect.h"
#include "product.h"
#include "nat.h"
#include "item.h"
#include "file.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "optlist.h"
#include "budg.h"
#include "commands.h"

static void calc_all(long int (*p_sect)[2], int *taxes, int *Ncivs,
		     int *Nuws, int *bars, int *Nbars, int *mil,
		     int *ships, int *sbuild, int *nsbuild, int *smaint,
		     int *units, int *lbuild, int *nlbuild, int *lmaint,
		     int *planes, int *pbuild, int *npbuild, int *pmaint);
static int change_prio(struct natstr *np, char code, char *newval);
static char *dotsprintf(char *buf, char *format, int data);
static void prexpense(long int cash, int *expensesp, int priority, int amount);

int
budg(void)
{
    int i, res;
    long p_sect[SCT_MAXDEF+1][2];
    int taxes, Ncivs, Nuws, bars, Nbars, mil;
    int ships, sbuild, nsbuild, smaint;
    int units, lbuild, nlbuild, lmaint;
    int planes, pbuild, npbuild, pmaint;
    int n, etu;
    int income, expenses;
    struct natstr *np;
    char buf[1024];
    char in[80];

    etu = etu_per_update;

    np = getnatp(player->cnum);
    if (player->argp[1]) {
	res = change_prio(np, player->argp[1][0], player->argp[2]);
	if (res != RET_OK) {
	    putnat(np);
	    return res;
	}
    }

    player->simulation = 1;
    calc_all(p_sect,
	     &taxes, &Ncivs, &Nuws, &bars, &Nbars, &mil,
	     &ships, &sbuild, &nsbuild, &smaint,
	     &units, &lbuild, &nlbuild, &lmaint,
	     &planes, &pbuild, &npbuild, &pmaint);

    income = taxes + bars;
    expenses = 0;
    pr("Sector Type\t\tAbbr\tProduction\tPriority\t    Cost\n");
    for (i = 0; i <= SCT_MAXDEF; i++) {
	if (!p_sect[i][1] && np->nat_priorities[i] == -1)
	    continue;
	if (!pchr[dchr[i].d_prd].p_cost &&
	    np->nat_priorities[i] == -1 && i != SCT_ENLIST)
	    continue;

	pr("%-17s\t%c\t", dchr[i].d_name, dchr[i].d_mnem);
	if (i == SCT_ENLIST)
	    pr("%ld mil    \t", p_sect[i][0]);
	else if (pchr[dchr[i].d_prd].p_cost != 0)
	    pr("%ld %-7s\t", p_sect[i][0], pchr[dchr[i].d_prd].p_sname);
	else
	    pr("\t\t");

	if (np->nat_priorities[i] != -1)
	    pr("%d", np->nat_priorities[i]);
	prexpense(np->nat_money + income, &expenses,
		  np->nat_priorities[i], p_sect[i][1]);
    }

    if (lbuild) {
	sprintf(buf, "%d unit%s", nlbuild, splur(nlbuild));
	pr("Unit building\t\tL\t%-16s", buf);
	if (np->nat_priorities[PRI_LBUILD] != -1)
	    pr("%d", np->nat_priorities[PRI_LBUILD]);
	prexpense(np->nat_money + income, &expenses,
		  np->nat_priorities[PRI_LBUILD], -1 * lbuild);
    }

    if (lmaint) {
	sprintf(buf, "%d unit%s", units, splur(units));
	pr("Unit maintenance\tA\t%-16s", buf);
	if (np->nat_priorities[PRI_LMAINT] != -1)
	    pr("%d", np->nat_priorities[PRI_LMAINT]);
	prexpense(np->nat_money + income, &expenses,
		  np->nat_priorities[PRI_LMAINT], -1 * lmaint);
    }

    if (sbuild) {
	sprintf(buf, "%d ship%s", nsbuild, splur(nsbuild));
	pr("Ship building\t\tS\t%-16s", buf);
	if (np->nat_priorities[PRI_SBUILD] != -1)
	    pr("%d", np->nat_priorities[PRI_SBUILD]);
	prexpense(np->nat_money + income, &expenses,
		  np->nat_priorities[PRI_SBUILD], -1 * sbuild);
    }

    if (smaint) {
	sprintf(buf, "%d ship%s", ships, splur(ships));
	pr("Ship maintenance\tM\t%-16s", buf);
	if (np->nat_priorities[PRI_SMAINT] != -1)
	    pr("%d", np->nat_priorities[PRI_SMAINT]);
	prexpense(np->nat_money + income, &expenses,
		  np->nat_priorities[PRI_SMAINT], -1 * smaint);
    }

    if (pbuild) {
	sprintf(buf, "%d plane%s", npbuild, splur(npbuild));
	pr("Plane building\t\tP\t%-16s", buf);
	if (np->nat_priorities[PRI_PBUILD] != -1)
	    pr("%d", np->nat_priorities[PRI_PBUILD]);
	prexpense(np->nat_money + income, &expenses,
		  np->nat_priorities[PRI_PBUILD], -1 * pbuild);
    }

    if (pmaint) {
	sprintf(buf, "%d plane%s", planes, splur(planes));
	pr("Plane maintenance\tN\t%-16s", buf);
	if (np->nat_priorities[PRI_PMAINT] != -1)
	    pr("%d", np->nat_priorities[PRI_PMAINT]);
	prexpense(np->nat_money + income, &expenses,
		  np->nat_priorities[PRI_PMAINT], -1 * pmaint);
    }
    if (p_sect[SCT_EFFIC][1]) {
	pr("Sector building\t\t\t\t%8ld sct(s)\t\t%8ld\n",
	   p_sect[SCT_EFFIC][0], p_sect[SCT_EFFIC][1]);
	expenses += p_sect[SCT_EFFIC][1];
    }
    if (mil) {
	n = (mil - np->nat_reserve * money_res * etu) / (etu * money_mil);
	sprintf(in, "%d mil, %d res", n, (int)np->nat_reserve);
	pr("Military payroll\t\t%-32s%8d\n", in, -mil);
	expenses -= mil;
    }
    if (p_sect[SCT_CAPIT][0]) {
	pr("%c%s maintenance\t\t",
	   toupper(dchr[SCT_CAPIT].d_name[0]),
	   dchr[SCT_CAPIT].d_name + 1);
	n = p_sect[SCT_CAPIT][0];
	sprintf(in, "%d %s", n, dchr[SCT_CAPIT].d_name);
	plurize(in, sizeof(in), n);
	pr("%-32s%8ld\n", in, p_sect[SCT_CAPIT][1]);
	expenses += p_sect[SCT_CAPIT][1];
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
    pr("Balance forward\t\t\t\t\t\t      %10ld\n", np->nat_money);
    pr("Estimated delta\t\t\t\t\t\t      %+10d\n", income - expenses);
    pr("Estimated new treasury%s\n",
       dotsprintf(buf, "%50d", np->nat_money + income - expenses));
    if (((np->nat_money + income - expenses) < 0) && !player->god) {
	pr("After processsing sectors, you will be broke!\n");
	pr("Sectors will not produce, distribute, or deliver!\n\n");
    }

    player->simulation = 0;
    return RET_OK;
}

static void
calc_all(long p_sect[][2],
	 int *taxes, int *Ncivs, int *Nuws, int *bars, int *Nbars, int *mil,
	 int *ships, int *sbuild, int *nsbuild, int *smaint,
	 int *units, int *lbuild, int *nlbuild, int *lmaint,
	 int *planes, int *pbuild, int *npbuild, int *pmaint)
{
    int y, z;
    struct natstr *np;
    int sm = 0, sb = 0, pm = 0, pb = 0, lm = 0, lb = 0;
    int *bp;
    long pop = 0;
    int n, civ_tax, uw_tax, mil_pay;
    struct sctstr *sp;
    int etu = etu_per_update;
    long tmp_money;

    lnd_money[player->cnum] = sea_money[player->cnum] = 0;
    air_money[player->cnum] = 0;
    mil_dbl_pay = 0;
    memset(p_sect, 0, sizeof(**p_sect) * (SCT_MAXDEF+1) * 2);
    *taxes = *Ncivs = *Nuws = *bars = *Nbars = *mil = 0;
    *ships = *sbuild = *nsbuild = *smaint = 0;
    *units = *lbuild = *nlbuild = *lmaint = 0;
    *planes = *pbuild = *npbuild = *pmaint = 0;
    
    np = getnatp(player->cnum);
    bp = calloc(WORLD_X * WORLD_Y * 8, sizeof(int));
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	fill_update_array(bp, sp);
	if (sp->sct_own == player->cnum) {
	    sp->sct_updated = 0;
	    tax(sp, np, etu, &pop, &civ_tax, &uw_tax, &mil_pay);
	    *Ncivs += sp->sct_item[I_CIVIL];
	    *Nuws += sp->sct_item[I_UW];
	    *taxes += civ_tax + uw_tax;
	    *mil += mil_pay;
	    if (sp->sct_type == SCT_BANK) {
		*bars += bank_income(sp, etu);
		*Nbars += sp->sct_item[I_BAR];
	    }
	}
    }
    tpops[player->cnum] = pop;
    *mil += (int)(np->nat_reserve * money_res * etu);

    *mil += (int)upd_slmilcosts(np->nat_cnum, etu);

    for (y = 1; y <= PRI_MAX; y++) {
	for (z = 0; z <= PRI_MAX; z++)
	    if (np->nat_priorities[z] == y)
		switch (z) {
		case PRI_SMAINT:
		    tmp_money = lnd_money[player->cnum];
		    *ships = prod_ship(etu, player->cnum, bp, 0);
		    *smaint = lnd_money[player->cnum] - tmp_money;
		    sm = 1;
		    break;
		case PRI_SBUILD:
		    tmp_money = sea_money[player->cnum];
		    *nsbuild = prod_ship(etu, player->cnum, bp, 1);
		    *sbuild = sea_money[player->cnum] - tmp_money;
		    sb = 1;
		    break;
		case PRI_LMAINT:
		    tmp_money = lnd_money[player->cnum];
		    *units = prod_land(etu, player->cnum, bp, 0);
		    *lmaint = lnd_money[player->cnum] - tmp_money;
		    lm = 1;
		    break;
		case PRI_LBUILD:
		    tmp_money = lnd_money[player->cnum];
		    *nlbuild = prod_land(etu, player->cnum, bp, 1);
		    *lbuild = lnd_money[player->cnum] - tmp_money;
		    lb = 1;
		    break;
		case PRI_PMAINT:
		    tmp_money = air_money[player->cnum];
		    *planes = prod_plane(etu, player->cnum, bp, 0);
		    *pmaint = air_money[player->cnum] - tmp_money;
		    pm = 1;
		    break;
		case PRI_PBUILD:
		    tmp_money = air_money[player->cnum];
		    *npbuild = prod_plane(etu, player->cnum, bp, 1);
		    *pbuild = air_money[player->cnum] - tmp_money;
		    pb = 1;
		    break;
		default:
		    produce_sect(player->cnum, etu, bp, p_sect, z);
		    break;
		}
    }
    /* 0 is maintain, 1 is build */
    if (!sm) {
	tmp_money = sea_money[player->cnum];
	*ships = prod_ship(etu, player->cnum, bp, 0);
	*smaint = sea_money[player->cnum] - tmp_money;
    }
    if (!sb) {
	tmp_money = sea_money[player->cnum];
	*nsbuild = prod_ship(etu, player->cnum, bp, 1);
	*sbuild = sea_money[player->cnum] - tmp_money;
    }
    if (!lm) {
	tmp_money = lnd_money[player->cnum];
	*units = prod_land(etu, player->cnum, bp, 0);
	*lmaint = lnd_money[player->cnum] - tmp_money;
    }
    if (!lb) {
	tmp_money = lnd_money[player->cnum];
	*nlbuild = prod_land(etu, player->cnum, bp, 1);
	*lbuild = lnd_money[player->cnum] - tmp_money;
    }
    if (!pm) {
	tmp_money = air_money[player->cnum];
	*planes = prod_plane(etu, player->cnum, bp, 0);
	*pmaint = air_money[player->cnum] - tmp_money;
    }
    if (!pb) {
	tmp_money = air_money[player->cnum];
	*npbuild = prod_plane(etu, player->cnum, bp, 1);
	*pbuild = air_money[player->cnum] - tmp_money;
    }

    /* produce all sects that haven't produced yet */
    produce_sect(player->cnum, etu, bp, p_sect, -1);

    lnd_money[player->cnum] = sea_money[player->cnum] = 0;
    air_money[player->cnum] = 0;
    free(bp);
}

static int
change_prio(struct natstr *np, char code, char *newval)
{
    int idx, i, prio;
    char *p;
    char buf[1024];

    switch (code) {
    case 'P':
	idx = PRI_PBUILD;
	break;
    case 'S':
	idx = PRI_SBUILD;
	break;
    case 'L':
	idx = PRI_LBUILD;
	break;
    case 'A':
	idx = PRI_LMAINT;
	break;
    case 'M':
	idx = PRI_SMAINT;
	break;
    case 'N':
	idx = PRI_PMAINT;
	break;
    case 'C':
	for (i = 0; i <= PRI_MAX; ++i)
	    np->nat_priorities[i] = -1;
	return RET_OK;
    default:
	idx = sct_typematch(player->argp[1]);
	if (idx < 0 || idx == SCT_CAPIT)
	    return RET_SYN;
    }

    if (!(p = getstarg(newval, "Priority? ", buf)))
	return RET_SYN;
    if (isdigit(p[0])) {
	prio = atoi(p);
	if (prio < 0 || PRI_MAX < prio) {
	    pr("Priorities must be between 0 and %d!\n", PRI_MAX);
	    return RET_FAIL;
	}
	for (i = 0; i <= PRI_MAX; i++) {
	    if (i != idx && prio && np->nat_priorities[i] == prio) {
		pr("Priorities must be unique!\n");
		return RET_FAIL;
	    }
	}
    } else if (p[0] == '~')
	prio = -1;
    else
	return RET_SYN;

    np->nat_priorities[idx] = prio;

    return RET_OK;
}


static char *
dotsprintf(char *buf, char *format, int data)
{
    sprintf(buf, format, data);
    return memset(buf, '.', strspn(buf, " "));
}

static void
prexpense(long int cash, int *expensesp, int priority, int amount)
{
    if (cash > *expensesp) {
	if (priority) {
	    pr("\t\t%8d\n", amount);
	    *expensesp += amount;
	} else
	    pr("\t\t(%7d)\n", amount);
    } else {
	if (priority) {
	    pr("\t\t[%7d]\n", amount);
	    *expensesp += amount;
	} else
	    pr("\t\t[(%6d)]\n", amount);
    }
}
