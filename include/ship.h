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
 *  ship.h: Definitions for things having to do with ships
 * 
 *  Known contributors to this file:
 *     Dave Pare
 *     Thomas Rushack, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#ifndef _SHIP_H_
#define _SHIP_H_

#include "queue.h"
#include "misc.h"
#include "retreat.h"
#include "var.h"

#define	SHIP_MINEFF	20

/* bit masks for the autonav mode flags */

#define AN_SAILDIR bit(1)
#define AN_AUTONAV bit(2)
#define AN_STANDBY bit(3)
#define AN_LOADING bit(4)
#define AN_SCUTTLE bit(5)	/* Auto-scuttle of trade ships */

/* TMAX is the number of cargo holds a ship use in the autonav code. */
#define TMAX 6


#define MAXSHPPATH	28
#define	MAXSHPNAMLEN	24

struct shpstr {
    short ef_type;
    natid shp_own;		/* owner's country num */
    short shp_uid;		/* unit id (ship #) */
    coord shp_x;		/* x location in abs coords */
    coord shp_y;		/* y location in abs coords */
    s_char shp_type;		/* index in mchr[] */
    s_char shp_effic;		/* 0% to 100% */
    s_char shp_mobil;		/* mobility units */
    short shp_tech;		/* tech level ship was built at */
    s_char shp_fleet;		/* group membership */
    coord shp_opx, shp_opy;	/* Op sector coords */
    short shp_mission;		/* mission code */
    short shp_radius;		/* mission radius */
    u_char shp_nplane;		/* number of planes on board */
    u_char shp_nland;		/* number of land units on board */
    short shp_armor;
    short shp_speed;
    short shp_visib;
    short shp_frnge;
    short shp_glim;
    coord shp_destx[2];		/* location for ship destination */
    coord shp_desty[2];
    s_char shp_tstart[TMAX];	/* what goods to pick up at start point */
    s_char shp_tend[TMAX];	/* what goods to pick up at end point   */
    short shp_lstart[TMAX];	/* How much do we pick up at the start  */
    short shp_lend[TMAX];	/* How much do we pick up at the end    */
    u_char shp_autonav;		/* autonavigation flags */
    short shp_item[I_MAX+1];	/* amount of items on board */
    u_short shp_pstage;		/* plague stage */
    u_short shp_ptime;		/* how many etus remain in this stage */
    time_t shp_access;		/* Last time mob was updated (MOB_ACCESS) */
    time_t shp_timestamp;	/* Last time this ship was touched. */
    u_char shp_mobquota;	/* mobility quota */
    s_char shp_path[MAXSHPPATH];
    short shp_follow;
    s_char shp_name[MAXSHPNAMLEN];	/* name set with the "name" command */
    u_char shp_fuel;		/* How much fuel do we have */
    u_char shp_nchoppers;	/* How many choppers on board? */
    u_char shp_nxlight;		/* How many xlight planes on board? */
    coord shp_orig_x;
    coord shp_orig_y;		/* Where we were built */
    natid shp_orig_own;		/* Who built us */
    int shp_rflags;		/* When do I retreat? */
    s_char shp_rpath[RET_LEN];	/* retreat path */
};

struct fltelemstr {
    int num;
    int own;
    double mobil, mobcost;
    struct fltelemstr *next;
};

struct fltheadstr {
    int leader;
    s_char real_q;
/* defines for the real_q member */
#define	LEADER_VIRTUAL	0
#define	LEADER_REAL	1
#define	LEADER_WRONGSECT	2
    coord x, y;
    natid own;
    unsigned maxmoves;
    struct fltelemstr *head;
    struct fltheadstr *next;
};

struct mchrstr {
    u_char m_nv;		/* number of variables it can hold */
    u_char m_vtype[MAXCHRNV];
    u_short m_vamt[MAXCHRNV];
    int m_lcm;			/* units of lcm to build */
    int m_hcm;			/* units of hcm to build */
    int m_armor;		/* how well armored it is */
    int m_speed;		/* how fast it can go */
    int m_visib;		/* how well it can be seen */
    int m_vrnge;		/* how well it can see */
    int m_frnge;		/* how far it can fire */
    int m_glim;			/* how many guns it can fire */
    u_char m_nxlight;		/* maximum number of xlight planes */
    u_char m_nchoppers;		/* maximum number of choppers */
    u_char m_fuelc;		/* fuel capacity */
    u_char m_fuelu;		/* fuel used per 10 mob */
    s_char *m_name;		/* full name of type of ship */
    int m_tech;			/* tech required to build */
    int m_cost;			/* how much it costs to build */
    long m_flags;		/* what special things can this ship do */
    u_char m_nplanes;		/* maximum number of planes this ship holds */
    u_char m_nland;		/* maximum number of units this ship holds */
};

