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
 *  prod.c: Calculate production levels
 * 
 *  Known contributors to this file:
 *     David Muir Sharnoff, 1987
 *     Steve McClure, 1997-2000
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "nsc.h"
#include "sect.h"
#include "product.h"
#include "nat.h"
#include "item.h"
#include "file.h"
#include "optlist.h"
#include "commands.h"

int
count_pop(register int n)
{
    register int i;
    register int pop = 0;
    struct sctstr *sp;
    int vec[I_MAX + 1];

    for (i = 0; NULL != (sp = getsectid(i)); i++) {
	if (sp->sct_own != n)
	    continue;
	if (sp->sct_oldown != n)
	    continue;
	if (getvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR) <= 0)
	    continue;
	pop += vec[I_CIVIL];
    }
    return pop;
}

int
prod(void)
{
    struct natstr *natp;
    struct sctstr sect;
    struct nstr_sect nstr;
    struct pchrstr *pp;
    double effic;
    double maxr;		/* floating version of max */
    double prodeff;
    double real;		/* floating pt version of act */
    double work;
    int totpop;
    int act;			/* actual production */
    int cost;
    int i;
    int max;			/* production w/infinate materials */
    double maxtake;
    int nsect;
    double take;
    double mtake;
    int there;
    int totcomp;		/* sum of component amounts */
    int used;			/* production w/infinite workforce */
    int wforce;
    int it;
    u_short *amount;		/* amount for component pointer */
    u_char *comp;		/* component pointer */
    u_char *endcomp;
    u_char vtype;
    s_char *resource;
    int c;
    s_char maxc[3][10];
    s_char use[3][10];
    int items[I_MAX + 1];
    int vec[I_MAX + 1], lcms, hcms;
    int civs = 0;
    int uws = 0;
    int bwork;
    int twork;
    int type;
    int eff;
    int maxpop;
    u_char otype;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    player->simulation = 1;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	getvec(VT_ITEM, items, (s_char *)&sect, EF_SECTOR);

	civs = min(9999, (int)((obrate * (double)etu_per_update + 1.0)
			       * (double)items[I_CIVIL]));
	uws = min(9999, (int)((uwbrate * (double)etu_per_update + 1.0)
			      * (double)items[I_UW]));
	if (opt_RES_POP) {
	    natp = getnatp(sect.sct_own);
	    maxpop = max_pop((float)natp->nat_level[NAT_RLEV], &sect);
	    civs = min(civs, maxpop);
	    uws = min(uws, maxpop);
	} else {		/* now RES_POP */
	    civs = min(999, civs);
	    uws = min(999, uws);
	}			/* end RES_POP */

	/* This isn't quite right, since research might rise/fall */
	/* during the update, but it's the best we can really do  */
	wforce = (int)(((double)civs * (double)sect.sct_work)
		       / 100.0 + (double)uws +
		       (double)items[I_MILIT] * 2.0 / 5.0);
	work = (double)etu_per_update *(double)wforce / 100.0;
 	if (opt_ROLLOVER_AVAIL) {
	    if (sect.sct_type == sect.sct_newtype) {
		work += sect.sct_avail;
	    }
	    if (work > 999) work = 999;
 	}
	bwork = (int)((double)work / 2.0);

	if (sect.sct_off)
	    continue;
	type = sect.sct_type;
	eff = sect.sct_effic;
	if (sect.sct_newtype != type) {
	    twork = (eff + 3) / 4;
	    if (twork > bwork) {
		twork = bwork;
	    }
	    bwork -= twork;
	    eff -= twork * 4;
	    otype = type;
	    if (eff <= 0) {
		type = sect.sct_newtype;
		eff = 0;
	    }
	    if (opt_BIG_CITY) {
		if (!eff && dchr[otype].d_pkg == UPKG &&
		    dchr[type].d_pkg != UPKG) {
		    if (opt_RES_POP) {
			natp = getnatp(sect.sct_own);
			civs =
			    min(civs,
				max_pop(natp->nat_level[NAT_RLEV], 0));
			uws =
			    min(uws,
				max_pop(natp->nat_level[NAT_RLEV], 0));
		    } else {
			civs = min(9999, civs);
			uws = min(9999, uws);
		    }
		    wforce =
			(int)((civs * sect.sct_work) / 100.0 + uws +
			      items[I_MILIT] * 2 / 5.0);
		    work = etu_per_update * wforce / 100.0;
		    bwork = min((int)(work / 2), bwork);
		}
	    }
	    twork = 100 - eff;
	    if (twork > bwork) {
		twork = bwork;
	    }
	    getvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
	    if (dchr[type].d_lcms > 0) {
		lcms = vec[I_LCM];
		lcms /= dchr[type].d_lcms;
		if (twork > lcms)
		    twork = lcms;
	    }
	    if (dchr[type].d_hcms > 0) {
		hcms = vec[I_HCM];
		hcms /= dchr[type].d_hcms;
		if (twork > hcms)
		    twork = hcms;
	    }
	    bwork -= twork;
	    eff += twork;
	} else if (eff < 100) {
	    twork = 100 - eff;
	    if (twork > bwork) {
		twork = bwork;
	    }
	    bwork -= twork;
	    eff += twork;
	}
	work = work / 2 + bwork;
	if (eff < 60 || (type != SCT_ENLIST && eff < 61))
	    continue;

	effic = eff / 100.0;
	if (effic > 1.0)
	    effic = 1.0;

	if (dchr[type].d_prd == 0 && type != SCT_ENLIST)
	    continue;
	totcomp = 0;
	pp = &pchr[dchr[type].d_prd];
	vtype = pp->p_type;
	natp = getnatp(sect.sct_own);
	/*
	 * sect effic  (inc improvements)
	 */
	if (type == SCT_ENLIST && sect.sct_own != sect.sct_oldown)
	    continue;
	if (type == SCT_ENLIST)
	    goto is_enlist;
	if (pp->p_nrndx != 0) {
	    totcomp++;
	    resource = ((s_char *)&sect) + pp->p_nrndx;
	    effic = (*resource * effic) / 100.0;
	    if (pp->p_nrdep > 0) {
		maxtake = (*resource * 100.0) / pp->p_nrdep;
		if (effic > maxtake)
		    effic = maxtake;
	    }
	}
	/*
	 * production effic.
	 */
	if (pp->p_nlndx >= 0) {
	    prodeff = natp->nat_level[pp->p_nlndx] - pp->p_nlmin;
	    if (prodeff < 0.0) {
		prodeff = 0.0;
	    }
	    prodeff = prodeff / (prodeff + pp->p_nllag);
	} else {
	    prodeff = 1.0;
	}
	used = 999;
	comp = pp->p_vtype;
	endcomp = pp->p_vtype + pp->p_nv;
	amount = pp->p_vamt;
	while (comp < endcomp) {
	    if (*amount == 0)
		totcomp++;
	    else {
		used = min(used, sect.sct_item[(int)*comp] / *amount);
		totcomp += *amount;
	    }
	    ++comp;
	    ++amount;
	}
	if (totcomp == 0)
	    continue;
	/*
	 * is production limited by resources or
	 * workforce?
	 */
	max = (int)(work * effic / (double)totcomp) + 0.5;
	act = min(used, max);
	/*
	 * some things are easier to make..  food,
	 * pet, etc.
	 */
	act = (int)(((double)pp->p_effic * 0.01 * (double)act) + 0.5);
	max = (int)(((double)pp->p_effic * 0.01 * (double)max) + 0.5);

	real = (double)act *(double)prodeff;
	maxr = (double)max *(double)prodeff;

	if (vtype != 0) {
	    if (real < 0.0)
		real = 0.0;
	    /* production backlog? */
	    if ((there = sect.sct_item[vtype]) >= 9999) {
		there = 9999;
	    }
	    act = min(act, (9999 - there));
	    max = min(max, (9999 - there));
	}

	if (prodeff != 0) {
	    take = real / prodeff;
	    mtake = maxr / prodeff;
	} else
	    mtake = take = 0.0;

	if (take > 999.0)
	    take = 999.0;
	if (mtake > 999.0)
	    mtake = 999.0;

	take = (double)take / ((double)pp->p_effic * 0.01);
	mtake = (double)mtake / ((double)pp->p_effic * 0.01);

	cost = (int)(take * (double)pp->p_cost);
	if (opt_TECH_POP) {
	    if (pp->p_level == NAT_TLEV) {
		totpop = count_pop(sect.sct_own);
		if (totpop > 50000)
		    cost = (int)((double)cost * (double)totpop / 50000.0);
	    }
	}

	comp = pp->p_vtype;
	amount = pp->p_vamt;
	i = 0;
	while (comp < endcomp) {
	    it = unitem((int)*comp);
	    if (it > 0 && it <= I_MAX && ichr[it].i_name != 0)
		c = ichr[it].i_name[0];
	    else
		c = ' ';
	    (void)sprintf(use[i], " %3d%c",
			  (int)((take * (double)(*amount)) + 0.5), c);
	    (void)sprintf(maxc[i], " %3d%c",
			  (int)((mtake * (double)(*amount)) + 0.5), c);
	    ++comp;
	    ++amount;
	    ++i;
	}
	while (i < 3) {
	    strcpy(use[i], "     ");
	    strcpy(maxc[i], "     ");
	    ++i;
	}

      is_enlist:

	if (nsect++ == 0) {
	    pr("PRODUCTION SIMULATION\n");
	    pr("   sect  des eff wkfc will make- p.e. cost  use1 use2 use3  max1 max2 max3  max\n");
	}

	prxy("%4d,%-4d", nstr.x, nstr.y, player->cnum);
	pr(" %c", dchr[type].d_mnem);
	pr(" %3.0f%%", effic * 100.0);

	pr(" %4d", wforce);
	if (vtype != 0) {
	    pr(" %4d", (int)(real + 0.5));
	} else if (type != SCT_ENLIST) {
	    switch (pp->p_level) {
	    case NAT_TLEV:
	    case NAT_RLEV:
		pr(" %1.2f", real);
		break;
	    case NAT_ELEV:
	    case NAT_HLEV:
		pr(" %4.0f", real);
		break;
	    default:
		pr("ERROR");
		break;
	    }
	} else {
	    int maxmil;
	    int enlisted;
	    int civs;

	    civs = min(999, (int)((obrate * (double)etu_per_update + 1.0)
				  * (double)items[I_CIVIL]));
	    natp = getnatp(sect.sct_own);
	    maxpop = max_pop((float)natp->nat_level[NAT_RLEV], &sect);
	    civs = min(civs, maxpop);
	    /* This isn't quite right, since research might
	       rise/fall during the update, but it's the best
	       we can really do  */
	    enlisted = 0;
	    maxmil = (civs / 2) - items[I_MILIT];
	    if (maxmil > 0) {
		enlisted = (etu_per_update * (10 + items[I_MILIT]) * 0.05);
		if (enlisted > maxmil)
		    enlisted = maxmil;
	    }
	    if (enlisted < 0)
		enlisted = 0;
	    if (natp->nat_priorities[type] == 0) {
		maxmil = 0;
	    }
	    pr(" %4d mil   1.00 $%-5d%3dc",
	       enlisted, enlisted * 3, enlisted);
	    pr("            %3dc           %4d\n",
	       enlisted, maxmil, maxmil);
	    continue;
	}

	pr(" %-5.5s", pp->p_sname);
	prodeff = prodeff * (double)pp->p_effic * 0.01;
	pr(" %.2f", prodeff);
	pr(" $%-4d", cost);
	for (i = 0; i < 3; i++) {
	    pr(use[i]);
	}
	pr(" ");
	for (i = 0; i < 3; i++) {
	    pr(maxc[i]);
	}
	if (natp->nat_priorities[type] == 0) {
	    max = 0;
	    maxr = 0;
	}
	if (vtype != 0 || pp->p_level == NAT_ELEV
	    || pp->p_level == NAT_HLEV)
	    pr(" %4d\n", min(999, (int)(max * prodeff + 0.05)));
	else
	    pr(" %1.2f\n", maxr);
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
