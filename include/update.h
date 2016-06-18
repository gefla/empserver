/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  update.h: Definitions related to the update
 *
 *  Known contributors to this file:
 *     Ville Virrankoski, 1995
 *     Markus Armbruster, 2004-2016
 */

#ifndef UPDATE_H
#define UPDATE_H

#include "sect.h"

#define IMPORT	0
#define EXPORT	1

#define SCT_EFFIC (SCT_TYPE_MAX + 1)
#define SCT_MAINT (SCT_TYPE_MAX + 2)
#define SCT_BUDG_MAX SCT_MAINT

struct budg_item {
    int money;			/* money delta */
    int count;			/* #things making/consuming the money */
};

/* A nation's budget for an update */
struct budget {
    /* military payroll */
    struct budg_item mil;
};

/* main.c */
extern struct budget nat_budget[MAXNOC];
extern int money[MAXNOC];
extern int pops[MAXNOC];
extern int sea_money[MAXNOC];
extern int lnd_money[MAXNOC];
extern int air_money[MAXNOC];
extern int tpops[MAXNOC];
/* nat.c */
extern float levels[MAXNOC][4];

/* age.c */
extern int age_people(int, int);
extern void age_levels(int);
/* anno.c */
extern void delete_old_announcements(void);
/* bp.c */
extern struct bp *bp_alloc(void);
extern void bp_set_from_sect(struct bp *, struct sctstr *);
extern void bp_to_sect(struct bp *, struct sctstr *);
/* deliver.c */
extern void dodeliver(struct sctstr *);
/* distribute.c */
extern int dodistribute(struct sctstr *, int, double);
/* finish.c */
extern void finish_sects(int);
/* human.c */
extern void do_feed(struct sctstr *, struct natstr *, int, int);
extern int feed_people(short *, int);
extern double food_needed(short *, int);
extern int famine_victims(short *, int);
/* land.c */
extern int prod_land(int, int, struct bp *, int);
/* main.c */
/* in server.h */
/* material.c */
extern int get_materials(struct sctstr *, short[], int);
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
/* nxtitemp.c */
/* in nsc.h */
/* plague.c */
extern void do_plague(struct sctstr *, int);
extern int plague_people(struct natstr *, short *, int *, int *, int);
/* plane.c */
extern int prod_plane(int, int, struct bp *, int);
/* populace.c */
extern void populace(struct sctstr *, int);
extern int total_work(int, int, int, int, int, int);
/* prepare.c */
extern void prepare_sects(int);
extern void tax(struct sctstr *, int, int *, int *, int *);
extern void upd_slmilcosts(int, natid);
extern int bank_income(struct sctstr *, int);
extern void pay_reserve(struct natstr *, int);
/* produce.c */
extern int produce(struct natstr *, struct sctstr *, int *);
extern int prod_materials_cost(struct pchrstr *, short[], int *);
extern int prod_resource_limit(struct pchrstr *, unsigned char *);
extern double prod_eff(int, float);
/* removewants.c */
extern int update_removewants(void);
/* revolt.c */
extern void revolt(struct sctstr *);
extern void guerrilla(struct sctstr *);
/* sect.c */
extern double buildeff(struct sctstr *);
extern void do_fallout(struct sctstr *, int);
extern void spread_fallout(struct sctstr *, int);
extern void decay_fallout(struct sctstr *, int);
extern void produce_sect(struct natstr *, int, struct bp *, int[][2]);
/* ship.c */
extern int prod_ship(int, int, struct bp *, int);

#endif
