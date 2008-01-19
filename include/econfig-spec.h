/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  econfig-spec.h: Combined include for optlist.h and emp_config.c
 * 
 *  Known contributors to this file:
 *     Marc Olzheim, 2004
 *     Markus Armbruster, 2004-2008
 */

/*
 * This file declares econfig keys.  It expand into the corresponding
 * variable declarations if defined(EMP_CONFIG_H_OUTPUT), and into
 * struct keymatch[] initializers if defined(EMP_CONFIG_C_OUTPUT).
 *
 * EMPCFBOTH() defines econfig keys.  EMPCF_OPT() is a convenience
 * wrapper for options.  EMPCF_COMMENT() defines comments.
 */

/*
 * Preprocessor of MSC C++ 6.0 (VC98) is broken and chokes on empty
 * macro arguments.  Fixed in MSC C++ 7.0.  Work around: pass
 * useless token `unused'.
 */

#if	defined(EMP_CONFIG_C_OUTPUT)

#define	EMPCFONLYC(fvname, vname, vtype, ctype, flags, descr)	\
	{ (fvname), (ctype), &(vname), (flags), (descr) },
#define	EMPCFBOTH(fvname, vname, vtype, ctype, flags, descr)	\
	{ (fvname), (ctype), &(vname), (flags), (descr) },

#elif	defined(EMP_CONFIG_H_OUTPUT)

#define	EMPCFONLYC(fvname, vname, vtype, ctype, flags, descr)
#define	EMPCFBOTH(fvname, vname, vtype, ctype, flags, descr)	\
	extern vtype vname;

#endif	/* EMP_CONFIG_C_OUTPUT || EMP_CONFIG_H_OUTPUT */

#define EMPCF_COMMENT(comment) \
EMPCFONLYC("", emp_config_dummy, unused , NSC_NOTYPE, 0, (comment))

#define EMPCF_OPT(fvname, vname, descr) \
EMPCFBOTH((fvname), (vname), int, NSC_INT, KM_OPTION, (descr))

/* econfig key definitions: */

EMPCF_COMMENT("\n### Server configuration and information")
EMPCFBOTH("custom_tables", custom_tables, char *, NSC_STRING, KM_INTERNAL,
    "Custom configuration table files, separated by space")
EMPCFBOTH("data", gamedir_conf, char *, NSC_STRING, KM_INTERNAL,
    "Directory where this game's data is stored")
EMPCFBOTH("info", infodir_conf, char *, NSC_STRING, KM_INTERNAL,
    "Directory where info pages are stored, can be shared among games")
EMPCF_COMMENT("# Set this to your build tree's info.nr to run the server without\n"
	      "# installing it.")
EMPCFBOTH("builtin", builtindir_conf, char *, NSC_STRING, KM_INTERNAL,
    "Directory where builtin files are stored")
EMPCF_COMMENT("# Set this to your source tree's src/lib/global to run the server\n"
	      "# without installing it, else leave it alone.")
EMPCFBOTH("listen_addr", listen_addr, char *, NSC_STRING, KM_INTERNAL,
    "Local IP address the server should listen on")
EMPCF_COMMENT("# \"\" listens on all, localhost just on the loopback interface.")
EMPCFBOTH("port", loginport, char *, NSC_STRING, KM_INTERNAL,
    "TCP port the server will bind")
EMPCFBOTH("keep_journal", keep_journal, int, NSC_INT, KM_INTERNAL,
    "Enable journal log file")
EMPCFBOTH("post_crash_dump_hook", post_crash_dump_hook, char *, NSC_STRING, KM_INTERNAL,
    "Shell command run right after a crash dump, in the game's data directory")
EMPCFBOTH("privname", privname, char *, NSC_STRING, 0,
    "Name of the deity")
EMPCFBOTH("privlog", privlog, char *, NSC_STRING, 0,
    "E-mail of the deity")
EMPCFBOTH("privip", privip, char *, NSC_STRING, KM_INTERNAL,
    "Deities may connect from these IPs or networks, \"\" allows all")
