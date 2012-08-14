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
 *  buy.c: Buy commodities from other nations
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Pat Loney, 1992
 *     Steve McClure, 1996-2000
 */

#include <config.h>

#include "chance.h"
#include "commands.h"
#include "commodity.h"
#include "item.h"
#include "news.h"
#include "optlist.h"
#include "trade.h"

/*
 * format: buy <COMMODITY>
 *
 */
int
buy(void)
{
    struct sctstr sect;
    struct natstr *natp;
    struct comstr comm;
    struct comstr comt;
    struct trdstr tmpt;
    struct ichrstr *ip;
    int qty;
    int o, n;
    coord x, y;
    char *p;
    float bid;
    time_t now;
    double tally;
    double canspend;
    char buf[1024];

    if (!opt_MARKET) {
	pr("The market is disabled.\n");
	return RET_FAIL;
    }
    natp = getnatp(player->cnum);
    ip = whatitem(player->argp[1], "Commodity you want to buy: ");
    if (!ip)
	return RET_SYN;
    display_mark(ip->i_uid, 0);
    pr("\n");
    p = getstarg(player->argp[2], "Which lot are you bidding on: ", buf);
    if (!p)
	return RET_SYN;
    if (*p == 0)
	return RET_SYN;
    o = atoi(p);
    if (o < 0)
	return RET_SYN;
    if (!getcomm(o, &comm) || comm.com_owner == 0) {
	pr("Invalid lot number.\n");
	return RET_OK;
    }
    if (comm.com_type != ip->i_uid) {
	pr("That lot is not of the type you specified.\n");
	return RET_OK;
    }
    if (comm.com_owner == player->cnum) {
	pr("You can't bid on your own lot.\n");
	return RET_OK;
    }
    if (!(p = getstarg(player->argp[3], "How much per unit: ", buf)))
	return RET_SYN;
    bid = atof(p);
    if (bid <= 0)
	return RET_FAIL;
    if (natp->nat_money < bid * comm.com_amount * buytax) {
	pr("This purchase would cost %.2f, %.2f more than you have.\n",
	   bid * comm.com_amount * buytax,
	   bid * comm.com_amount * buytax - natp->nat_money);
	return RET_FAIL;
    }
/*  check to see if all of the bids that this player has out plus this new bid
    would make him go broke.  Ken, I ought to skin you alive for making me code
    this part up.*/
    tally = 0.0;
    for (n = 0; gettrade(n, &tmpt); n++) {
	if (tmpt.trd_maxbidder == player->cnum &&
	    tmpt.trd_unitid >= 0 && tmpt.trd_owner != player->cnum) {
	    tally += tmpt.trd_price * tradetax;
	}
    }
    for (n = 0; getcomm(n, &comt); n++) {
	if (comt.com_maxbidder == player->cnum &&
	    comt.com_owner != 0 && comt.com_owner != player->cnum) {
	    tally += comt.com_price * comt.com_amount * buytax;
	}
    }
    canspend = natp->nat_money - tally;
    check_comm_ok(&comm);
    if (bid * comm.com_amount * buytax > canspend) {
	pr("You have overextended yourself in the market\n");
	pr("You can not bid on the current items at that price.\n");
	return RET_OK;
    }
    if (!(p = getstarg(player->argp[4], "destination sector : ", buf)))
	return RET_SYN;
    if (!sarg_xy(p, &x, &y))
	return RET_SYN;
    if (!getsect(x, y, &sect)) {
	pr("Could not access sector");
	return RET_FAIL;
    }
    if ((sect.sct_type != SCT_WAREH && sect.sct_type != SCT_HARBR) ||
	sect.sct_own != player->cnum) {
	pr("The destination sector is not one of your warehouses.\n");
	return RET_FAIL;
    }
    if (sect.sct_effic < 60) {
	pr("That sector is under construction.\n");
	return RET_FAIL;
    }
    n = sect.sct_item[ip->i_uid];
    qty = comm.com_amount;
    if (qty + n > ITEM_MAX) {
	pr("That sector cannot hold %d more %s. It currently holds %d.\n",
	   qty, ip->i_name, n);
	return RET_FAIL;
    }
    if (bid * comm.com_amount > natp->nat_money) {
	pr("You don't have that much to spend!\n");
	return RET_FAIL;
    }
    if (!check_comm_ok(&comm))
	return RET_FAIL;
    if (bid > 0.04 + comm.com_price) {
	comm.com_price = bid;
	/* Add five minutes to the time if less than 5 minutes */
	time(&now);
	if (((MARK_DELAY - (now - comm.com_markettime)) < 300) &&
	    comm.com_maxbidder != player->cnum) {
	    comm.com_markettime += 300;
	    /* Special case - what if so much time has gone by?  Well,
	       Just reset the markettime  so that only 5 minutes are left */
	    if ((MARK_DELAY - (now - comm.com_markettime)) < 0)
		comm.com_markettime = (now - (MARK_DELAY - 300));
	}
	comm.com_maxbidder = player->cnum;
	comm.com_x = x;
	comm.com_y = y;
	putcomm(o, &comm);
	pr("Your bid is being considered.\n");
    } else {
	pr("Your bid wasn't high enough (you need to bid at least $0.05 higher\n");
	pr("than the last bid.\n");
	return RET_OK;
    }

    check_market();

    return RET_OK;
}