#define M_FOOD		bit(0)	/* catch that fish! */
#define M_TORP		bit(1)	/* fire torpedoes */
#define M_DCH		bit(2)	/* drop depth charges on subs */
#define M_FLY		bit(3)	/* launch and recover planes */
/* M_MSL will be automatically set in init_global() if m_nplanes > 0
   and M_FLY is not set */
#define M_MSL		bit(4)	/* launch missiles */
#define M_OIL		bit(5)	/* drill for oil */
#define M_SONAR		bit(6)	/* locate submarines */
#define M_MINE		bit(7)	/* drop mines */
#define M_SWEEP		bit(8)	/* clean up the mines */
#define M_SUB		bit(9)	/* a submarine */
#define M_SPY		bit(10)	/* gather intelligence info */
#define M_LAND		bit(11)	/* allows full landing ability */
#define M_SUBT		bit(12)	/* allows torping of other subs */
#define M_TRADE		bit(13)	/* is a trade ship */
#define M_SEMILAND	bit(14)	/* can land 1/4 */
/* M_XLIGHT will be automatically set in init_global() if m_nxlight > 0 */
#define	M_XLIGHT	bit(15)	/* can hold xlight planes */
/* M_CHOPPER will be automatically set in init_global() if m_nchoppers > 0 */
#define M_CHOPPER	bit(16)	/* can hold choppers */
#define M_OILER		bit(17)	/* can re-fuel ships */
#define M_SUPPLY	bit(18)	/* Can supply units/sects/ships */
/* M_UNIT will be automatically set in init_global() if m_nland > 0 */
#define M_UNIT		bit(19)	/* Can carry units */
#define M_ANTIMISSILE   bit(20)	/* Shoot down missile */

#define getship(n, p) \
	ef_read(EF_SHIP, n, (caddr_t)p)
#define putship(n, p) \
	ef_write(EF_SHIP, n, (caddr_t)p)
#define getshipp(n) \
	(struct shpstr *) ef_ptr(EF_SHIP, n)

extern struct mchrstr mchr[];
extern int shp_maxno;

struct mlist {
    struct emp_qelem queue;	/* list of ships */
    struct mchrstr *mcp;	/* pointer to desc of ship */
    struct shpstr ship;		/* struct ship */
    double mobil;		/* how much mobility the ship has left */
    int done;			/* has this ship been interdicted yet? */
};

#define SHP_DEF(b, t) (t ? (b * (logx((double)t, (double)40.0) < 1.0 ? 1.0 : \
			     logx((double)t, (double)40.0))) : b)
#define SHP_SPD(b, t) (t ? (b * (logx((double)t, (double)35.0) < 1.0 ? 1.0 : \
			     logx((double)t, (double)35.0))) : b)
#define SHP_VIS(b, t) (b * (1 - (sqrt((double)t) / 50)))
#define SHP_RNG(b, t) (t ? (b * (logx((double)t, (double)35.0) < 1.0 ? 1.0 : \
			     logx((double)t, (double)35.0))) : b)
#define SHP_FIR(b, t) (t ? (b * (logx((double)t, (double)60.0) < 1.0 ? 1.0 : \
			     logx((double)t, (double)60.0))) : b)

/* Work required for building 100% */
#define SHP_BLD_WORK(lcm, hcm) (20 + (lcm) + 2 * (hcm))

 /* return codes from shp_check_nav */
#define CN_NAVIGABLE	(0)
#define CN_LANDLOCKED	(1)
#define CN_CONSTRUCTION	(2)
#define CN_ERROR	(-1)

enum {
    M_TORP_SHELLS = 3		/* number of shells used by a torpedo */
};

#endif /* _SHIP_H_ */
