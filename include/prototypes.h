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
 *  prototypes.h: Prototypes for server functions
 * 
 *  Known contributors to this file:
 *     Curtis Larsen, 1998
 *     Steve McClure, 1998,2000
 */

#ifndef _PROTOTYPES_H_
#define _PROTOTYPES_H_

#include "misc.h"
#include "empthread.h"
#include "player.h"
#include "commands.h"
#include "common.h"
#include "gen.h"
#include "global.h"
#include "subs.h"
#include "update.h"
#include "empio.h"
#include "commodity.h"

/* src/server/main.c */
extern void panic(int sig);
extern void shutdwn(int sig);

/*****************************************************************************
 * src/lib/ * / *.c 
 *****************************************************************************/

/* src/lib/empthread/lwp.c */
/* in empthread.h */

/*
 * src/lib/commands/ *.c 
 */
extern int check_market(void);
extern void set_coastal(struct sctstr *);
extern int sendmessage(struct natstr *, struct natstr *, char *, int);
extern void gift(int, int, s_char *, int, s_char *);
extern int display_mark(int, int);
extern int want_to_abandon(struct sctstr *, int, int, struct lndstr *);
extern int would_abandon(struct sctstr *, int, int, struct lndstr *);
extern int nav_map(int, int, int);
extern int count_pop(register int);
extern int scuttle_tradeship(struct shpstr *, int);
extern void scuttle_ship(struct shpstr *);
extern void resnoise(struct sctstr *sptr, int public_amt, s_char *name,
		     int old, int new);
extern int line_of_sight(char **rad, int ax, int ay, int bx, int by);
extern void plane_sona(struct emp_qelem *, int, int, struct shiplook *);
extern int tend_nxtitem(struct nstr_item *, caddr_t);
extern s_char *prsub(struct shpstr *);
extern int check_trade(void);
extern int ontradingblock(int, int *);
extern void trdswitchown(int, int *, int);
/* more in commands.h */

/*
 * src/lib/common/ *.c 
 */
/* bestpath.c */
extern s_char *bestownedpath(s_char *, s_char *, int, int, int,
			     int, s_char *, int);
/* bridgefall.c */
extern void bridgefall(struct sctstr *, struct emp_qelem *);
extern void knockdown(struct sctstr *, struct emp_qelem *);
/* damage.c */
extern void landdamage(struct lndstr *, int);
extern void ship_damage(struct shpstr *, int);
extern int damage(register int, int);
extern void shipdamage(struct shpstr *, int);
extern void land_damage(struct lndstr *, int);
extern void planedamage(struct plnstr *, int);
extern int nukedamage(struct nchrstr *, int, int);
extern int effdamage(register int, int);
extern void item_damage(int, short *);
extern int commdamage(register int, int, int);
/* check.c */
extern int check_sect_ok(struct sctstr *);
extern int check_ship_ok(struct shpstr *);
extern int check_land_ok(struct lndstr *);
extern int check_nuke_ok(struct nukstr *);
extern int check_plane_ok(struct plnstr *);
extern int check_comm_ok(struct comstr *);
extern int check_loan_ok(struct lonstr *);
/* fsize.c */
extern int fsize(int);
extern int blksize(int);
extern time_t fdate(int);
extern void filetruncate(s_char *);
/* hap_fact.c */
extern double hap_fact(struct natstr *, struct natstr *);
/* hours.c */
extern int is_wday_allowed(int, char *);
extern int is_daytime_allowed(int, char *);
extern int gamehours(time_t);
extern int is_daytime_near(int, char *, int);
extern int min_to_next_daytime(int, char *);
/* land.c */
extern int has_units(coord, coord, natid, struct lndstr *);
extern int has_units_with_mob(coord, coord, natid);
extern int adj_units(coord, coord, natid);
extern int has_helpful_engineer(coord x, coord y, natid cn);
/* log.c */
extern void loginit(s_char *);
extern void logerror(s_char *, ...) ATTRIBUTE((format (printf, 1, 2)));
extern int oops(char *, char *, int);
/* maps.c */
extern int draw_map(int, s_char, int, struct nstr_sect *, int);
extern int unit_map(int, int, struct nstr_sect *, s_char *);
extern int bmaps_intersect(natid, natid);
extern int share_bmap(natid, natid, struct nstr_sect *, s_char, s_char *);
/* move.c */
/* in path.h */
/* nstr_subs.c */
/* in nsc.h */
/* path.c */
extern void bp_enable_cachepath(void);
extern void bp_disable_cachepath(void);
extern void bp_clear_cachepath(void);
extern s_char *BestDistPath(s_char *, struct sctstr *, struct sctstr *,
			    double *, int);
