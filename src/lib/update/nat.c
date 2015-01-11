/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nat.c: Accumulate tech, edu, research and happiness.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1997
 *     Markus Armbruster, 2006-2015
 */

#include <config.h>

#include <math.h>
#include "budg.h"
#include "game.h"
#include "item.h"
#include "update.h"

/*
 * hap and edu avg mean that the weight on current happiness is
 *  (cur_hap * hap_avg + hap_prod * etu) / (hap_avg + etu);
 * same for education.
 * right now, happiness has 1 day (48 etu) average, prod of 10 from
 * initial level of 0 yields (1) 1.42, (6) 6.03, (12) 8.42, (18) 9.37
 *
 * education has 4 day (192 etu) average, prod of 10 from initial
 * level of 0 yields (1) 0.4, (6) 2.2, (12) 3.9, (18) 5.2.
 */

static void share_incr(double *, double *);

/*
 * for values below the "easy level" values, production is
 * as normal.  For values above "easy", production gets harder
 * based on an equation in "limit_level()" in update/nat.c.
 * Basically, the smaller the the values for "level_log", the
 * smaller return on investment above level_easy[] values.
 */
/*
 * Damn! I hate this, but ...
 * The values here for tech are *not* the real ones.
 * They are changed later in the limit_level routine.
 */
			/*tech   res   edu   hap */
static float level_easy[4] = { 0.75, 0.75, 5.00, 5.00 };
static float level_log[4] = { 1.75, 2.00, 4.00, 6.00 };

float levels[MAXNOC][4];

/*
 * technique to limit the sharpers who turn entire countries
 * into tech plants overnight...
 */

static double
logx(double d, double base)
{
    if (base == 1.0)
	return d;
    return log10(d) / log10(base);
}

static double
limit_level(double level, int type, int flag)
{
    double above_easy;
    double above;
    double logbase;
    double easy;

/*
 * Begin ugly hack.
 */
    level_easy[0] = easy_tech;
    level_log[0] = tech_log_base;
/*
 * End ugly hack.
 */

    if (level > level_easy[type]) {
	logbase = level_log[type];
	easy = level_easy[type];
	above_easy = level - easy;
	if (flag)
	    above = above_easy / logx(logbase + above_easy, logbase);
	else
	    above = logx(above_easy + 1.0, logbase);
	if (above > 250)
	    above = 250;
	return above < 0 ? easy : easy + above;
    } else
	return level;
}

