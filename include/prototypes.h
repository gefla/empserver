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
#include "misc.h"
#include "types.h"

/* src/server/main.c */
extern void shutdwn(int sig);
extern void start_server(int);
extern void finish_server(void);

/* src/lib/empthread/lwp.c */
/* in empthread.h */

/*
 * src/lib/commands/ *.c
 */
extern int check_market(void);
extern int edit_sect(struct sctstr *, char *, char *);
extern int edit_sect_i(struct sctstr *, char *, int);
extern int load_comm_ok(struct sctstr *, natid, i_type, int);
extern void gift(natid, natid, void *, char *);
extern int display_mark(i_type, int);
extern int line_of_sight(char **rad, int ax, int ay, int bx, int by);
extern void plane_sona(struct emp_qelem *, int, int, struct shiplist **);
extern char *prsub(struct shpstr *);
extern int check_trade(void);
extern int ontradingblock(int, void *);
extern void trdswitchown(int, struct empobj *, int);
extern int do_look(int);
extern void look_at_sect(struct sctstr *, int);
extern int radar(int);
extern void update_power(void);
extern int show_first_tel(char *);
/* Commands */
int c_accept(void);
int c_add(void);
int c_anti(void);
int c_apropos(void);
int c_arm(void);
int c_army(void);
int c_assault(void);
int c_attack(void);
int c_bdes(void);
int c_bestpath(void);
int c_board(void);
int c_bomb(void);
int c_break(void);
int c_budget(void);
int c_build(void);
int c_buy(void);
int c_capital(void);
int c_cargo(void);
int c_census(void);
int c_change(void);
int c_coastwatch(void);
int c_collect(void);
int c_commodity(void);
int c_consider(void);
int c_convert(void);
int c_country(void);
int c_cutoff(void);
int c_declare(void);
int c_deliver(void);
int c_demobilize(void);
int c_designate(void);
int c_disable(void);
int c_disarm(void);
int c_distribute(void);
int c_drop(void);
int c_dump(void);
int c_echo(void);
int c_edit(void);
int c_enable(void);
int c_enlist(void);
int c_execute(void);
int c_explore(void);
int c_financial(void);
int c_fire(void);
int c_flash(void);
int c_fleetadd(void);
int c_fly(void);
int c_force(void);
int c_fortify(void);
int c_give(void);
int c_grind(void);
int c_harden(void);
int c_headlines(void);
int c_improve(void);
int c_info(void);
int c_land(void);
int c_launch(void);
int c_lboard(void);
int c_lcargo(void);
int c_ldump(void);
int c_ledger(void);
int c_level(void);
int c_list(void);
int c_lload(void);
int c_llookout(void);
int c_lmine(void);
int c_load(void);
int c_lookout(void);
int c_lost(void);
int c_lradar(void);
int c_lretreat(void);
int c_lstat(void);
int c_ltend(void);
int c_map(void);
int c_march(void);
int c_market(void);
int c_mine(void);
int c_mission(void);
int c_morale(void);
int c_motd(void);
int c_move(void);
int c_name(void);
int c_nation(void);
int c_navigate(void);
int c_ndump(void);
int c_newcap(void);
int c_neweff(void);
int c_newspaper(void);
int c_nuke(void);
int c_offer(void);
int c_origin(void);
int c_paradrop(void);
int c_path(void);
int c_payoff(void);
int c_pboard(void);
int c_pdump(void);
int c_peek(void);
int c_plane(void);
int c_players(void);
int c_power(void);
int c_production(void);
int c_pstat(void);
int c_quit(void);
int c_radar(void);
int c_range(void);
int c_read(void);
int c_realm(void);
int c_recon(void);
int c_reject(void);
int c_relations(void);
int c_reload(void);
int c_repay(void);
int c_report(void);
int c_reset(void);
int c_resource(void);
int c_retreat(void);
int c_route(void);
int c_sabotage(void);
int c_satellite(void);
int c_scrap(void);
int c_scuttle(void);
int c_sdump(void);
int c_sect(void);
int c_sell(void);
int c_set(void);
int c_setresource(void);
int c_setsector(void);
int c_sharebmap(void);
int c_shark(void);
int c_ship(void);
int c_shoot(void);
int c_show(void);
int c_shutdown(void);
int c_sinfrastructure(void);
int c_skywatch(void);
int c_sonar(void);
int c_spy(void);
int c_sstat(void);
int c_start(void);
int c_starvation(void);
int c_stockpile(void);
int c_stop(void);
int c_strength(void);
int c_supply(void);
int c_survey(void);
int c_swapsector(void);
int c_telegram(void);
int c_tend(void);
int c_territory(void);
int c_threshold(void);
int c_toggle(void);
int c_torpedo(void);
int c_trade(void);
int c_transport(void);
int c_turn(void);
int c_update(void);
int c_upgrade(void);
int c_version(void);
int c_wall(void);
int c_wingadd(void);
int c_wipe(void);
int c_work(void);
int c_xdump(void);
int c_zdone(void);

