/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  trade.h: Definitions for trading units/ships/planes in a time delay
 *           system
 *
 *  Known contributors to this file:
 *     Dave Pare
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#ifndef TRADE_H
#define TRADE_H

#include <time.h>
#include "types.h"

struct trdstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned trd_seqno: 12;
    unsigned trd_generation: 12;
    int trd_uid;
    time_t trd_timestamp;
    natid trd_owner;
    /* end of part matching struct empobj */
    short trd_type;
    int trd_unitid;
    int trd_price;
    int trd_maxbidder;
    time_t trd_markettime;
    coord trd_x;		/* destination for teleporting trade */
    coord trd_y;
};

extern int trade_check_ok(struct trdstr *, struct empobj *);
extern char *trade_nameof(struct trdstr *, struct empobj *);
extern int trade_desc(struct empobj *);
extern int trade_has_unsalable_cargo(struct empobj *, int);
extern int trade_getitem(struct trdstr *, union empobj_storage *);

#define gettrade(n, p) ef_read(EF_TRADE, (n), (p))
#define puttrade(n, p) ef_write(EF_TRADE, (n), (p))
#define gettradep(n) ((struct trdstr *)ef_ptr(EF_TRADE, (n)))

#endif
