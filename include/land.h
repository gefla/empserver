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
 *  land.h: Definitions for land units
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2008
 */

#ifndef LAND_H
#define LAND_H

#include <time.h>
#include "item.h"
#include "queue.h"
#include "retreat.h"
#include "types.h"

#define LND_TYPE_MAX	30
#define LAND_MINEFF	10
#define LAND_MINFIREEFF 40	/* arty must be this effic to fire */

struct lndstr {
    /* initial part must match struct empobj */
    short ef_type;
    short lnd_uid;		/* unit id (land unit) */
    unsigned lnd_seqno;
    time_t lnd_timestamp;	/* Last time this unit was touched */
    natid lnd_own;		/* owner's country num */
    coord lnd_x;		/* x location in abs coords */
    coord lnd_y;		/* y location in abs coords */
    signed char lnd_type;	/* index in lchr[] */
    signed char lnd_effic;	/* 0% to 100% */
    signed char lnd_mobil;	/* mobility units */
    unsigned char lnd_off;	/* repairs stopped? */
    short lnd_tech;		/* tech level ship was built at */
    char lnd_army;		/* group membership */
    coord lnd_opx, lnd_opy;	/* Op sector coords */
    short lnd_mission;		/* mission code */
    short lnd_radius;		/* mission radius */
    /* end of part matching struct empobj */
    signed char lnd_flags;	/* unit flags (unused) */
    short lnd_ship;		/* pointer to transporting ship */
    signed char lnd_harden;	/* fortification */
    short lnd_retreat;		/* retreat percentage */
    int lnd_rflags;		/* When do I retreat? */
    char lnd_rpath[RET_LEN];	/* retreat path */
    unsigned char lnd_scar;	/* how experienced the unit is (not used) */
    short lnd_item[I_MAX+1];	/* amount of items on board */
    short lnd_pstage;		/* plague stage */
    short lnd_ptime;		/* how many etus remain in this stage */
    short lnd_land;		/* pointer to transporting unit */
    short lnd_access;		/* Last tick mob was updated (MOB_ACCESS) */
};

struct lchrstr {
    short l_item[I_MAX+1];	/* load limit */
    char *l_name;		/* full name of type of land unit */
    int l_lcm;			/* units of lcm to build */
    int l_hcm;			/* units of hcm to build */
    int l_mil;			/* how many mil it takes to build (unused) */
    int l_gun;			/* how many guns it takes to build (unused) */
    int l_shell;		/* #shells it takes to build (unused) */
    int l_tech;			/* tech required to build */
    int l_cost;			/* how much it costs to build */
    float l_att;		/* attack multiplier */
    float l_def;		/* defense multiplier */
    int l_vul;			/* vulnerability (0-100) */
    int l_spd;			/* speed */
    int l_vis;			/* visibility */
    int l_spy;			/* Seeing distance */
    int l_rad;			/* reaction radius */
    int l_frg;			/* firing range */
    int l_acc;			/* firing accuracy */
    int l_dam;			/* # of guns firing */
    int l_ammo;			/* firing ammu used per shot */
    int l_aaf;			/* aa fire */
    unsigned char l_nxlight;	/* maximum number of xlight planes */
    unsigned char l_nland;	/* maximum number of units */
    signed char l_type;		/* index in lchr[] */
    long l_flags;		/* what special things can this unit do */
};

/* Land unit ability flags */
#define	L_ENGINEER	bit(1)	/* Do engineering things */
#define	L_SUPPLY	bit(2)	/* supply other units/sects */
#define	L_SECURITY	bit(3)	/* anti-terrorist troops */
#define	L_LIGHT		bit(4)	/* can go on ships */
#define	L_MARINE	bit(5)	/* marine units, good at assaulting */
#define	L_RECON		bit(6)	/* recon units, good at spying */
#define L_RADAR		bit(7)	/* radar unit */
#define L_ASSAULT	bit(8)	/* can assault */
#define L_FLAK		bit(9)	/* flak unit */
#define L_SPY           bit(10)	/* spy unit - way cool */
#define L_TRAIN         bit(11)	/* train unit - neato */
#define L_HEAVY         bit(12)	/* heavy unit - can't go on trains */

/* Work required for building 100% */
#define LND_BLD_WORK(lcm, hcm) (20 + (lcm) + 2 * (hcm))

/* Chance to detect L_SPY unit (percent) */
#define LND_SPY_DETECT_CHANCE(eff) ((110-(eff))/100.0)

#define getland(n, p) ef_read(EF_LAND, (n), (p))
#define putland(n, p) ef_write(EF_LAND, (n), (p))
#define getlandp(n) ((struct lndstr *)ef_ptr(EF_LAND, (n)))

extern struct lchrstr lchr[LND_TYPE_MAX + 2];

enum {
    LND_AIROPS_EFF = 50		/* min. efficiency for air ops */
};

extern float l_att(struct lchrstr *, int);
extern float l_def(struct lchrstr *, int);
extern int l_vul(struct lchrstr *, int);
extern int l_spd(struct lchrstr *, int);
extern int l_frg(struct lchrstr *, int);
extern int l_acc(struct lchrstr *, int);
extern int l_dam(struct lchrstr *, int);
extern int l_aaf(struct lchrstr *, int);
extern float lnd_att(struct lndstr *);
extern float lnd_def(struct lndstr *);
extern int lnd_vul(struct lndstr *);
extern int lnd_spd(struct lndstr *);
extern int lnd_vis(struct lndstr *);
extern int lnd_frg(struct lndstr *);
extern int lnd_acc(struct lndstr *);
extern int lnd_dam(struct lndstr *);
extern int lnd_aaf(struct lndstr *);

/* src/lib/common/cargo.c */
extern void lnd_carrier_change(struct lndstr *, int, int, int);
extern int lnd_first_on_ship(struct shpstr *);
extern int lnd_first_on_land(struct lndstr *);
extern int lnd_next_on_unit(int);
extern int lnd_nxlight(struct lndstr *);
extern int lnd_nland(struct lndstr *);

extern int lnd_fire(struct lndstr *);
extern double lnd_fire_range(struct lndstr *);

/* src/lib/subs/lndsub.c */
extern void lnd_sweep(struct emp_qelem *, int, int, natid);
extern int lnd_interdict(struct emp_qelem *, coord, coord, natid);
extern void lnd_sel(struct nstr_item *, struct emp_qelem *);
extern int lnd_check_mines(struct emp_qelem *);
extern double lnd_pathcost(struct lndstr *, double);
extern int lnd_mobtype(struct lndstr *);
extern double lnd_mobcost(struct lndstr *, struct sctstr *);

extern double attack_val(int, struct lndstr *);
extern double defense_val(struct lndstr *);
extern int lnd_reaction_range(struct lndstr *);
extern void lnd_print(struct ulist *, char *);
extern void lnd_delete(struct ulist *, char *);
extern int lnd_take_casualty(int, struct ulist *, int);
extern void lnd_submil(struct lndstr *, int);
extern void lnd_takemob(struct emp_qelem *, double);
extern int lnd_spyval(struct lndstr *);
extern void intelligence_report(int, struct lndstr *, int, char *);
extern void lnd_mar(struct emp_qelem *, double *, double *, int *, natid);
extern int lnd_hardtarget(struct lndstr *);
extern int lnd_mar_one_sector(struct emp_qelem *, int, natid, int);
extern int lnd_support(natid, natid, coord, coord, int);
extern int lnd_can_attack(struct lndstr *);
extern int lnd_fortify (struct lndstr *lp, int hard_amt);
extern void lnd_set_tech(struct lndstr *, int);

#endif