int
check_market(void)
{
    struct comstr comm;
    struct sctstr *sect;
    struct natstr *natp;
    int m;
    int n;
    time_t now;
    double tleft;
    double gain;
    double price;

    for (n = 0; getcomm(n, &comm); n++) {
	if (comm.com_maxbidder == comm.com_owner || comm.com_owner == 0)
	    continue;
	(void)time(&now);
	tleft = MARK_DELAY / 3600.0 - (now - comm.com_markettime) / 3600.0;
	if (tleft < 0)
	    tleft = 0;
	if (tleft > 0.0)
	    continue;
	if (CANT_HAPPEN(comm.com_type <= I_NONE || comm.com_type > I_MAX))
	    continue;
	sect = getsectp(comm.com_x, comm.com_y);
	m = sect->sct_item[comm.com_type];

	price = comm.com_price * comm.com_amount * buytax;
	gain = comm.com_price * comm.com_amount;

	natp = getnatp(comm.com_maxbidder);
	if (natp->nat_money < price) {
	    nreport(comm.com_maxbidder, N_WELCH_DEAL, comm.com_owner, 1);
	    wu(0, comm.com_maxbidder,
	       "You didn't have enough cash to cover the cost.\n");
	    wu(0, comm.com_owner,
	       "Sale #%d fell through.  Goods remain on the market.\n", n);
	    comm.com_maxbidder = comm.com_owner;
	} else if (sect->sct_type != SCT_WAREH
		   && sect->sct_type != SCT_HARBR) {
	    wu(0, comm.com_maxbidder,
	       "Sector not a warehouse now, sale #%d fell though.\n", n);
	    wu(0, comm.com_owner,
	       "Sale #%d fell through.  Goods remain on the market.\n", n);
	    comm.com_maxbidder = comm.com_owner;
	} else if (m + comm.com_amount > ITEM_MAX) {
	    wu(0, comm.com_maxbidder,
	       "Warehouse full,  sale #%d fell though.\n", n);
	    wu(0, comm.com_owner,
	       "Sale #%d fell through.  Goods remain on the market.\n", n);
	    comm.com_maxbidder = comm.com_owner;
	} else {
	    sect->sct_item[comm.com_type] = m + comm.com_amount;
	    putsect(sect);
	    nreport(comm.com_owner, N_MAKE_SALE, comm.com_maxbidder, 1);
	    wu(0, comm.com_owner, "%s bought %d %s from you for $%.2f\n",
	       cname(comm.com_maxbidder), comm.com_amount,
	       ichr[comm.com_type].i_name, gain);
	    wu(0, comm.com_maxbidder,
	       "You just bought %d %s from %s for $%.2f\n",
	       comm.com_amount, ichr[comm.com_type].i_name,
	       cname(comm.com_owner), price);
	    natp->nat_money -= roundavg(price);
	    putnat(natp);
	    natp = getnatp(comm.com_owner);
	    natp->nat_money += roundavg(gain);
	    putnat(natp);
	    comm.com_owner = 0;
	}
	comm.com_owner = 0;
	putcomm(n, &comm);
    }
    return RET_OK;
}
