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
 *  econfig-spec.h: Combined include for optlist.h and emp_config.c
 */

#if	defined(EMP_CONFIG_C_OUTPUT)

#define	EMPCFONLYC(fvname, vname, vtype, ctype, num, descr)	\
	{ (fvname), (ctype), &(vname), (num), (descr) },
#define	EMPCFBOTH(fvname, vname, vtype, ctype, num, descr)	\
	{ (fvname), (ctype), &(vname), (num), (descr) },

#elif	defined(EMP_CONFIG_H_OUTPUT)

#define	EMPCFONLYC(fvname, vname, vtype, ctype, num, descr)
#define	EMPCFBOTH(fvname, vname, vtype, ctype, num, descr)	\
	extern vtype vname;

#endif	/* EMP_CONFIG_C_OUTPUT || EMP_CONFIG_H_OUTPUT */

/* things that can be changed */
EMPCFONLYC("", emp_config_dummy, int, intset, 0,
    "\n### Server configuration and information")
EMPCFBOTH("data", datadir, s_char *, optstrset, 0,
    "Directory the data is stored in")
EMPCFBOTH("info", infodir, s_char *, optstrset, 0,
    "Directory the info pages are stored in")
EMPCFBOTH("port", loginport, s_char *, optstrset, 0,
    "TCP/IP port the server will start up on")
EMPCFBOTH("privname", privname, s_char *, optstrset, 0,
    "Name of the deity")
EMPCFBOTH("privlog", privlog, s_char *, optstrset, 0,
    "E-mail of the deity")
EMPCFBOTH("WORLD_X", WORLD_X, int, worldxset, 0,
    "World size X dimension (enforced to be even by subtracting 1 if necessary)")
EMPCFBOTH("WORLD_Y", WORLD_Y, int, intset, 0,
    "World size Y dimension")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Update policy")
EMPCFBOTH("update_policy", update_policy, int, intset, 0,
    "0 - normal, 1 - update_times, 2 - blitz, 3 - demand only")
EMPCFBOTH("etu_per_update", etu_per_update, int, intset, 0,
    "Number of ETUs per update")
EMPCFBOTH("s_p_etu", s_p_etu, int, intset, 0,
    "Seconds per etu, updates will occur every s_p_etu * etu_per_update seconds")
EMPCFBOTH("adj_update", adj_update, int, intset, 0,
    "Move the update forward or backward (in seconds)")
EMPCFBOTH("update_window", update_window, int, intset, 0,
    "Window the update will occur in (in seconds) before and after the update time")
EMPCFBOTH("update_times", update_times, s_char *, optstrset, 0,
    "Times when updates occur under policy #1.  Must coincide with schedule.")
EMPCFBOTH("hourslop", hourslop, int, intset, 0,
    "Number of minutes update check can slip to match update_times")
EMPCFBOTH("blitz_time", blitz_time, int, intset, 0,
    "Number of minutes between updates under policy #2.")

EMPCFONLYC("", emp_config_dummy, int, intset, 0,
    "\n\n### Demand update policy")
EMPCFBOTH("update_demandpolicy", update_demandpolicy, int, intset, 0,
    "0 - emp_tm checks, 1 - after setting, 2 - demand updates disabled")
EMPCFBOTH("update_wantmin", update_wantmin, int, intset, 0,
    "number of requests needed for demand update")
EMPCFBOTH("update_missed", update_missed, int, intset, 0,
    "number of demand updates country can miss before veto update")
EMPCFBOTH("update_demandtimes", update_demandtimes, s_char *, optstrset, 0,
    "Times when demand updates can occur.  Ranges CANNOT cross midnight.")

EMPCFONLYC("", emp_config_dummy, int, intset, 0,
    "\n\n### Game hours restrictions")
EMPCFBOTH("game_days", game_days, s_char *, optstrset, 0,
    "Days game is up and running (Su Mo Tu We Th Fr Sa)")
EMPCFBOTH("game_hours", game_hours, s_char *, optstrset, 0,
    "Hours game is up and running (6:00-18:00)")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Options\n")
EMPCFONLYC("option", emp_config_dummy, option, optionset, 0, NULL)
EMPCFONLYC("nooption", emp_config_dummy, nooption, optiondel, 0, NULL)


EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Countries")
EMPCFBOTH("btu_build_rate", btu_build_rate, float, floatset, 0,
    "Rate at which BTUs accumulate (etu * civ * eff * btu_build_rate)")
EMPCFBOTH("m_m_p_d", m_m_p_d, int, intset, 0,
    "Maximum minutes per day a country is allowed to be logged in")
EMPCFBOTH("max_btus", max_btus, int, intset, 0,
    "Maximum number of BTUs a country can have")
EMPCFBOTH("max_idle", max_idle, int, intset, 0,
    "Maximum number of minutes a player can sit idle while logged in")
EMPCFBOTH("players_at_00", players_at_00, int, intset, 0,
    "Players have their coordinate system at deity 0,0 (0 - no, 1 - yes)")
EMPCFBOTH("at_least_one_100", at_least_one_100, int, intset, 0,
    "Initialize new countries with at least one sector with 100 of all resource")
EMPCFBOTH("powe_cost", powe_cost, double, doubleset, 0,
    "Number of BTUs needed to generate a new power report")
EMPCFBOTH("war_cost", War_Cost, int, intset, 0,
    "Cost to declare war (if SLOW_WAR is on)")

EMPCFONLYC("", emp_config_dummy, int, intset, 0,
    "\n\n### Technology/Research/Education/Happiness")
EMPCFBOTH("easy_tech", easy_tech, float, floatset, 0,
    "Amount of tech built with no penalty")
EMPCFBOTH("hard_tech", hard_tech, float, floatset, 0,
    "Amount of in-efficiently built tech")
EMPCFBOTH("start_tech", start_technology, float, floatset, 0,
    "Starting technology for new countries")
EMPCFBOTH("start_happy", start_happiness, float, floatset, 0,
    "Starting happiness for new countries")
EMPCFBOTH("start_research", start_research, float, floatset, 0,
    "Starting research for new countries")
EMPCFBOTH("start_edu", start_education, float, floatset, 0,
    "Starting education for new countries")
EMPCFBOTH("level_age_rate", level_age_rate, float, floatset, 0,
    "ETU rate at which tech decays (0 -> no decline)")
EMPCFBOTH("tech_log_base", tech_log_base, float, floatset, 0,
    "Log base to apply to tech breakthroughs above the easy tech level")
EMPCFBOTH("ally_factor", ally_factor, float, floatset, 0,
    "Shared tech with allies (1 / ally_factor)")
EMPCFBOTH("edu_avg", edu_avg, float, floatset, 0,
    "Number of ETUs education is averaged over")
EMPCFBOTH("hap_avg", hap_avg, float, floatset, 0,
    "Number of ETUs happiness is averaged over")
EMPCFBOTH("edu_cons", edu_cons, double, doubleset, 0,
    "Education consumption (1 breakthrough per edu_cons)")
EMPCFBOTH("hap_cons", hap_cons, double, doubleset, 0,
    "Happiness consumption (1 breakthrough per hap_cons)")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Sectors")
EMPCFBOTH("startmob", startmob, int, intset, 0,
    "Starting mobility for sanctuaries")
EMPCFBOTH("sect_mob_scale", sect_mob_scale, float, floatset, 0,
    "Sector mobility accumulation (sect_mob_scale * ETUs per update)")
EMPCFBOTH("sect_mob_max", sect_mob_max, int, intset, 0,
    "Maximum mobility for sectors")
EMPCFBOTH("buil_bh", buil_bh, int, intset, 0,
    "Number of hcms required to build a bridge span")
EMPCFBOTH("buil_bc", buil_bc, double, doubleset, 0,
    "Cash required to build a bridge span")
EMPCFBOTH("buil_bt", buil_bt, double, doubleset, 0,
    "Technology required to build a bridge span")
EMPCFBOTH("buil_tower_bh", buil_tower_bh, int, intset, 0,
    "Number of hcms required to build a bridge tower")
EMPCFBOTH("buil_tower_bc", buil_tower_bc, double, doubleset, 0,
    "Cash required to build a bridge tower")
EMPCFBOTH("buil_tower_bt", buil_tower_bt, double, doubleset, 0,
    "Technology required to build a bridge tower")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Land Units")