void
prod_nat(int etu)
{
    struct natstr *np;
    float hap;
    float edu;
    float hap_edu;
    int pop;
    double rlev;
    double tlev;
    double tech[MAXNOC];
    double res[MAXNOC];
    double newvalue;
    natid n;
    int cn;
    struct natstr *cnp;

    for (n = 0; NULL != (np = getnatp(n)); n++) {
	grant_btus(np, game_reset_tick(&np->nat_access));
	if (np->nat_stat < STAT_ACTIVE)
	    continue;
	/*
	 * hap_edu: the more education people have, the
	 * more happiness they want.
	 */
	hap_edu = np->nat_level[NAT_ELEV];
	hap_edu = 1.5 - ((hap_edu + 10.0) / (hap_edu + 20.0));
	pop = pops[n] + 1;
	/*
	 * get per-population happiness and education
	 * see what the total per-civilian production is
	 * for this time period.
	 */
	hap = levels[n][NAT_HLEV] * hap_edu * hap_cons /
	    ((float)pop * etu);
	edu = levels[n][NAT_ELEV] * edu_cons / ((float)pop * etu);
	wu(0, n, "%3.0f happiness, %3.0f education produced\n",
	   levels[n][NAT_HLEV], levels[n][NAT_ELEV]);
	hap = limit_level(hap, NAT_HLEV, 1);
	edu = limit_level(edu, NAT_ELEV, 1);
	/*
	 * change the "moving average"...old happiness and
	 * education levels are weighted heavier than current
	 * production.
	 */
	newvalue = (np->nat_level[NAT_HLEV] * hap_avg + hap * etu) /
	    (hap_avg + etu);
	np->nat_level[NAT_HLEV] = newvalue;
	newvalue = (np->nat_level[NAT_ELEV] * edu_avg + edu * etu) /
	    (edu_avg + etu);
	np->nat_level[NAT_ELEV] = newvalue;
	/*
	 * limit tech/research production
	 */
	levels[n][NAT_TLEV] =
	    limit_level(levels[n][NAT_TLEV] / 1, NAT_TLEV, 0) * 1;
	levels[n][NAT_RLEV] =
	    limit_level(levels[n][NAT_RLEV] / 1, NAT_RLEV, 0) * 1;
	wu(0, n, "total pop was %d, yielding %4.2f hap, %4.2f edu\n",
	   pop - 1, hap, edu);
    }
    if (ally_factor > 0.0)
	share_incr(res, tech);
    else {
	memset(res, 0, sizeof(res));
	memset(tech, 0, sizeof(tech));
    }
    for (n = 0; NULL != (np = getnatp(n)); n++) {
	if (np->nat_stat < STAT_ACTIVE)
	    continue;
	tlev = levels[n][NAT_TLEV];
	rlev = levels[n][NAT_RLEV];
	if (tech[n] != 0.0 || res[n] != 0.0) {
	    wu(0, n, "%5.4f technology (%5.4f + %5.4f), "
	       "%5.4f research (%5.4f + %5.4f) produced\n",
	       tlev + tech[n], tlev, tech[n],
	       rlev + res[n], rlev, res[n]);
	} else
	    wu(0, n, "%5.4f tech, %5.4f research produced\n", tlev, rlev);
	rlev += res[n];
	tlev += tech[n];
	if (rlev != 0.0)
	    np->nat_level[NAT_RLEV] += rlev;
	if (tlev != 0.0)
	    np->nat_level[NAT_TLEV] += tlev;
	if ((sea_money[n] != 0) || (air_money[n] != 0) ||
	    (lnd_money[n] != 0))
	    wu(0, n,
	       "Army delta $%d, Navy delta $%d, Air force delta $%d\n",
	       lnd_money[n], sea_money[n], air_money[n]);
	wu(0, n, "money delta was $%d for this update\n",
	   np->nat_money - money[n]);
	if (opt_LOSE_CONTACT) {
	    for (cn = 1; cn < MAXNOC; cn++) {
		if ((cnp = getnatp(cn)) != NULL)
		    agecontact(cnp);
	    }
	}
    }
}

/*
 * find out everyones increment
 */
static void
share_incr(double *res, double *tech)
{
    struct natstr *np;
    struct natstr *other;
    natid i;
    natid j;
    int rnc;
    int tnc;

    for (i = 0; NULL != (np = getnatp(i)); i++) {
	res[i] = tech[i] = 0.0;
	if (np->nat_stat < STAT_SANCT || np->nat_stat == STAT_GOD)
	    continue;
	rnc = tnc = 0;
	for (j = 0; NULL != (other = getnatp(j)); j++) {
	    if (j == i)
		continue;
	    if (other->nat_stat != STAT_ACTIVE)
		continue;
	    if (opt_HIDDEN) {
		if (!getcontact(np, j))
		    continue;
	    }
	    if (!opt_ALL_BLEED) {
		if (relations_with(i, j) != ALLIED)
		    continue;
		if (relations_with(j, i) != ALLIED)
		    continue;
		res[i] += levels[j][NAT_RLEV];
		tech[i] += levels[j][NAT_TLEV];
		rnc++;
		tnc++;
	    } else {
		if (levels[j][NAT_TLEV] > 0.001) {
		    tech[i] += levels[j][NAT_TLEV];
		    tnc++;
		}
		if (levels[j][NAT_RLEV] > 0.001) {
		    res[i] += levels[j][NAT_RLEV];
		    rnc++;
		}
	    }
	}
	if (rnc > 0) {
	    res[i] /= rnc * ally_factor;
	}
	if (tnc > 0) {
	    tech[i] /= tnc * ally_factor;
	}
    }
}
