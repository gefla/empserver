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
extern emp_sig_t panic(int sig);
extern emp_sig_t shutdwn(int sig);

/*****************************************************************************
 * src/lib/ * / *.c 
 *****************************************************************************/

/* src/lib/empthread/lwp.c */
#ifdef _EMPTH_LWP
extern int empth_init(char **ctx, int flags);
extern empth_t *empth_create(int prio, void (*entry) (), int size,
			     int flags, char *name, char *desc, void *ud);
#if 0
extern void empth_setctx(void *ctx);
#endif
extern empth_t *empth_self(void);
extern void empth_exit(void);
extern void empth_yield(void);
extern void empth_terminate(empth_t *a);
extern void empth_select(int fd, int flags);
extern void empth_wakeup(empth_t *a);
extern void empth_sleep(long int until);
extern empth_sem_t *empth_sem_create(char *name, int cnt);
extern void empth_sem_signal(empth_sem_t *sm);
extern void empth_sem_wait(empth_sem_t *sm);
extern emp_sig_t empth_alarm(int sig);
#endif /* _EMPTH_LWP */

/* src/lib/empthread/pthread.c */
#ifdef _EMPTH_POSIX
extern int empth_init(char **ctx_ptr, int flags);
extern empth_t *empth_create(int prio, void (*entry) (),
			     int size, int flags,
			     char *name, char *desc, void *ud);
extern empth_t *empth_self(void);
extern void empth_exit(void);
extern void empth_yield(void);
extern void empth_terminate(empth_t *a);
extern void empth_select(int fd, int flags);
extern emp_sig_t empth_alarm(int sig);
extern void empth_wakeup(empth_t *a);
extern void empth_sleep(long until);
extern empth_sem_t *empth_sem_create(char *name, int cnt);
extern void empth_sem_signal(empth_sem_t *sm);
extern void empth_sem_wait(empth_sem_t *sm);
#endif /* _EMPTH_POSIX */

/*
 * src/lib/commands/ *.c 
 */
extern int acce(void);
extern int add(void);
extern int anti(void);
extern int arm(void);
extern int army(void);
extern int assa(void);
extern int atta(void);
extern int bdes(void);
extern int best(void);
extern int boar(void);
extern int bomb(void);
extern int brea(void);
extern int budg(void);
extern int buil(void);
extern int buy(void);
extern int check_market(void);
extern int capi(void);
extern int carg(void);
extern int lcarg(void);
extern int cede(void);
extern int cens(void);
extern void set_coastal(struct sctstr *);
extern int chan(void);
extern int coas(void);
extern int coll(void);
extern int comm(void);
extern int cons(void);
extern int conv(void);
extern int coun(void);
extern int cuto(void);
extern int decl(void);
extern int deli(void);
extern int demo(void);
extern int desi(void);
extern int disa(void);
extern int diss(void);
extern int dist(void);
extern int drop(void);
extern int dump(void);
extern int echo(void);
extern int edit(void);
extern int enab(void);
extern int enli(void);
extern int explore(void);
extern int fina(void);
extern int flash(void);
extern int sendmessage(struct natstr *, struct natstr *, char *, int);
extern int wall(void);
extern int flee(void);
extern int fly(void);
extern int foll(void);
extern int force(void);
extern int fort(void);
extern int fuel(void);
extern int give(void);
extern int grin(void);
extern int hard(void);
extern int head(void);
extern int hidd(void);
extern int improve(void);
extern int info(void);
extern int apro(void);
extern int land(void);
extern int laun(void);
extern int ldump(void);
extern int ledg(void);
extern int leve(void);
extern void gift(int, int, s_char *, int, s_char *);
extern int load(void);
extern int lload(void);
extern int look(void);
extern int llook(void);
extern int lost(void);
extern int lsta(void);
extern int ltend(void);
extern int map(void);
extern int march(void);
extern int mark(void);
extern int display_mark(s_char *);
extern int multifire(void);
extern int mine(void);
extern int landmine(void);
extern int mission(void);
extern int mobq(void);
extern int mobupdate(void);
extern int morale(void);
extern int move(void);
extern int want_to_abandon(struct sctstr *, int, int, struct lndstr *);
extern int would_abandon(struct sctstr *, int, int, struct lndstr *);
extern int mult(void);
extern int name(void);
extern int nati(void);
extern int navi(void);
extern int nav_map(int, int, int);
extern int ndump(void);
extern int new(void);
extern int newe(void);
extern int news(void);
extern int nuke(void);
extern int offe(void);
extern int offs(void);
extern int orde(void);
extern int qorde(void);
extern int sorde(void);
extern int orig(void);
extern int para(void);
extern int path(void);
extern int pdump(void);
extern int plan(void);
extern int play(void);
extern int powe(void);
extern int count_pop(register int);
extern int prod(void);
extern int pstat(void);
extern int rada(void);
extern int range(void);
extern int lrange(void);
extern int rea(void);
extern int real(void);
extern int reco(void);
extern int reje(void);
extern int rela(void);
extern int repa(void);
extern int repo(void);
extern int rese(void);
extern int reso(void);
extern int retr(void);
extern int lretr(void);
extern int rout(void);
extern int sail(void);
extern int sate(void);
extern int scra(void);
extern int scuttle_tradeship(struct shpstr *, int);
extern int scut(void);
extern void scuttle_ship(struct shpstr *);
extern int sdump(void);
extern int sct(void);
extern int sell(void);
extern int set(void);
extern int setres(void);
extern int setsector(void);
extern void resnoise(struct sctstr *sptr, int public_amt, s_char *name,
		     int old, int new);