extern s_char *BestLandPath(s_char *, struct sctstr *, struct sctstr *,
			    double *, int);
extern s_char *BestShipPath(s_char *, int, int, int, int, int);
extern s_char *BestAirPath(s_char *, int, int, int, int);
extern double pathcost(struct sctstr *, s_char *, int);
/* res_pop.c */
extern int max_pop(float, struct sctstr *);
/* sectdamage.c */
extern int sect_damage(struct sctstr *, int, struct emp_qelem *);
extern int sectdamage(struct sctstr *, int, struct emp_qelem *);
/* snxtit_subs.c */
extern void snxtitem_area(register struct nstr_item *, int,
			  struct range *);
extern void snxtitem_dist(register struct nstr_item *, int, int, int, int);
extern void snxtitem_xy(register struct nstr_item *, int, coord, coord);
extern void snxtitem_all(register struct nstr_item *, int);
extern void snxtitem_group(register struct nstr_item *, int, s_char);
extern void snxtitem_rewind(struct nstr_item *);
extern int snxtitem_list(register struct nstr_item *, int, int *, int);
/* snxtsect_subs.c */
extern void snxtsct_area(register struct nstr_sect *, struct range *);
extern void xydist_range(coord, coord, register int, struct range *);
extern void xysize_range(register struct range *);
extern void snxtsct_all(struct nstr_sect *);
extern void snxtsct_rewind(struct nstr_sect *);
extern void snxtsct_dist(register struct nstr_sect *, coord, coord, int);
/* stmtch.c */
/* in match.h */
/* type.c */
extern int sct_typematch(char *);
extern int typematch(char *, int);
/* vlist.c */
extern int vl_find(register int, u_char *, u_short *, int);
extern int vl_set(register int, u_int, u_char *, u_short *, u_char *, int);
extern int vl_damage(register int, register u_char *, register u_short *,
		     register int);
extern int vl_getvec(register u_char *, register u_short *, register int,
		     register int, register int *);
extern int vl_setvec(register u_char *, register u_short *, u_char *,
		     int, register int, register int *);
/* wantupd.c */
extern void update_policy_check(void);
extern int demand_update_want(int *, int *, int);
extern int demandupdatecheck(void);
extern int updatetime(time_t *);
extern void next_update_time(time_t *, time_t *, time_t *);
extern void next_update_check_time(time_t *, time_t *, time_t *);
extern int updates_disabled(void);

/*
 * src/lib/gen/ *.c 
 */
/* emp_config.c */
extern int emp_config(char *file);
extern void print_config(FILE * fp);

extern int roll(int);
extern int roundavg(double);
extern int chance(double);
extern void disassoc(void);
extern int diffx(int, int);
extern int diffy(int, int);
extern int deltax(int, int);
extern int deltay(int, int);
extern int mapdist(int, int, int, int);
#if !defined(_WIN32)
extern int max(int, int);
extern int min(int, int);
#endif
extern s_char *effadv(int);
extern int onearg(s_char *, s_char *);
extern int parse(register s_char *, s_char **, s_char **, s_char *,
		 s_char **);
extern int ldround(double, int);
extern int roundintby(int, int);
extern int scthash(register int, register int, int);
#ifdef NOSTRDUP
extern char *strdup(char *x);
#endif
/* more in misc.h */

/*
 * src/lib/global/ *.c 
 */
extern void global_init(void);


/*
 * src/lib/player/ *.c 
 */
