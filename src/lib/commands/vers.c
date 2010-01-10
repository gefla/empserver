/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  vers.c: Print out the Empire version
 *
 *  Known contributors to this file:
 *     Dave Pare
 *     Jeff Bailey
 *     Thomas Ruschak
 *     Ken Stevens
 *     Steve McClure
 *     Ron Koenderink, 2005-2006
 *     Markus Armbruster, 2005-2008
 */

#include <config.h>

#include "commands.h"
#include "nuke.h"
#include "optlist.h"
#include "ship.h"
#include "version.h"

static void show_custom(void);
static void show_opts(int val);
static char *prwrap(char *, char *, int *);

int
vers(void)
{
    time_t now;

    (void)time(&now);
    pr("%s\n\n", version);
    pr("The following parameters have been set for this game:\n");
    pr("World size is %d by %d.\n", WORLD_X, WORLD_Y);
    pr("There can be up to %d countries.\n", MAXNOC);
    pr("By default, countries use %s coordinate system.\n",
       (players_at_00) ? "the deity's" : "their own");
    pr("\n");
    pr("Use the 'show' command to find out the time of the next update.\n");
    pr("The current time is %19.19s.\n", ctime(&now));
    pr("An update consists of %d empire time units.\n", etu_per_update);
    pr("Each country is allowed to be logged in %d minutes a day.\n",
       m_m_p_d);
    if (*game_days != 0)
	pr("Game days are %s\n", game_days);
    if (*game_hours != 0)
	pr("Game hours are %s\n", game_hours);
    pr("It takes %.2f civilians to produce a BTU in one time unit.\n",
       (1.0 / (btu_build_rate * 100.0)));
    pr("\n");

    pr("A non-aggi, 100 fertility sector can grow %.2f food per etu.\n",
       100.0 * fgrate);
    pr("1000 civilians will harvest %.1f food per etu.\n",
       1000.0 * fcrate);
    pr("1000 civilians will give birth to %.1f babies per etu.\n",
       1000.0 * obrate);
    pr("1000 uncompensated workers will give birth to %.1f babies.\n",
       1000.0 * uwbrate);
    pr("In one time unit, 1000 people eat %.1f units of food.\n",
       1000.0 * eatrate);
    pr("1000 babies eat %.1f units of food becoming adults.\n",
       1000.0 * babyeat);
    if (opt_NOFOOD)
	pr("No food is needed!\n");

    pr("\n");

    pr("Banks pay $%.2f in interest per 1000 gold bars per etu.\n",
       bankint * 1000.0);
    pr("1000 civilians generate $%.2f, uncompensated workers $%.2f each time unit.\n",
       1000.0 * money_civ, 1000.0 * money_uw);
    pr("1000 active military cost $%.2f, reserves cost $%.2f.\n",
       -money_mil * 1000.0, -money_res * 1000.0);
    if (rollover_avail_max)
	pr("Up to %d avail can roll over an update.\n",
	   rollover_avail_max);
    if (opt_SLOW_WAR)
	pr("Declaring war will cost you $%i\n\n", War_Cost);
    pr("Happiness p.e. requires 1 happy stroller per %d civ.\n",
       (int)hap_cons / etu_per_update);
    pr("Education p.e. requires 1 class of graduates per %d civ.\n",
       (int)edu_cons / etu_per_update);
    pr("Happiness is averaged over %d time units.\n", (int)hap_avg);
    pr("Education is averaged over %d time units.\n", (int)edu_avg);
    if (opt_ALL_BLEED == 0)
	pr("The technology/research boost you get from your allies is %.2f%%.\n",
	   100.0 / ally_factor);
    else
	pr("The technology/research boost you get from the world is %.2f%%.\n",
	   100.0 / ally_factor);

    pr("Nation levels (tech etc.) decline 1%% every %d time units.\n",
       (int)(level_age_rate));

    pr("Tech Buildup is ");
    if (tech_log_base <= 1.0) {
	pr("not limited\n");
    }
    if (tech_log_base > 1.0) {
	pr("limited to logarithmic growth (base %.2f)", tech_log_base);
	if (easy_tech == 0.0)
	    pr(".\n");
	else
	    pr(" after %0.2f.\n", easy_tech);
    }
    pr("\n");
    pr("\t\t\t\tSectors\tShips\tPlanes\tUnits\n");
    pr("Maximum mobility\t\t%d\t%d\t%d\t%d\n",
       sect_mob_max, ship_mob_max, plane_mob_max, land_mob_max);
    pr("Max mob gain per update\t\t%d\t%d\t%d\t%d\n",
       (int)(sect_mob_scale * (float)etu_per_update),
       (int)(ship_mob_scale * (float)etu_per_update),
       (int)(plane_mob_scale * (float)etu_per_update),
       (int)(land_mob_scale * (float)etu_per_update));
    pr("Max eff gain per update\t\t--\t%d\t%d\t%d\n",
       MIN((int)(ship_grow_scale * (float)etu_per_update), 100),
       MIN((int)(plane_grow_scale * (float)etu_per_update), 100),
       MIN((int)(land_grow_scale * (float)etu_per_update), 100));
    pr("Maintenance cost per update\t--\t%0.1f%%\t%0.1f%%\t%0.1f%%\n",
       money_ship * -100.0 * etu_per_update,
       money_plane * -100.0 * etu_per_update,
       money_land * -100.0 * etu_per_update);
    pr("Max interdiction range\t\t%d\t%d\t--\t%d\n",
       fort_max_interdiction_range,
       ship_max_interdiction_range,
       land_max_interdiction_range);
    pr("\n");
    pr("The maximum amount of mobility used for land unit combat is %0.2f.\n",
       combat_mob);
    if (opt_MOB_ACCESS)
        pr("The starting mobility when acquiring a sector or unit is %d.\n",
	   -(etu_per_update / sect_mob_neg_factor));
    pr("\n");
    pr("Ships on autonavigation may use %i cargo holds per ship.\n", TMAX);
    if (opt_TRADESHIPS) {
	pr("Trade-ships that go at least %d sectors get a return of %.1f%% per sector.\n",
	   trade_1_dist, trade_1 * 100.0);
	pr("Trade-ships that go at least %d sectors get a return of %.1f%% per sector.\n",
	   trade_2_dist, trade_2 * 100.0);
	pr("Trade-ships that go at least %d sectors get a return of %.1f%% per sector.\n",
	   trade_3_dist, trade_3 * 100.0);
	pr("Cashing in trade-ships with an ally nets you a %.1f%% bonus.\n",
	   trade_ally_bonus * 100.0);
	pr("Cashing in trade-ships with an ally nets your ally a %.1f%% bonus.\n\n",
	   trade_ally_cut * 100.0);
    }
    if (opt_MARKET) {
	pr("The tax you pay on selling things on the trading block is %.1f%%\n",
	   (1.00 - tradetax) * 100.0);
	pr("The tax you pay on buying commodities on the market is %.1f%%\n",
	   (buytax - 1.00) * 100.0);
	pr("The amount of time to bid on commodities is %d seconds.\n",
	   MARK_DELAY);
	pr("The amount of time to bid on a unit is %d seconds.\n\n",
	   TRADE_DELAY);
    }

    if (!ef_nelem(EF_NUKE_CHR))
	pr("Nukes are disabled.\n");
    else if (drnuke_const > MIN_DRNUKE_CONST) {
	pr("In order to build a nuke, you need %1.2f times the tech level in research\n",
	   drnuke_const);
	pr("\tExample: In order to build a 300 tech nuke, you need %d research\n\n",
	   (int)(300.0 * drnuke_const));
    }

    pr("Fire ranges are scaled by %.2f.\n", fire_range_factor);
    pr("Flak damage is scaled by %0.2f.\n", flakscale);
    pr("Torpedo damage is 2d%d+%d.\n", torpedo_damage, torpedo_damage - 2);
    pr("The attack factor for para & assault troops is %0.2f.\n",
       assault_penalty);
    pr("%.0f%% of fallout leaks into each surrounding sector.\n",
       fallout_spread * 100.0 * MIN(24, etu_per_update));
    pr("Fallout decays by %.0f%% per update\n",
       100.0 - (decay_per_etu + 6.0) * fallout_spread * MIN(24, etu_per_update) * 100.0);
    pr("\n");
    pr ("Damage to\t\t\tSpills to\n");
    pr("\t      Sector  People  Mater.   Ships  Planes  LandU.\n");
    pr("Sector\t\t --\t%3.0f%%\t100%%\t  0%%\t%3.0f%%\t%3.0f%%\n",
       people_damage * 100.0, unit_damage / 0.07, unit_damage * 100.0);
    pr("People\t\t%3.0f%%\t --\t --\t --\t --\t --\n",
       collateral_dam * 100.0);
    pr("Materials\t%3.0f%%\t --\t --\t --\t --\t --\n",
       collateral_dam * 100.0);
    pr("Efficiency\t%3.0f%%\t --\t --\t --\t --\t --\n",
       collateral_dam * 100.0);
    pr("Ships\t\t%3.0f%%\t100%%\t100%%\t --\t  0%%\t  0%%\n",
       collateral_dam * 100.0);
    pr("Planes\t\t%3.0f%%\t  0%%\t  0%%\t --\t --\t --\n",
       collateral_dam * 100.0);
    pr("Land units\t%3.0f%%\t  0%%\t100%%\t --\t  0%%\t  0%%\n",
       collateral_dam * 100.0);
    pr("\n");
    pr("You can have at most %d BTUs.\n", max_btus);
    pr("You are disconnected after %d minutes of idle time.\n", max_idle);
    pr("\nOptions enabled in this game:\n");
    show_opts(1);
    pr("\n\nOptions disabled in this game:\n");
    show_opts(0);
    pr("\n\nSee \"info Options\" for a detailed list of options and descriptions.\n");
    show_custom();
    pr("\nThe person to annoy if something goes wrong is:\n\t%s\n\t(%s).\n",
       privname, privlog);
    pr("\nYou can get your own copy of the source from "
       "http://www.wolfpackempire.com/\n\n");
    pr("%s", legal);
    return RET_OK;
}

static void
show_custom(void)
{
    char *sep;
    int col, i;

    sep = "\nCustomized in this game:\n\t";
    col = 0;
    for (i = 0; i < EF_MAX; i++) {
	if (ef_flags(i) & EFF_CUSTOM)
	    sep = prwrap(sep, ef_nameof(i), &col);
    }
    if (*sep == ',')
	pr("\nCheck \"show\" for details.\n");
}

static void
show_opts(int val)
{
    char *sep;
    int col;
    struct keymatch *kp;

    sep = "\t";
    col = 0;
    for (kp = configkeys; kp->km_key; kp++) {
	if (!(kp->km_flags & KM_OPTION))
	    continue;
	if (CANT_HAPPEN(kp->km_type != NSC_INT))
	    continue;
	if (!*(int *)kp->km_data != !val)
	    continue;
	sep = prwrap(sep, kp->km_key, &col);
    }
}

static char *
prwrap(char *sep, char *it, int *col)
{
    size_t len = strlen(it);

    if (*col != 0 && *col + len > 70) {
	sep = ",\n\t";
	*col = 0;
    }
    pr("%s%s", sep, it);
    sep = ", ";
    *col += len + 2;
    return sep;
}
