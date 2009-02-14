/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  prototypes.h: Prototypes for server functions
 *
 *  Known contributors to this file:
 *     Curtis Larsen, 1998
 *     Steve McClure, 1998,2000
 */

#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "item.h"
#include "nsc.h"
#include "misc.h"
#include "types.h"

/* src/server/main.c */
extern void shutdwn(int sig);
extern void init_server(unsigned);
extern void start_server(int);
extern void finish_server(void);

/* src/lib/empthread/lwp.c */
/* in empthread.h */

/*
 * src/lib/commands/ *.c
 */
extern int check_market(void);
extern void gift(natid, natid, void *, char *);
extern int display_mark(i_type, int);
extern int want_to_abandon(struct sctstr *, i_type, int, struct lndstr *);
extern int would_abandon(struct sctstr *, i_type, int, struct lndstr *);
extern int nav_map(int, int, int);
extern int do_unit_move(struct emp_qelem *land_list, int *together,
	     double *minmob, double *maxmob);
extern int count_pop(int);
extern int scuttle_tradeship(struct shpstr *, int);
extern void resnoise(struct sctstr *sptr, int public_amt, char *name,
		     int old, int new);
extern int line_of_sight(char **rad, int ax, int ay, int bx, int by);
extern void plane_sona(struct emp_qelem *, int, int, struct shiplist **);
extern char *prsub(struct shpstr *);
extern int check_trade(void);
extern int ontradingblock(int, void *);
extern void trdswitchown(int, void *, int);
extern int do_look(short);
extern int radar(short);
extern void update_power(void);
extern int show_first_tel(char *);
/* Commands */
int acce(void);
int add(void);
int anti(void);
int apro(void);
int arm(void);
int army(void);
int assa(void);
int atta(void);
int bdes(void);
int best(void);
int boar(void);
int bomb(void);
int brea(void);
int budg(void);
int buil(void);
int buy(void);
int capi(void);
int carg(void);
int cede(void);
int cens(void);
int chan(void);
int coas(void);
int coll(void);
int comm(void);
int cons(void);
int conv(void);
int coun(void);
int cuto(void);
int decl(void);
int deli(void);
int demo(void);
int desi(void);
int disa(void);
int disarm(void);
int dist(void);
int drop(void);
int dump(void);
int echo(void);
int edit(void);
int enab(void);
int enli(void);
int execute(void);
int explain(void);
int explore(void);
int fina(void);
int flash(void);
int flee(void);
int fly(void);
int foll(void);
int force(void);
int fort(void);
int give(void);
int grin(void);
int hard(void);
int head(void);
int improve(void);
int info(void);
int land(void);
int landmine(void);
int laun(void);
int lboa(void);
int lcarg(void);
int ldump(void);
int ledg(void);
int leve(void);
int lload(void);
int llook(void);
int load(void);
int look(void);
int lost(void);
int lrad(void);
int lretr(void);
int lsta(void);
int ltend(void);
int map(void);
int march(void);
int mark(void);
int mine(void);
int mission(void);
int mobq(void);
int morale(void);
int move(void);
int multifire(void);
int name(void);
int nati(void);
int navi(void);
int ndump(void);
int new(void);
int newe(void);
int news(void);
int nuke(void);
int offe(void);
int orde(void);
int orig(void);
int para(void);
int path(void);
int payo(void);
int pboa(void);
int pdump(void);
int peek(void);
int plan(void);
int play(void);
int powe(void);
int prod(void);
int pstat(void);
int qorde(void);
int quit(void);
int rada(void);
int range(void);
int rea(void);
int real(void);
int reco(void);
int reje(void);
int rela(void);
int relo(void);
int repa(void);
int repo(void);
int rese(void);
int reso(void);
int retr(void);
int rout(void);
int sabo(void);
int sail(void);
int sate(void);
int scra(void);
int sct(void);
int scut(void);
int sdump(void);
int sell(void);
int set(void);
int setres(void);
int setsector(void);
int shar(void);
int shark(void);
int shi(void);
int shoo(void);
int show(void);
int show_motd(void);
int shut(void);
int sinfra(void);
int skyw(void);
int sona(void);
int sorde(void);
int spy(void);
int sstat(void);
int start(void);
int starve(void);
int stop(void);
int stre(void);
int supp(void);
int surv(void);
int swaps(void);
int tele(void);
int tend(void);
int terr(void);
int thre(void);
int togg(void);
int torp(void);
int trad(void);
int tran(void);
int trea(void);
int turn(void);
int upda(void);
int upgr(void);
int vers(void);
int wall(void);
int wing(void);
int wipe(void);
int work(void);
int xdump(void);
int xedit(void);
int zdon(void);

