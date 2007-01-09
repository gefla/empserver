/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  constants.c: Global constants
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1996
 */

#include <config.h>

#include "wantupd.h"

/* Name of the deity */
char *privname = "Deity forgot to edit econfig";
/* E-mail of the deity */
char *privlog = "careless@invalid";
/* Divine hosts and networks */
char *privip = "127.0.0.1 ::1 ::ffff:127.0.0.1";

char *disabled_commands = "";

int keep_journal = 0;		/* journal log file enabled */

int WORLD_X = 64;		/* World size - x */
int WORLD_Y = 32;		/* World size - y */

int MARK_DELAY = 7200;		/* Seconds to bid on commodities */
int TRADE_DELAY = 7200;		/* Seconds to bid on units */

int m_m_p_d = 1440;		/* max mins of play per day (per country) */
int s_p_etu = 10;		/* seconds per Empire Time Unit */
int etu_per_update = 60;	/* # of etu's per update */
int adj_update = 0;		/* update time adjustment, in seconds */
int update_window = 0;		/* update window adjustment, in seconds */
int hourslop = 5;		/* update_times matching fuzz, in minutes */
char *update_times = "";	/* update times for policy UDP_TIMES */
int update_policy = UDP_DEFAULT; /* update policy for regular updates */
int update_demandpolicy = UDDEM_DEFAULT; /* update policy for demand updates */
int update_missed = 999;	/* demand updates missed before veto */
int update_wantmin = 1;		/* votes required for demand update */
int blitz_time = 10;		/* number of minutes between blitz updates */
char *update_demandtimes = "";	/* demand update time ranges */
char *game_days = "";		/* days game is running */
char *game_hours = "";		/* hours game is running */
char *pre_update_hook = "";
int max_idle = 15;		/* session dies after max_idle minutes idle */

int sect_mob_max = 127;		/* sector mobility limits */
float sect_mob_scale = 1.0;	/* accumulation multiplier */
int land_mob_max = 127;		/* land mobility limits */
float land_mob_scale = 1.0;	/* accumulation multiplier */
int ship_mob_max = 127;		/* ship mobility limits */
float ship_mob_scale = 1.5;	/* accumulation multiplier */
int plane_mob_max = 127;	/* plane mobility limits */
float plane_mob_scale = 1.0;	/* accumulation multiplier */

float fire_range_factor = 1.0;	/* Increase/reduce firing ranges */

int morale_base = 42;		/* base for morale */

/* opt_MOB_ACCESS */
int sect_mob_neg_factor = 2;	/* ETU/neg_factor = negative amount of mobility
				   a sector has after being taken */

int anno_keep_days = 7;		/* How long until annos expire (<0 never) */
int news_keep_days = 10;	/* How long until news expire (<0 never) */
int lost_items_timeout = 172800;	/* How long before removing from database */

double combat_mob = 5.0;	/* how much mob do units spend for combat? */

/* if you find that naving is taking too long, try reducing these */
int fort_max_interdiction_range = 8;
int ship_max_interdiction_range = 8;
int land_max_interdiction_range = 8;

double mission_mob_cost = 0.0;	/* Cost is percentage of max mob */

double unit_damage = 0.30;	/* Units take this fraction of normal damage */
double people_damage = 1.00;	/* Civs/mil/uw take this fraction of damage */
double collateral_dam = 0.10;	/* Side effect damage to sector */
double assault_penalty = 0.50;	/* attack factor for para & assault troops */

/* opt_FUEL */
int fuel_mult = 10;		/* 1 fuel = 10 mob */

float land_grow_scale = 2.0;	/* how fast eff grows for land units (xETUS) */
float ship_grow_scale = 3.0;	/* how fast eff grows for ships (xETUS) */
float plane_grow_scale = 2.0;	/* how fast eff grows for planes (xETUS) */

double fgrate = 0.0012;		/* food growth rate (dt * fert) */
double fcrate = 0.0013;		/* food cultivate rate (dt * workforce) */
double eatrate = 0.0005;	/* food eating rate (dt * people) */
double babyeat = 0.0060;	/* food to mature 1 baby into a civilian */

double obrate = 0.005;		/* other sectors birth rate */
double uwbrate = 0.0025;	/* uncompensated worker birth rate */
				/* values > 0.25 for either will overflow */
int rollover_avail_max = 0;	/* max. avail that can roll over an update */