/* accept.c */
extern void player_init(void);
extern struct player *player_new(int, struct sockaddr_in *);
extern struct player *player_delete(struct player *);
extern struct player *player_next(struct player *);
extern struct player *player_prev(struct player *);
extern struct player *getplayer(natid);
extern struct player *player_find_other(struct player *, register natid);
extern void player_wakeup_all(natid);
extern void player_wakeup(struct player *);
extern void player_accept(void *);
/* dispatch.c */
extern int dispatch(s_char *, s_char *);
/* empdis.c */
extern int getcommand(s_char *);
extern void init_player_commands(void);
extern void log_last_commands(void);
extern int gamedown(void);
extern void daychange(time_t);
extern int getminleft(time_t, int);
/* more in commands.h */
/* empmod.c */
/* init_nats.c */
extern int init_nats(void);
/* login.c */
extern void player_login(void *);
/* nat.c */
extern int natbyname(s_char *, natid *);
extern int natpass(int, s_char *);
/* player.c */
extern struct player *player;	/* current player's context */
extern s_char *praddr(struct player *);
extern void player_main(struct player *);
extern int match_user(char *, struct player *);
extern int status(void);
extern int command(void);
/* more in commands.h */
/* recvclient.c */
extern int recvclient(s_char *, int);

/*
 * src/lib/subs/ *.c 
 */
/* askyn.c */
extern int confirm(s_char *);
extern int askyn(s_char *);
/* bsanct.c */
extern void bsanct(void);
/* caploss.c */
extern void caploss(struct sctstr *, natid, s_char *);
/* chkmoney.c */
extern int chkmoney(long, long, s_char *);
extern int check_cost(int, int, long, int *, s_char *);
/* cnumb.c */
extern int cnumb(s_char *);
/* control.c */
extern int military_control(struct sctstr *);
/* detonate.c */
extern int detonate(struct plnstr *, int, int);
/* disloan.c */
extern int disloan(int, register struct lonstr *);
/* distrea.c */
extern int distrea(int, register struct trtstr *);
/* fileinit.c */
extern void ef_init(void);
/* fortdef.c */
extern int sd(natid, natid, coord, coord, int, int, int);
extern int dd(natid, natid, coord, coord, int, int);
extern int sb(natid, natid, struct sctstr *, coord, coord, int, int);
extern int shipdef(natid, natid, coord, coord);
/* getbit.c */
extern int emp_getbit(int, int, u_char *);
extern void emp_setbit(int, int, u_char *);
extern void emp_setbitmap(int, int, u_char *, int *);
extern void bitinit2(struct nstr_sect *, u_char *, int);
/* getele.c */
extern int getele(s_char *, s_char *);
/* land.c */
extern s_char *prland(struct lndstr *);
extern int lnd_postread(int, s_char *);
extern int lnd_prewrite(int, s_char *);
extern void lnd_init(int, s_char *);
/* landgun.c */
extern double seagun(int, int);
extern double landgun(int, int);
extern double landunitgun(int, int, int, int, int);
extern int roundrange(double);
/* list.c */
extern int shipsatxy(coord, coord, int, int);
extern int carriersatxy(coord, coord, int, int, natid);
extern int unitsatxy(coord, coord, int, int);
extern int planesatxy(coord, coord, int, int, struct emp_qelem *);
extern int asw_shipsatxy(coord, coord, int, int, struct plnstr *,
			 struct shiplook *);
extern int num_shipsatxy(coord, coord, int, int);
extern int islist(s_char *);
/* src/lib/subs/mission.c */
extern s_char *mission_name(short);
extern s_char *nameofitem(struct genitem *, int);
extern int collateral_damage(coord, coord, int, struct emp_qelem *);
extern int mission_pln_equip(struct plist *, struct ichrstr *, int,
			     s_char);
extern int ground_interdict(coord, coord, natid, s_char *);
extern int unit_interdict(coord, coord, natid, s_char *, int, int);
extern int off_support(coord, coord, natid, natid);
extern int def_support(coord, coord, natid, natid);
extern int oprange(struct genitem *, int, int *);
extern int cando(int, int);
extern s_char *mission_short_name(int);
extern void show_mission(int, struct nstr_item *);
extern int air_defense(coord, coord, natid, struct emp_qelem *,
		       struct emp_qelem *);