EMPCFBOTH("WORLD_X", WORLD_X, int, NSC_INT, 0,
    "World size X dimension (enforced to be even by subtracting 1 if necessary)")
EMPCFBOTH("WORLD_Y", WORLD_Y, int, NSC_INT, 0,
    "World size Y dimension")

EMPCF_COMMENT("\n\n### Update policy\n\n"
    "# Note: the update schedule is defined in the file schedule in the\n"
    "# same directory as this file.")
EMPCFBOTH("etu_per_update", etu_per_update, int, NSC_INT, 0,
    "Number of ETUs per update")
EMPCFBOTH("update_window", update_window, int, NSC_INT, 0,
    "Time window the update will occur in after the update time, in seconds")
EMPCFBOTH("pre_update_hook", pre_update_hook, char *, NSC_STRING, KM_INTERNAL,
    "Shell command run right before the update, in the game's data directory")
EMPCFBOTH("update_demand", update_demand, int, NSC_INT, 0,
    "Demand update policy")
EMPCF_COMMENT("# 0 - No demand updates\n"
    "# 1 - Scheduled updates are demand updates\n"
    "# 2 - Demand updates run right after the deciding vote is cast,\n"
    "#     in addition to (non-demand) scheduled updates")
EMPCFBOTH("update_wantmin", update_wantmin, int, NSC_INT, 0,
    "Number of votes required for a demand update")
EMPCFBOTH("update_demandtimes", update_demandtimes, char *, NSC_STRING, 0,
    "Times when unscheduled demand updates can occur, separated by space")
EMPCF_COMMENT("# Give time ranges as HOUR:MINUTE-HOUR:MINUTE, e.g. 20:00-24:00.\n"
	      "# Ranges CANNOT cross midnight.")

EMPCF_COMMENT("\n\n### Game hours restrictions")
EMPCFBOTH("game_days", game_days, char *, NSC_STRING, 0,
    "Days of week the game is up and running, separated by space")
EMPCF_COMMENT("# Give days as Su Mo Tu We Th Fr Sa.")
EMPCFBOTH("game_hours", game_hours, char *, NSC_STRING, 0,
    "Time of day ranges when the game is open, separated by space")
EMPCF_COMMENT("# Give time ranges as HOUR:MINUTE-HOUR:MINUTE, e.g. 20:00-24:00.\n"
	      "# Ranges CANNOT cross midnight.")

EMPCF_COMMENT("\n\n### Options")
EMPCF_OPT("ALL_BLEED", opt_ALL_BLEED,
    "Let tech bleed to everyone, not just allies")
EMPCF_OPT("AUTO_POWER", opt_AUTO_POWER,
    "Power report is updated only automatically, at the update")
EMPCF_OPT("BLITZ", opt_BLITZ,
    "Enable blitz mode")
EMPCF_OPT("BRIDGETOWERS", opt_BRIDGETOWERS,
    "Allow bridge towers")
EMPCF_OPT("EASY_BRIDGES", opt_EASY_BRIDGES,
    "Allow bridge building without bridge heads")
EMPCF_OPT("FALLOUT", opt_FALLOUT,
    "Enable secondary effects caused by radiation")
EMPCF_OPT("FUEL", opt_FUEL,
    "Make ships use fuel to move")
EMPCF_OPT("GODNEWS", opt_GODNEWS,
    "Inform the world when deities give/take away")
EMPCF_OPT("GO_RENEW", opt_GO_RENEW,
    "Make gold and oil renewable resources")
EMPCF_OPT("GUINEA_PIGS", opt_GUINEA_PIGS,
    "Enable experimental stuff not ready for prime time")
EMPCF_OPT("HIDDEN", opt_HIDDEN,
    "Hide information between players")
EMPCF_OPT("INTERDICT_ATT", opt_INTERDICT_ATT,
    "Interdict post-attack move in")
EMPCF_OPT("LANDSPIES", opt_LANDSPIES,
    "Enable the land unit type spies")
EMPCF_OPT("LIMBER", opt_LIMBER,
    "Artillery requires mobility to start firing")
EMPCF_OPT("LOANS", opt_LOANS,
    "Allow bailing out of other countries via S&L scandals")
