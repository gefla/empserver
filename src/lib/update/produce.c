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
 *  produce.c: Produce goodies
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include <math.h>
#include "chance.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "product.h"
#include "prototypes.h"
#include "update.h"

static void materials_charge(struct pchrstr *, short *, double);

static char *levelnames[] = {
    "Technology", "Research", "Education", "Happiness"
};

void
produce(struct natstr *np, struct sctstr *sp)
{
    struct budget *budget = &nat_budget[sp->sct_own];
    struct pchrstr *product;
    double prodeff;
    double output;
    double cost;

    if (dchr[sp->sct_type].d_prd < 0)
	return;
    product = &pchr[dchr[sp->sct_type].d_prd];

    prodeff = prod_eff(sp->sct_type, np->nat_level[product->p_nlndx]);
    output = prod_output(sp, prodeff);
    if (!output)
	return;

    cost = product->p_cost * output / prodeff;
    if (opt_TECH_POP) {
	if (product->p_level == NAT_TLEV) {
	    if (budget->oldowned_civs > 50000)
		cost *= budget->oldowned_civs / 50000.0;
	}
    }

    budget->prod[sp->sct_type].count += ldround(output, 1);
    budget->prod[sp->sct_type].money -= cost;
    budget->money -= cost;
}

double
prod_output(struct sctstr *sp, double prodeff)
{
    struct pchrstr *product = &pchr[dchr[sp->sct_type].d_prd];
    i_type item = product->p_type;
    unsigned char *resource;
    double p_e;
    double material_limit, worker_limit, res_limit;
    double material_consume, output;
    int unit_work, work_used;
    int val;

    if (product->p_nrndx)
	resource = (unsigned char *)sp + product->p_nrndx;
    else
	resource = NULL;

    material_limit = prod_materials_cost(product, sp->sct_item,
					 &unit_work);

    /* sector p.e. */
    p_e = sp->sct_effic / 100.0;
    if (resource) {
	unit_work++;
	p_e *= *resource / 100.0;
    }
    if (unit_work == 0)
	unit_work = 1;

    worker_limit = sp->sct_avail * p_e / unit_work;
    res_limit = prod_resource_limit(product, resource);

    material_consume = res_limit;
    if (material_consume > worker_limit)
	material_consume = worker_limit;
    if (material_consume > material_limit)
	material_consume = material_limit;
    if (CANT_HAPPEN(material_consume < 0.0))
	material_consume = 0.0;
    if (material_consume == 0.0)
	return 0.0;

    if (prodeff <= 0.0) {
	if (!player->simulation)
	    wu(0, sp->sct_own,
	       "%s level too low to produce in %s (need %d)\n",
	       levelnames[product->p_nlndx], ownxy(sp), product->p_nlmin);
	return 0.0;
    }

    /*
     * Adjust produced amount by commodity production ratio
     */
    output = material_consume * prodeff;
    if (item == I_NONE) {
	if (!player->simulation) {
	    levels[sp->sct_own][product->p_level] += output;
	    wu(0, sp->sct_own, "%s (%.2f) produced in %s\n",
	       product->p_name, output, ownxy(sp));
	}
    } else {
	output = floor(output);
	if (output > 999.0)
	    output = 999.0;
	if (sp->sct_item[item] + output > ITEM_MAX) {
	    output = ITEM_MAX - sp->sct_item[item];
	    if (sp->sct_own && !player->simulation)
		wu(0, sp->sct_own,
		   "%s production backlog in %s\n",
		   product->p_name, ownxy(sp));
	}
	material_consume = output / prodeff;
	sp->sct_item[item] += output;
    }

    /*
     * Reset produced amount by commodity production ratio
     */
    materials_charge(product, sp->sct_item, material_consume);
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

    if (CANT_HAPPEN(p_e <= 0.0))
	return 0.0;
    work_used = roundavg(unit_work * material_consume / p_e);
    if (CANT_HAPPEN(work_used > sp->sct_avail))
	work_used = sp->sct_avail;
    sp->sct_avail -= work_used;

    return output;
}

/*
 * Return how much of product @pp can be made from materials @vec[].
 * Store amount of work per unit in *@costp.
 */
double
prod_materials_cost(struct pchrstr *pp, short vec[], int *costp)
{
    double count, n;
    int cost, i;

    count = ITEM_MAX;
    cost = 0;
    for (i = 0; i < MAXPRCON; ++i) {
	if (!pp->p_camt[i])
	    continue;
	if (CANT_HAPPEN(pp->p_ctype[i] <= I_NONE || I_MAX < pp->p_ctype[i]))
	    continue;
	n = (double)vec[pp->p_ctype[i]] / pp->p_camt[i];
	if (n < count)
	    count = n;
	cost += pp->p_camt[i];
    }
    *costp = cost;
    return count;
}

static void
materials_charge(struct pchrstr *pp, short *vec, double count)
{
    int i;
    i_type item;
    double n;

    for (i = 0; i < MAXPRCON; ++i) {
	item = pp->p_ctype[i];
	if (!pp->p_camt[i])
	    continue;
	if (CANT_HAPPEN(item <= I_NONE || I_MAX < item))
	    continue;
	n = vec[item] - pp->p_camt[i] * count;
	if (CANT_HAPPEN(n < 0.0))
	    n = 0.0;
	vec[item] = roundavg(n);
    }
}

/*
 * Return how much of product @pp can be made from its resource.
 * If @pp depletes a resource, @resource must point to its value.
 */
double
prod_resource_limit(struct pchrstr *pp, unsigned char *resource)
{
    if (CANT_HAPPEN(pp->p_nrndx && !resource))
	return 0;
    if (resource && pp->p_nrdep != 0)
	return *resource * 100.0 / pp->p_nrdep;
    return ITEM_MAX;
}

/*
 * Return p.e. for sector type @type.
 * Zero means level is too low for production.
 * @level is the level affecting production.
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