/* move.c */
extern int check_lmines(coord, coord, double);
extern int move_ground(s_char *, struct sctstr *, struct sctstr *,
		       double, s_char *,
		       int (*)(s_char *, coord, coord, s_char *),
		       int, int *);
extern int fly_map(coord, coord);
/* mslsub.c */
extern int msl_intercept(coord, coord, natid, int, int, int, int);
extern int msl_equip(struct plnstr *);
extern int msl_hit(struct plnstr *, int, int, int, int, s_char *,
		   coord, coord, int);
extern void msl_sel(struct emp_qelem *, coord, coord, natid, int,
		    int, int);
extern int msl_launch_mindam(struct emp_qelem *, coord, coord, int,
			     int, int, s_char *, int, int);
/* mtch.c */
extern int intmatch(register int, register int *, int);
extern int comtch(register s_char *, struct cmndstr *, int, int);
/* natarg.c */
extern int natarg(s_char *, s_char *);
/* neigh.c */
extern int neigh(coord, coord, natid);
/* nreport.c */
extern void nreport(natid, int, natid, int);
/* nuke.c */
extern int nuk_postread(int, s_char *);
extern int nuk_prewrite(int, s_char *);
extern void nuk_init(int, s_char *);
extern void nuk_add(coord, coord, int, int);
extern void nuk_delete(struct nukstr *, int, int);
/* nxtitem.c */
extern int nxtitem(struct nstr_item *, caddr_t);
/* nxtsct.c */
extern int nxtsct(register struct nstr_sect *, struct sctstr *);
/* plane.c */
extern s_char *prplane(struct plnstr *);
extern int pln_postread(int, s_char *);
extern int pln_prewrite(int, s_char *);
extern void pln_init(int, s_char *);
/* plnsub.c */
extern void count_planes(struct shpstr *);
extern int pln_onewaymission(struct sctstr *, int *, int *);
extern int pln_oneway_to_carrier_ok(struct emp_qelem *,
				    struct emp_qelem *, int);
extern void pln_newlanding(struct emp_qelem *, coord, coord, int);
extern int can_be_on_ship(int, int);
extern int put_plane_on_ship(struct plnstr *, struct shpstr *);
extern void pln_dropoff(struct emp_qelem *, struct ichrstr *,
			coord, coord, void *, int);
extern void pln_sel(struct nstr_item *, struct emp_qelem *,
		    struct sctstr *, int, int, int, int);
extern int pln_arm(struct emp_qelem *, int, int, struct ichrstr *,
		   int, int, int *);
extern int pln_mobcost(int, struct plnstr *, int);
extern void pln_put(struct emp_qelem *);
extern void pln_removedupes(struct emp_qelem *, struct emp_qelem *);
extern int take_plane_off_ship(struct plnstr *, struct shpstr *);
extern int take_plane_off_land(struct plnstr *, struct lndstr *);
extern void plane_sweep(struct emp_qelem *, coord, coord);
extern void count_land_planes(struct lndstr *);
extern int count_sect_planes(struct sctstr *);
extern int put_plane_on_land(struct plnstr *, struct lndstr *);
extern int pln_hitchance(struct plnstr *, int, int);
extern int pln_damage(struct plnstr *, coord, coord, s_char, int *, int);
extern int pln_identchance(struct plnstr *, int, int);
extern void pln_set_tech(struct plnstr *, int);
/* pr.c */
extern void pr(s_char *, ...) ATTRIBUTE((format (printf, 1, 2)));
extern void prnf(s_char *buf);
extern void pr_id(struct player *, int, s_char *, ...)
		ATTRIBUTE((format (printf, 3, 4)));
extern void pr_flash(struct player *, s_char *, ...)
		ATTRIBUTE((format (printf, 2, 3)));
extern void pr_inform(struct player *, s_char *, ...)
		ATTRIBUTE((format (printf, 2, 3)));
extern void pr_wall(s_char *, ...)
		ATTRIBUTE((format (printf, 1, 2)));