EMPCF_OPT("LOSE_CONTACT", opt_LOSE_CONTACT,
    "Let contact be lost after a few updates")
EMPCF_OPT("MARKET", opt_MARKET,
    "Enable time-based market and trading")
EMPCF_OPT("MOB_ACCESS", opt_MOB_ACCESS,
    "Update mobility in real-time rather than at the update")
EMPCF_OPT("NOFOOD", opt_NOFOOD,
    "Disable food consumption")
EMPCF_OPT("NOMOBCOST", opt_NOMOBCOST,
    "Don't charge mobility for firing from ships")
EMPCF_OPT("NO_FORT_FIRE", opt_NO_FORT_FIRE,
    "Disable fortress fire")
EMPCF_OPT("NO_PLAGUE", opt_NO_PLAGUE,
    "Disable plague")
EMPCF_OPT("PINPOINTMISSILE", opt_PINPOINTMISSILE,
    "Enable marine missiles")
EMPCF_OPT("RAILWAYS", opt_RAILWAYS,
    "Highways double as rail")
EMPCF_OPT("RES_POP", opt_RES_POP,
    "Population is limited by research")
EMPCF_OPT("SAIL", opt_SAIL,
    "Enable sail command")
EMPCF_OPT("SHOWPLANE", opt_SHOWPLANE,
    "Show planes and land units embarked on ships or land units up for trade")
EMPCF_OPT("SLOW_WAR", opt_SLOW_WAR,
    "Declaring war takes time")
EMPCF_OPT("SUPER_BARS", opt_SUPER_BARS,
    "Make bars immune to damage")
EMPCF_OPT("TECH_POP", opt_TECH_POP,
    "Technology costs more as population rises")
EMPCF_OPT("TRADESHIPS", opt_TRADESHIPS,
    "Enable Tradeships")
EMPCF_OPT("TREATIES", opt_TREATIES,
    "Allow treaties")

EMPCF_COMMENT("\n\n### Countries")
EMPCFBOTH("btu_build_rate", btu_build_rate, float, NSC_FLOAT, 0,
    "Rate at which BTUs accumulate (etu * civ * eff * btu_build_rate)")
EMPCFBOTH("m_m_p_d", m_m_p_d, int, NSC_INT, 0,
    "Maximum minutes per day a country is allowed to be logged in")
EMPCFBOTH("max_btus", max_btus, int, NSC_INT, 0,
    "Maximum number of BTUs a country can have")
EMPCFBOTH("max_idle", max_idle, int, NSC_INT, 0,
    "Maximum number of minutes a player can sit idle while logged in")
EMPCFBOTH("players_at_00", players_at_00, int, NSC_INT, 0,
    "Players have their coordinate system at deity 0,0 (0 - no, 1 - yes)")
EMPCFBOTH("at_least_one_100", at_least_one_100, int, NSC_INT, KM_INTERNAL,
    "Initialize new countries with at least one sector with 100 of all resource")
EMPCFBOTH("start_cash", start_cash, long, NSC_LONG, KM_INTERNAL,
    "Starting cash for new countries")
EMPCFBOTH("war_cost", War_Cost, int, NSC_INT, 0,
    "Cost to declare war (if SLOW_WAR is on)")

EMPCF_COMMENT("\n\n### Technology/Research/Education/Happiness")
EMPCFBOTH("easy_tech", easy_tech, float, NSC_FLOAT, 0,
    "Amount of tech built with no penalty")
EMPCFBOTH("start_tech", start_technology, float, NSC_FLOAT, KM_INTERNAL,
    "Starting technology for new countries")
EMPCFBOTH("start_happy", start_happiness, float, NSC_FLOAT, KM_INTERNAL,
    "Starting happiness for new countries")
EMPCFBOTH("start_research", start_research, float, NSC_FLOAT, KM_INTERNAL,
    "Starting research for new countries")
EMPCFBOTH("start_edu", start_education, float, NSC_FLOAT, KM_INTERNAL,
    "Starting education for new countries")
