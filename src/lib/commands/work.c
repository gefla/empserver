/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  work.c: Implementation of the work command
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2008
 */

#include <config.h>

#include "commands.h"
#include "land.h"
#include "optlist.h"

static int buildeff(struct sctstr *, int, double *);

int
work(void)
{
    int nunits;
    struct nstr_item ni;
    struct sctstr sect;
    struct lndstr land;
    int work_amt, eff_amt, w;
    char *p;
    char buf[1024];
    double cost;
    struct natstr *natp = getnatp(player->cnum);

    if (!snxtitem(&ni, EF_LAND, player->argp[1], NULL))
	return RET_SYN;
    p = getstarg(player->argp[2], "Amount: ", buf);
    if (!p || !*p)
	return RET_SYN;
    work_amt = atoi(p);
    if ((work_amt < 0) || (work_amt > land_mob_max)) {
	pr("Mobility used must be from 0 to %d\n", land_mob_max);
	return RET_FAIL;
    }
    nunits = 0;
    while (nxtitem(&ni, &land)) {
	if (!player->owner || land.lnd_own == 0)
	    continue;
	if (!(lchr[(int)land.lnd_type].l_flags & L_ENGINEER))
	    continue;
	if (land.lnd_mobil <= 0) {
	    pr("%s has no mobility!\n", prland(&land));
	    continue;
	}
	getsect(land.lnd_x, land.lnd_y, &sect);
	if (sect.sct_effic >= 100 && sect.sct_type == sect.sct_newtype) {
	    pr("Nothing to do for %s in %s\n",
	       prland(&land), xyas(sect.sct_x, sect.sct_y, player->cnum));
	    continue;
	}
	eff_amt = MIN(land.lnd_mobil, work_amt);
	w = (eff_amt * land.lnd_effic) / 600;
	if (w < 1) {
	    pr("%s doesn't work enough to change efficiency (try increasing amount)\n",
	       prland(&land));
	    continue;
	}
	cost = 0.0;
	w = buildeff(&sect, w, &cost);
	if (w == 0) {
	    pr("%s can't change efficiency in %s\n",
	       prland(&land), xyas(land.lnd_x, land.lnd_y, player->cnum));
	    continue;
	}
	if (player->dolcost + cost > natp->nat_money) {
	    pr("You can't afford to work that much in %s!\n",
	       xyas(land.lnd_x, land.lnd_y, player->cnum));
	    break;
	}
	player->dolcost += cost;
	land.lnd_mission = 0;
	land.lnd_mobil -= roundavg(w * 600.0 / land.lnd_effic);
	nunits++;
	pr("%s %s efficiency at %s to %d\n",
	   prland(&land),
	   sect.sct_type == sect.sct_newtype ? "raised" : "lowered",
	   xyas(land.lnd_x, land.lnd_y, player->cnum),
	   sect.sct_effic);
	putland(land.lnd_uid, &land);
	putsect(&sect);
    }
    if (nunits == 0) {
	if (player->argp[1])
	    pr("%s: No unit(s)\n", player->argp[1]);
	else
	    pr("%s: No unit(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d unit%s\n", nunits, splur(nunits));
    return RET_OK;
}

static int
buildeff(struct sctstr *sp, int work, double *money)
{
    int work_cost;
    int n, hcms, lcms;
    int effdone = 0;

    work_cost = 0;
    if (sp->sct_type != sp->sct_newtype) {
	/*
	 * Tear down existing sector.
	 * Easier to destroy than to build.
	 */
	work_cost = (sp->sct_effic + 3) / 4;
	if (work_cost > work)
	    work_cost = work;
	n = sp->sct_effic - work_cost * 4;
	if (n <= 0) {
	    n = 0;
	    sp->sct_type = sp->sct_newtype;
	}
	sp->sct_effic = n;
	work -= work_cost;
	*money += work_cost;
	effdone += work_cost;
    }
    if (sp->sct_type == sp->sct_newtype) {
	work_cost = 100 - sp->sct_effic;
	if (work_cost > work)
	    work_cost = work;

	if (dchr[sp->sct_type].d_lcms > 0) {
	    lcms = sp->sct_item[I_LCM];
	    lcms /= dchr[sp->sct_type].d_lcms;
	    if (work_cost > lcms)
		work_cost = lcms;
	}
	if (dchr[sp->sct_type].d_hcms > 0) {
	    hcms = sp->sct_item[I_HCM];
	    hcms /= dchr[sp->sct_type].d_hcms;
	    if (work_cost > hcms)
		work_cost = hcms;
	}

	sp->sct_effic += work_cost;
	*money += work_cost * dchr[sp->sct_type].d_build;

	if ((dchr[sp->sct_type].d_lcms > 0) ||
	    (dchr[sp->sct_type].d_hcms > 0)) {
	    sp->sct_item[I_LCM] -= work_cost * dchr[sp->sct_type].d_lcms;
	    sp->sct_item[I_HCM] -= work_cost * dchr[sp->sct_type].d_hcms;
	}
	effdone += work_cost;
    }
    return effdone;
}
