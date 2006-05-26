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
 *  human.c: Food related functions
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 */

#include <config.h>

#include <math.h>
#include "misc.h"
#include "sect.h"
#include "nat.h"
#include "item.h"
#include "news.h"
#include "xy.h"
#include "optlist.h"
#include "budg.h"
#include "player.h"
#include "update.h"
#include "common.h"
#include "gen.h"
#include "subs.h"

static int growfood(struct sctstr *, short *, int, int);
static int starve_some(short *, i_type, int);
static void trunc_people(struct sctstr *, struct natstr *, short *);
static int grow_people(struct sctstr *, int, struct natstr *, int *, int,
		       short *);
static int babies(int, int, double, int, int);

/*
 * feed the individual sector
 */
int
do_feed(struct sctstr *sp, struct natstr *np, short *vec,
	int *workp, int *bp, int etu)
{
    int work_avail;
    int starved, sctwork;
    int needed;
    int maxpop;

    /* grow people & stuff */
    sctwork = sp->sct_work;

    maxpop = max_pop(np->nat_level[NAT_RLEV], sp);
    work_avail = new_work(sp,
			  total_work(sctwork, etu,
				     vec[I_CIVIL], vec[I_MILIT], vec[I_UW],
				     maxpop));

    if (sp->sct_type != SCT_SANCT) {
	if (opt_NOFOOD == 0) {
	    needed = (int)ceil(food_needed(vec, etu));
	    if (vec[I_FOOD] < needed) {
		/* need to grow "emergency rations" */
		work_avail -= 2 * growfood(sp, vec, work_avail / 2, etu);
		/* It's twice as hard to grow those than norm */
		pt_bg_nmbr(bp, sp, I_MAX + 1, work_avail);
		if (!player->simulation)
		    sp->sct_avail = work_avail;
	    }
	    if (vec[I_FOOD] < needed && sp->sct_own == sp->sct_oldown) {
		/* steal food from warehouses, headquarters,
		   supply ships in port, or supply units */
		vec[I_FOOD] = supply_commod(sp->sct_own,
					    sp->sct_x, sp->sct_y,
					    I_FOOD, needed);
	    }
	}
	starved = feed_people(vec, etu);
	if (starved > 0) {
	    if (!player->simulation) {
		/* don't report POGO starvation */
		if (sp->sct_own) {
		    wu(0, sp->sct_own, "%d starved in %s.\n", starved,
		       xyas(sp->sct_x, sp->sct_y, sp->sct_own));
		    if (starved > 25)
			nreport(sp->sct_own, N_DIE_FAMINE, 0, 1);
		}
		sp->sct_work = 0;
		sp->sct_loyal += (random() % 8) + 2;
	    }
	    sctwork = 0;
	} else {
	    if (sp->sct_work < 100)
		sctwork = sp->sct_work + 8 + (random() % 15);
	    if (sctwork > 100)
		sctwork = 100;
	    if (!player->simulation)
		sp->sct_work = sctwork;
	    grow_people(sp, etu, np, &work_avail, sctwork, vec);
	}
    } else
	sctwork = sp->sct_work = 100;
    /* Here is where we truncate extra people, always */
    trunc_people(sp, np, vec);

    pt_bg_nmbr(bp, sp, I_CIVIL, vec[I_CIVIL]);
    pt_bg_nmbr(bp, sp, I_UW, vec[I_UW]);
    pt_bg_nmbr(bp, sp, I_MILIT, vec[I_MILIT]);
    *workp = work_avail;
    return sctwork;
}

int
new_work(struct sctstr *sp, int delta)
{
    if (sp->sct_type == sp->sct_newtype)
	return MIN(rollover_avail_max, sp->sct_avail) + delta;

    return delta;
}