/*
 * src/lib/common/ *.c
 */
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
/* mapdist.c */
extern int mapdist(int, int, int, int);
/* move.c */
/* in path.h */
/* nstreval.c */
/* in nsc.h */
/* rdsched.c */
extern int read_schedule(char *, time_t[], int, time_t, time_t);
/* res_pop.c */
extern int max_population(float, int, int);
extern int max_pop(float, struct sctstr *);
extern int max_workers(float, struct sctstr *);
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
/* disassoc.c */
extern int disassoc(void);
/* fnameat.c */
/* in fnameat.h */
/* fsize.c */
extern int fsize(int);
extern int blksize(int);
extern time_t fdate(int);
/* ioqueue.c */
/* in ioqueue.h */
/* log.c */
extern int loginit(char *);
extern int logreopen(void);
extern void logerror(char *, ...) ATTRIBUTE((format (printf, 1, 2)));
/* more in misc.h */
/* in  */
/* numstr.c */
extern char *effadv(int);
extern char *numstr(char buf[], int n);
/* parse.c */
extern int parse(char *, char *, char **, char **, char **, char **);
/* plur.c */
extern char *esplur(int n);
extern char *splur(int n);
extern char *iesplur(int n);
/* queue.c */
/* in queue.h */
/* round.c */
extern int ldround(double, int);
extern int roundintby(int, int);
/* scthash.c */
extern int scthash(int, int, int);
/* tcp_listen.c */
extern int tcp_listen(char *, char *, size_t *);

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
extern time_t player_io_deadline(struct player *, int);
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
/* in com.h */
/* init_nats.c */
extern int init_nats(void);
extern int player_set_nstat(struct player *, struct natstr *);
/* login.c */
extern void player_login(void *);
/* nat.c */
extern int natbyname(char *, natid *);
extern int natpass(natid, char *);
/* player.c */
extern struct player *player;	/* current player's context */
extern char *praddr(struct player *);
extern void player_main(struct player *);
extern void make_stale_if_command_arg(char *);
/* more under Commands */
/* recvclient.c */
extern int recvclient(char *, int);

/* service.c */
extern void stop_service(void);
/* more in service.h */

/*
 * src/lib/subs/ *.c
 */
/* actofgod.c */
/* in actofgod.h */
/* askyn.c */
extern int confirm(char *);
extern int askyn(char *);
/* bridgefall.c */
extern int bridge_support_at(struct sctstr *, int);
extern void bridge_damaged(struct sctstr *);
extern void bridgefall(struct sctstr *);
/* bsanct.c */
extern void bsanct(void);
/* caploss.c */
extern void caploss(struct sctstr *, natid, char *);
/* check.c */
extern int check_obj_ok(struct empobj *);
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
extern double security_strength(struct sctstr *, int *);
extern int military_control(struct sctstr *);
extern int abandon_askyn(struct sctstr *, i_type, int, struct ulist *);
extern int would_abandon(struct sctstr *, i_type, int, struct ulist *);
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
/* fileinit.c */
extern void ef_init_srv(int);
extern void ef_fin_srv(void);
/* fortdef.c */
extern int sd(natid, natid, coord, coord, int, int, int);
extern int dd(natid, natid, coord, coord, int, int);
extern int shipdef(natid, natid, coord, coord);
extern int feels_like_helping(natid, natid, natid);
/* getbit.c */
extern int emp_getbit(int, int, unsigned char *);
extern void emp_setbit(int, int, unsigned char *);
extern void bitinit2(struct nstr_sect *, unsigned char *, int);
/* getele.c */
extern int getele(char *, char *);
/* land.c */
extern char *prland(struct lndstr *);
extern void lnd_postread(int, void *);
extern void lnd_prewrite(int, void *, void *);
/* landgun.c */
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
extern void print_shiplist(struct shiplist *);
extern int has_units(coord, coord, natid);
extern int adj_units(coord, coord, natid);
/* maps.c */
/* in map.h */
/* mission.c */
extern char *mission_name(int);
extern int collateral_damage(coord, coord, int);
extern int mission_pln_equip(struct plist *, struct ichrstr *, char);
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
		       int (*)(coord, coord, char *, char *),
		       int, int *);
