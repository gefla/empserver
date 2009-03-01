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
 *  ship.h: Definitions for things having to do with ships
 *
 *  Known contributors to this file:
 *     Dave Pare
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2008
 */

#ifndef SHIP_H
#define SHIP_H

#include <time.h>
#include "item.h"
#include "queue.h"
#include "retreat.h"
#include "types.h"

#define SHP_TYPE_MAX	46
#define	SHIP_MINEFF	20

/* bit masks for the autonav mode flags */

#define AN_AUTONAV bit(2)
#define AN_STANDBY bit(3)
#define AN_LOADING bit(4)
#define AN_SCUTTLE bit(5)	/* Auto-scuttle of trade ships */

/* TMAX is the number of cargo holds a ship use in the autonav code. */
#define TMAX 6


#define MAXSHPPATH	28
#define	MAXSHPNAMLEN	24

struct shpstr {
    /* initial part must match struct empobj */
    short ef_type;
    short shp_uid;		/* unit id (ship #) */
    unsigned shp_seqno;
    unsigned shp_generation;
    time_t shp_timestamp;	/* Last time this ship was touched. */
    natid shp_own;		/* owner's country num */
    coord shp_x;		/* x location in abs coords */
    coord shp_y;		/* y location in abs coords */
    signed char shp_type;	/* index in mchr[] */
    signed char shp_effic;	/* 0% to 100% */
    signed char shp_mobil;	/* mobility units */
    unsigned char shp_off;	/* repairs stopped? */
    short shp_tech;		/* tech level ship was built at */
    char shp_fleet;		/* group membership */
    coord shp_opx, shp_opy;	/* Op sector coords */
    short shp_mission;		/* mission code */
    short shp_radius;		/* mission radius */
    /* end of part matching struct empobj */
    coord shp_destx[2];		/* location for ship destination */
    coord shp_desty[2];
    i_type shp_tstart[TMAX];	/* what goods to pick up at start point */
    i_type shp_tend[TMAX];	/* what goods to pick up at end point   */
    short shp_lstart[TMAX];	/* How much do we pick up at the start  */
    short shp_lend[TMAX];	/* How much do we pick up at the end    */
    unsigned char shp_autonav;	/* autonavigation flags */
    short shp_item[I_MAX+1];	/* amount of items on board */
    short shp_pstage;		/* plague stage */
    short shp_ptime;		/* how many etus remain in this stage */
    short shp_access;		/* Last tick mob was updated (MOB_ACCESS) */
    unsigned char shp_mobquota;	/* mobility quota */
    char shp_path[MAXSHPPATH];
    short shp_follow;
    char shp_name[MAXSHPNAMLEN]; /* name set with the "name" command */
    coord shp_orig_x;
    coord shp_orig_y;		/* Where we were built */
    natid shp_orig_own;		/* Who built us */
    int shp_rflags;		/* When do I retreat? */
    char shp_rpath[RET_LEN];	/* retreat path */
};

struct mchrstr {
    short m_item[I_MAX+1];	/* load limit */
    int m_lcm;			/* units of lcm to build */
    int m_hcm;			/* units of hcm to build */
    int m_armor;		/* how well armored it is */
    int m_speed;		/* how fast it can go */
    int m_visib;		/* how well it can be seen */
    int m_vrnge;		/* how well it can see */
    int m_frnge;		/* how far it can fire */
    int m_glim;			/* how many guns it can fire */
    unsigned char m_nxlight;	/* maximum number of xlight planes */
    unsigned char m_nchoppers;	/* maximum number of choppers */
    char *m_name;		/* full name of type of ship */
    int m_tech;			/* tech required to build */
    int m_cost;			/* how much it costs to build */
    long m_flags;		/* what special things can this ship do */
    unsigned char m_nplanes;	/* maximum number of planes this ship holds */
    unsigned char m_nland;	/* maximum number of units this ship holds */
    signed char m_type;		/* index in mchr[] */
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
/* unused		bit(10)	*/
#define M_LAND		bit(11)	/* allows full landing ability */
#define M_SUBT		bit(12)	/* allows torping of other subs */
#define M_TRADE		bit(13)	/* is a trade ship */
#define M_SEMILAND	bit(14)	/* can land 1/4 */
/* unused		bit(15)	*/
/* unused		bit(16)	*/
/* unused		bit(17)	*/
#define M_SUPPLY	bit(18)	/* Can supply units/sects/ships */
#define M_CANAL		bit(19) /* Can navigate a canal (BIG CITY) */
#define M_ANTIMISSILE   bit(20)	/* Shoot down missile */

#define getship(n, p) ef_read(EF_SHIP, (n), (p))
#define putship(n, p) ef_write(EF_SHIP, (n), (p))
#define getshipp(n) ((struct shpstr *)ef_ptr(EF_SHIP, (n)))

extern struct mchrstr mchr[SHP_TYPE_MAX + 2];

/* Work required for building 100% */
#define SHP_BLD_WORK(lcm, hcm) (20 + (lcm) + 2 * (hcm))

 /* return codes from shp_check_nav */
#define CN_NAVIGABLE	0
#define CN_LANDLOCKED	1
#define CN_CONSTRUCTION	2
#define CN_ERROR	-1

enum {
    SHP_AIROPS_EFF = 50,	/* min. efficiency for air ops */
    SHP_TORP_SHELLS = 3		/* number of shells used by a torpedo */
};

extern int m_armor(struct mchrstr *, int);
extern int m_speed(struct mchrstr *, int);
extern int m_visib(struct mchrstr *, int);
extern int m_frnge(struct mchrstr *, int);
extern int m_glim(struct mchrstr *, int);
extern int shp_armor(struct shpstr *);
extern int shp_speed(struct shpstr *);
extern int shp_visib(struct shpstr *);
extern int shp_frnge(struct shpstr *);
extern int shp_glim(struct shpstr *);

extern int shp_nplane(struct shpstr *, int *, int *, int *);
extern int shp_nland(struct shpstr *);

extern int shp_dchrg(struct shpstr *);
extern int shp_fire(struct shpstr *);
extern int shp_torp(struct shpstr *, int);
extern double shp_fire_range(struct shpstr *);
extern int shp_usable_guns(struct shpstr *);
extern double shp_torp_hitchance(struct shpstr *, int);

#endif
