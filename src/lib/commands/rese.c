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
 *  rese.c: Reset (lower) commodity prices
 *
 *  Known contributors to this file:
 *     Jeff Bailey
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#include <config.h>

#include "commands.h"
#include "commodity.h"
#include "item.h"
#include "land.h"
#include "optlist.h"
#include "plane.h"
#include "ship.h"

int
rese(void)
{
    struct comstr comm;
    struct sctstr sect;
    int number_set;
    int m;
    char *p;
    float price;
    time_t now;
    char buf[1024];

    if (!opt_MARKET) {
	pr("The market is disabled.\n");
	return RET_FAIL;
    }
    check_market();
    check_trade();
    if ((p = getstarg(player->argp[1], "Which lot :  ", buf)) == 0
	|| *p == 0)
	return RET_SYN;
    number_set = atoi(p);
    getcomm(number_set, &comm);
    if (comm.com_owner != player->cnum && !player->god) {
	pr("That's not your lot.\n");
	return RET_OK;
    }
    if (comm.com_maxbidder != player->cnum && !player->god) {
	pr("Some one already has a bid out on that\n");
	return RET_OK;
    }
    if ((p = getstarg(player->argp[2], "New (lower) price: ", buf)) == 0
	|| *p == 0)
	return RET_SYN;
    if (!check_comm_ok(&comm))
	return RET_FAIL;
    price = (float)atof(p);
    if (price <= 0) {
	getsect(comm.sell_x, comm.sell_y, &sect);
	if (!player->owner && !player->god) {
	    pr("The destination sector must be one of yours\n");
	    return RET_OK;
	}
	if (player->god) {
	    if ((p = getstring("Really destroy that lot? ", buf)) == 0)
		return RET_FAIL;
	    if (!check_comm_ok(&comm))
		return RET_FAIL;
	    if (*p == 'Y' || *p == 'y') {
		comm.com_owner = 0;
		putcomm(number_set, &comm);
		pr("Goods destroyed.\n");
		return RET_OK;
	    }
	    return RET_FAIL;
	}
	if (sect.sct_type != SCT_HARBR && sect.sct_type != SCT_WAREH) {
	    pr("That sector is not available for receiving goods.\n");
	    return RET_OK;
	}
	if (sect.sct_effic < 60) {
	    pr("The destination sector must be at least 60%% efficient.\n");
	    return RET_OK;
	}
	if (CANT_HAPPEN(comm.com_type <= I_NONE || comm.com_type > I_MAX)) {
	    pr("The goods have been eaten by a grue.");
	    return RET_OK;
	}
	m = sect.sct_item[comm.com_type];
	m = m + comm.com_amount;
	if (m > ITEM_MAX)
	    m = ITEM_MAX;
	sect.sct_item[comm.com_type] = m;
	putsect(&sect);
	comm.com_owner = 0;
	putcomm(number_set, &comm);
	pr("The goods have been returned to your trading post.\n");
	return RET_OK;
    }
    if (price >= comm.com_price) {
	pr("You can only lower the price.\n");
	return RET_OK;
    }
    if (price < 0) {
	pr("New price must be greater than or equal to zero.\n");
	return RET_OK;
    }
    comm.com_price = price;
    (void)time(&now);
    comm.com_markettime = now;
    if (!putcomm(number_set, &comm)) {
	pr("Problems with the commodities file, Call the Deity\n");
	return RET_OK;
    }
    return RET_OK;
}
