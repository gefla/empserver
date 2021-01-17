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
 *  nuke.h: Definitions for things having to do with nukes
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Markus Armbruster, 2004-2020
 */

#ifndef NUKE_H
#define NUKE_H

#include "file.h"
#include "item.h"
#include "types.h"

#define MIN_DRNUKE_CONST	0.001

struct nukstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned nuk_seqno: 12;
    unsigned nuk_generation: 12;
    int nuk_uid;		/* unit ID (nuke #) */
    time_t nuk_timestamp;	/* Last time this nuke was touched */
    natid nuk_own;
    coord nuk_x, nuk_y;		/* current loc of device */
    signed char nuk_type;	/* index in nchr[] */
    signed char nuk_effic;	/* unused, always 100% */
    signed char nuk_mobil;	/* unused, always 0 */
    unsigned char nuk_off;	/* repairs stopped? (unused) */
    short nuk_tech;		/* nuke's tech level */
    char nuk_stockpile;		/* group membership */
    coord nuk_opx, nuk_opy;	/* Op sector coords, unused */
    short nuk_mission;		/* mission code, unused */
    short nuk_radius;		/* mission radius, unused */
    /* end of part matching struct empobj */
    int nuk_plane;		/* UID of transporting plane, or -1 */
};

struct nchrstr {
    char *n_name;		/* warhead unit name */
    int n_blast;		/* blast radius */
    int n_dam;			/* damage at center */
    short n_mat[I_MAX+1];	/* materials to build 100% */
				/* only I_LCM, I_HCM, I_OIL, I_RAD non-zero */
    int n_bwork;		/* work to build 100% */
    int n_tech;			/* tech needed to build */
    int n_cost;			/* how much it costs to build */
    int n_weight;
    int n_flags;		/* description of capability */
    signed char n_type;		/* index in nchr[] */
};

#define N_NEUT	bit(0)		/* Neutron bomb (low damage, high fallout) */

#define getnuke(n, p) ef_read(EF_NUKE, (n), (p))
#define putnuke(n, p) ef_write(EF_NUKE, (n), (p))
#define getnukep(n) ((struct nukstr *)ef_ptr(EF_NUKE, (n)))

extern struct nchrstr nchr[64];

/* src/lib/common/cargo.c */
extern void nuk_carrier_change(struct nukstr *, int, int, int);
extern int nuk_on_plane(struct plnstr *);

#endif