/*
 * src/lib/common/ *.c
 */
/* bestpath.c */
extern char *bestownedpath(char *, char *, int, int, int, int, int);
/* conftab.c */
extern int read_builtin_tables(void);
extern int read_custom_tables(void);
/* cnumb.c */
extern int cnumb(char *);
/* ef_verify.c */
/* in file.h */
/* emp_config.c */
extern int emp_config(char *file);
extern void print_config(FILE * fp);
/* hap_fact.c */
extern double hap_fact(struct natstr *, struct natstr *);
extern double hap_req(struct natstr *np);
/* hours.c */
extern int is_wday_allowed(int, char *);
extern int is_daytime_allowed(int, char *);
extern int gamehours(time_t);
/* log.c */
extern int loginit(char *);
extern int logreopen(void);
extern void logerror(char *, ...) ATTRIBUTE((format (printf, 1, 2)));
/* more in misc.h */
/* mapdist.c */
extern int deltax(int, int);
extern int deltay(int, int);
extern int mapdist(int, int, int, int);
/* move.c */
/* in path.h */
/* nstreval.c */
/* in nsc.h */
/* path.c */
extern void bp_enable_cachepath(void);
extern void bp_disable_cachepath(void);
extern void bp_clear_cachepath(void);
extern char *BestDistPath(char *, struct sctstr *, struct sctstr *,
			  double *);
extern char *BestLandPath(char *, struct sctstr *, struct sctstr *,
			  double *, int);
extern char *BestShipPath(char *, int, int, int, int, int);
extern char *BestAirPath(char *, int, int, int, int);
extern double pathcost(struct sctstr *, char *, int);
/* rdsched.c */
extern int read_schedule(char *, time_t[], int, time_t, time_t);
/* res_pop.c */
extern int max_population(float, int, int);
extern int max_pop(float, struct sctstr *);
/* stmtch.c */
/* in match.h */
/* type.c */
extern int sct_typematch(char *);
/* wantupd.c */
extern int demand_update_want(int *, int *, int);
extern int demand_check(void);
extern int demandupdatecheck(void);
/* xdump.c xundump.c */
/* in xdump.h */

/*
 * src/lib/gen/ *.c
 */
/* fnameat.c */
extern char *fnameat(const char *, const char *);
extern FILE *fopenat(const char *, const char *, const char *);
/* fsize.c */
extern int fsize(int);
extern int blksize(int);
extern time_t fdate(int);

extern int roll(int);
extern int roundavg(double);
extern int chance(double);
extern int disassoc(void);
extern char *effadv(int);
extern int parse(char *, char *, char **, char **, char **, char **);
extern int ldround(double, int);
extern int roundintby(int, int);
extern int scthash(int, int, int);
extern int tcp_listen(char *, char *, size_t *);
extern char *numstr(char buf[], int n);
/* plur.c */
extern char *esplur(int n);
extern char *splur(int n);
extern char *iesplur(int n);
extern char *plurize(char *buf, int max_len, int n);
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
extern struct player *player_new(int);
extern struct player *player_delete(struct player *);
extern struct player *player_next(struct player *);
extern struct player *player_prev(struct player *);
extern struct player *getplayer(natid);
extern void player_accept(void *);
/* dispatch.c */
extern int dispatch(char *, char *);
/* empdis.c */
extern int getcommand(char *);
extern void init_player_commands(void);
extern void log_last_commands(void);
extern void update_timeused_login(time_t now);
extern void update_timeused(time_t now);
extern void enforce_minimum_session_time(void);
extern int may_play_now(struct natstr *, time_t);
/* more under Commands */
/* empmod.c */
/* init_nats.c */
extern int init_nats(void);
extern int player_set_nstat(struct player *, struct natstr *);
/* login.c */
extern void player_login(void *);
/* nat.c */
extern int natbyname(char *, natid *);
extern int natpass(int, char *);
/* player.c */
extern struct player *player;	/* current player's context */
extern char *praddr(struct player *);
extern void player_main(struct player *);
/* more under Commands */
/* recvclient.c */
extern int recvclient(char *, int);

