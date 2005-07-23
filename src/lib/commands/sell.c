/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sell.c: Sell commodities to other nations.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Jeff Bailey
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#include "misc.h"
#include "xy.h"
#include "file.h"
#include "sect.h"
#include "item.h"
#include "nsc.h"
#include "nat.h"
#include "plane.h"
#include "ship.h"
#include <math.h>		/* bailey@math-cs.kent.edu */
#include "commodity.h"
#include "land.h"
#include "player.h"
#include "commands.h"
#include "optlist.h"
/*#define EF_COMM 10*/

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
    int number_sub;
    int totalcom;
    int amt;
    int com;
    char *p;
    float price;
    time_t now;
    int ii = 0;
    coord x, y;
    s_char buf[1024];

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
    number_sub = 0;
    if ((p = getstarg(player->argp[3], "Quantity: ", buf)) == 0 || *p == 0)
	return RET_SYN;
    if (!check_sect_ok(&sect))
	return RET_FAIL;
    number_set = atoi(p);
    if ((p = getstarg(player->argp[4], "Price per unit: ", buf)) == 0 ||
	*p == 0)
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
    if ((amt = sect.sct_item[ip->i_vtype]) == 0) {
	pr("You don't have any %s to sell there.\n", ip->i_name);
	return RET_FAIL;
    }
    if (number_set >= 0)
	com = min(number_set, amt);
    else
	com = amt + number_set;
    if (com <= 0)
	return RET_SYN;
    totalcom += com;
    amt -= com;
    pr("Sold %d %s at %s (%d left)\n", com, ip->i_name,
       xyas(sect.sct_x, sect.sct_y, player->cnum), amt);
    sect.sct_item[ip->i_vtype] = amt;
    putsect(&sect);
    if (totalcom > 0) {
	for (ii = 0; getcomm(ii, &comm); ii++) {
	    if (comm.com_owner == 0)
		break;
	}
	if (getcomm(ii, &comm) == 0)
	    ef_extend(EF_COMM, 1);
	(void)time(&now);
	comm.com_type = ip->i_vtype;
	comm.com_owner = player->cnum;
	comm.com_price = price;
	comm.com_maxbidder = player->cnum;
	comm.com_markettime = now;
	comm.com_amount = totalcom;
	comm.com_x = 0;
	comm.com_y = 0;
	comm.sell_x = sect.sct_x;
	comm.sell_y = sect.sct_y;
	comm.com_uid = ii;
	if (!putcomm(ii, &comm)) {
	    pr("Problems with the commodities file, call the Deity\n");
	    return RET_FAIL;
	}
    } else {
	pr("No eligible %s for sale\n", ip->i_name);
	return RET_FAIL;
    }
    return RET_OK;
}