extern void pr_player(struct player *pl, int id, s_char *buf);
extern void pr_hilite(s_char *buf);
extern void prredir(s_char *redir);
extern void prexec(s_char *file);
extern void prprompt(int min, int btu);
extern void showvers(int vers);
extern int prmptrd(s_char *prompt, s_char *str, int size);
extern void prdate(void);
extern void prxy(s_char *format, coord x, coord y, natid country);
extern void PR(int, s_char *, ...) ATTRIBUTE((format (printf, 2, 3)));
extern void PRdate(natid cn);
extern void pr_beep(void);
extern void mpr(int, s_char *, ...) ATTRIBUTE((format (printf, 2, 3)));

/* radmap.c */
extern int deltx(struct range *, coord);
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int delty(struct range *, coord);
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern void radmap(int, int, int, int, double);
extern void radmapnopr(int, int, int, int, double);
extern void radmapupd(int, int, int, int, int, double);
/* rej.c */
extern int setrel(natid, natid, int);
extern int setcont(natid, natid, int);
extern int setrej(natid, natid, int, int);
/* retreat.c */
extern void retreat_ship(struct shpstr *, s_char);
extern void retreat_land(struct lndstr *, s_char);
extern int check_retreat_and_do_shipdamage(struct shpstr *, int);
extern int check_retreat_and_do_landdamage(struct lndstr *, int);
/* sarg.c */
extern ns_seltype sarg_type(char *);
extern int sarg_xy(char *, coord *, coord *);
extern int sarg_area(char *, register struct range *);
extern int sarg_range(char *, coord *, coord *, int *);
extern int sarg_list(char *, register int *, int);
/* satmap.c */
extern void satdisp(struct sctstr *, int, int);
extern void satmap(int, int, int, int, int, int);
extern void sathead(void);
/* sect.c */
extern int sct_postread(int, s_char *);
extern int sct_prewrite(int, s_char *);
extern int issector(s_char *);
extern void sct_init(coord, coord, s_char *);
/* ship.c */
extern s_char *prship(struct shpstr *);
extern int shp_postread(int, s_char *);
extern int shp_prewrite(int, s_char *);
extern void shp_init(int, s_char *);
/* show.c */
extern void show_bridge(int);
extern void show_tower(int);
extern void show_nuke_stats(int);
extern void show_nuke_capab(int);
extern void show_nuke_build(int);
extern void show_ship_build(int);
extern void show_ship_stats(int);
extern void show_ship_capab(int);
extern void show_plane_stats(int);
extern void show_plane_capab(int);
extern void show_plane_build(int);
extern void show_land_build(int);
extern void show_land_capab(int);
extern void show_land_stats(int);
extern void show_sect_build(int);
extern void show_sect_stats(int);
extern void show_sect_capab(int);
/* shpsub.c */
extern void shp_sel(struct nstr_item *, struct emp_qelem *);
extern void shp_nav(struct emp_qelem *, double *, double *, int *, natid);
extern int shp_sweep(struct emp_qelem *, int, natid);
extern s_char *shp_path(int, struct shpstr *, s_char *);
extern void shp_put(struct emp_qelem *, natid);
extern void shp_list(struct emp_qelem *);
extern int shp_hardtarget(struct shpstr *);
extern void shp_view(struct emp_qelem *);
extern int shp_nav_one_sector(struct emp_qelem *, int, natid, int);
extern int shp_missile_defense(coord, coord, natid, int);
extern void shp_missdef(struct shpstr *, natid);
extern void shp_set_tech(struct shpstr *, int);
/* snxtitem.c */
extern int snxtitem(register struct nstr_item *, int, s_char *);
/* snxtsct.c */
extern int snxtsct(register struct nstr_sect *, s_char *);
/* supply.c */
extern void resupply_all(struct lndstr *);
extern void resupply_commod(struct lndstr *, int);
extern int supply_commod(int, int, int, int, int);
extern int try_supply_commod(int, int, int, int, int);
extern int has_supply(struct lndstr *);
extern int use_supply(struct lndstr *);
/* takeover.c */
extern void takeover_land(register struct lndstr *, natid, int);
extern void takeover(register struct sctstr *, natid);
extern void takeover_ship(register struct shpstr *, natid, int);
extern void takeover_plane(register struct plnstr *, natid);
/* trdsub.c */
/* trechk.c */
extern int trechk(register natid, register natid, int);
/* whatitem.c */
/* in item.h */
/* wu.c */
extern void clear_telegram_is_new(natid);
extern int typed_wu(natid, natid, s_char *, int);
extern int wu(natid, natid, s_char *, ...) ATTRIBUTE((format (printf, 3, 4)));