/* service.c */
extern void stop_service(void);
/* more in service.h */

/*
 * src/lib/subs/ *.c
 */
/* askyn.c */
extern int confirm(char *);
extern int askyn(char *);
/* bridgefall.c */
extern void bridge_damaged(struct sctstr *);
extern void bridgefall(struct sctstr *);
/* bsanct.c */
extern void bsanct(void);
/* caploss.c */
extern void caploss(struct sctstr *, natid, char *);
/* check.c */
extern int check_sect_ok(struct sctstr *);
extern int check_ship_ok(struct shpstr *);
extern int check_land_ok(struct lndstr *);
extern int check_nuke_ok(struct nukstr *);
extern int check_plane_ok(struct plnstr *);
extern int check_comm_ok(struct comstr *);
extern int check_loan_ok(struct lonstr *);
extern int check_trade_ok(struct trdstr *);
/* coastal.c */
extern void set_coastal(struct sctstr *, int, int);
/* control.c */
extern int military_control(struct sctstr *);
/* damage.c */
extern void landdamage(struct lndstr *, int);
extern void ship_damage(struct shpstr *, int);
extern int damage(int, int);
extern void shipdamage(struct shpstr *, int);
extern void land_damage(struct lndstr *, int);
extern void planedamage(struct plnstr *, int);
extern int nukedamage(struct nchrstr *, int, int);
extern int effdamage(int, int);
extern void item_damage(int, short *);
extern int commdamage(int, int, i_type);
/* detonate.c */
extern int detonate(struct nukstr *, coord, coord, int);
/* disloan.c */
extern int disloan(int, struct lonstr *);
/* distrea.c */
extern int distrea(int, struct trtstr *);
/* fileinit.c */
extern void ef_init_srv(void);
extern void ef_fin_srv(void);
/* fortdef.c */
extern int sd(natid, natid, coord, coord, int, int, int);
extern int dd(natid, natid, coord, coord, int, int);
extern int shipdef(natid, natid, coord, coord);
/* getbit.c */
extern int emp_getbit(int, int, unsigned char *);
extern void emp_setbit(int, int, unsigned char *);
extern void bitinit2(struct nstr_sect *, unsigned char *, int);
/* getele.c */
extern int getele(char *, char *);
/* land.c */
extern char *prland(struct lndstr *);
extern void lnd_oninit(void *);
extern void lnd_postread(int, void *);
extern void lnd_prewrite(int, void *, void *);
/* landgun.c */
extern double seagun(int, int);
extern double fortgun(int, int);
extern double landunitgun(int, int);
extern double torprange(struct shpstr *);
extern double fortrange(struct sctstr *);
extern int roundrange(double);
/* list.c */
extern int shipsatxy(coord, coord, int, int, int);
extern int carriersatxy(coord, coord, natid);
extern int unitsatxy(coord, coord, int, int);
extern int planesatxy(coord, coord, int, int);
extern int asw_shipsatxy(coord, coord, int, int, struct plnstr *,
			 struct shiplist **);
extern int has_units(coord, coord, natid, struct lndstr *);
extern int adj_units(coord, coord, natid);
extern int islist(char *);
/* maps.c */
extern int do_map(int bmap, int unit_type, char *arg1, char *arg2);
extern int display_region_map(int bmap, int unit_type, coord curx,
			      coord cury, char *arg);
extern int bmaps_intersect(natid, natid);
extern int share_bmap(natid, natid, struct nstr_sect *, char, char *);
/* mission.c */
extern char *mission_name(short);
extern int collateral_damage(coord, coord, int);
extern int mission_pln_equip(struct plist *, struct ichrstr *, int, char);
extern int ground_interdict(coord, coord, natid, char *);
extern int unit_interdict(coord, coord, natid, char *, int, int);
extern int off_support(coord, coord, natid, natid);
extern int def_support(coord, coord, natid, natid);
extern int oprange(struct empobj *);
extern int in_oparea(struct empobj *, coord, coord);
extern int cando(int, int);
/* move.c */
extern int check_lmines(coord, coord, double);
extern int move_ground(struct sctstr *, struct sctstr *,
		       double, char *,
		       int (*)(coord, coord, char *),
		       int, int *);
