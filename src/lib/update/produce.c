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
 *  produce.c: Produce goodies
 * 
 *  Known contributors to this file:
 *    
 */

#include "misc.h"
#include "var.h"
#include "sect.h"
#include "product.h"
#include "nat.h"
#include "file.h"
#include "xy.h"
#include "player.h"
#include "update.h"
#include "gen.h"
#include "subs.h"
#include "common.h"
#include "optlist.h"
#include "budg.h"

static void materials_charge(struct pchrstr *, short *, int);
static int materials_cost(struct pchrstr *, short *, int *);

s_char *levelnames[] =
    { "Technology", "Research", "Education", "Happiness" };

int
produce(struct natstr *np, struct sctstr *sp, short *vec, int work,
	int desig, int neweff, int *cost, int *amount)
{
    register struct pchrstr *product;
    int vtype;
    double p_e;
    double level_p_e;
    s_char *resource;
    int output;
    int actual;
    int unit_work;
    double depend;
    int item;
    int worker_limit;
    int material_limit;
    int material_consume;
    int val;

    product = &pchr[dchr[desig].d_prd];
    if (product == &pchr[0])
	return 0;
    vtype = product->p_type;
    item = vtype & ~VT_ITEM;
    *amount = 0;
    *cost = 0;

    if ((material_limit = materials_cost(product, vec, &unit_work)) <= 0)
	return 0;
    /*
     * calculate production efficiency.
     */
    p_e = neweff / 100.0;
    if (product->p_nrndx != 0) {
	unit_work++;
	resource = ((s_char *)sp) + product->p_nrndx;
	p_e = (*resource * p_e) / 100.0;
	if (product->p_nrdep > 0) {
	    /* XXX this looks way wrong */
	    depend = (*resource * 100.0) / product->p_nrdep;
	    if (p_e > depend)
		p_e = depend;
	}
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
    if (material_consume == 0)
	return 0;
    level_p_e = 1.0;
    if (product->p_nlndx >= 0) {
	level_p_e = np->nat_level[product->p_nlndx] - product->p_nlmin;
	if ((level_p_e < 0.0) && (!player->simulation)) {
	    wu(0, sp->sct_own,
	       "%s level too low to produce in %s (need %d)\n",
	       levelnames[product->p_nlndx], ownxy(sp), product->p_nlmin);
	    return 0;
	}
	level_p_e = level_p_e / (level_p_e + product->p_nllag);
    }
    /*
     * Adjust produced amount by commodity production ratio
     */
    output = roundavg(product->p_effic * 0.01 * material_consume);
    if ((vtype == 0) && (!player->simulation)) {
	levels[sp->sct_own][product->p_level] += output * level_p_e;
	wu((natid)0, sp->sct_own, "%s (%.2f) produced in %s\n",
	   product->p_name, output * level_p_e, ownxy(sp));
    } else {
	if ((actual = roundavg(level_p_e * output)) <= 0)
	    return 0;
	if (product->p_nrdep != 0) {
	    if (*resource * 100 < product->p_nrdep * actual)
		actual = *resource * 100 / product->p_nrdep;
	}
	if (actual > 999) {
	    actual = 999;
	    material_consume = (int)(actual / (product->p_effic * 0.01));
	}
	vec[item] += actual;
	if (vec[item] > 9999) {
	    material_consume =
		roundavg((9999.0 - vec[item] + actual) *
			 material_consume / actual);
	    if (material_consume < 0)
		material_consume = 0;
	    vec[item] = 9999;
	    if (( /* vtype != V_FOOD && */ sp->sct_own) &&
		(!player->simulation))
		wu(0, sp->sct_own,
		   "%s production backlog in %s\n",
		   product->p_name, ownxy(sp));
	}
    }
    /*
     * Reset produced amount by commodity production ratio
     */
    if (!player->simulation) {
	materials_charge(product, vec, material_consume);
	if (product->p_nrdep != 0) {
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
		*cost =
		    (double)*cost * (double)tpops[sp->sct_own] / 50000.0;
	}
    }

    /* The min() here is to take care of integer rounding errors */
    if (p_e > 0.0) {
	return min(work, (int)(unit_work * material_consume / p_e));
    }
    return 0;
}

static int
materials_cost(struct pchrstr *product, short *vec, int *costp)
{
    register u_char *vp;
    register u_short *ap;
    register int count;
    register int cost;
    register int n;
    register u_char *endp;

    count = 9999;
    cost = 0;
    ap = product->p_vamt;
    endp = product->p_vtype + product->p_nv;
    for (vp = product->p_vtype; vp < endp; vp++, ap++) {
	if (!*ap)
	    continue;
	n = vec[*vp & ~VT_ITEM] / *ap;
	if (n < count)
	    count = n;
	cost += *ap;
    }
    *costp = cost;
    return count;
}

static void
materials_charge(struct pchrstr *product, short *vec, int count)
{
    register u_char *vp;
    register u_short *ap;
    register u_char *endp;
    register int item;
    register int n;

    ap = product->p_vamt;
    endp = product->p_vtype + product->p_nv;
    for (vp = product->p_vtype; vp < endp; vp++, ap++) {
	item = *vp & ~VT_ITEM;
	if (item < 0 || item > I_MAX) {
	    logerror("materials_charge: bad item %d", item);
	    continue;
	}
	if ((n = vec[item] - *ap * count) < 0) {
	    logerror("materials_charge: %d > %d item #%d",
		     n, vec[item], item);
	    n = 0;
	}
	vec[item] = n;
    }
}
