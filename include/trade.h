/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  trade.h: Definitions for trading units/ships/planes in a time delay
 *           system
 * 
 *  Known contributors to this file:
 *     Dave Pare
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#ifndef _TRADE_H_
#define _TRADE_H_

#include "genitem.h"
#include "nuke.h"
#include "plane.h"
#include "land.h"
#include "ship.h"

struct trdstr {
    short ef_type;
    natid trd_owner;
    short trd_uid;
    char trd_type;
    short trd_unitid;
    long trd_price;
    int trd_maxbidder;
    time_t trd_markettime;
    coord trd_x;
    coord trd_y;
};

struct genstr {
    short trg_ef_type;
    natid trg_own;
    short trg_uid;
    coord trg_x;
    coord trg_y;
    s_char trg_type;
};

union trdgenstr {
    struct genstr gen;
    struct nukstr nuk;
    struct plnstr pln;
    struct lndstr lnd;
    struct shpstr shp;
};

extern int trade_check_ok(struct trdstr *, union trdgenstr *);
extern s_char *trade_nameof(struct trdstr *, union trdgenstr *);
extern int trade_desc(struct trdstr *, union trdgenstr *);
extern int trade_getitem(struct trdstr *, union trdgenstr *);
extern long get_couval(int);
extern long get_outstand(int);
extern double loan_owed(struct lonstr *loan, time_t paytime);

#define gettrade(n, p) \
	ef_read(EF_TRADE, n, p)
#define puttrade(n, p) \
	ef_write(EF_TRADE, n, p)
#define gettradep(n) \
	(struct trdstr *) ef_ptr(EF_TRADE, n)

#endif /* _TRADE_H_ */
