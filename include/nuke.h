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
 *  nuke.h: Definitions for things having to do with nukes
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#ifndef NUKE_H
#define NUKE_H

#include <time.h>
#include "types.h"

#define	N_MAXNUKE	20
#define	MIN_DRNUKE_CONST	0.001

struct nukstr {
    /* initial part must match struct empobj */
    short ef_type;
    short nuk_uid;
    unsigned nuk_seqno;
    time_t nuk_timestamp;	/* Last time this nuke was touched */
    natid nuk_own;
    coord nuk_x, nuk_y;		/* current loc of device */
    signed char nuk_type;	/* index in nchr[] */
    signed char nuk_effic;	/* unused, always 100% */
    signed char nuk_mobil;	/* unused, always 0 */
    unsigned char nuk_off;	/* repairs stopped? (unused) */
    short nuk_tech;		/* nuke's tech level */
    char nuk_stockpile;		/* group membership, unused */
    coord nuk_opx, nuk_opy;	/* Op sector coords, unused */
    short nuk_mission;		/* mission code, unused */
    short nuk_radius;		/* mission radius, unused */
    /* end of part matching struct empobj */
    short nuk_plane;		/* currently aboard plane */
};

struct nchrstr {
    char *n_name;		/* warhead unit name */
    int n_lcm;			/* costs to build */
    int n_hcm;
    int n_oil;
    int n_rad;
    int n_blast;		/* blast radius */
    int n_dam;			/* damage at center */
    int n_cost;
    int n_tech;			/* tech needed to build */
    int n_weight;
    int n_flags;		/* description of capability */
    signed char n_type;		/* index in nchr[] */
};

#define	N_NEUT	bit(0)		/* Neutron bomb (low damage, high fallout) */

#define getnuke(n, p) ef_read(EF_NUKE, (n), (p))
#define putnuke(n, p) ef_write(EF_NUKE, (n), (p))
#define getnukep(n) ((struct nukstr *)ef_ptr(EF_NUKE, (n)))

/* Work required for building */
#define NUK_BLD_WORK(lcm, hcm, oil, rad) \
  (((lcm) + 2 * (hcm) + (oil) + (rad) + 4) / 5)

extern struct nchrstr nchr[N_MAXNUKE + 1];

/* src/lib/common/cargo.c */
extern void nuk_carrier_change(struct nukstr *, int, int, int);
extern int nuk_on_plane(struct plnstr *);

#endif
