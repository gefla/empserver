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
 *  combat.h: Definitions for things having to do with combat
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#ifndef COMBAT_H
#define COMBAT_H

#include "types.h"
#include "ship.h"

struct combat {
    int type;			/* EF_BAD, EF_SECTOR, EF_SHIP, or EF_PLANE */
    int set;			/* Has the struct been initialized? */
    int last;			/* The index of the last element in off array */
    coord x;			/* location of attacking sector/ship/plane */
    coord y;
    natid own;
    int eff;
    int mob;
    int mobcost;
    int mil;			/* mil in the sector/ship/unit */
    int troops;			/* mil in the combat */
    int plague;
    int sct_type;
    int shp_uid;
    int lnd_uid;
    struct dchrstr *sct_dcp;
    struct mchrstr *shp_mcp;
    struct lchrstr *lnd_lcp;
    char shp_name[MAXSHPNAMLEN]; /* FIXME huh? pulls in ship.h */
    int relations_checked;
};

/* Types of combat for use by attack routines */
#define A_DEFEND        0
#define	A_ATTACK	1
#define	A_ASSAULT	2
#define	A_PARA		3
#define A_BOARD		4
#define A_LBOARD        5

/* src/lib/subs/attsub.c */
extern double att_combat_eff(struct combat *);
extern void att_move_in_off(int, struct combat *, struct emp_qelem *,
			    struct combat *);
extern int att_combat_init(struct combat *, int);
extern int att_get_combat(struct combat *, int);
extern int att_abort(int, struct combat *, struct combat *);
extern int att_approach(struct combat *, struct combat *);
extern int att_show(struct combat *);
extern int att_ask_support(int, int *, int *, int *, int *);
extern int att_ask_offense(int, struct combat *, struct combat *,
			   struct emp_qelem *, int *, int *);
extern double sector_strength(struct sctstr *);
extern int att_get_offense(int, struct combat *, struct emp_qelem *,
			   struct combat *);
extern int att_empty_attack(int, int, struct combat *);
extern int att_get_defense(struct emp_qelem *, struct combat *,
			   struct emp_qelem *, int, int);
extern double att_reacting_units(struct combat *, struct emp_qelem *, int,
				 int *, int);
extern int att_get_support(int, int, int, int, int, struct emp_qelem *,
			   struct combat *, struct emp_qelem *,
			   struct combat *, double *, double *, int);
extern int att_fight(int, struct combat *, struct emp_qelem *, double,
		     struct combat *, struct emp_qelem *, double);
extern int att_free_lists(struct emp_qelem *, struct emp_qelem *);


extern char *att_mode[A_LBOARD + 1];

#endif