static int
growfood(struct sctstr *sp, short *vec, int work, int etu)
{
    int food_fertil;
    int food_workers;
    int food;
    int work_used;

    food_workers = work * fcrate;
    food_fertil = etu * sp->sct_fertil * fgrate;
    food = MIN(food_workers, food_fertil);
    if (food > ITEM_MAX - vec[I_FOOD])
	food = ITEM_MAX - vec[I_FOOD];
    /*
     * Be nice; grow minimum one food unit.
     * This makes life simpler for the player.
     */
    vec[I_FOOD] += food;
    if (vec[I_FOOD] == 0)
	vec[I_FOOD] = 1;
    work_used = food / fcrate;
    return work_used;
}

/*
 * returns the number who starved, if any.
 */
int
feed_people(short *vec, int etu)
{
    int to_starve, starved;

    if (opt_NOFOOD)
	return 0;

    to_starve = famine_victims(vec, etu);
    starved = starve_some(vec, I_UW, to_starve);
    starved += starve_some(vec, I_CIVIL, to_starve - starved);
    starved += starve_some(vec, I_MILIT, to_starve - starved);
    vec[I_FOOD] -= roundavg(food_needed(vec, etu));
    if (vec[I_FOOD] < 0)
	vec[I_FOOD] = 0;
    return starved;
}

/*
 * Return food eaten by people in VEC[] in ETU ETUs.
 */
double
food_needed(short *vec, int etu)
{
    int people = vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW];
    double need = etu * eatrate * people;
    return need;
}

/*
 * Return number of famine victims in VEC[] for ETU ETUs.
 */
int
famine_victims(short *vec, int etu)
{
    double can_eat = vec[I_FOOD] / (etu * eatrate);
    int people = vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW];
    if (people < can_eat)
	return 0;
    if (can_eat < people / 2)
	return people / 2;
    return (int)(people - can_eat);
}

/*
 * Starve up to NUM people of VEC[WHOM].
 * Return the number of actually starved.
 */
static int
starve_some(short *vec, i_type whom, int num)
{
    int retval = MIN(num, vec[whom]);
    vec[whom] -= retval;
    return retval;
}

/*
 * Truncate any extra people that may be around
 */
static void
trunc_people(struct sctstr *sp, struct natstr *np,
	     short *vec)
{
    int maxpop = max_pop(np->nat_level[NAT_RLEV], sp);

    if (vec[I_CIVIL] > maxpop)
	vec[I_CIVIL] = maxpop;
    if (vec[I_UW] > maxpop)
	vec[I_UW] = maxpop;
}

/*
 * Grow babies, and add to populace.
 * XXX Might think about dropping in a birth
 * rate limitation on countries with high tech
 * production?  Maybe with just high education?
 */
static int
grow_people(struct sctstr *sp, int etu,
	    struct natstr *np, int *workp, int sctwork,
	    short *vec)
{
    int newciv;
    int newuw;
    int maxpop = max_pop(np->nat_level[NAT_RLEV], sp);

    newciv = babies(vec[I_CIVIL], etu, obrate, vec[I_FOOD], maxpop);
    vec[I_CIVIL] += newciv;
    newuw = babies(vec[I_UW], etu, uwbrate, vec[I_FOOD], maxpop);
    vec[I_UW] += newuw;
    /*
     * subtract the baby eat food (if we are using FOOD) and return
     * # of births.
     */
    if (opt_NOFOOD == 0 && (newciv || newuw))
	vec[I_FOOD] -= roundavg((newciv + newuw) * babyeat);
    *workp += total_work(sctwork, etu, newciv, 0, newuw, ITEM_MAX);
    return newciv + newuw;
}

/*
 * Return the number of babies born to ADULTS in ETU ETUs.
 * BRATE is the birth rate.
 * FOOD is the food available for growing babies.
 * MAXPOP is the population limit.
 */
static int
babies(int adults, int etu, double brate, int food, int maxpop)
{
    int new_birth, new_food, new;

    if (adults >= maxpop)
	return 0;

    new_birth = roundavg(brate * etu * adults);
    if (opt_NOFOOD)
	new_food = new_birth;
    else
	new_food = (int)(0.5 + food / (2.0 * babyeat));

    new = new_birth;
    if (new > new_food)
	new = new_food;
    if (adults + new > maxpop)
	new = maxpop - adults;

    return new;
}
