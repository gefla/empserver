/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#define	N_MAXNUKE	20
#define	MIN_DRNUKE_CONST	0.001

struct nukstr {
    /* initial part must match struct genitem */
    short ef_type;
    natid nuk_own;
    short nuk_uid;
    coord nuk_x;		/* current loc of device */
    coord nuk_y;
    /* end of part matching struct genitem */
    signed char nuk_n;		/* number of nukes in list */
    short nuk_ship;		/* currently aboard ship (unused) */
    short nuk_trade;		/* index into trade file (unused) */
    short nuk_types[N_MAXNUKE];	/* # of nukes in sector of given type */
    time_t nuk_timestamp;	/* Last time this nuke was touched */
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
#define getnukep(n) (struct nukstr *)ef_ptr(EF_NUKE, (n))

/* Work required for building */
#define NUK_BLD_WORK(lcm, hcm, oil, rad) \
  (((lcm) + 2 * (hcm) + (oil) + (rad) + 4) / 5)

extern struct nchrstr nchr[N_MAXNUKE + 1];

#endif
