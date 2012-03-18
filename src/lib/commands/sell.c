/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sell.c: Sell commodities to other nations.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Jeff Bailey
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#include <config.h>

#include "commands.h"
#include "commodity.h"
#include "item.h"
#include "optlist.h"

/*
 * format: sell <COMMODITY> <SECTS> <NUMBER> <PRICE>
 *   where NUMBER represents either the number to reach
 *   or, if negative, the abs number to try and get from
 *   each sector.
 */
int
sell(void)
{
    struct sctstr sect;
    struct ichrstr *ip;
    struct comstr comm;
    int number_set;
    int totalcom;
    int amt;
    int com;
    char *p;
    float price;
    time_t now;
    int ii = 0;
    coord x, y;
    char buf[1024];

    if (!opt_MARKET) {
	pr("The market is disabled.\n");
	return RET_FAIL;
    }
    check_market();
    check_trade();
    if (!(ip = whatitem(player->argp[1], "Commodity you want to sell: ")))
	return RET_SYN;
    if (ip->i_sell == 0) {
	pr("You can't sell %s\n", ip->i_name);
	return RET_FAIL;
    }
    if (!(p = getstarg(player->argp[2], "Sector to sell from: ", buf)))
	return RET_SYN;
    if (!sarg_xy(p, &x, &y))
	return RET_SYN;
    if (!getsect(x, y, &sect))
	pr("Could not access that sector.\n");
    if ((sect.sct_type != SCT_HARBR && sect.sct_type != SCT_WAREH) ||
	!player->owner) {
	pr("That sector cannot sell goods.\n");
	return RET_FAIL;
    }
    if (sect.sct_effic < 60) {
	pr("Sectors need to be >= 60%% efficient to sell goods.\n");
	return RET_FAIL;
    }
    if (sect.sct_mobil <= 0) {
	pr("Sectors need at least 1 mobility to sell goods.\n");
	return RET_FAIL;
    }
    p = getstarg(player->argp[3], "Quantity: ", buf);
    if (!p || !*p)
	return RET_SYN;
    if (!check_sect_ok(&sect))
	return RET_FAIL;
    number_set = atoi(p);
    p = getstarg(player->argp[4], "Price per unit: ", buf);
    if (!p || !*p)
	return RET_SYN;
    if (!check_sect_ok(&sect))
	return RET_FAIL;
    price = atof(p);
    if (price <= 0.0) {
	pr("No sale.\n");
	return RET_FAIL;
    }
    if (price > 1000.0)		/* Inf can cause overflow */
	price = 1000.0;		/* bailey@math-cs.kent.edu */
    totalcom = 0;
    if (!military_control(&sect)) {
	pr("Military control required to sell goods.\n");
	return RET_FAIL;
    }
    if ((amt = sect.sct_item[ip->i_uid]) == 0) {
	pr("You don't have any %s to sell there.\n", ip->i_name);
	return RET_FAIL;
    }
    if (number_set >= 0)
	com = MIN(number_set, amt);
    else
	com = amt + number_set;
    if (com <= 0)
	return RET_SYN;
    totalcom += com;
    amt -= com;
    pr("Sold %d %s at %s (%d left)\n", com, ip->i_name,
       xyas(sect.sct_x, sect.sct_y, player->cnum), amt);
    sect.sct_item[ip->i_uid] = amt;
    putsect(&sect);

    for (ii = 0; getcomm(ii, &comm); ii++) {
	if (comm.com_owner == 0)
	    break;
    }
    (void)time(&now);
    ef_blank(EF_COMM, ii, &comm);
    comm.com_type = ip->i_uid;
    comm.com_owner = player->cnum;
    comm.com_price = price;
    comm.com_maxbidder = player->cnum;
    comm.com_markettime = now;
    comm.com_amount = totalcom;
    comm.com_x = 0;
    comm.com_y = 0;
    comm.sell_x = sect.sct_x;
    comm.sell_y = sect.sct_y;
    if (!putcomm(ii, &comm)) {
	pr("Problems with the commodities file, call the Deity\n");
	return RET_FAIL;
    }
    return RET_OK;
}