extern int fly_map(coord, coord);
/* mslsub.c */
extern int msl_intercept(coord, coord, natid, int, int, int, int);
extern int msl_equip(struct plnstr *, char);
extern int msl_hit(struct plnstr *, int, int, int, int, char *,
		   coord, coord, int);
extern void msl_sel(struct emp_qelem *, coord, coord, natid, int,
		    int, int);
extern int msl_launch_mindam(struct emp_qelem *, coord, coord, int,
			     int, int, char *, int, int);
/* mtch.c */
extern int comtch(char *, struct cmndstr *, int);
/* natarg.c */
extern struct natstr *natargp(char *, char *);
extern int natarg(char *, char *);
/* neigh.c */
extern int neigh(coord, coord, natid);
/* nreport.c */
extern void delete_old_news(void);
extern void init_nreport(void);
extern void nreport(natid, int, natid, int);
/* nuke.c */
extern char *prnuke(struct nukstr *);
extern void nuk_oninit(void *);
extern void nuk_postread(int, void *);
extern void nuk_prewrite(int, void *, void *);
/* nxtitem.c */
extern int nxtitem(struct nstr_item *, void *);
/* nxtsct.c */
extern int nxtsct(struct nstr_sect *, struct sctstr *);
/* onearg.c */
extern int onearg(char *, char *);
/* plane.c */
extern char *prplane(struct plnstr *);
extern void pln_oninit(void *);
extern void pln_postread(int, void *);
extern void pln_prewrite(int, void *, void *);
/* plnsub.c */
extern int get_planes(struct nstr_item *, struct nstr_item *,
		      char *, char *);
extern struct sctstr *get_assembly_point(char *, struct sctstr *, char *);
extern int pln_onewaymission(struct sctstr *, int *, int *);
extern int pln_oneway_to_carrier_ok(struct emp_qelem *,
				    struct emp_qelem *, int);
extern void pln_newlanding(struct emp_qelem *, coord, coord, int);
extern int could_be_on_ship(struct plnstr *, struct shpstr *,
			    int, int, int, int);
extern int put_plane_on_ship(struct plnstr *, struct shpstr *);
extern void pln_dropoff(struct emp_qelem *, struct ichrstr *,
			coord, coord, int);
extern void pln_mine(struct emp_qelem *, coord, coord);
extern int pln_capable(struct plnstr *, int, int);
extern int pln_airbase_ok(struct plnstr *, int, int);
extern int carrier_planes(struct shpstr *, int);
extern void pln_sel(struct nstr_item *, struct emp_qelem *,
		    struct sctstr *, int, int, int, int);
extern void pln_arm(struct emp_qelem *, int, char, struct ichrstr *, int);
extern int pln_mobcost(int, struct plnstr *, int);
extern void pln_put(struct emp_qelem *);
extern void pln_put1(struct plist *);
extern void plane_sweep(struct emp_qelem *, coord, coord);
extern int put_plane_on_land(struct plnstr *, struct lndstr *);
extern int pln_hitchance(struct plnstr *, int, int);
extern int pln_damage(struct plnstr *, coord, coord, char, int *, int);
extern int pln_identchance(struct plnstr *, int, int);
extern int pln_is_in_orbit(struct plnstr *);
extern void pln_set_tech(struct plnstr *, int);
/* pr.c */
extern void pr(char *, ...) ATTRIBUTE((format (printf, 1, 2)));
extern void uprnf(char *buf);
extern void pr_id(struct player *, int, char *, ...)
		ATTRIBUTE((format (printf, 3, 4)));
extern void pr_flash(struct player *, char *, ...)
		ATTRIBUTE((format (printf, 2, 3)));
extern void pr_inform(struct player *, char *, ...)
		ATTRIBUTE((format (printf, 2, 3)));
