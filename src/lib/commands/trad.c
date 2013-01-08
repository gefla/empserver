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
 *  trad.c: Buy units/ships/planes/nukes from other nations.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Pat Loney, 1992
 *     Steve McClure, 1996-2000
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "commodity.h"
#include "empobj.h"
#include "land.h"
#include "loan.h"
#include "news.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"
#include "ship.h"
#include "trade.h"
#include "unit.h"

/*
 * format: trade
 */
int
trad(void)
{
    struct sctstr sect;
    struct natstr *natp;
    struct comstr comt;
    int lotno;
    float price;
    coord sx, sy;
    int n;
    char *p;
    struct nstr_item ni;
    struct trdstr trade;
    struct trdstr tmpt;
    union empobj_storage tg;
    double canspend;
    time_t now;
    int bid;
    double tleft;
    double tally;
    int i;
    char buf[1024];

    if (!opt_MARKET) {
	pr("The market is disabled.\n");
	return RET_FAIL;
    }
    /* First, we execute all trades, so that we can only buy what is available. */
    check_market();
    check_trade();

    pr("\n     Empire Trade Report\n  ");
    prdate();
    n = 0;
    pr(" lot high bid  by time left owner  description\n");
    pr(" --- --------  -- --------- -----  -------------------------\n");

    snxtitem_all(&ni, EF_TRADE);
    while (nxtitem(&ni, &trade)) {
	if (trade.trd_owner == 0)
	    continue;
	if (!trade_getitem(&trade, &tg)) {
	    continue;
	};
	pr(" %3d ", ni.cur);
	(void)time(&now);
	tleft =
	    TRADE_DELAY / 3600.0 - (now - trade.trd_markettime) / 3600.0;
	if (tleft < 0.0)
	    tleft = 0.0;
	pr("$%7ld  %2d %5.2f hrs ",
	   trade.trd_price, trade.trd_maxbidder, tleft);
	trade_desc(&tg.gen);	/* XXX */
	pr("\n");
	if (trade.trd_owner == player->cnum && !player->god)
	    pr(" (your own lot)\n");
	n++;
    }
    if (n == 0) {
	pr("Nothing to buy at the moment...\n");
	return RET_OK;
    }
    p = getstring("Which lot to buy: ", buf);
    if (!p || !*p)
	return RET_OK;
    if (isdigit(*p) == 0)
	return RET_OK;
    lotno = atoi(p);
    if (lotno < 0 || lotno >= ni.cur) {
	pr("Bad lot number\n");
	return RET_OK;
    }
    if (!gettrade(lotno, &trade)) {
	pr("No such lot number\n");
	return RET_OK;
    }
    if (trade.trd_unitid < 0) {
	pr("Invalid lot number.\n");
	return RET_OK;
    }
    if (!trade_getitem(&trade, &tg)) {
	pr("Can't find trade #%d!\n", trade.trd_unitid);
	trade.trd_owner = 0;
	trade.trd_unitid = -1;
	if (!puttrade(lotno, &trade)) {
	    logerror("trad: can't write trade");
	    pr("Couldn't save after getitem failed; get help!\n");
	    return RET_FAIL;
	}
	return RET_OK;
    }
    switch (trade.trd_type) {
    case EF_NUKE:
    case EF_PLANE:
    case EF_SHIP:
    case EF_LAND:
	break;
    default:
	pr("Bad unit type on lot number %d\n", lotno);
	return RET_FAIL;
    }
    if (trade.trd_owner == player->cnum) {
	pr("You can't buy from yourself!\n");
	return RET_OK;
    }
    price = trade.trd_price;
    natp = getnatp(player->cnum);
    if (natp->nat_money < price) {
	pr("You don't have %.2f to spend!\n", price);
	return RET_OK;
    }
    tally = 0.0;
    for (i = 0; gettrade(i, &tmpt); i++) {
	if (tmpt.trd_maxbidder == player->cnum &&
	    tmpt.trd_unitid >= 0 && tmpt.trd_owner != player->cnum) {
	    tally += tmpt.trd_price * tradetax;
	}
    }
    for (i = 0; getcomm(i, &comt); i++) {
	if (comt.com_maxbidder == player->cnum &&
	    comt.com_owner != 0 && comt.com_owner != player->cnum) {
	    tally += (comt.com_price * comt.com_amount) * buytax;
	}
    }
    canspend = natp->nat_money - tally;

    /* Find the destination sector for the trade */
    if (((trade.trd_type == EF_PLANE) && !pln_is_in_orbit(&tg.plane))
	|| (trade.trd_type == EF_NUKE)) {
	while (1) {
	    p = getstring("Destination sector: ", buf);
	    if (!trade_check_ok(&trade, &tg.gen))
		return RET_FAIL;
	    if (!p) {
		return RET_FAIL;
	    }
	    if (!sarg_xy(p, &sx, &sy) || !getsect(sx, sy, &sect)) {
		pr("Bad sector designation; try again!\n");
		continue;
	    }
	    if (!player->owner) {
		pr("You don't own that sector; try again!\n");
		continue;
	    }
	    if (!(plchr[tg.plane.pln_type].pl_flags & P_V)) {
		if (!player->god && (sect.sct_type != SCT_AIRPT)) {
		    pr("Destination sector is not an airfield!\n");
		    continue;
		}
		if (!player->god && (sect.sct_effic < 60)) {
		    pr("That airport still under construction!\n");
		    continue;
		}
	    }
	    break;
	}
    } else if (trade.trd_type == EF_LAND) {
	while (1) {
	    p = getstring("Destination sector: ", buf);
	    if (!trade_check_ok(&trade, &tg.gen))
		return RET_FAIL;
	    if (!p) {
		return RET_FAIL;
	    }
	    if (!sarg_xy(p, &sx, &sy) || !getsect(sx, sy, &sect)) {
		pr("Bad sector designation; try again!\n");
		continue;
	    }
	    if (!player->owner) {
		pr("You don't own that sector; try again!\n");
		continue;
	    }
	    if (!player->god && (sect.sct_type != SCT_HEADQ)) {
		pr("Destination sector is not a headquarters!\n");
		continue;
	    }
	    if (!player->god && (sect.sct_effic < 60)) {
		pr("That headquarters still under construction!\n");
		continue;
	    }
	    break;
	}
    } else {
	/* This trade doesn't teleport; make destination invalid */
	sx = 1;
	sy = 0;
    }

    p = getstring("How much do you bid: ", buf);
    if (!p || !*p)
	return RET_OK;
    if (!trade_check_ok(&trade, &tg.gen))
	return RET_FAIL;
    bid = atoi(p);
    if (bid < price)
	bid = price;
    if (bid > canspend) {
	pr("You don't have %.2f to spend!\n", price);
	return RET_OK;
    }
    if (bid > trade.trd_price) {
	/* Add five minutes to the time if less than 5 minutes left. */
	time(&now);
	if (((TRADE_DELAY - (now - trade.trd_markettime)) < 300) &&
	    trade.trd_maxbidder != player->cnum)
	    trade.trd_markettime += 300;
	trade.trd_price = bid;
	trade.trd_maxbidder = player->cnum;
	trade.trd_x = sx;
	trade.trd_y = sy;
	pr("Your bid on lot #%d is being considered.\n", lotno);
	if (!puttrade(lotno, &trade))
	    pr("Problems with the trade file.  Get help\n");
    } else
	pr("Your bid wasn't high enough (you need to bid more than someone else.)\n");

    check_trade();

    return RET_OK;
}