EMPCFBOTH("level_age_rate", level_age_rate, float, NSC_FLOAT, 0,
    "ETU rate at which tech decays (0 -> no decline)")
EMPCFBOTH("tech_log_base", tech_log_base, float, NSC_FLOAT, 0,
    "Log base to apply to tech breakthroughs above the easy tech level")
EMPCFBOTH("ally_factor", ally_factor, float, NSC_FLOAT, 0,
    "Shared tech with allies (1 / ally_factor)")
EMPCFBOTH("edu_avg", edu_avg, float, NSC_FLOAT, 0,
    "Number of ETUs education is averaged over")
EMPCFBOTH("hap_avg", hap_avg, float, NSC_FLOAT, 0,
    "Number of ETUs happiness is averaged over")
EMPCFBOTH("edu_cons", edu_cons, double, NSC_DOUBLE, 0,
    "Education consumption (1 breakthrough per edu_cons)")
EMPCFBOTH("hap_cons", hap_cons, double, NSC_DOUBLE, 0,
    "Happiness consumption (1 breakthrough per hap_cons)")

EMPCF_COMMENT("\n\n### Sectors")
EMPCFBOTH("startmob", startmob, int, NSC_INT, KM_INTERNAL,
    "Starting mobility for sanctuaries")
EMPCFBOTH("sect_mob_scale", sect_mob_scale, float, NSC_FLOAT, 0,
    "Sector mobility accumulation per ETU")
EMPCFBOTH("sect_mob_max", sect_mob_max, int, NSC_INT, 0,
    "Maximum mobility for sectors")
EMPCFBOTH("buil_bh", buil_bh, int, NSC_INT, 0,
    "Number of hcms required to build a bridge span")
EMPCFBOTH("buil_bc", buil_bc, double, NSC_DOUBLE, 0,
    "Cash required to build a bridge span")
EMPCFBOTH("buil_bt", buil_bt, double, NSC_DOUBLE, 0,
    "Technology required to build a bridge span")
EMPCFBOTH("buil_tower_bh", buil_tower_bh, int, NSC_INT, 0,
    "Number of hcms required to build a bridge tower")
EMPCFBOTH("buil_tower_bc", buil_tower_bc, double, NSC_DOUBLE, 0,
    "Cash required to build a bridge tower")
EMPCFBOTH("buil_tower_bt", buil_tower_bt, double, NSC_DOUBLE, 0,
    "Technology required to build a bridge tower")

EMPCF_COMMENT("\n\n### Land Units")
EMPCFBOTH("land_mob_scale", land_mob_scale, float, NSC_FLOAT, 0,
    "Land unit mobility accumulation per ETU")
EMPCFBOTH("land_grow_scale", land_grow_scale, float, NSC_FLOAT, 0,
    "How fast efficiency grows for land units, per ETU")
EMPCFBOTH("land_mob_max", land_mob_max, int, NSC_INT, 0,
    "Maximum mobility for land units")
EMPCFBOTH("money_land", money_land, double, NSC_DOUBLE, 0,
    "Cost per ETU to maintain land units (fraction of unit price)")
EMPCFBOTH("morale_base", morale_base, int, NSC_INT, KM_INTERNAL,
    "Base level for setting morale of land units")

EMPCF_COMMENT("\n\n### Planes")
EMPCFBOTH("plane_mob_scale", plane_mob_scale, float, NSC_FLOAT, 0,
    "Plane mobility accumulation per ETU")
EMPCFBOTH("plane_grow_scale", plane_grow_scale, float, NSC_FLOAT, 0,
    "How fast efficiency grows for planes each update, per ETU")
EMPCFBOTH("plane_mob_max", plane_mob_max, int, NSC_INT, 0,
    "Maximum mobility for planes")
EMPCFBOTH("money_plane", money_plane, double, NSC_DOUBLE, 0,
    "Cost per ETU to maintain planes (fraction of plane price)")

EMPCF_COMMENT("\n\n### Ships")
EMPCFBOTH("ship_mob_scale", ship_mob_scale, float, NSC_FLOAT, 0,
    "Ship mobility accumulation per ETU")
