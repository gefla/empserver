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
 *  optlist.h: Definitions of option listings
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#ifndef _OPTLIST_H_
#define _OPTLIST_H_

struct option_list {
    char *opt_key;
    int *opt_valuep;
};

extern struct option_list Options[];

/* Options, can be switched in econfig */
extern int opt_ALL_BLEED;
extern int opt_BIG_CITY;
extern int opt_BLITZ;
extern int opt_BRIDGETOWERS;
extern int opt_DEFENSE_INFRA;
extern int opt_DEMANDUPDATE;
extern int opt_DRNUKE;
extern int opt_EASY_BRIDGES;
extern int opt_FALLOUT;
extern int opt_FUEL;
extern int opt_GODNEWS;
extern int opt_GO_RENEW;
extern int opt_HIDDEN;
extern int opt_INTERDICT_ATT;
extern int opt_LANDSPIES;
extern int opt_LOANS;
extern int opt_LOSE_CONTACT;
extern int opt_MARKET;
extern int opt_MOB_ACCESS;
extern int opt_NEUTRON;
extern int opt_NOFOOD;
extern int opt_NOMOBCOST;
extern int opt_NONUKES;
extern int opt_NO_FORT_FIRE;
extern int opt_NO_HCMS;
extern int opt_NO_LCMS;
extern int opt_NO_OIL;
extern int opt_NO_PLAGUE;
extern int opt_NUKEFAILDETONATE;
extern int opt_ORBIT;
extern int opt_PINPOINTMISSILE;
extern int opt_PLANENAMES;
extern int opt_RES_POP;
extern int opt_ROLLOVER_AVAIL;
extern int opt_SAIL;
extern int opt_SHIPNAMES;
extern int opt_SHIP_DECAY;
extern int opt_SHOWPLANE;
extern int opt_SLOW_WAR;
extern int opt_SNEAK_ATTACK;
extern int opt_SUPER_BARS;
extern int opt_TECH_POP;
extern int opt_TRADESHIPS;
extern int opt_TREATIES;
extern int opt_UPDATESCHED;

/* Game parameters, can be set in econfig */
extern s_char *datadir;
extern s_char *infodir;
extern s_char *loginport;
extern s_char *privname;
extern s_char *privlog;
extern int WORLD_X;
extern int WORLD_Y;
extern int update_policy;
extern int etu_per_update;
extern int s_p_etu;
extern int adj_update;
extern int update_window;
extern s_char *update_times;
extern int hourslop;
extern int blitz_time;
extern int update_demandpolicy;
extern int update_wantmin;
extern int update_missed;
extern s_char *update_demandtimes;
extern s_char *game_days;
extern s_char *game_hours;
extern float btu_build_rate;
extern int m_m_p_d;
extern int max_btus;
extern int max_idle;
extern int players_at_00;
extern int at_least_one_100;
extern double powe_cost;
extern int War_Cost;
extern float easy_tech;
extern float hard_tech;
extern float start_technology;
extern float start_happiness;
extern float start_research;
extern float start_education;
extern float level_age_rate;
extern float tech_log_base;
extern float ally_factor;
extern float edu_avg;
extern float hap_avg;
extern double edu_cons;
extern double hap_cons;
extern int startmob;
extern float sect_mob_scale;
extern int sect_mob_max;
extern int buil_bh;
extern double buil_bc;
extern double buil_bt;
extern int buil_tower_bh;
extern double buil_tower_bc;
extern double buil_tower_bt;
extern float land_mob_scale;
extern int land_grow_scale;
extern int land_mob_max;
extern double money_land;
extern int morale_base;
extern float plane_mob_scale;
extern int plane_grow_scale;
extern int plane_mob_max;
extern double money_plane;
extern float ship_mob_scale;
extern int ship_grow_scale;
extern int ship_mob_max;
extern double money_ship;
extern int torpedo_damage;
extern int fort_max_interdiction_range;
extern int land_max_interdiction_range;
extern int ship_max_interdiction_range;
extern double flakscale;
extern double combat_mob;
extern double people_damage;
extern double unit_damage;
extern double collateral_dam;
extern double assault_penalty;
extern float fire_range_factor;
extern int sect_mob_neg_factor;
extern double mission_mob_cost;
extern double uwbrate;
extern double money_civ;
extern double money_mil;
extern double money_res;
extern double money_uw;
extern double babyeat;
extern double bankint;
extern double eatrate;
extern double fcrate;
extern double fgrate;
extern double obrate;
extern double decay_per_etu;
extern double fallout_spread;
extern float drnuke_const;
extern int MARK_DELAY;
extern int TRADE_DELAY;
extern double buytax;
extern double tradetax;
extern int trade_1_dist;
extern int trade_2_dist;
extern int trade_3_dist;
extern float trade_1;
extern float trade_2;
extern float trade_3;
extern float trade_ally_bonus;
extern float trade_ally_cut;
extern int fuel_mult;
extern int lost_items_timeout;
extern long last_demand_update;

/* Game parameters that cannot be set in econfig (yet) */
extern int melt_item_denom[];
#ifdef START_UNITS
int start_unit_type[];
#endif

/* Variables that get values derived from econfig */
extern s_char *upfil;
extern s_char *downfil;
extern s_char *disablefil;
extern s_char *banfil;
extern s_char *authfil;
extern s_char *annfil;
extern s_char *timestampfil;
extern s_char *teldir;
extern s_char *telfil;

struct keymatch {
    s_char *km_key;		/* the key */
    void (*km_func)(struct keymatch * kp, s_char **av);
				/* the function to call if matches */
    caddr_t km_data;		/* associated data */
    int km_flags;		/* useful flags */
#define KM_ALLOC	0x01	/* memory allocated */
    s_char *km_comment;		/* Comment (hopefully useful) */
};

extern struct keymatch configkeys[];

#endif /* _OPTLIST_H_ */
