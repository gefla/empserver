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
 *  emp_config.c: Allows config file to control server config. from a file
 * 
 *  Known contributors to this file:
 *     Julian Onions, 1995
 *     Steve McClure, 1998-2000
 */

/*
 * STILL TO DO
 *
 * 1. Change other constants - such as Num Countries etc.
 *    Just requires variables to be assigned, then dynamic allocation in
 *    a few places. Some checks needed in the server to check the world
 *    hasn't changed size etc.
 * 2. Could look at loading in planes, units etc. Should be easy enough.
 *
 */

#include <stdio.h>
#include <stdlib.h>		/* atoi free atol */
#include <string.h>

#include "misc.h"
#include "com.h"
#include "match.h"
#include "file.h"
#include "optlist.h"
#include "tel.h"
#include "gen.h"		/* parse */

/* for systems without strdup  */
#ifdef NOSTRDUP
extern char *strdup();
#endif /* NOSTRDUP */

/* Dummy one */
static int emp_config_dummy;

static void optstrset(struct keymatch *kp, s_char **av);
static void intset(struct keymatch *kp, s_char **av);
static void floatset(struct keymatch *kp, s_char **av);
static void doubleset(struct keymatch *kp, s_char **av);
static void longset(struct keymatch *kp, s_char **av);
static void optionset(struct keymatch *kp, s_char **av);
static void optiondel(struct keymatch *kp, s_char **av);
static void worldxset(struct keymatch *kp, s_char **av);

