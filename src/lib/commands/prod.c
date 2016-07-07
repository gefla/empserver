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
 *  prod.c: Calculate production levels
 *
 *  Known contributors to this file:
 *     David Muir Sharnoff, 1987
 *     Steve McClure, 1997-2000
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include <math.h>
#include "commands.h"
#include "item.h"
#include "optlist.h"
#include "product.h"
#include "update.h"

static void prprod(struct sctstr *, double, double, char,
		   double, double, double, char[], int[], int[], int);

int
count_pop(int n)
{
    int i;
    int pop = 0;
    struct sctstr *sp;

    for (i = 0; NULL != (sp = getsectid(i)); i++) {
	if (sp->sct_own != n)
	    continue;
	if (sp->sct_oldown != n)
	    continue;
	pop += sp->sct_item[I_CIVIL];
    }
    return pop;
}

int
prod(void)
{
    struct natstr *natp;
    struct sctstr sect, scratch_sect;
    struct nstr_sect nstr;
    struct pchrstr *pp;
    double p_e;
    double prodeff;
    int totpop;
    double cost;
    int i;
    int nsect;
    double real, maxr;
    double take, mtake;
    i_type it;
    unsigned char *resource;
    char cmnem[MAXPRCON];
    int cuse[MAXPRCON], cmax[MAXPRCON];
    char mnem;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    player->simulation = 1;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (sect.sct_off)
	    continue;

	natp = getnatp(sect.sct_own);
	do_feed(&sect, natp, etu_per_update, 1);
	buildeff(&sect);
	if (sect.sct_effic < 60)
	    continue;

	if (sect.sct_type == SCT_ENLIST) {
	    int maxmil;
	    int enlisted;

	    if (sect.sct_own != sect.sct_oldown)
		continue;
	    enlisted = 0;
	    maxmil = sect.sct_item[I_CIVIL] / 2 - sect.sct_item[I_MILIT];
	    if (maxmil > 0) {
		enlisted = (etu_per_update
			    * (10 + sect.sct_item[I_MILIT])
			    * 0.05);
		if (enlisted > maxmil)
		    enlisted = maxmil;
	    }
	    if (enlisted < 0)
		enlisted = 0;
	    prprod(&sect, sect.sct_effic / 100.0, 1.0,
		   ichr[I_MILIT].i_mnem, enlisted, maxmil, enlisted * 3,
		   "c\0\0", &enlisted, &enlisted, nsect++);
	    continue;
	}

	if (dchr[sect.sct_type].d_prd < 0)
	    continue;
	pp = &pchr[dchr[sect.sct_type].d_prd];
	if (pp->p_nrndx)
	    resource = (unsigned char *)&sect + pp->p_nrndx;
	else
	    resource = NULL;

	/* sector p.e. */
	p_e = sect.sct_effic / 100.0;
	if (resource)
	    p_e *= *resource / 100.0;

	prodeff = prod_eff(sect.sct_type, natp->nat_level[pp->p_nlndx]);

	scratch_sect = sect;
	real = prod_output(&scratch_sect, prodeff);

	scratch_sect = sect;
	for (i = 0; i < MAXPRCON; ++i)
	    scratch_sect.sct_item[pp->p_ctype[i]] = ITEM_MAX;
	scratch_sect.sct_item[pp->p_type] = 0;
	maxr = prod_output(&scratch_sect, prodeff);

	if (prodeff != 0) {
	    take = real / prodeff;
	    mtake = maxr / prodeff;
	} else
	    mtake = take = 0.0;

	cost = take * pp->p_cost;
	if (opt_TECH_POP) {
	    if (pp->p_level == NAT_TLEV) {
		totpop = count_pop(sect.sct_own);
		if (totpop > 50000)
		    cost *= totpop / 50000.0;
	    }
	}

	for (i = 0; i < MAXPRCON; ++i) {
	    cmnem[i] = cuse[i] = cmax[i] = 0;
	    if (!pp->p_camt[i])
		continue;
	    it = pp->p_ctype[i];
	    if (CANT_HAPPEN(it <= I_NONE || I_MAX < it))
		continue;
	    cmnem[i] = ichr[it].i_mnem;
	    cuse[i] = (int)ceil(take * pp->p_camt[i]);
	    cmax[i] = (int)ceil(mtake * pp->p_camt[i]);
	}

	if (pp->p_type != I_NONE)
	    mnem = ichr[pp->p_type].i_mnem;
	else if (pp->p_level == NAT_TLEV || pp->p_level == NAT_RLEV)
	    mnem = '.';
	else
	    mnem = 0;
	prprod(&sect, p_e, prodeff, mnem, real, maxr, cost,
	       cmnem, cuse, cmax, nsect++);
    }
    player->simulation = 0;
    if (nsect == 0) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return RET_OK;
}

static void
prprod(struct sctstr *sp, double p_e, double prodeff,
       char mnem, double make, double max, double cost,
       char cmnem[], int cuse[], int cmax[], int nsect)
{
    int i;

    if (nsect == 0) {
	pr("PRODUCTION SIMULATION\n");
	pr("   sect  des eff avail  make p.e. cost   use1 use2 use3  max1 max2 max3   max\n");
    }

    prxy("%4d,%-4d", sp->sct_x, sp->sct_y);
    pr(" %c %3.0f%% %5d",
       dchr[sp->sct_type].d_mnem, p_e * 100.0, sp->sct_avail);
    if (mnem == '.')
	pr(" %5.2f", make);
    else
	pr(" %4.0f%c", make, mnem ? mnem : ' ');
    pr(" %.2f $%-5.0f", prodeff, cost);
    for (i = 0; i < 3; i++) {
	if (i < MAXPRCON && cmnem[i])
	    pr("%4d%c", cuse[i], cmnem[i]);
	else
	    pr("     ");
    }
    pr(" ");
    for (i = 0; i < 3; i++) {
	if (i < MAXPRCON && cmnem[i])
	    pr("%4d%c", cmax[i], cmnem[i]);
	else
	    pr("     ");
    }
    if (mnem == '.')
	pr(" %5.2f\n", max);
    else
	pr(" %5.0f\n", max);
}
