/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  human.c: Food related functions
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include <math.h>
#include "chance.h"
#include "item.h"
#include "optlist.h"
#include "nat.h"
#include "news.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "update.h"
#include "xy.h"

static int new_work(struct sctstr *, int);
static int growfood(struct sctstr *, int, int);
static int starve_some(short *, i_type, int);
static void trunc_people(struct sctstr *, struct natstr *);
static int grow_people(struct sctstr *, int, struct natstr *, int);
static int babies(int, int, double, int, int, int);

/*
 * feed the individual sector
 */
void
do_feed(struct sctstr *sp, struct natstr *np, int etu,
	int round_babies_down)
{
    int work_avail;
    int starved, sctwork;
    int needed;
    int maxworkers;
    int manna;

    maxworkers = max_workers(np->nat_level[NAT_RLEV], sp);
    work_avail = new_work(sp,
			  total_work(sp->sct_work, etu,
				     sp->sct_item[I_CIVIL],
				     sp->sct_item[I_MILIT],
				     sp->sct_item[I_UW],
				     maxworkers));

    if (sp->sct_type != SCT_WATER && sp->sct_type != SCT_SANCT) {
	manna = 0;
	if (opt_NOFOOD == 0) {
	    needed = (int)ceil(food_needed(sp->sct_item, etu));
	    if (sp->sct_item[I_FOOD] < needed) {
		/* need to grow "emergency rations" */
		work_avail -= 2 * growfood(sp, work_avail / 2, etu);
		/* It's twice as hard to grow those than norm */
		if (sp->sct_item[I_FOOD] == 0)
		    /* Conjure up 1f to make life easier for the player */
		    manna = sp->sct_item[I_FOOD] = 1;
	    }
	}
	starved = feed_people(sp->sct_item, etu);
	if (starved > 0) {
	    if (!player->simulation) {
		/* don't report POGO starvation */
		if (sp->sct_own) {
		    wu(0, sp->sct_own, "%d starved in %s.\n", starved,
		       xyas(sp->sct_x, sp->sct_y, sp->sct_own));
		    if (starved > 25)
			nreport(sp->sct_own, N_DIE_FAMINE, 0, 1);
		}
	    }
	    sp->sct_loyal += roll(8) + 1;
	    sctwork = 0;
	} else {
	    sctwork = sp->sct_work;
	    if (sctwork < 100)
		sctwork += 7 + roll(15);
	    if (sctwork > 100)
		sctwork = 100;
	    grow_people(sp, etu, np, round_babies_down);
	    work_avail = new_work(sp,
				  total_work(sp->sct_work, etu,
					     sp->sct_item[I_CIVIL],
					     sp->sct_item[I_MILIT],
					     sp->sct_item[I_UW],
					     maxworkers));
	    /* FIXME restores work charged for growfood() */
	    /* age che */
	    sp->sct_che = age_people(sp->sct_che, etu);
	}
	if (manna)
	    /* Take away food we conjured up */
	    sp->sct_item[I_FOOD] = 0;
    } else
	sctwork = 100;

    /* Here is where we truncate extra people, always */
    trunc_people(sp, np);

    sp->sct_work = sctwork;
    sp->sct_avail = work_avail;
}

static int
new_work(struct sctstr *sp, int delta)
{
    int rollover = sp->sct_avail;

    if (sp->sct_type != sp->sct_newtype)
	rollover = 0;
    if (rollover > rollover_avail_max)
	rollover = rollover_avail_max;
    if (rollover > delta / 2 + 1)
	rollover = delta / 2 + 1;

    return rollover + delta;
}

static int
growfood(struct sctstr *sp, int work, int etu)
{
    int food_fertil;
    int food_workers;
    int food;
    int work_used;

    food_workers = work * fcrate;
    food_fertil = etu * sp->sct_fertil * fgrate;
    food = MIN(food_workers, food_fertil);
    if (food > ITEM_MAX - sp->sct_item[I_FOOD])
	food = ITEM_MAX - sp->sct_item[I_FOOD];
    sp->sct_item[I_FOOD] += food;
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
 * Return food eaten by people in @vec[] in @etu ETUs.
 */
double
food_needed(short *vec, int etu)
{
    int people = vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW];
    double need = etu * eatrate * people;
    return need;
}

/*
 * Return number of famine victims in @vec[] for @etu ETUs.
 */
int
famine_victims(short *vec, int etu)
{
    double can_eat = vec[I_FOOD] / (etu * eatrate);
    int people = vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW];
    if (people <= can_eat)
	return 0;
    if (can_eat <= people / 2)
	return people / 2;
    return (int)(people - can_eat);
}

/*
 * Starve up to @num people of @vec[@whom].
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
trunc_people(struct sctstr *sp, struct natstr *np)
{
    int maxpop = max_pop(np->nat_level[NAT_RLEV], sp);

    if (sp->sct_item[I_CIVIL] > maxpop)
	sp->sct_item[I_CIVIL] = maxpop;
    if (sp->sct_item[I_UW] > maxpop)
	sp->sct_item[I_UW] = maxpop;
}

/*
 * Grow babies, and add to populace.
 * XXX Might think about dropping in a birth
 * rate limitation on countries with high tech
 * production?  Maybe with just high education?
 */
static int
grow_people(struct sctstr *sp, int etu, struct natstr *np, int round_down)
{
    int newciv;
    int newuw;
    int maxpop = max_pop(np->nat_level[NAT_RLEV], sp);

    newciv = babies(sp->sct_item[I_CIVIL], etu, obrate,
		    sp->sct_item[I_FOOD], maxpop, round_down);
    sp->sct_item[I_CIVIL] += newciv;
    newuw = babies(sp->sct_item[I_UW], etu, uwbrate,
		   sp->sct_item[I_FOOD], maxpop, round_down);
    sp->sct_item[I_UW] += newuw;
    /*
     * subtract the baby eat food (if we are using FOOD) and return
     * # of births.
     */
    if (opt_NOFOOD == 0 && (newciv || newuw))
	sp->sct_item[I_FOOD] -= roundavg((newciv + newuw) * babyeat);
    return newciv + newuw;
}

/*
 * Return the number of babies born to @adults in @etu ETUs.
 * @brate is the birth rate.
 * @food is the food available for growing babies.
 * @maxpop is the population limit.
 * If @round_down, discard fractions instead of rounding them
 * randomly.
 */
static int
babies(int adults, int etu, double brate, int food, int maxpop,
       int round_down)
{
    double new_birth;
    int new_food, new;

    if (adults >= maxpop)
	return 0;

    new_birth = brate * etu * adults;
    new = round_down ? (int)new_birth : roundavg(new_birth);

    if (!opt_NOFOOD) {
	new_food = (int)(food / (2.0 * babyeat));
	if (new > new_food)
	    new = new_food;
    }

    if (adults + new > maxpop)
	new = maxpop - adults;

    return new;
}