extern void pr_wall(char *, ...)
		ATTRIBUTE((format (printf, 1, 2)));
extern void prredir(char *redir);
extern void prexec(char *file);
extern void prprompt(int min, int btu);
extern int prmptrd(char *prompt, char *str, int size);
extern int uprmptrd(char *prompt, char *str, int size);
extern void prdate(void);
extern void prxy(char *format, coord x, coord y, natid country);
extern void PR(int, char *, ...) ATTRIBUTE((format (printf, 2, 3)));
extern void PRdate(natid cn);
extern void pr_beep(void);
extern void mpr(int, char *, ...) ATTRIBUTE((format (printf, 2, 3)));
extern size_t copy_ascii_no_funny(char *, char *);
extern size_t copy_utf8_no_funny(char *, char *);
extern size_t copy_utf8_to_ascii_no_funny(char *, char *);
extern int ufindpfx(char *, int);

/* radmap.c */
extern int deltx(struct range *, coord);
extern int delty(struct range *, coord);
extern void radmap(int, int, int, int, double);
extern void radmapnopr(int, int, int, int, double);
extern void radmapupd(int, int, int, int, int, double);
/* rej.c */
extern int setrel(natid, natid, int);
extern int setcont(natid, natid, int);
extern int setrej(natid, natid, int, int);
/* retreat.c */
extern void retreat_ship(struct shpstr *, char);
extern void retreat_land(struct lndstr *, char);
extern int check_retreat_and_do_shipdamage(struct shpstr *, int);
extern int check_retreat_and_do_landdamage(struct lndstr *, int);
/* sarg.c */
extern enum ns_seltype sarg_type(char *);
extern int sarg_xy(char *, coord *, coord *);
extern int sarg_area(char *, struct range *);
extern int sarg_range(char *, coord *, coord *, int *);
extern int sarg_list(char *, int *, int);
/* satmap.c */
extern void satdisp_sect(struct sctstr *, int);
extern void satdisp_units(coord, coord);
extern void satmap(int, int, int, int, int, int);
extern void sathead(void);
/* sect.c */
extern void sct_postread(int, void *);
extern void sct_prewrite(int, void *, void *);
extern void item_prewrite(short *);
extern int issector(char *);
/* sectdamage.c */
extern int sect_damage(struct sctstr *, int);
extern int sectdamage(struct sctstr *, int);
/* ship.c */
extern char *prship(struct shpstr *);
extern void shp_postread(int, void *);
extern void shp_prewrite(int, void *, void *);
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
extern void show_item(int);
extern void show_news(int);
extern void show_product(int);
extern void show_updates(int);
/* shpsub.c */
extern void shp_sel(struct nstr_item *, struct emp_qelem *);
extern void shp_nav(struct emp_qelem *, double *, double *, int *, natid);
extern int shp_sweep(struct emp_qelem *, int, int, natid);
extern int shp_check_nav(struct sctstr *, struct shpstr *);
extern int sect_has_dock(struct sctstr *);
extern int shp_hardtarget(struct shpstr *);
extern int shp_nav_one_sector(struct emp_qelem *, int, natid, int);
extern int shp_missile_defense(coord, coord, natid, int);
extern void shp_missdef(struct shpstr *, natid);
extern double shp_mobcost(struct shpstr *);
extern void shp_set_tech(struct shpstr *, int);
/* snxtitem.c */
extern int snxtitem(struct nstr_item *, int, char *, char *);
extern void snxtitem_area(struct nstr_item *, int,
			  struct range *);