/*
 * src/lib/update/ *.c 
 */
/* age.c */
extern void age_levels(int);
/* anno.c */
extern void delete_old_announcements(void);
/* bp.c */
/* in budg.h */
/* deliver.c */
extern int deliver(register struct sctstr *, struct ichrstr *, int, int,
		   int, int);
/* distribute.c */
extern int dodistribute(struct sctstr *, int, s_char *, double, double);
/* finish.c */
extern void finish_sects(int);
/* human.c */
extern int new_work(struct sctstr *, int);
extern int do_feed(register struct sctstr *, register struct natstr *,
		   short *, int *, int *, int);
extern int feed_people(short *, int, int *);
/* land.c */
extern int prod_land(int, int, int *, int);
extern int feed_land(struct lndstr *, int, int *, int);
/* main.c */
/* in server.h */
/* material.c */
/* in budg.h */
/* mobility.c */
extern void mob_sect(register int);
extern void mob_ship(register int);
extern void mob_land(register int);
extern void mob_plane(register int);
extern void update_all_mob(void);
extern void update_timestamps(time_t offset);
extern void sct_do_upd_mob(register struct sctstr *sp);
extern void shp_do_upd_mob(register struct shpstr *sp);
extern void lnd_do_upd_mob(register struct lndstr *lp);
extern void pln_do_upd_mob(register struct plnstr *pp);
/* move_sat.c */
extern void move_sat(register struct plnstr *);
/* nat.c */
extern double logx(double, double);
extern void prod_nat(int);
/* nav_ship.c */
extern int nav_ship(register struct shpstr *);
/* nav_util.c */
extern int check_nav(struct sctstr *);
extern int load_it(register struct shpstr *, register struct sctstr *,
		   int);
extern void unload_it(register struct shpstr *);
extern void auto_fuel_ship(register struct shpstr *);
/* nxtitemp.c */
extern s_char *nxtitemp(struct nstr_item *, int);
/* nxtsctp.c */
extern struct sctstr *nxtsctp(register struct nstr_sect *);
/* plague.c */
extern void do_plague(struct sctstr *, struct natstr *, int);
extern int plague_people(struct natstr *, short *, u_short *, u_short *, int);
/* plane.c */
extern int prod_plane(int, int, int *, int);
/* populace.c */
extern void populace(struct natstr *, register struct sctstr *, int);
extern int total_work(register int, register int, register int,
		      register int, register int);
/* prepare.c */
extern void tax(struct sctstr *, struct natstr *, int, long *, int *,
		int *, int *);
extern int upd_slmilcosts(natid, int);
extern void prepare_sects(int, int *);
extern int bank_income(struct sctstr *, int);
/* produce.c */
extern int produce(struct natstr *, struct sctstr *, short *, int, int,
		   int, int *, int *);
/* removewants.c */
extern int update_removewants(void);
/* revolt.c */
extern void revolt(struct sctstr *);
extern void guerrilla(struct sctstr *);
/* sail.c */
extern void sail_ship(natid);
/* sect.c */
extern void dodeliver(struct sctstr *);
extern void do_fallout(register struct sctstr *, register int);
extern void spread_fallout(struct sctstr *, int);
extern void decay_fallout(struct sctstr *, int);
extern void produce_sect(int natnum, int etu, int *bp,
			 long int (*p_sect)[2], int sector_type);
/* ship.c */
extern int prod_ship(int, int, int *, int);
extern int feed_ship(struct shpstr *, int, int *, int);

/*
 * src/server
 */
/* shutdown.c */
/* in server.h */

#endif /* _PROTOTYPES_H_ */
