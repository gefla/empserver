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

enum {
    BUDG_SHP_BUILD,
    BUDG_SHP_MAINT,
    BUDG_PLN_BUILD,
    BUDG_PLN_MAINT,
    BUDG_LND_BUILD,
    BUDG_LND_MAINT,
    BUDG_SCT_BUILD,
    BUDG_SCT_MAINT,
    BUDG_BLD_MAX = BUDG_SCT_MAINT
};

struct budg_item {
    double money;		/* money delta */
    int count;			/* #things making/consuming the money */
};

/* A nation's budget for an update */
struct budget {
    /* production by sector type */
    struct budg_item prod[SCT_TYPE_MAX + 1];
    /* level production output */
    float level[4];
    /* building and maintenance */
    struct budg_item bm[BUDG_BLD_MAX + 1];
    /* population, taxes, military payroll, bank interest */
    struct budg_item civ, mil, uw, bars;
    /* treasury */
    int start_money;		/* at beginning of update */
    double money;		/* current */
    /* population before growth */
    int oldowned_civs;
};

/* main.c */
extern int update_running;
extern struct budget nat_budget[MAXNOC];

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
extern void prep_lands(int);
extern void prod_land(int, struct bp *, int);
/* main.c */
/* in server.h */
/* material.c */
extern int get_materials(struct sctstr *, short[], int);
/* mobility.c */
extern void mob_inc_all(int);
extern void mob_inc_sect(struct sctstr *, int);
extern void mob_inc_ship(struct shpstr *, int);
extern void mob_inc_plane(struct plnstr *, int);
extern void mob_inc_land(struct lndstr *, int);
extern void mob_access_all(void);
/* move_sat.c */
extern void move_sat(struct plnstr *);
/* nat.c */
extern void prod_nat(int);
/* nxtitemp.c */
/* in nsc.h */
/* plague.c */
extern void do_plague(struct sctstr *, int);
extern int plague_people(struct natstr *, short *, int *, int *, int);
extern void plague_report(natid, int, int, int, int, char *, char *);
/* plane.c */
extern void prep_planes(int);
extern void prod_plane(int, struct bp *, int);
/* populace.c */
extern void populace(struct sctstr *, int);
extern int total_work(int, int, int, int, int, int);
/* prepare.c */
extern void prepare_sects(int, struct bp *);
extern void pay_reserve(struct natstr *, int);
/* produce.c */
extern void produce(struct natstr *, struct sctstr *);
extern double prod_output(struct sctstr *, double);
extern double prod_materials_cost(struct pchrstr *, short[], int *);
extern double prod_resource_limit(struct pchrstr *, unsigned char *);
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
extern void produce_sect(struct natstr *, int, struct bp *);
/* ship.c */
extern void prep_ships(int);
extern void prod_ship(int, struct bp *, int);

#endif