int
check_trade(void)
{
    int n;
    struct natstr *natp;
    struct trdstr trade;
    union empobj_storage tg;
    time_t now;
    double tleft;
    float price;
    int saveid;
    natid seller;

    for (n = 0; gettrade(n, &trade); n++) {
	if (trade.trd_unitid < 0)
	    continue;
	if (!trade_getitem(&trade, &tg))
	    continue;
	if (tg.gen.own == 0) {
	    trade.trd_owner = 0;
	    trade.trd_unitid = -1;
	    puttrade(n, &trade);
	    continue;
	}
	if (tg.gen.own != trade.trd_owner) {
	    logerror("Something weird, tg.gen.own != trade.trd_owner!\n");
	    trade.trd_owner = 0;
	    trade.trd_unitid = -1;
	    puttrade(n, &trade);
	    continue;
	}

	if (trade.trd_owner == trade.trd_maxbidder)
	    continue;

	(void)time(&now);
	tleft =
	    TRADE_DELAY / 3600.0 - (now - trade.trd_markettime) / 3600.0;
	if (tleft < 0.0)
	    tleft = 0.0;
	if (tleft > 0.0)
	    continue;

	saveid = trade.trd_unitid;
	seller = trade.trd_owner;
	trade.trd_owner = 0;
	trade.trd_unitid = -1;
	if (!puttrade(n, &trade)) {
	    logerror("Couldn't save trade after purchase; get help!\n");
	    continue;
	}

	price = trade.trd_price;
	natp = getnatp(trade.trd_maxbidder);
	if (natp->nat_money < price) {
	    nreport(trade.trd_maxbidder, N_WELCH_DEAL, seller, 1);
	    wu(0, seller,
	       "%s tried to buy a %s #%d from you for $%.2f\n",
	       cname(trade.trd_maxbidder), trade_nameof(&trade, &tg.gen),
	       saveid, price * tradetax);
	    wu(0, seller, "   but couldn't afford it.\n");
	    wu(0, seller,
	       "   Your item was taken off the market.\n");
	    wu(0, trade.trd_maxbidder,
	       "You tried to buy %s #%d from %s for $%.2f\n",
	       trade_nameof(&trade, &tg.gen), saveid, cname(seller),
	       price);
	    wu(0, trade.trd_maxbidder, "but couldn't afford it.\n");
	    continue;
	}

/* If we get this far, the sale will go through. */

	natp->nat_money -= price;
	putnat(natp);

	natp = getnatp(seller);
	natp->nat_money += roundavg(price * tradetax);
	putnat(natp);

	switch (trade.trd_type) {
	case EF_NUKE:
	    tg.nuke.nuk_x = trade.trd_x;
	    tg.nuke.nuk_y = trade.trd_y;
	    tg.nuke.nuk_plane = -1;
	    break;
	case EF_PLANE:
	    if (!pln_is_in_orbit(&tg.plane)) {
		tg.plane.pln_x = trade.trd_x;
		tg.plane.pln_y = trade.trd_y;
	    }
	    if (opt_MOB_ACCESS) {
		tg.plane.pln_mobil = -(etu_per_update / sect_mob_neg_factor);
		game_tick_to_now(&tg.plane.pln_access);
	    } else {
		tg.plane.pln_mobil = 0;
	    }
	    tg.plane.pln_harden = 0;
	    tg.plane.pln_ship = -1;
	    tg.plane.pln_land = -1;
	    break;
	case EF_SHIP:
	    break;
	case EF_LAND:
	    tg.land.lnd_x = trade.trd_x;
	    tg.land.lnd_y = trade.trd_y;
	    if (opt_MOB_ACCESS) {
		tg.land.lnd_mobil = -(etu_per_update / sect_mob_neg_factor);
		game_tick_to_now(&tg.land.lnd_access);
	    } else {
		tg.land.lnd_mobil = 0;
	    }
	    tg.land.lnd_harden = 0;
	    unit_drop_cargo(&tg.gen, 0);
	    tg.land.lnd_ship = -1;
	    tg.land.lnd_land = -1;
	    break;
	default:
	    logerror("Bad trade type %d in trade\n", trade.trd_type);
	    break;
	}
	unit_give_away(&tg.gen, trade.trd_maxbidder, 0);
	put_empobj(trade.trd_type, saveid, &tg.gen);

	nreport(seller, N_MAKE_SALE, trade.trd_maxbidder, 1);
	wu(0, seller, "%s bought %s #%d from you for $%.2f\n",
	   cname(trade.trd_maxbidder), trade_nameof(&trade, &tg.gen),
	   saveid, price * tradetax);
	wu(0, trade.trd_maxbidder,
	   "The bidding is over & you bought %s #%d from %s for $%.2f\n",
	   trade_nameof(&trade, &tg.gen), saveid, cname(seller),
	   price);
    }
    return RET_OK;
}

int
ontradingblock(int type, void *ptr)
{
    struct trdstr trade;
    union empobj_storage tg;
    int n;

    for (n = 0; gettrade(n, &trade); n++) {
	if (trade.trd_unitid < 0)
	    continue;
	if (!trade_getitem(&trade, &tg))
	    continue;
	if (trade.trd_type != type)
	    continue;
	if (tg.gen.uid == ((struct empobj *)ptr)->uid)
	    return 1;
    }
    return 0;
}

void
trdswitchown(int type, void *ptr, int newown)
{
    struct trdstr trade;
    union empobj_storage tg;
    int n;

    for (n = 0; gettrade(n, &trade); n++) {
	if (trade.trd_unitid < 0)
	    continue;
	if (!trade_getitem(&trade, &tg))
	    continue;
	if (trade.trd_type != type)
	    continue;
	if (tg.gen.uid != ((struct empobj *)ptr)->uid)
	    continue;
	if (trade.trd_owner == trade.trd_maxbidder)
	    trade.trd_maxbidder = newown;
	trade.trd_owner = newown;
	if (newown == 0)
	    trade.trd_unitid = -1;
	puttrade(n, &trade);
	return;
    }
}