extern void snxtitem_dist(struct nstr_item *, int, int, int, int);
extern void snxtitem_xy(struct nstr_item *, int, coord, coord);
extern void snxtitem_all(struct nstr_item *, int);
extern void snxtitem_group(struct nstr_item *, int, char);
extern void snxtitem_rewind(struct nstr_item *);
extern int snxtitem_list(struct nstr_item *, int, int *, int);
extern void snxtitem_cargo(struct nstr_item *, int, int, int);
/* snxtsct.c */
extern int snxtsct(struct nstr_sect *, char *);
extern void snxtsct_area(struct nstr_sect *, struct range *);
extern void xydist_range(coord, coord, int, struct range *);
extern void xysize_range(struct range *);
extern void snxtsct_all(struct nstr_sect *);
extern void snxtsct_rewind(struct nstr_sect *);
extern void snxtsct_dist(struct nstr_sect *, coord, coord, int);
/* supply.c */
extern void resupply_all(struct lndstr *);
extern void resupply_commod(struct lndstr *, i_type);
extern int supply_commod(int, int, int, i_type, int);
extern int lnd_in_supply(struct lndstr *);
extern int lnd_could_be_supplied(struct lndstr *);
/* takeover.c */
extern void takeover_land(struct lndstr *, natid);
extern void takeover(struct sctstr *, natid);
extern void takeover_ship(struct shpstr *, natid);
extern void takeover_plane(struct plnstr *, natid);
/* trdsub.c */
/* trechk.c */
extern int trechk(natid, natid, int);
/* whatitem.c */
/* in item.h */
/* wu.c */
extern void clear_telegram_is_new(natid);
extern int typed_wu(natid, natid, char *, int);
extern int wu(natid, natid, char *, ...) ATTRIBUTE((format (printf, 3, 4)));

/*
 * src/lib/update/ *.c
 */
/* age.c */
extern int age_people(int, int);
extern void age_levels(int);
/* anno.c */
extern void delete_old_announcements(void);
/* bp.c */
/* in budg.h */
/* deliver.c */
extern void dodeliver(struct sctstr *);
/* distribute.c */
extern int dodistribute(struct sctstr *, int, char *, double, double);
/* finish.c */
extern void finish_sects(int);
/* human.c */
extern int new_work(struct sctstr *, int);
extern int do_feed(struct sctstr *, struct natstr *, short *, int *, int);
extern int feed_people(short *, int);
extern double food_needed(short *, int);
extern int famine_victims(short *, int);
/* land.c */
extern int prod_land(int, int, struct bp *, int);
/* main.c */
/* in server.h */
/* material.c */
/* in budg.h */
/* mobility.c */
extern void mob_sect(void);
extern void mob_ship(void);
extern void mob_land(void);
extern void mob_plane(void);
extern void sct_do_upd_mob(struct sctstr *sp);
extern void shp_do_upd_mob(struct shpstr *sp);
extern void lnd_do_upd_mob(struct lndstr *lp);
extern void pln_do_upd_mob(struct plnstr *pp);
/* move_sat.c */
extern void move_sat(struct plnstr *);
/* nat.c */
extern void prod_nat(int);
/* nav_ship.c */
extern int nav_ship(struct shpstr *);
/* nav_util.c */
extern int load_it(struct shpstr *, struct sctstr *,
		   int);
extern void unload_it(struct shpstr *);
/* nxtitemp.c */
extern void *nxtitemp(struct nstr_item *);
/* plague.c */
extern void do_plague(struct sctstr *, struct natstr *, int);
extern int plague_people(struct natstr *, short *, int *, int *, int);
/* plane.c */
extern int prod_plane(int, int, struct bp *, int);
/* populace.c */
extern void populace(struct natstr *, struct sctstr *, int);
extern int total_work(int, int, int, int, int, int);
/* prepare.c */
extern void tax(struct sctstr *, struct natstr *, int, long *, int *,
		int *, int *);
extern int upd_slmilcosts(natid, int);
extern void prepare_sects(int, struct bp *);
extern int bank_income(struct sctstr *, int);
/* produce.c */
extern int produce(struct natstr *, struct sctstr *, short *, int, int,
		   int, int *, int *);
extern double prod_eff(int, float);
/* removewants.c */
extern int update_removewants(void);
/* revolt.c */
extern void revolt(struct sctstr *);
extern void guerrilla(struct sctstr *);
/* sail.c */
extern void sail_ship(natid);
/* sect.c */
extern void do_fallout(struct sctstr *, int);
extern void spread_fallout(struct sctstr *, int);
extern void decay_fallout(struct sctstr *, int);
extern void produce_sect(int, int, struct bp *, long [][2]);
/* ship.c */
extern int prod_ship(int, int, struct bp *, int);

/*
 * src/server
 */
/* shutdown.c */
/* in server.h */

#endif