extern int shar(void);
extern int shark(void);
extern int shi(void);
extern int shoo(void);
extern int show(void);
extern int shut(void);
extern int sinfra(void);
extern int skyw(void);
extern int line_of_sight(s_char **rad, int ax, int ay, int bx, int by);
extern int sona(void);
extern void plane_sona(struct emp_qelem *, int, int, struct shiplook *);
extern int spy(void);
extern int sstat(void);
extern int start(void);
extern int stop(void);
extern int stre(void);
extern int starve(void);
extern int supp(void);
extern int surv(void);
extern int swaps(void);
extern int tele(void);
extern int tend(void);
extern int tend_nxtitem(struct nstr_item *, caddr_t);
extern int terr(void);
extern int thre(void);
extern int togg(void);
extern s_char *prsub(struct shpstr *);
extern int torp(void);
extern int trad(void);
extern int check_trade(void);
extern int ontradingblock(int, int *);
extern void trdswitchown(int, int *, int);
extern int tran(void);
extern int trea(void);
extern int turn(void);
extern int upda(void);
extern int upgr(void);
extern int vers(void);
extern int wai(void);
extern int wing(void);
extern int wipe(void);
extern int work(void);
extern int zdon(void);

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
/* getvar.c */
extern int getvar(int, s_char *, int);
extern int getvec(int, int *, s_char *, int);
extern int putvar(int, int, s_char *, int);
extern int putvec(int, int *, s_char *, int);
/* hap_fact.c */
extern double hap_fact(struct natstr *, struct natstr *);
/* hours.c */
extern int gamehours(time_t, int *);
/* land.c */
extern int has_units(coord, coord, natid, struct lndstr *);
extern int has_units_with_mob(coord, coord, natid);
extern int adj_units(coord, coord, natid);
extern int is_engineer(int x, int y);
/* log.c */
extern void loginit(void);
extern void logerror(s_char *, ...);
/* maps.c */
extern int draw_map(int, s_char, int, struct nstr_sect *, int);
extern int unit_map(int, int, struct nstr_sect *, s_char *);
extern int bmaps_intersect(natid, natid);
extern int share_bmap(natid, natid, struct nstr_sect *, s_char, s_char *);
/* move.c */
extern double sector_mcost(struct sctstr *, int);
/* nstr_subs.c */
extern s_char *decodep(long, void *);
extern int decode(natid, long, void *, int);
extern int nstr_exec(struct nscstr *, register int, void *, int);
/* path.c */
extern void bp_enable_cachepath();
extern void bp_disable_cachepath();
extern void bp_clear_cachepath();
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
extern int stmtch(register s_char *, s_char *, int, int);
extern int mineq(register s_char *, register s_char *);
/* type.c */
extern int typematch(s_char *, int);
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

extern int atoip(s_char **);
extern int roll(int);
extern int roundavg(double);
extern int chance(double);
extern void disassoc(void);
extern int getfdtablesize(void);
extern s_char *getstarg(s_char *, s_char *, s_char *);
extern s_char *getstring(s_char *, s_char *);
#if !defined(_WIN32)
extern s_char *inet_ntoa(struct in_addr);
#endif
extern int file_lock(int);
extern int file_unlock(int);
extern int diffx(int, int);
extern int diffy(int, int);
extern int deltax(int, int);
extern int deltay(int, int);
extern int mapdist(int, int, int, int);
extern double dmin(double, double);
#if !defined(_WIN32)
extern int max(int, int);
extern int min(int, int);
#endif
extern double dmax(double, double);
extern s_char *numstr(s_char *, int);
extern s_char *effadv(int);
extern int onearg(s_char *, s_char *);
extern int parse(register s_char *, s_char **, s_char **, s_char *,
		 s_char **);