EMPCFBOTH("ship_grow_scale", ship_grow_scale, float, NSC_FLOAT, 0,
    "How fast efficiency grows for ships each update, per ETU")
EMPCFBOTH("ship_mob_max", ship_mob_max, int, NSC_INT, 0,
    "Maximum mobility for ships")
EMPCFBOTH("money_ship", money_ship, double, NSC_DOUBLE, 0,
    "Cost per ETU to maintain ships (fraction of ship price)")
EMPCFBOTH("torpedo_damage", torpedo_damage, int, NSC_INT, 0,
    "Torpedo damage (damage is X + 1dX + 1dX)")

EMPCF_COMMENT("\n\n### Combat/Damage")
EMPCFBOTH("fort_max_interdiction_range", fort_max_interdiction_range, int, NSC_INT, 0,
    "Maximum range (in sectors) a fort will try to interdict another country")
EMPCFBOTH("land_max_interdiction_range", land_max_interdiction_range, int, NSC_INT, 0,
    "Maximum range (in sectors) a land unit will try to interdict another country")
EMPCFBOTH("ship_max_interdiction_range", ship_max_interdiction_range, int, NSC_INT, 0,
    "Maximum range (in sectors) a ship will try to interdict another country")
EMPCFBOTH("flakscale", flakscale, float, NSC_FLOAT, 0,
    "Scale factor for flak damage")
EMPCFBOTH("combat_mob", combat_mob, double, NSC_DOUBLE, 0,
    "How much mobility do units spend for combat (* casualties/bodies)")
EMPCFBOTH("people_damage", people_damage, double, NSC_DOUBLE, 0,
    "People take this amount of normal damage")
EMPCFBOTH("unit_damage", unit_damage, double, NSC_DOUBLE, 0,
    "Land units take this amount of normal damage")
EMPCFBOTH("collateral_dam", collateral_dam, double, NSC_DOUBLE, 0,
    "Side effect damage amount done to sector")
EMPCFBOTH("assault_penalty", assault_penalty, double, NSC_DOUBLE, 0,
    "Attack strength multiplier for paratroopers and assaulting")
EMPCFBOTH("fire_range_factor", fire_range_factor, float, NSC_FLOAT, 0,
    "Scale normal firing ranges by this amount")
EMPCFBOTH("sect_mob_neg_factor", sect_mob_neg_factor, int, NSC_INT, 0,
    "Initial mobility for MOB_ACCESS (-etu_per_update / sect_mob_neg_factor)")
EMPCF_COMMENT("# Applies to sector takeover, ship build, plane or land unit build, and trade.")
EMPCFBOTH("mission_mob_cost", mission_mob_cost, double, NSC_DOUBLE, 0,
    "Cost to put something on a mission (percentage of max mob)")

EMPCF_COMMENT("\n\n### Populace")
EMPCFBOTH("uwbrate", uwbrate, double, NSC_DOUBLE, 0,
    "Birth rate for uncompensated workers")
EMPCFBOTH("money_civ", money_civ, double, NSC_DOUBLE, 0,
    "Money gained from taxes on a civilian in one ETU")
EMPCFBOTH("money_mil", money_mil, double, NSC_DOUBLE, 0,
    "Money gained from taxes on an active soldier in one ETU")
EMPCFBOTH("money_res", money_res, double, NSC_DOUBLE, 0,
    "Money gained from taxes on a soldier on active reserve in one ETU")
EMPCFBOTH("money_uw", money_uw, double, NSC_DOUBLE, 0,
    "Money gained from taxes on an uncompensated worker in one ETU")
EMPCFBOTH("babyeat", babyeat, double, NSC_DOUBLE, 0,
    "Amount of food to mature 1 baby into a civilian")
EMPCFBOTH("bankint", bankint, double, NSC_DOUBLE, 0,
    "Bank dollar gain (per bar per etu)")
EMPCFBOTH("eatrate", eatrate, double, NSC_DOUBLE, 0,
    "Food eating rate for mature people")
EMPCFBOTH("fcrate", fcrate, double, NSC_DOUBLE, 0,
    "Food cultivation rate (* workforce in sector)")
