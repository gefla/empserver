/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plane.h: Definitions for planes and plane types
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#ifndef _PLANE_H_
#define _PLANE_H_

#include "queue.h"
#include "sect.h"

#define	PLANE_MINEFF	10

/*
 * some routines assume that from pln_x to pln_wing remains
 * exactly equivalent with shp_x to shp_fleet
 */
struct plnstr {
    /* initial part must match struct genitem */
    short ef_type;
    natid pln_own;		/* owning country */
    short pln_uid;		/* plane unit id */
    coord pln_x;		/* plane x-y */
    coord pln_y;
    s_char pln_type;		/* index in plchr[] */
    s_char pln_effic;		/* actually "training" */
    s_char pln_mobil;		/* plane mobility */
    short pln_tech;		/* plane's tech level */
    s_char pln_wing;		/* like fleet */
    coord pln_opx;		/* Op sector coords */
    coord pln_opy;		/* Op sector coords */
    short pln_mission;		/* mission code */
    short pln_radius;		/* mission radius */
    /* end of part matching struct genitem */
    u_char pln_range;		/* total distance, not radius */
    u_char pln_range_max;	/* max range for this plane */
    short pln_ship;		/* pointer to carrier */
    short pln_land;		/* pointer to carrier */
    int pln_att;		/* plane air-air attack/defense values */
    int pln_def;
    int pln_acc;
    int pln_load;
    int pln_fuel;		/* (unused) */
    s_char pln_harden;		/* for missiles */
    s_char pln_nuketype;	/* type of nuclear armament (if any) */
    s_char pln_flags;		/* State of the plane */
    time_t pln_access;		/* Last time mob was updated (MOB_ACCESS) */
    time_t pln_timestamp;	/* Last time this plane was touched */
    float pln_theta;		/* position in orbital sine wave */
};

#define	PLN_LAUNCHED	bit(0)	/* A satellite that's in orbit */
#define	PLN_SYNCHRONOUS	bit(1)	/* A satellite in geo-synchronous orbit */
#define	PLN_AIRBURST	bit(2)	/* Airburst the nuke we're armed with */

struct plchrstr {
    char *pl_name;		/* full name of type of plane */
    int pl_lcm;			/* costs to build */
    int pl_hcm;
    int pl_cost;
    int pl_tech;		/* tech needed to build */
    int pl_acc;			/* bombing accuracy (higher the better) */
    int pl_load;		/* bomb load, also for carrying cargo */
    int pl_att;			/* air-air attack/defense strengths */
    int pl_def;
    int pl_range;		/* how many sectors it can fly */
    int pl_crew;		/* number of mil to crew it */
    int pl_fuel;		/* fuel consumption */
    int pl_stealth;		/* how stealthy is it? */
    int pl_flags;		/* description of capability */
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
#define P_X	bit(10)		/* Stealth ability */
#define P_N	bit(11)		/* Nuclear RV interceptor (abm) */
#define	P_H	bit(12)		/* Half stealthy */
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

#define PLN_MAXDEF	50

	/* internal to "fly" module -- really shouldn't be here */

struct plist {
    struct emp_qelem queue;	/* list of planes */
    int state;			/* state of plane */
    int bombs;			/* bombs carried for bombing mission */
    int misc;			/* amount of random item being transported */
    struct plchrstr *pcp;	/* pointer to desc of plane */
    struct plnstr plane;	/* struct plane */
};

#define	P_OK		0
#define P_FIRED		1
#define P_DEL		2

#define getplane(n, p) \
	ef_read(EF_PLANE, n, p)
#define putplane(n, p) \
	ef_write(EF_PLANE, n, p)
#define getplanep(n) \
	(struct plnstr *) ef_ptr(EF_PLANE, n)

extern struct plchrstr plchr[];
extern int pln_maxno;

struct shiplist {
    short uid;
    struct shiplist *next;
};

#define PLN_ATTDEF(b, t) (b + ((b?1:0) * ((t/20)>10?10:(t/20))))
#define PLN_ACC(b, t) (b * (1.0 - (sqrt((double)t) / 50.)))
#define PLN_RAN(b, t) (t ? (b + (logx((double)t, (double)2.0))) : b)
#define PLN_LOAD(b, t) (t ? (b * (logx((double)t, (double)50.0) < 1.0 ? 1.0 : \
				  logx((double)t, (double)50.0))) : b)

/* Work required for building 100% */
#define PLN_BLD_WORK(lcm, hcm) (20 + (lcm) + 2 * (hcm))

/* src/lib/subs/aircombat.c */
extern void ac_planedamage(struct plist *, natid, int, natid, int,
			   int, s_char *);
extern void ac_intercept(struct emp_qelem *, struct emp_qelem *,
			 struct emp_qelem *, natid, coord, coord);
extern void ac_combat_headers(natid, natid);
extern void ac_airtoair(struct emp_qelem *, struct emp_qelem *);
extern void ac_doflak(struct emp_qelem *, struct sctstr *);
extern void ac_shipflak(struct emp_qelem *, coord, coord);
extern void ac_landflak(struct emp_qelem *, coord, coord);
extern void ac_fireflak(struct emp_qelem *, natid, natid, int);
extern int ac_flak_dam(int);
extern void ac_encounter(struct emp_qelem *, struct emp_qelem *, coord,
			 coord, s_char *, int, int,
			 struct emp_qelem *, struct emp_qelem *);
extern int ac_isflying(struct plnstr *, struct emp_qelem *);
extern int do_evade(struct emp_qelem *, struct emp_qelem *);
extern void sam_intercept(struct emp_qelem *, struct emp_qelem *,
			  natid, natid, coord, coord, int);
extern int all_missiles(struct emp_qelem *);
extern int can_fly(int);

/* src/lib/subs/aswplnsubs.c */
extern int on_shiplist(short, struct shiplist *);
extern void add_shiplist(short, struct shiplist **);
extern void free_shiplist(struct shiplist **);
extern void print_shiplist(struct shiplist *);

#endif /* _PLANE_H_ */