extern s_char *esplur(int);
extern s_char *splur(int);
extern s_char *iesplur(int);
extern s_char *plur(int, s_char *, s_char *);
extern int ldround(double, int);
extern int roundintby(int, int);
extern int scthash(register int, register int, int);
#ifdef NOSTRDUP
extern char *strdup(char *x);
#endif


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
extern int explain(void);
extern int gamedown(void);
extern void daychange(time_t);
extern int getminleft(time_t, int *, int *);
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
extern int show_motd(void);
extern int match_user(char *, struct player *);
extern int status(void);
extern int command(void);
extern int execute(void);
extern int quit(void);
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
extern s_char *nameofitem(s_char *, struct genitem *, int);
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
		       double, double, s_char *,
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
extern void pln_newlanding(struct emp_qelem *, coord, coord, int);
extern int can_be_on_ship(int, int);
extern int put_plane_on_ship(struct plnstr *, struct shpstr *);
extern void pln_dropoff(struct emp_qelem *, struct ichrstr *, coord,
			coord, s_char *, int);
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
/* pr.c */
extern void pr(s_char *, ...);
extern void prnf(s_char *buf);
extern void pr_id(struct player *, int, s_char *, ...);
extern void pr_flash(struct player *, s_char *, ...);
extern void pr_inform(struct player *, s_char *, ...);
extern void pr_wall(s_char *, ...);
extern void pr_player(struct player *pl, int id, s_char *buf);
extern void pr_hilite(s_char *buf);
extern void prredir(s_char *redir);
extern void prexec(s_char *file);
extern void prprompt(int min, int btu);
extern void showvers(int vers);
extern int prmptrd(s_char *prompt, s_char *str, int size);
extern void prdate(void);
extern void prxy(s_char *format, coord x, coord y, natid country);
extern void PR(int, s_char *, ...);
extern void PRdate(natid cn);
extern void pr_beep(void);
extern void mpr(int, s_char *, ...);

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
extern int sarg_type(s_char *);
extern int sarg_xy(s_char *, coord *, coord *);
extern int sarg_area(s_char *, register struct range *);
extern int sarg_range(s_char *, coord *, coord *, int *);
extern int sarg_list(s_char *, register int *, int);
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
extern void show_item(int);
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
extern int trade_check_ok(int, struct trdstr *, union trdgenstr *);
extern s_char *trade_nameof(struct trdstr *, union trdgenstr *);
extern int trade_desc(struct trdstr *, union trdgenstr *);
extern int trade_getitem(struct trdstr *, union trdgenstr *);
extern long get_couval(int);
extern long get_outstand(int);
extern struct ichrstr *whichitem(char);
/* trechk.c */
extern int trechk(register natid, register natid, int);
/* whatitem.c */
extern struct ichrstr *whatitem(s_char *, s_char *);
/* wu.c */
extern void clear_telegram_is_new(natid);
extern int typed_wu(natid, natid, s_char *, int);
extern int wu(natid, natid, s_char *, ...);

/*
 * src/lib/update/ *.c 
 */
/* age.c */
extern void age_levels(int);
/* anno.c */
extern void delete_old_announcements(void);
/* bp.c */
extern void fill_update_array(int *, struct sctstr *);
extern int *get_wp(int *, struct sctstr *, int);
extern int gt_bg_nmbr(int *, struct sctstr *, int);
extern void pt_bg_nmbr(int *, struct sctstr *, int, int);
/* deliver.c */
extern int deliver(register struct sctstr *, struct ichrstr *, int, int,
		   int, int);
/* distribute.c */
extern int dodistribute(struct sctstr *, int, s_char *, double, double);
/* finish.c */
extern void finish_sects(int);
/* human.c */
extern int do_feed(register struct sctstr *, register struct natstr *,
		   int *, int *, int *, int);
extern int feed_people(register int *, int, int *);
/* land.c */
extern int prod_land(int, int, int *, int);
extern int feed_land(struct lndstr *, register int *, int, int *, int);
/* main.c */
extern void update_main(void *);
/* material.c */
extern void get_materials(struct sctstr *, int *, int *, int);
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
extern int com_num(s_char *);
extern void unload_it(register struct shpstr *);
extern void auto_fuel_ship(register struct shpstr *);
/* nxtitemp.c */
extern s_char *nxtitemp(struct nstr_item *, int);
/* nxtsctp.c */
extern struct sctstr *nxtsctp(register struct nstr_sect *);
/* plague.c */
extern void do_plague(struct sctstr *, struct natstr *, int);
extern int plague_people(struct natstr *, register int *, register int *,
			 int);
/* plane.c */
extern int prod_plane(int, int, int *, int);
/* populace.c */
extern void populace(struct natstr *, register struct sctstr *,
		     register int *, int);
extern int total_work(register int, register int, register int,
		      register int, register int);
/* prepare.c */
extern void tax(struct sctstr *, struct natstr *, int, long *, int *,
		int *, int *);
extern int upd_slmilcosts(natid, int);
extern void prepare_sects(int, int *);
extern int bank_income(struct sctstr *, int);
/* produce.c */
extern int produce(struct natstr *, struct sctstr *, int *, int, int, int,
		   int, int *, int *);
/* removewants.c */
extern int update_removewants(void);
/* revolt.c */
extern void revolt(struct sctstr *);
extern void guerrilla(struct sctstr *);
/* sail.c */
extern void sail_ship(natid);
/* sect.c */
extern int dodeliver(struct sctstr *, int *);
extern void do_fallout(register struct sctstr *, register int);
extern void spread_fallout(struct sctstr *, int);
extern void decay_fallout(struct sctstr *, int);
extern void produce_sect(int natnum, int etu, int *bp,
			 long int (*p_sect)[2], int sector_type);
/* ship.c */
extern int prod_ship(int, int, int *, int);
extern int feed_ship(struct shpstr *, register int *, int, int *, int);

/*
 * src/server
 */
/* shutdown.c */
extern void shutdown_init(void);
extern void shutdown_sequence(int argc, s_char **argv);

#endif /* _PROTOTYPES_H_ */