/* opt_FALLOUT */
double decay_per_etu = 0.006;	/* This gives a half life of ? etus,
				   about ? days.
				   half life in Jt
				   log(.5) / log(1-decay_per_etu)
				   Calculate your own if you don't like it.
				   If you want a specific half life you can
				   calc it with this formula:
				   decay_per_etu = 1 - (.5)^(1/etus)
				   due to the discrete nature of empire
				   you can not get a precise number
				   without intensive simulation
				   (more than 3 digits). */
double fallout_spread = 0.005;	/* fraction of fallout that leaks into 
				   each surrounding sector */
/* end opt_FALLOUT */

double bankint = 0.25;		/* bank interest rate (dt * bars) */

/* Note in the taxes below:
   tradetax - this is charged to the seller, so it is < 1 (because the seller
              gets the (price paid * tradetax)
   buytax - this is charged to the buyer, so it is > 1 (because the buyer is
              charged (price paid * buytax).
   Not perfect, but it works. :) */

double tradetax = 0.99;		/* Tax charged on trade */
double buytax = 1.0;		/* Tax charged on market purchases */
int startmob = 127;		/* Sanctuary starting mobility */
float flakscale = 1.75f;	/* Scale factor for flak damage */

/* money gained from taxes, paid to military, and reservists */
double money_civ = 0.0083333;
double money_uw = 0.0017777;
double money_mil = -0.0833333;
double money_res = -0.0083333;

/* pct cost per ETU for maintaining a ship or plane or land unit */
double money_plane = -0.001;
double money_ship = -0.001;
double money_land = -0.001;

/* edu and hap consumption factors -- hap_cons civs need 1 hap --> hlev++ */
double hap_cons = 600000.0;
double edu_cons = 600000.0;

/* hap and edu avg mean that the weight on current happiness is
 *  (cur_hap * hap_avg + hap_prod * etu) / (hap_avg + etu);  		*/
float hap_avg = 16.0 * 3.0;
float edu_avg = 16.0 * 12.0;


/* tech build limitations.  */
float easy_tech = 1.00;		/* amount of tech built with no penality */
float tech_log_base = 2.0;	/* base of log to take of in-efficient tech */

float ally_factor = 2.0;	/* shared tech with allies = 1/factor */
float level_age_rate = 96.0;	/* 1% per 96 etu's; 0 -> no decline */


int players_at_00 = 0;		/* players coord system on deity 0,0? */
int at_least_one_100 = 1;	/* init player with 100/100/100/100 sector? */



float btu_build_rate = 0.0012;	/* etu * civ * eff * btu_build_rate */
				/* 8 * 999 * 100 * 0.0004 = 319 */
int max_btus = 640;		/* maximum # of btu's */

double buil_bt = 10.0;		/* tech level required to build a bridge */
int buil_bh = 100;		/* hcm required to build a bridge */
double buil_bc = 2000.0;	/* cash required to build a bridge */

double buil_tower_bt = 100.0;	/* tech level required to build a tower */
int buil_tower_bh = 400;	/* hcm required to build a bridge tower */
double buil_tower_bc = 7500.0;	/* cash required to build a bridge tower */

/* opt_SLOW_WAR */
int War_Cost = 1000;		/* Cost to declare war */

float drnuke_const = 0.0;	/* research must be at least drnuke_const*tech */
				/* in order to build a nuke. For example, if
				 * drnuke_const is .25, you need a 75 res to
				 * build a nuke that takes 300 tech
				 */
/* opt_TRADESHIPS */
int trade_1_dist = 8;		/* less than this gets no money */
int trade_2_dist = 14;		/* less than this gets trade_1 money */
int trade_3_dist = 25;		/* less than this gets trade_2 money */
float trade_1 = 0.025;		/* return on trade_1 distance */
float trade_2 = 0.035;		/* return on trade_2 distance */
float trade_3 = 0.050;		/* return on trade_3 distance */
float trade_ally_bonus = 0.20;	/* 20% bonus for trading with allies */
float trade_ally_cut = 0.10;	/* 10% bonus for ally you trade with */
/* end opt_TRADESHIPS */

int torpedo_damage = 40;	/* damage is X + 1dX + 1dX, so 40+1d40+1d40 */

long start_cash = 25000;

/* initial levels */
float start_education = 0.0;
float start_happiness = 0.0;
float start_technology = 0.0;
float start_research = 0.0;
