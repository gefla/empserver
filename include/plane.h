/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plane.h: Definitions for planes and plane types
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2016
 */

#ifndef PLANE_H
#define PLANE_H

#include "file.h"
#include "item.h"
#include "queue.h"
#include "types.h"

#define PLN_TYPE_MAX	46
#define PLANE_MINEFF	10

struct plnstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned pln_seqno: 12;
    unsigned pln_generation: 12;
    int pln_uid;		/* unit ID (plane #) */
    time_t pln_timestamp;	/* Last time this plane was touched */
    natid pln_own;		/* owning country */
    coord pln_x;		/* plane x-y */
    coord pln_y;
    signed char pln_type;	/* index in plchr[] */
    signed char pln_effic;	/* actually "training" */
    signed char pln_mobil;	/* plane mobility */
    unsigned char pln_off;	/* repairs stopped? */
    short pln_tech;		/* plane's tech level */
    char pln_wing;		/* like fleet */
    coord pln_opx;		/* Op sector coords */
    coord pln_opy;		/* Op sector coords */
    short pln_mission;		/* mission code */
    short pln_radius;		/* mission radius */
    /* end of part matching struct empobj */
    unsigned char pln_range;	/* total distance, not radius */
    signed char pln_harden;	/* for missiles */
    int pln_ship;		/* UID of carrier, or -1 */
    int pln_land;		/* UID of transporting land unit, or -1 */
    int pln_flags;		/* State of the plane */
    short pln_access;		/* Last tick mob was updated (MOB_ACCESS) */
    float pln_theta;		/* position in orbital sine wave */
};

#define PLN_LAUNCHED	bit(0)	/* Flying (satellite: in orbit) */
#define PLN_SYNCHRONOUS	bit(1)	/* A satellite in geo-synchronous orbit */
#define PLN_AIRBURST	bit(2)	/* Airburst the nuke we're armed with */

struct plchrstr {
    char *pl_name;		/* full name of type of plane */
    short pl_mat[I_MAX+1];	/* materials to build 100% */
				/* only I_LCM, I_HCM, I_MILIT non-zero */
    int pl_bwork;		/* work to build 100% */
    int pl_tech;		/* tech needed to build */
    int pl_cost;		/* how much it costs to build */
    int pl_acc;			/* bombing accuracy (higher the better) */
    int pl_load;		/* bomb load, also for carrying cargo */
    int pl_att;			/* air-air attack/defense strengths */
    int pl_def;
    int pl_range;		/* how many sectors it can fly */
    int pl_fuel;		/* fuel consumption */
    int pl_stealth;		/* how stealthy is it? */
    int pl_flags;		/* description of capability */
    signed char pl_type;	/* index in plchr[] */
};

#define P_B	bit(0)		/* bombing factor only for bombing */
#define P_T	bit(1)		/* tactical bombing ability */
#define P_F	bit(2)		/* interceptor/escort */
#define P_C	bit(3)		/* bombing factor for transport only */
#define P_V	bit(4)		/* vtol ability */
#define P_M	bit(5)		/* missile, used only once, can't be intercepted */
#define P_L	bit(6)		/* light plane, lands on carriers */
#define P_S	bit(7)		/* Spy ability */
#define P_I	bit(8)		/* Imaging ability (advanced spying) */
#define P_O	bit(9)		/* Orbital ability (a satellite) */
/* unused	bit(10) */
#define P_N	bit(11)		/* Nuclear RV interceptor (ABM) */
/* unused	bit(12) */
#define P_E	bit(13)		/* extra light */
#define P_K	bit(14)		/* chopper */
#define P_A	bit(15)		/* ASW (Anti-Sub Warfare) */
#define P_P	bit(16)		/* paradropping */
#define P_ESC	bit(17)		/* escort */
#define P_MINE	bit(18)		/* can lay mines */
#define P_SWEEP	bit(19)		/* can sweep mines */
#define P_MAR	bit(20)		/* missile can hit ships */

#define PM_R	bit(0)		/* Recon mission */
#define PM_S	bit(1)		/* Mine-sweeping mission */

	/* internal to "fly" module -- really shouldn't be here */

struct plist {
    struct emp_qelem queue;	/* list of planes */
    int load;			/* number of bombs or items carried */
    short pstage;		/* plague stage */
    struct plchrstr *pcp;	/* pointer to desc of plane */
    struct plnstr plane;	/* struct plane */
};

#define getplane(n, p) ef_read(EF_PLANE, (n), (p))
#define putplane(n, p) ef_write(EF_PLANE, (n), (p))
#define getplanep(n) ((struct plnstr *)ef_ptr(EF_PLANE, (n)))

extern struct plchrstr plchr[PLN_TYPE_MAX + 2];

struct shiplist {
    int uid;
    struct shiplist *next;
};

extern int pl_att(struct plchrstr *, int);
extern int pl_def(struct plchrstr *, int);
extern int pl_acc(struct plchrstr *, int);
extern int pl_range(struct plchrstr *, int);
extern int pl_load(struct plchrstr *, int);
extern int pln_att(struct plnstr *);
extern int pln_def(struct plnstr *);
extern int pln_acc(struct plnstr *);
extern int pln_range_max(struct plnstr *);
extern int pln_load(struct plnstr *);

/* src/lib/common/cargo.c */
extern void pln_carrier_change(struct plnstr *, int, int, int);
extern int pln_first_on_ship(struct shpstr *);
extern int pln_first_on_land(struct lndstr *);
extern int pln_next_on_unit(int);

/* src/lib/subs/aircombat.c */
extern int ac_flak_dam(int, int, int);
extern void ac_encounter(struct emp_qelem *, struct emp_qelem *, coord,
			 coord, char *, int);
extern int ac_damage_plane(struct plnstr *, natid, int, int, char *);

/* src/lib/subs/aswplnsubs.c */
extern int on_shiplist(int, struct shiplist *);
extern void add_shiplist(int, struct shiplist **);
extern void free_shiplist(struct shiplist **);

#endif