EMPCFBOTH("land_mob_scale", land_mob_scale, float, floatset, 0,
    "Land unit mobility accumulation (land_mob_scale * ETUs per update)")
EMPCFBOTH("land_grow_scale", land_grow_scale, int, intset, 0,
    "How fast efficiency grows for land units each update (* ETUs)")
EMPCFBOTH("land_mob_max", land_mob_max, int, intset, 0,
    "Maximum mobility for land units")
EMPCFBOTH("money_land", money_land, double, doubleset, 0,
    "Cost per ETU to maintain land units (percentage of unit price)")
EMPCFBOTH("morale_base", morale_base, int, intset, 0,
    "Base level for setting morale of land units")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Planes")
EMPCFBOTH("plane_mob_scale", plane_mob_scale, float, floatset, 0,
    "Plane mobility accumulation (plane_mob_scale * ETUs per update)")
EMPCFBOTH("plane_grow_scale", plane_grow_scale, int, intset, 0,
    "How fast efficiency grows for planes each update (* ETUs)")
EMPCFBOTH("plane_mob_max", plane_mob_max, int, intset, 0,
    "Maximum mobility for planes")
EMPCFBOTH("money_plane", money_plane, double, doubleset, 0,
    "Cost per ETU to maintain planes (percentage of plane price)")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Ships")
EMPCFBOTH("ship_mob_scale", ship_mob_scale, float, floatset, 0,
    "Ship mobility accumulation (ship_mob_scale * ETUs per update)")
EMPCFBOTH("ship_grow_scale", ship_grow_scale, int, intset, 0,
    "How fast efficiency grows for ships each update (* ETUs)")
EMPCFBOTH("ship_mob_max", ship_mob_max, int, intset, 0,
    "Maximum mobility for ships")
EMPCFBOTH("money_ship", money_ship, double, doubleset, 0,
    "Cost per ETU to maintain ships (percentage of ship price)")
EMPCFBOTH("torpedo_damage", torpedo_damage, int, intset, 0,
    "Torpedo damage (damage is X + 1dX + 1dX)")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Combat/Damage")
EMPCFBOTH("fort_max_interdiction_range", fort_max_interdiction_range, int, intset, 0,
    "Maximum range (in sectors) a fort will try to interdict another country")
EMPCFBOTH("land_max_interdiction_range", land_max_interdiction_range, int, intset, 0,
    "Maximum range (in sectors) a land unit will try to interdict another country")
EMPCFBOTH("ship_max_interdiction_range", ship_max_interdiction_range, int, intset, 0,
    "Maximum range (in sectors) a ship will try to interdict another country")
EMPCFBOTH("flakscale", flakscale, double, doubleset, 0,
    "Scale factor for flak damage")
EMPCFBOTH("combat_mob", combat_mob, double, doubleset, 0,
    "How much mobility do units spend for combat (* casualties/bodies)")
EMPCFBOTH("people_damage", people_damage, double, doubleset, 0,
    "People take this amount of normal damage")
EMPCFBOTH("unit_damage", unit_damage, double, doubleset, 0,
    "Land units take this amount of normal damage")
EMPCFBOTH("collateral_dam", collateral_dam, double, doubleset, 0,
    "Side effect damage amount done to sector")
EMPCFBOTH("assault_penalty", assault_penalty, double, doubleset, 0,
    "Amount of normal attacking efficiency for paratroopers and assaulting")
EMPCFBOTH("fire_range_factor", fire_range_factor, float, floatset, 0,
    "Scale normal firing ranges by this amount")
EMPCFBOTH("sect_mob_neg_factor", sect_mob_neg_factor, int, intset, 0,
    "Amount of negative mobility a sector has after takeover (ETU / x) (MOB_ACCESS)")
EMPCFBOTH("mission_mob_cost", mission_mob_cost, double, doubleset, 0,
    "Cost to put something on a mission (percentage of max mob)")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Populace")
EMPCFBOTH("uwbrate", uwbrate, double, doubleset, 0,
    "Birth rate for uw's")
EMPCFBOTH("money_civ", money_civ, double, doubleset, 0,
    "Money gained from taxes on a civilian in one ETU")
