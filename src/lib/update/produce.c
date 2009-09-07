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
 *  produce.c: Produce goodies
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "budg.h"
#include "player.h"
#include "product.h"
#include "update.h"

static void materials_charge(struct pchrstr *, short *, int);
static int materials_cost(struct pchrstr *, short *, int *);

static char *levelnames[] = {
    "Technology", "Research", "Education", "Happiness"
};

int
produce(struct natstr *np, struct sctstr *sp, short *vec, int work,
	int desig, int neweff, int *cost, int *amount)
{
    struct pchrstr *product;
    double p_e;
    double prodeff;
    unsigned char *resource;
    double output;
    int actual;
    int unit_work;
    i_type item;
    int worker_limit;
    int material_limit;
    int material_consume;
    int val;

    if (dchr[desig].d_prd < 0)
	return 0;
    product = &pchr[dchr[desig].d_prd];
    item = product->p_type;
    if (product->p_nrndx)
	resource = (unsigned char *)sp + product->p_nrndx;
    else
	resource = NULL;
    *amount = 0;
    *cost = 0;

    if ((material_limit = materials_cost(product, vec, &unit_work)) <= 0)
	return 0;
    /*
     * calculate production efficiency.
     */
    p_e = neweff / 100.0;
    if (resource) {
	unit_work++;
	p_e *= *resource / 100.0;
    }
    /*
     * determine number that can be made with
     * the available workforce
     */
    if (unit_work == 0)
	unit_work = 1;
    material_consume = material_limit;
    worker_limit = roundavg(work * p_e / unit_work);
    if (material_consume > worker_limit)
	material_consume = worker_limit;
    if (resource && product->p_nrdep != 0) {
	if (*resource * 100 < product->p_nrdep * material_consume)
	    material_consume = *resource * 100 / product->p_nrdep;
    }
    if (material_consume == 0)
	return 0;
    prodeff = prod_eff(desig, np->nat_level[product->p_nlndx]);
    if (prodeff <= 0.0 && !player->simulation) {
	wu(0, sp->sct_own,
	   "%s level too low to produce in %s (need %d)\n",
	   levelnames[product->p_nlndx], ownxy(sp), product->p_nlmin);
	return 0;
    }
    /*
     * Adjust produced amount by commodity production ratio
     */
    output = material_consume * prodeff;
    if (item == I_NONE) {
	actual = ldround(output, 1);
	if (!player->simulation) {
	    levels[sp->sct_own][product->p_level] += output;
	    wu((natid)0, sp->sct_own, "%s (%.2f) produced in %s\n",
	       product->p_name, output, ownxy(sp));
	}
    } else {
	actual = roundavg(output);
	if (actual <= 0)
	    return 0;
	if (actual > 999) {
	    material_consume = roundavg(999.0 * material_consume / actual);
	    actual = 999;
	}
	if (vec[item] + actual > ITEM_MAX) {
	    material_consume = roundavg((double)(ITEM_MAX - vec[item])
					* material_consume / actual);
	    if (material_consume < 0)
		material_consume = 0;
	    actual = ITEM_MAX - vec[item];
	    if (sp->sct_own && !player->simulation)
		wu(0, sp->sct_own,
		   "%s production backlog in %s\n",
		   product->p_name, ownxy(sp));
	}
	vec[item] += actual;
    }
    /*
     * Reset produced amount by commodity production ratio
     */
    if (!player->simulation) {
	materials_charge(product, vec, material_consume);
	if (resource && product->p_nrdep != 0) {
	    /*
	     * lower natural resource in sector depending on
	     * amount produced
	     */
	    val = *resource - roundavg(product->p_nrdep *
				       material_consume / 100.0);
	    if (val < 0)
		val = 0;
	    *resource = val;
	}
    }
    *amount = actual;
    *cost = product->p_cost * material_consume;

    if (opt_TECH_POP) {
	if (product->p_level == NAT_TLEV) {
	    if (tpops[sp->sct_own] > 50000)
		*cost *= tpops[sp->sct_own] / 50000.0;
	}
    }

    /* The MIN() here is to take care of integer rounding errors */
    if (p_e > 0.0) {
	return MIN(work, (int)(unit_work * material_consume / p_e));
    }
    return 0;
}

static int
materials_cost(struct pchrstr *pp, short *vec, int *costp)
{
    int count;
    int cost;
    int i, n;

    count = 9999;
    cost = 0;
    for (i = 0; i < MAXPRCON; ++i) {
	if (!pp->p_camt[i])
	    continue;
	if (CANT_HAPPEN(pp->p_ctype[i] <= I_NONE || I_MAX < pp->p_ctype[i]))
	    continue;
	n = vec[pp->p_ctype[i]] / pp->p_camt[i];
	if (n < count)
	    count = n;
	cost += pp->p_camt[i];
    }
    *costp = cost;
    return count;
}

static void
materials_charge(struct pchrstr *pp, short *vec, int count)
{
    int i, n;
    i_type item;

    for (i = 0; i < MAXPRCON; ++i) {
	item = pp->p_ctype[i];
	if (!pp->p_camt[i])
	    continue;
	if (CANT_HAPPEN(item <= I_NONE || I_MAX < item))
	    continue;
	n = vec[item] - pp->p_camt[i] * count;
	if (CANT_HAPPEN(n < 0))
	    n = 0;
	vec[item] = n;
    }
}

/*
 * Return level p.e. for sector type TYPE.
 * Zero means level is too low for production.
 * LEVEL is the affecting production of PP; it must match PP->p_nlndx.
 */
double
prod_eff(int type, float level)
{
    double level_p_e;
    struct dchrstr *dp = &dchr[type];
    struct pchrstr *pp = &pchr[dp->d_prd];

    if (CANT_HAPPEN(dp->d_prd < 0))
	return 0.0;

    if (pp->p_nlndx < 0)
	level_p_e = 1.0;
    else {
	double delta = (double)level - (double)pp->p_nlmin;

	if (delta < 0.0)
	    return 0.0;
	if (CANT_HAPPEN(delta + pp->p_nllag <= 0))
	    return 0.0;
	level_p_e = delta / (delta + pp->p_nllag);
    }

    return level_p_e * dp->d_peffic * 0.01;
}
