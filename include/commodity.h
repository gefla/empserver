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
 *  commodity.h: Definitions for buy/market/sell commands in a time delay
 *               market
 * 
 *  Known contributors to this file:
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#ifndef _COMMODITY_H_
#define _COMMODITY_H_

struct comstr {
    short ef_type;
    natid com_owner;
    short com_uid;
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

#define getcomm(n, p) \
	ef_read(EF_COMM, n, p)
#define putcomm(n, p) \
	ef_write(EF_COMM, n, p)

#endif /* _COMMODITY_H_ */