/* things that can be changed */
struct keymatch configkeys[] = {
    {"", intset, (caddr_t)&emp_config_dummy, 0,
     "\n### Server configuration and information"},
    {"data", optstrset, (caddr_t)&datadir, 0,
     "Directory the data is stored in"},
    {"info", optstrset, (caddr_t)&infodir, 0,
     "Directory the info pages are stored in"},
    {"port", optstrset, (caddr_t)&loginport, 0,
     "TCP/IP port the server will start up on"},
    {"privname", optstrset, (caddr_t)&privname, 0,
     "Name of the deity"},
    {"privlog", optstrset, (caddr_t)&privlog, 0,
     "E-mail of the deity"},
    {"WORLD_X", worldxset, (caddr_t)&WORLD_X, 0,
     "World size X dimension (enforced to be even by subtracting 1 if necessary)"},
    {"WORLD_Y", intset, (caddr_t)&WORLD_Y, 0,
     "World size Y dimension"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Update policy"},
    {"update_policy", intset, (caddr_t)&update_policy, 0,
     "0 - normal, 1 - update_times, 2 - blitz, 3 - demand only"},
    {"etu_per_update", intset, (caddr_t)&etu_per_update, 0,
     "Number of ETUs per update"},
    {"s_p_etu", longset, (caddr_t)&s_p_etu, 0,
     "Seconds per etu, updates will occur every s_p_etu * etu_per_update seconds"},
    {"adj_update", longset, (caddr_t)&adj_update, 0,
     "Move the update forward or backward (in seconds)"},
    {"update_window", intset, (caddr_t)&update_window, 0,
     "Window the update will occur in (in seconds) before and after the update time"},
    {"update_times", optstrset, (caddr_t)&update_times, 0,
     "Times when updates occur under policy #1.  Must coincide with schedule."},
    {"hourslop", intset, (caddr_t)&hourslop, 0,
     "Number of minutes update check can slip to match update_times"},
    {"blitz_time", intset, (caddr_t)&blitz_time, 0,
     "Number of minutes between updates under policy #2."},

    {"", intset, (caddr_t)&emp_config_dummy, 0,
     "\n\n### Demand update policy"},
    {"update_demandpolicy", intset, (caddr_t)&update_demandpolicy, 0,
     "0 - emp_tm checks, 1 - after setting, 2 - demand updates disabled"},
    {"update_wantmin", intset, (caddr_t)&update_wantmin, 0,
     "number of requests needed for demand update"},
    {"update_missed", intset, (caddr_t)&update_missed, 0,
     "number of demand updates country can miss before veto update"},
    {"update_demandtimes", optstrset, (caddr_t)&update_demandtimes, 0,
     "Times when demand updates can occur.  Ranges CANNOT cross midnight."},

    {"", intset, (caddr_t)&emp_config_dummy, 0,
     "\n\n### Game hours restrictions"},
    {"game_days", optstrset, (caddr_t)&game_days, 0,
     "Days game is up and running (Su Mo Tu We Th Fr Sa)"},
    {"game_hours", optstrset, (caddr_t)&game_hours, 0,
     "Hours game is up and running (6:00-18:00)"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Options\n"},
    {"option", optionset, (caddr_t)NULL, 0, NULL},
    {"nooption", optiondel, (caddr_t)NULL, 0, NULL},


    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Countries"},
    {"btu_build_rate", floatset, (caddr_t)&btu_build_rate, 0,
     "Rate at which BTUs accumulate (etu * civ * eff * btu_build_rate)"},
    {"m_m_p_d", intset, (caddr_t)&m_m_p_d, 0,
     "Maximum minutes per day a country is allowed to be logged in"},
    {"max_btus", intset, (caddr_t)&max_btus, 0,
     "Maximum number of BTUs a country can have"},
    {"max_idle", intset, (caddr_t)&max_idle, 0,
     "Maximum number of minutes a player can sit idle while logged in"},
    {"players_at_00", intset, (caddr_t)&players_at_00, 0,
     "Players have their coordinate system at deity 0,0 (0 - no, 1 - yes)"},
    {"at_least_one_100", intset, (caddr_t)&at_least_one_100, 0,
     "Initialize new countries with at least one sector with 100 of all resource"},
    {"powe_cost", doubleset, (caddr_t)&powe_cost, 0,
     "Number of BTUs needed to generate a new power report"},
    {"war_cost", intset, (caddr_t)&War_Cost, 0,
     "Cost to declare war (if SLOW_WAR is on)"},

    {"", intset, (caddr_t)&emp_config_dummy, 0,
     "\n\n### Technology/Research/Education/Happiness"},
    {"easy_tech", floatset, (caddr_t)&easy_tech, 0,
     "Amount of tech built with no penalty"},
    {"hard_tech", floatset, (caddr_t)&hard_tech, 0,
     "Amount of in-efficiently built tech"},
    {"start_tech", floatset, (caddr_t)&start_technology, 0,
     "Starting technology for new countries"},
    {"start_happy", floatset, (caddr_t)&start_happiness, 0,
     "Starting happiness for new countries"},
    {"start_research", floatset, (caddr_t)&start_research, 0,
     "Starting research for new countries"},
    {"start_edu", floatset, (caddr_t)&start_education, 0,
     "Starting education for new countries"},
    {"level_age_rate", floatset, (caddr_t)&level_age_rate, 0,
     "ETU rate at which tech decays (0 -> no decline)"},
    {"tech_log_base", floatset, (caddr_t)&tech_log_base, 0,
     "Log base to apply to tech breakthroughs above the easy tech level"},
    {"ally_factor", floatset, (caddr_t)&ally_factor, 0,
     "Shared tech with allies (1 / ally_factor)"},
    {"edu_avg", floatset, (caddr_t)&edu_avg, 0,
     "Number of ETUs education is averaged over"},
    {"hap_avg", floatset, (caddr_t)&hap_avg, 0,
     "Number of ETUs happiness is averaged over"},
    {"edu_cons", doubleset, (caddr_t)&edu_cons, 0,
     "Education consumption (1 breakthrough per edu_cons)"},
    {"hap_cons", doubleset, (caddr_t)&hap_cons, 0,
     "Happiness consumption (1 breakthrough per hap_cons)"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Sectors"},
    {"startmob", intset, (caddr_t)&startmob, 0,
     "Starting mobility for sanctuaries"},
    {"sect_mob_scale", floatset, (caddr_t)&sect_mob_scale, 0,
     "Sector mobility accumulation (sect_mob_scale * ETUs per update)"},
    {"sect_mob_max", intset, (caddr_t)&sect_mob_max, 0,
     "Maximum mobility for sectors"},
    {"buil_bh", intset, (caddr_t)&buil_bh, 0,
     "Number of hcms required to build a bridge span"},
    {"buil_bc", doubleset, (caddr_t)&buil_bc, 0,
     "Cash required to build a bridge span"},
    {"buil_bt", doubleset, (caddr_t)&buil_bt, 0,
     "Technology required to build a bridge span"},
    {"buil_tower_bh", intset, (caddr_t)&buil_tower_bh, 0,
     "Number of hcms required to build a bridge tower"},
    {"buil_tower_bc", doubleset, (caddr_t)&buil_tower_bc, 0,
     "Cash required to build a bridge tower"},
    {"buil_tower_bt", doubleset, (caddr_t)&buil_tower_bt, 0,
     "Technology required to build a bridge tower"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Land Units"},
    {"land_mob_scale", floatset, (caddr_t)&land_mob_scale, 0,
     "Land unit mobility accumulation (land_mob_scale * ETUs per update)"},
    {"land_grow_scale", intset, (caddr_t)&land_grow_scale, 0,
     "How fast efficiency grows for land units each update (* ETUs)"},
    {"land_mob_max", intset, (caddr_t)&land_mob_max, 0,
     "Maximum mobility for land units"},
    {"money_land", doubleset, (caddr_t)&money_land, 0,
     "Cost per ETU to maintain land units (percentage of unit price)"},
    {"morale_base", intset, (caddr_t)&morale_base, 0,
     "Base level for setting morale of land units"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Planes"},
    {"plane_mob_scale", floatset, (caddr_t)&plane_mob_scale, 0,
     "Plane mobility accumulation (plane_mob_scale * ETUs per update)"},
    {"plane_grow_scale", intset, (caddr_t)&plane_grow_scale, 0,
     "How fast efficiency grows for planes each update (* ETUs)"},
    {"plane_mob_max", intset, (caddr_t)&plane_mob_max, 0,
     "Maximum mobility for planes"},
    {"money_plane", doubleset, (caddr_t)&money_plane, 0,
     "Cost per ETU to maintain planes (percentage of plane price)"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Ships"},
    {"ship_mob_scale", floatset, (caddr_t)&ship_mob_scale, 0,
     "Ship mobility accumulation (ship_mob_scale * ETUs per update)"},
    {"ship_grow_scale", intset, (caddr_t)&ship_grow_scale, 0,
     "How fast efficiency grows for ships each update (* ETUs)"},
    {"ship_mob_max", intset, (caddr_t)&ship_mob_max, 0,
     "Maximum mobility for ships"},
    {"money_ship", doubleset, (caddr_t)&money_ship, 0,
     "Cost per ETU to maintain ships (percentage of ship price)"},
    {"torpedo_damage", intset, (caddr_t)&torpedo_damage, 0,
     "Torpedo damage (damage is X + 1dX + 1dX)"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Combat/Damage"},
    {"fort_max_interdiction_range", intset,
     (caddr_t)&fort_max_interdiction_range, 0,
     "Maximum range (in sectors) a fort will try to interdict another country"},
    {"land_max_interdiction_range", intset,
     (caddr_t)&land_max_interdiction_range, 0,
     "Maximum range (in sectors) a land unit will try to interdict another country"},
    {"ship_max_interdiction_range", intset,
     (caddr_t)&ship_max_interdiction_range, 0,
     "Maximum range (in sectors) a ship will try to interdict another country"},
    {"flakscale", doubleset, (caddr_t)&flakscale, 0,
     "Scale factor for flak damage"},
    {"combat_mob", doubleset, (caddr_t)&combat_mob, 0,
     "How much mobility do units spend for combat (* casualties/bodies)"},
    {"people_damage", doubleset, (caddr_t)&people_damage, 0,
     "People take this amount of normal damage"},
    {"unit_damage", doubleset, (caddr_t)&unit_damage, 0,
     "Land units take this amount of normal damage"},
    {"collateral_dam", doubleset, (caddr_t)&collateral_dam, 0,
     "Side effect damage amount done to sector"},
    {"assault_penalty", doubleset, (caddr_t)&assault_penalty, 0,
     "Amount of normal attacking efficiency for paratroopers and assaulting"},
    {"fire_range_factor", floatset, (caddr_t)&fire_range_factor, 0,
     "Scale normal firing ranges by this amount"},
    {"sect_mob_neg_factor", intset, (caddr_t)&sect_mob_neg_factor, 0,
     "Amount of negative mobility a sector has after takeover (ETU / x) (MOB_ACCESS)"},
    {"mission_mob_cost", doubleset, (caddr_t)&mission_mob_cost, 0,
     "Cost to put something on a mission (percentage of max mob)"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Populace"},
    {"uwbrate", doubleset, (caddr_t)&uwbrate, 0,
     "Birth rate for uw's"},
    {"money_civ", doubleset, (caddr_t)&money_civ, 0,
     "Money gained from taxes on a civilian in one ETU"},
    {"money_mil", doubleset, (caddr_t)&money_mil, 0,
     "Money gained from taxes on an active soldier in one ETU"},
    {"money_res", doubleset, (caddr_t)&money_res, 0,
     "Money gained from taxes on a soldier on active reserve in one ETU"},
    {"money_uw", doubleset, (caddr_t)&money_uw, 0,
     "Money gained from taxes on an uncompensated worker in one ETU"},
    {"babyeat", doubleset, (caddr_t)&babyeat, 0,
     "Amount of food to mature 1 baby into a civilian"},
    {"bankint", doubleset, (caddr_t)&bankint, 0,
     "Bank dollar gain (per bar per etu)"},
    {"eatrate", doubleset, (caddr_t)&eatrate, 0,
     "Food eating rate for mature people"},
    {"fcrate", doubleset, (caddr_t)&fcrate, 0,
     "Food cultivation rate (* workforce in sector)"},
    {"fgrate", doubleset, (caddr_t)&fgrate, 0,
     "Food growth rate (* fertility of sector)"},
    {"obrate", doubleset, (caddr_t)&obrate, 0,
     "Civilian birth rate"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Nukes"},
    {"decay_per_etu", doubleset, (caddr_t)&decay_per_etu, 0,
     "Decay of fallout per ETU"},
    {"fallout_spread", doubleset, (caddr_t)&fallout_spread, 0,
     "Amount of fallout that leaks into surrounding sectors"},
    {"drnuke_const", floatset, (caddr_t)&drnuke_const, 0,
     "Amount of research to tech needed to build a nuke (if DR_NUKE is on)"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Market/Trade"},
    {"MARK_DELAY", intset, (caddr_t)&MARK_DELAY, 0,
     "Number of seconds commodities stay on the market for bidding"},
    {"TRADE_DELAY", intset, (caddr_t)&TRADE_DELAY, 0,
     "Number of seconds ships, planes, and units stay on the market for bidding"},
    {"buytax", doubleset, (caddr_t)&buytax, 0,
     "Tax (in percentage points) charged to the buyer on market purchases"},
    {"tradetax", doubleset, (caddr_t)&tradetax, 0,
     "Amount of a trade transaction the seller makes (the rest is tax)"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Trade ships"},
    {"trade_1_dist", intset, (caddr_t)&trade_1_dist, 0,
     "Less than this distance no money for cashing in"},
    {"trade_2_dist", intset, (caddr_t)&trade_2_dist, 0,
     "Less than this distance gets trade_1 money for cashing in"},
    {"trade_3_dist", intset, (caddr_t)&trade_3_dist, 0,
     "Less than this distance gets trade_2 money for cashing in (>= gets trade_3"},
    {"trade_1", floatset, (caddr_t)&trade_1, 0,
     "Return per sector on trade_1 distance amount"},
    {"trade_2", floatset, (caddr_t)&trade_2, 0,
     "Return per sector on trade_2 distance amount"},
    {"trade_3", floatset, (caddr_t)&trade_3, 0,
     "Return per sector on trade_3 distance amount"},
    {"trade_ally_bonus", floatset, (caddr_t)&trade_ally_bonus, 0,
     "Bonus you get for cashing in with an ally"},
    {"trade_ally_cut", floatset, (caddr_t)&trade_ally_cut, 0,
     "Bonus your ally gets for you cashing in with them"},

    {"", intset, (caddr_t)&emp_config_dummy, 0, "\n\n### Misc."},
    {"fuel_mult", intset, (caddr_t)&fuel_mult, 0,
     "Multiplier for fuel to mobility calculation"},
    {"lost_items_timeout", intset, (caddr_t)&lost_items_timeout, 0,
     "Seconds before a lost item is timed out of the database"},
    {"last_demand_update", longset, (caddr_t)&last_demand_update, 0,
     "When was the last demand update occured"},

    {NULL, NULL, (caddr_t)0, 0, NULL}
};

static void fixup_files(void);
static struct keymatch *keylookup(s_char *key, struct keymatch tbl[]);


/*
 * read in empire configuration
 */
int
emp_config(char *file)
{
    FILE *fp;
    s_char scanspace[1024];
    s_char *av[65];
    char buf[BUFSIZ];
    struct keymatch *kp;

    if (file == NULL || (fp = fopen(file, "r")) == NULL) {
	fixup_files();
	return RET_OK;
    }
    while (fgets(buf, sizeof buf, fp) != NULL) {
	if (buf[0] == '#' || buf[0] == '\n')
	    continue;
	if (parse(buf, av, 0, scanspace, 0) < 0) {
	    fprintf(stderr, "Can't parse line %s", buf);
	    continue;
	}
	if ((kp = keylookup(av[0], configkeys)) != NULL) {
	    (*kp->km_func) (kp, av + 1);
	} else {
	    fprintf(stderr, "Unknown config key %s\n", av[0]);
	}
    }
    fclose(fp);
    fixup_files();

    return RET_OK;
}

struct otherfiles {
    s_char **files;
    char *name;
};

/* list of other well known files... -maybe tailor these oneday
 * anyway - meantime they are all relative to datadir */
static struct otherfiles ofiles[] = {
    {&upfil, "up"},
    {&downfil, "down"},
    {&disablefil, "disable"},
    {&banfil, "ban"},
    {&authfil, "auth"},
    {&commfil, "comm"},
    {&annfil, "ann"},
    {&timestampfil, "timestamp"},
    {&teldir, "tel"},
#if !defined(_WIN32)
    {&telfil, "tel/tel"},
#else
    {&telfil, "tel\\tel"},
#endif
    {NULL, NULL}
};

/* fix up the empfile struct to reference full path names */
static void
fixup_files(void)
{
    struct empfile *ep;
    struct otherfiles *op;
    s_char buf[1024];

    for (ep = empfile; ep < &empfile[EF_MAX]; ep++) {
#if !defined(_WIN32)
	sprintf(buf, "%s/%s", datadir, ep->name);
#else
	sprintf(buf, "%s\\%s", datadir, ep->name);
#endif
	ep->file = strdup(buf);
    }

    for (op = ofiles; op->files; op++) {
#if !defined(_WIN32)
	sprintf(buf, "%s/%s", datadir, op->name);
#else
	sprintf(buf, "%s\\%s", datadir, op->name);
#endif
	*op->files = strdup(buf);
    }
}

/* find the key in the table */
static struct keymatch *
keylookup(register s_char *command, struct keymatch *tbl)
{
    register struct keymatch *kp;

    if (command == 0 || *command == 0)
	return 0;
    for (kp = tbl; kp->km_key != 0; kp++) {
	if (strcmp(kp->km_key, command) == 0)
	    return kp;
    }
    return NULL;
}

/* worldx int setting function */
static void
worldxset(struct keymatch *kp, s_char **av)
{
    int *intptr = (int *)kp->km_data;

    if (*av == NULL || intptr == NULL)
	return;
    *intptr = atoi(*av);
    if (!((*intptr % 2) == 0)) {
	/* Must be div / 2, so subtract one */
	*intptr = *intptr - 1;
    }
}

/* generic int setting function */
static void
intset(struct keymatch *kp, s_char **av)
{
    int *intptr = (int *)kp->km_data;

    if (*av == NULL || intptr == NULL)
	return;
    *intptr = atoi(*av);
}

/* generic float set function */
static void
floatset(struct keymatch *kp, s_char **av)
{
    float *floatptr = (float *)kp->km_data;

    if (*av == NULL || floatptr == NULL)
	return;
    *floatptr = atof(*av);
}



/* generic string set function */
static void
optstrset(struct keymatch *kp, s_char **av)
{
    s_char **confstrp = (s_char **)kp->km_data;

    if (*av == NULL || confstrp == NULL)
	return;
    if (kp->km_flags & KM_ALLOC)
	free(*confstrp);
    *confstrp = strdup(*av);
    kp->km_flags |= KM_ALLOC;
}

/* generic double set function */
static void
doubleset(struct keymatch *kp, s_char **av)
{
    double *doublep = (double *)kp->km_data;

    if (*av == NULL || doublep == NULL)
	return;
    *doublep = atof(*av);
}

/* generic long set function */
static void
longset(struct keymatch *kp, s_char **av)
{
    long int *longp = (long int *)kp->km_data;

    if (*av == NULL || longp == NULL)
	return;
    *longp = atol(*av);
}

void
print_config(FILE * fp)
{
    struct empfile *ep;
    struct otherfiles *op;
    struct keymatch *kp;

    fprintf(fp, "# Empire Configuration File:\n");
    for (kp = configkeys; kp->km_key; kp++) {
	/* We print a few special things here */
	if (kp->km_comment) {
	    if (kp->km_comment[0]) {
		if (kp->km_comment[0] != '\n')
		    fprintf(fp, "\n# ");
		fprintf(fp, "%s\n", kp->km_comment);
	    }
	}
	if (!kp->km_key[0])
	    continue;
	if (kp->km_func == optstrset) {
	    fprintf(fp, "%s \"%s\"\n", kp->km_key,
		    *(s_char **)kp->km_data);
	} else if (kp->km_func == intset) {
	    fprintf(fp, "%s %d\n", kp->km_key, *(int *)kp->km_data);
	} else if (kp->km_func == worldxset) {
	    fprintf(fp, "%s %d\n", kp->km_key, *(int *)kp->km_data);
	} else if (kp->km_func == floatset) {
	    fprintf(fp, "%s %g\n", kp->km_key, *(float *)kp->km_data);
	} else if (kp->km_func == doubleset) {
	    fprintf(fp, "%s %g\n", kp->km_key, *(double *)kp->km_data);
	} else if (kp->km_func == longset) {
	    fprintf(fp, "%s %ld\n", kp->km_key, *(long *)kp->km_data);
	} else if (kp->km_func == optionset) {
	    struct option_list *op;

	    for (op = Options; op->opt_key; op++) {
		if (*op->opt_valuep)
		    fprintf(fp, "%s %s\n", kp->km_key, op->opt_key);
	    }
	} else if (kp->km_func == optiondel) {
	    struct option_list *op;

	    for (op = Options; op->opt_key; op++) {
		if (*op->opt_valuep == 0)
		    fprintf(fp, "%s %s\n", kp->km_key, op->opt_key);
	    }
	} else
	    fprintf(fp, "# Unknown format %s\n", kp->km_key);
    }

    fprintf(fp, "\n");
    for (ep = empfile; ep < &empfile[EF_MAX]; ep++)
	fprintf(fp, "# File %s -> %s\n", ep->name, ep->file);
    for (op = ofiles; op->files; op++)
	fprintf(fp, "# File %s -> %s\n", op->name, *(op->files));

}


/* add an option to the list */
static void
set_option(const char *s)
{
    struct option_list *op;

    for (op = Options; op->opt_key; op++) {
	if (strcmp(op->opt_key, s) == 0) {
	    *op->opt_valuep = 1;
	    return;
	}
    }
    fprintf(stderr, "Unknown option %s\n", s);
}

/* delete an option from the list */
static void
delete_option(const char *s)
{
    struct option_list *op;

    for (op = Options; op->opt_key; op++) {
	if (strcmp(op->opt_key, s) == 0) {
	    *op->opt_valuep = 0;
	    return;
	}
    }
    fprintf(stderr, "Unknown option %s\n", s);
}

/* config interface */
static void
optionset(struct keymatch *kp, s_char **av)
				/* unused - we have a well known global */
{
    char **cpp;

    for (cpp = (char **)av; *cpp; cpp++)
	set_option(*cpp);
}

/* config interface */
static void
optiondel(struct keymatch *kp, s_char **av)
				/* unused - we have a well known global */
{
    char **cpp;

    for (cpp = (char **)av; *cpp; cpp++)
	delete_option(*cpp);
}
