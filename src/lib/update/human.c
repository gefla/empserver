/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  human.c: Food related functions
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 */

#include <math.h>
#include "misc.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "item.h"
#include "news.h"
#include "file.h"
#include "xy.h"
#include "optlist.h"
#include "budg.h"
#include "player.h"
#include "update.h"
#include "common.h"
#include "gen.h"
#include "subs.h"

static int grow_people(struct sctstr *, register int,
		       register struct natstr *, int *, int,
		       short *);
static int growfood(struct sctstr *, short *, int, int);
static void starvation(struct sctstr *);
static void trunc_people(struct sctstr *, register struct natstr *,
			 short *);

/*
 * feed the individual sector
 *
 */
int
do_feed(struct sctstr *sp, struct natstr *np, short *vec,
	int *workp, int *bp, int etu)
{
    int people;
    int work_avail;
    int starved, sctwork;
    int needed, dummy;
    int civvies, uws;
    int mil;
    int maxpop;

    /* grow people & stuff */
    sctwork = sp->sct_work;

    maxpop = max_pop(np->nat_level[NAT_RLEV], sp);
    civvies = (vec[I_CIVIL] > maxpop) ? maxpop : vec[I_CIVIL];
    uws = (vec[I_UW] > maxpop) ? maxpop : vec[I_UW];
    mil = (vec[I_MILIT] > maxpop) ? maxpop : vec[I_MILIT];
    work_avail = new_work(sp, total_work(sctwork, etu, civvies, mil, uws));

    people = vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW];
    if (sp->sct_type != SCT_SANCT) {
	if (opt_NOFOOD == 0) {
	    if (vec[I_FOOD] < 1 + etu * people * eatrate) {
		/* need to grow "emergency rations" */
		work_avail -= (2 *
			       growfood(sp, vec, (int)(work_avail / 2),
					etu));
		/* It's twice as hard to grow those than norm */
		pt_bg_nmbr(bp, sp, I_MAX + 1, work_avail);
		if (!player->simulation)
		    sp->sct_avail = work_avail;
	    }
	    if ((vec[I_FOOD] < 1 + etu * people * eatrate) &&
		(sp->sct_own == sp->sct_oldown)) {

		/* steal food from warehouses, headquarters,
		   supply ships in port, or supply units */
		int needed;

		needed = ldround((double)(1 + etu * people * eatrate), 1);

		/* Now, find some food */
		vec[I_FOOD] = supply_commod(sp->sct_own, sp->sct_x,
					    sp->sct_y, I_FOOD, needed);

	    }
	}
	starved = feed_people(vec, etu, &needed);
	if ((starved > 0 && sp->sct_own) && (!player->simulation)) {
	    /* don't report POGO starvation */
	    wu(0, sp->sct_own, "%d starved in %s.\n", starved,
	       xyas(sp->sct_x, sp->sct_y, sp->sct_own));
	    if (starved > 25)
		nreport(sp->sct_own, N_DIE_FAMINE, 0, 1);
	}
	if (starved > 0) {
	    if (!player->simulation)
		starvation(sp);
	    sctwork = 0;
	} else {
	    if (sp->sct_work < 100)
		sctwork = sp->sct_work + 8 + (random() % 15);
	    if (sctwork > 100)
		sctwork = 100;
	    if (!player->simulation)
		sp->sct_work = sctwork;
	    dummy = grow_people(sp, etu, np, &work_avail, sctwork, vec);
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
	return min(rollover_avail_max, sp->sct_avail) + delta;

    return delta;
}

static int
growfood(struct sctstr *sp, short *vec, int work, int etu)
{
    int food_fertil;
    int food_workers;
    int food;
    int work_used;

    /* I'm being very nice and commenting out this so players
     * won't whine about starvation
     if (sp->sct_fertil == 0 || work == 0)
     return 0;
     */
    food_workers = work * fcrate;
    food_fertil = etu * sp->sct_fertil * fgrate;
    food = min(food_workers, food_fertil);
    /*
     * be nice; grow minimum one food unit.
     * This makes life simpler for the player.
     */
    if (food > ITEM_MAX - vec[I_FOOD])
	food = ITEM_MAX - vec[I_FOOD];
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
feed_people(short *vec, int etu, int *needed)
{
    double food_eaten;
    int ifood_eaten;
    int can_eat;
    int total_people;
    int to_starve;
    int starved;

    if (opt_NOFOOD)
	return 0;

    total_people = vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW];
    food_eaten = etu * eatrate * total_people;
    ifood_eaten = (int)food_eaten;
    if (food_eaten - ifood_eaten > 0)
	ifood_eaten++;
    if (ifood_eaten <= 1)
	return 0;
    starved = 0;
    *needed = 0;
    if (ifood_eaten > vec[I_FOOD]) {
	*needed = ifood_eaten - vec[I_FOOD];
	can_eat = vec[I_FOOD] / (etu * eatrate);
	/* only want to starve off at most 1/2 the populace. */
	if (can_eat < total_people / 2)
	    can_eat = total_people / 2;

	to_starve = total_people - can_eat;
	while (to_starve && vec[I_UW]) {
	    to_starve--;
	    starved++;
	    vec[I_UW]--;
	}
	while (to_starve && vec[I_CIVIL]) {
	    to_starve--;
	    starved++;
	    vec[I_CIVIL]--;
	}
	while (to_starve && vec[I_MILIT]) {
	    to_starve--;
	    starved++;
	    vec[I_MILIT]--;
	}

	vec[I_FOOD] = 0;
    } else {
	vec[I_FOOD] -= roundavg(food_eaten);
    }
    return starved;
}

/*
 * Truncate any extra people that may be around
 */
static void
trunc_people(struct sctstr *sp, register struct natstr *np,
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
grow_people(struct sctstr *sp, register int etu,
	    register struct natstr *np, int *workp, int sctwork,
	    short *vec)
{
    int newciv;
    int newuw;
    int new_birth;
    int new_food;
    int maxpop = max_pop(np->nat_level[NAT_RLEV], sp);

    newciv = 0;
    newuw = 0;
    if (vec[I_CIVIL] < maxpop) {
	new_birth = (int)roundavg(obrate * (double)(etu * vec[I_CIVIL]));
	if (opt_NOFOOD)
	    new_food = (int)(0.5 + maxpop / (2.0 * babyeat));
	else			/* we are using food */
	    new_food = (int)(0.5 + vec[I_FOOD] / (2.0 * babyeat));

	newciv = new_birth;
	if (newciv > new_food)
	    newciv = new_food;
	/* Now, check max pops */
	if ((vec[I_CIVIL] + newciv) > maxpop)
	    newciv = maxpop - vec[I_CIVIL];
	vec[I_CIVIL] += newciv;
    }
    if (vec[I_UW] < maxpop) {
	/*
	 * now grow uw's
	 */
	new_birth = (int)roundavg(uwbrate * (double)(etu * vec[I_UW]));
	if (opt_NOFOOD)
	    new_food = (int)(0.5 + maxpop / (2.0 * babyeat));
	else			/* food is important */
	    new_food = (int)(0.5 + vec[I_FOOD] / (2.0 * babyeat));

	newuw = new_birth;
	if (newuw > new_food)
	    newuw = new_food;
	/* Now, check max pops */
	if ((vec[I_UW] + newuw) > maxpop)
	    newuw = maxpop - vec[I_UW];
	vec[I_UW] += newuw;
    }
    /*
     * subtract the baby eat food (if we are using FOOD) and return
     * # of births.
     */
    if (opt_NOFOOD == 0 && (newciv || newuw))
	vec[I_FOOD] -= roundavg((newciv + newuw) * babyeat);
    *workp += total_work(sctwork, etu, newciv, 0, newuw);
    return newciv + newuw;
}

/*
 * percentage of people who starved
 */
static void
starvation(struct sctstr *sp)
{
    sp->sct_work = 0;
    sp->sct_loyal += (random() % 8) + 2;
}
