/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  commodity.h: Definitions for buy/market/sell commands in a time delay
 *               market
 *
 *  Known contributors to this file:
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#ifndef COMMODITY_H
#define COMMODITY_H

#include <time.h>
#include "types.h"
#include "item.h"

struct comstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned com_seqno: 12;
    unsigned com_generation: 12;
    int com_uid;
    time_t com_timestamp;
    natid com_owner;
    /* end of part matching struct empobj */
    i_type com_type;
    int com_amount;
    float com_price;
    int com_maxbidder;
    time_t com_markettime;
    coord com_x;
    coord com_y;
    coord sell_x;
    coord sell_y;
};

#define getcomm(n, p) ef_read(EF_COMM, (n), (p))
#define putcomm(n, p) ef_write(EF_COMM, (n), (p))

#endif