EMPCFBOTH("money_mil", money_mil, double, doubleset, 0,
    "Money gained from taxes on an active soldier in one ETU")
EMPCFBOTH("money_res", money_res, double, doubleset, 0,
    "Money gained from taxes on a soldier on active reserve in one ETU")
EMPCFBOTH("money_uw", money_uw, double, doubleset, 0,
    "Money gained from taxes on an uncompensated worker in one ETU")
EMPCFBOTH("babyeat", babyeat, double, doubleset, 0,
    "Amount of food to mature 1 baby into a civilian")
EMPCFBOTH("bankint", bankint, double, doubleset, 0,
    "Bank dollar gain (per bar per etu)")
EMPCFBOTH("eatrate", eatrate, double, doubleset, 0,
    "Food eating rate for mature people")
EMPCFBOTH("fcrate", fcrate, double, doubleset, 0,
    "Food cultivation rate (* workforce in sector)")
EMPCFBOTH("fgrate", fgrate, double, doubleset, 0,
    "Food growth rate (* fertility of sector)")
EMPCFBOTH("obrate", obrate, double, doubleset, 0,
    "Civilian birth rate")
EMPCFBOTH("rollover_avail_max", rollover_avail_max, int, intset, 0,
    "Maximum avail that can roll over an update")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Nukes")
EMPCFBOTH("decay_per_etu", decay_per_etu, double, doubleset, 0,
    "Decay of fallout per ETU")
EMPCFBOTH("fallout_spread", fallout_spread, double, doubleset, 0,
    "Amount of fallout that leaks into surrounding sectors")
EMPCFBOTH("drnuke_const", drnuke_const, float, floatset, 0,
    "Amount of research to tech needed to build a nuke (if DR_NUKE is on)")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Market/Trade")
EMPCFBOTH("MARK_DELAY", MARK_DELAY, int, intset, 0,
    "Number of seconds commodities stay on the market for bidding")
EMPCFBOTH("TRADE_DELAY", TRADE_DELAY, int, intset, 0,
    "Number of seconds ships, planes, and units stay on the market for bidding")
EMPCFBOTH("buytax", buytax, double, doubleset, 0,
    "Tax (in percentage points) charged to the buyer on market purchases")
EMPCFBOTH("tradetax", tradetax, double, doubleset, 0,
    "Amount of a trade transaction the seller makes (the rest is tax)")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Trade ships")
EMPCFBOTH("trade_1_dist", trade_1_dist, int, intset, 0,
    "Less than this distance no money for cashing in")
EMPCFBOTH("trade_2_dist", trade_2_dist, int, intset, 0,
    "Less than this distance gets trade_1 money for cashing in")
EMPCFBOTH("trade_3_dist", trade_3_dist, int, intset, 0,
    "Less than this distance gets trade_2 money for cashing in (>= gets trade_3")
EMPCFBOTH("trade_1", trade_1, float, floatset, 0,
    "Return per sector on trade_1 distance amount")
EMPCFBOTH("trade_2", trade_2, float, floatset, 0,
    "Return per sector on trade_2 distance amount")
EMPCFBOTH("trade_3", trade_3, float, floatset, 0,
    "Return per sector on trade_3 distance amount")
EMPCFBOTH("trade_ally_bonus", trade_ally_bonus, float, floatset, 0,
    "Bonus you get for cashing in with an ally")
EMPCFBOTH("trade_ally_cut", trade_ally_cut, float, floatset, 0,
    "Bonus your ally gets for you cashing in with them")

EMPCFONLYC("", emp_config_dummy, int, intset, 0, "\n\n### Misc.")
EMPCFBOTH("anno_keep_days", anno_keep_days, int, intset, 7,
	  "How long until announcements expire (<0 means never)")
EMPCFBOTH("fuel_mult", fuel_mult, int, intset, 0,
    "Multiplier for fuel to mobility calculation")
EMPCFBOTH("lost_items_timeout", lost_items_timeout, int, intset, 0,
    "Seconds before a lost item is timed out of the database")

EMPCFONLYC(NULL, emp_config_dummy, NULL, NULL, 0, NULL)

#undef	EMPCFONLYC
#undef	EMPCFBOTH