EMPCFBOTH("fgrate", fgrate, double, NSC_DOUBLE, 0,
    "Food growth rate (* fertility of sector)")
EMPCFBOTH("obrate", obrate, double, NSC_DOUBLE, 0,
    "Civilian birth rate")
EMPCFBOTH("rollover_avail_max", rollover_avail_max, int, NSC_INT, 0,
    "Maximum avail that can roll over an update")

EMPCF_COMMENT("\n\n### Nukes")
EMPCFBOTH("decay_per_etu", decay_per_etu, double, NSC_DOUBLE, 0,
    "Decay of fallout per ETU")
EMPCFBOTH("fallout_spread", fallout_spread, double, NSC_DOUBLE, 0,
    "Amount of fallout that leaks into surrounding sectors")
EMPCFBOTH("drnuke_const", drnuke_const, float, NSC_FLOAT, 0,
    "Research needed to build a nuke, as fraction of the tech needed")
EMPCF_COMMENT("# A common value is 0.33.")

EMPCF_COMMENT("\n\n### Market/Trade")
EMPCFBOTH("MARK_DELAY", MARK_DELAY, int, NSC_INT, 0,
    "Time commodities stay on the market for bidding (seconds)")
EMPCFBOTH("TRADE_DELAY", TRADE_DELAY, int, NSC_INT, 0,
    "Time ships, planes, lands and nukes stay on the market for bidding (seconds)")
EMPCFBOTH("buytax", buytax, double, NSC_DOUBLE, 0,
    "Tax (in percentage points) charged to the buyer on market purchases")
EMPCFBOTH("tradetax", tradetax, double, NSC_DOUBLE, 0,
    "Amount of a trade transaction the seller makes (the rest is tax)")

EMPCF_COMMENT("\n\n### Trade ships")
EMPCFBOTH("trade_1_dist", trade_1_dist, int, NSC_INT, 0,
    "Less than this distance gets no money for cashing in")
EMPCFBOTH("trade_2_dist", trade_2_dist, int, NSC_INT, 0,
    "Less than this distance gets trade_1 money for cashing in")
EMPCFBOTH("trade_3_dist", trade_3_dist, int, NSC_INT, 0,
    "Less than this distance gets trade_2 money for cashing in, more gets trade_3")
EMPCFBOTH("trade_1", trade_1, float, NSC_FLOAT, 0,
    "Return per sector on trade_1 distance amount")
EMPCFBOTH("trade_2", trade_2, float, NSC_FLOAT, 0,
    "Return per sector on trade_2 distance amount")
EMPCFBOTH("trade_3", trade_3, float, NSC_FLOAT, 0,
    "Return per sector on trade_3 distance amount")
EMPCFBOTH("trade_ally_bonus", trade_ally_bonus, float, NSC_FLOAT, 0,
    "Bonus you get for cashing in with an ally")
EMPCFBOTH("trade_ally_cut", trade_ally_cut, float, NSC_FLOAT, 0,
    "Bonus your ally gets for you cashing in with them")

EMPCF_COMMENT("\n\n### Miscellaneous")
EMPCFBOTH("disabled_commands", disabled_commands, char *, NSC_STRING, KM_INTERNAL,
	  "Disabled commands, separated by space")
EMPCFBOTH("anno_keep_days", anno_keep_days, int, NSC_INT, KM_INTERNAL,
	  "How long until announcements expire (<0 means never)")
EMPCFBOTH("news_keep_days", news_keep_days, int, NSC_INT, KM_INTERNAL,
	  "How long until news expire")
EMPCFBOTH("fuel_mult", fuel_mult, int, NSC_INT, 0,
    "Multiplier for fuel to mobility calculation")
EMPCFBOTH("lost_items_timeout", lost_items_timeout, int, NSC_INT, KM_INTERNAL,
    "Seconds before a lost item is timed out of the database")

/* Sentinel */
EMPCFONLYC(NULL, emp_config_dummy, NULL, NSC_NOTYPE, 0, NULL)

#undef EMPCFONLYC
#undef EMPCFBOTH
#undef EMPCF_COMMENT