extern int fly_map(coord, coord);
/* mslsub.c */
extern int msl_launch(struct plnstr *, int, char *, coord, coord, natid,
		      int *);
extern int msl_hit(struct plnstr *, int, int, int, int, int, natid);
extern void msl_sel(struct emp_qelem *, coord, coord, natid, int,
		    int, int);
extern int msl_abm_intercept(struct plnstr *, coord, coord, int);
extern int msl_asat_intercept(struct plnstr *, coord, coord);
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
extern void nuk_postread(int, void *);
extern void nuk_prewrite(int, void *, void *);
/* nxtitem.c */
/* nxtsct.c */
/* in nsc.h */
/* onearg.c */
extern int onearg(char *, char *);
/* plane.c */
extern char *prplane(struct plnstr *);
extern void pln_postread(int, void *);
extern void pln_prewrite(int, void *, void *);
/* plnsub.c */
extern int get_planes(struct nstr_item *, struct nstr_item *,
		      char *, char *);
extern struct sctstr *get_assembly_point(char *, struct sctstr *, char *);
extern int pln_where_to_land(coord, coord, union empobj_storage *, int *);
extern int pln_can_land_on_carrier(struct emp_qelem *, struct emp_qelem *,
				   struct shpstr *);
extern void pln_newlanding(struct emp_qelem *, coord, coord, int);
extern int could_be_on_ship(struct plnstr *, struct shpstr *);
extern int put_plane_on_ship(struct plnstr *, struct shpstr *);
extern void pln_dropoff(struct emp_qelem *, struct ichrstr *,
			coord, coord, int);
extern void pln_mine(struct emp_qelem *, coord, coord);
extern int pln_capable(struct plnstr *, int, int);
extern int pln_caps(struct emp_qelem *);
extern int pln_airbase_ok(struct plnstr *, int, int);
extern int carrier_planes(struct shpstr *, int);
extern void pln_sel(struct nstr_item *, struct emp_qelem *,
		    struct sctstr *, int, int, int, int);
extern void pln_arm(struct emp_qelem *, int, char, struct ichrstr *);
extern int pln_equip(struct plist *, struct ichrstr *, char);
extern int pln_mobcost(int, struct plnstr *, char);
extern void pln_put(struct emp_qelem *);
extern void pln_put1(struct plist *);
extern void plane_sweep(struct emp_qelem *, coord, coord);
extern int put_plane_on_land(struct plnstr *, struct lndstr *);
extern int pln_hitchance(struct plnstr *, int, int);
extern int pln_damage(struct plnstr *, char, char *);
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
extern void prxy(char *, coord, coord);
extern void pr_beep(void);
extern void mpr(int, char *, ...) ATTRIBUTE((format (printf, 2, 3)));
extern size_t copy_ascii_no_funny(char *, char *);
extern size_t copy_utf8_no_funny(char *, char *);
extern size_t copy_utf8_to_ascii_no_funny(char *, char *);
extern int ufindpfx(char *, int);
/* radmap.c */
extern int deltx(struct range *, coord);
extern int delty(struct range *, coord);
extern void radmap(int, int, int, double, int, double);
extern void rad_map_set(natid, int, int, int, double, int);
/* rej.c */
/* in nat.h */
/* retreat.c */
/* in retreat.h */
/* sarg.c */
/* in nsc.h */
/* satmap.c */
extern void satdisp_sect(struct sctstr *, int);
extern void satdisp_units(coord, coord);
extern int satmap(int, int, int, int, int, int);
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
/* in ship.h */
/* snxtitem.c */
/* snxtsct.c */
/* in nsc.h */
/* supply.c */
extern int sct_supply(struct sctstr *, i_type, int);
extern int shp_supply(struct shpstr *, i_type, int);
extern int lnd_supply(struct lndstr *, i_type, int);
extern int lnd_supply_all(struct lndstr *);
extern int lnd_in_supply(struct lndstr *);
extern int lnd_could_be_supplied(struct lndstr *);
/* takeover.c */
extern void takeover_land(struct lndstr *, natid);
extern void takeover(struct sctstr *, natid);
extern void takeover_ship(struct shpstr *, natid);
extern void takeover_plane(struct plnstr *, natid);
/* trdsub.c */
/* in trade.h */
/* whatitem.c */
/* in item.h */
/* wu.c */
extern void clear_telegram_is_new(natid);
extern int typed_wu(natid, natid, char *, int);
extern int wu(natid, natid, char *, ...) ATTRIBUTE((format (printf, 3, 4)));

/*
 * src/lib/update/ *.c
 */
/* in update.h */

/*
 * src/server
 */
/* shutdown.c */
/* in server.h */

#endif
