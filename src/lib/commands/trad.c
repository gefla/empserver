/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  trad.c: Buy units/ships/planes/nukes from other nations.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Pat Loney, 1992
 *     Steve McClure, 1996-2000
 */

#include <ctype.h>
#include "misc.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "news.h"
#include "item.h"
#include "ship.h"
#include "nuke.h"
#include "land.h"
#include "plane.h"
#include "trade.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "player.h"
#include "commodity.h"
#include "loan.h"
#include "commands.h"
#include "optlist.h"

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
    union trdgenstr tg;
    int plflags;
    double canspend;
    time_t now;
    int bid;
    double tleft;
    double tally;
    int q;
    s_char buf[1024];


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
    while (nxtitem(&ni, (char *)&trade)) {
	if (trade.trd_unitid < 0)
	    continue;
	if (!trade_getitem(&trade, &tg)) {
	    continue;
	};
	/* fix up database if things get weird */
	/*if (trade.trd_owner != tg.gen.trg_own) {
	   trade.trd_unitid = -1;
	   (void) puttrade(ni.cur, &trade);
	   continue;
	   } */
	pr(" %3d ", ni.cur);
	(void)time(&now);
	tleft =
	    TRADE_DELAY / 3600.0 - (now - trade.trd_markettime) / 3600.0;
	if (tleft < 0.0)
	    tleft = 0.0;
	pr("$%7d  %2d %5.2f hrs ", trade.trd_price,
	   trade.trd_maxbidder, tleft);
	(void)trade_desc(&trade, &tg);	/* XXX */
	pr("\n");
	if (trade.trd_owner == player->cnum && !player->god)
	    pr(" (your own lot)\n");
	n++;
    }
    if (n == 0) {
	pr("Nothing to buy at the moment...\n");
	return RET_OK;
    }
    if ((p = getstring("Which lot to buy: ", buf)) == 0 || *p == 0)
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
	trade.trd_unitid = -1;
	if (!puttrade(lotno, &trade)) {
	    pr("Couldn't save after getitem failed; get help!\n");
	    return RET_SYS;
	}
	return RET_OK;
    }
    switch (trade.trd_type) {
    case EF_NUKE:
	/*
	   if (!getsect(tg.gen.trg_x, tg.gen.trg_y, &sect)) {
	   return RET_FAIL;
	   }
	   trade.trd_owner = sect.sct_own;
	   break;
	 */
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
    for (q = 0; gettrade(q, &tmpt); q++) {
	if (tmpt.trd_maxbidder == player->cnum &&
	    tmpt.trd_unitid >= 0 && tmpt.trd_owner != player->cnum) {
	    tally += tmpt.trd_price * tradetax;
	}
    }
    for (q = 0; getcomm(q, &comt); q++) {
	if (comt.com_maxbidder == player->cnum &&
	    comt.com_owner != 0 && comt.com_owner != player->cnum) {
	    tally += (comt.com_price * comt.com_amount) * buytax;
	}
    }
    canspend = natp->nat_money - tally;
    /*
     * Find the destination sector for the plane before the trade
     * is actually made. Must be owned (except for satellites) and
     * must be a 60% airfield (except for VTOL planes).
     */
    if (((trade.trd_type == EF_PLANE) || (trade.trd_type == EF_NUKE))
	&& ((trade.trd_type == EF_NUKE) ||
	    !(tg.pln.pln_flags & PLN_LAUNCHED))) {
	plflags = plchr[(int)tg.pln.pln_type].pl_flags;
	while (1) {
	    p = getstring("Destination sector: ", buf);
	    if (!trade_check_ok(lotno, &trade, &tg))
		return RET_FAIL;
	    if (p == 0) {
		return RET_FAIL;
	    }
	    if (!sarg_xy(p, &sx, &sy) || !getsect(sx, sy, &sect)) {
		pr("Bad sector designation; try again!\n");
		continue;
	    }
	    if (!player->owner && !(plflags & P_O)) {
		pr("You don't own that sector; try again!\n");
		continue;
	    }
	    if (!(plflags & (P_V | P_O))) {
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
    }
    if (trade.trd_type == EF_LAND) {
	while (1) {
	    p = getstring("Destination sector: ", buf);
	    if (!trade_check_ok(lotno, &trade, &tg))
		return RET_FAIL;
	    if (p == 0) {
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
    }

    pr("WARNING!  This market issues credit.  If you make more\n");
    pr("  bids than your treasury can cover at the time of sale,\n");
    pr("  you can potentially go into financial ruin, and see no\n");
    pr("  gains.  You have been warned.\n\n");

    if ((p = getstring("How much do you bid: ", buf)) == 0 || *p == 0)
	return RET_OK;
    if (!trade_check_ok(lotno, &trade, &tg))
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
    int j;
    struct nstr_item ni;
    struct plnstr plane;
    struct lndstr land;
    struct natstr *natp;
    struct trdstr trade;
    union trdgenstr tg;
    time_t now;
    double subleft;
    double monleft;
    double tleft;
    float price;
    int saveid;
    struct lonstr loan;
    long outstanding;		/* Outstanding debt */
    long couval;		/* Value of country's goods */
    int foundloan;

/*    logerror("Checking the trades.\n");*/
    for (n = 0; gettrade(n, &trade); n++) {
	if (trade.trd_unitid < 0)
	    continue;
	if (!trade_getitem(&trade, &tg))
	    continue;
	if (tg.gen.trg_own == 0) {
	    trade.trd_unitid = -1;
	    puttrade(n, &trade);
	    continue;
	}
	if (tg.gen.trg_own != trade.trd_owner) {
	    logerror("Something weird, tg.gen.trg_own != trade.trd_owner!\n");
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
	trade.trd_unitid = -1;
	if (!puttrade(n, &trade)) {
	    logerror("Couldn't save trade after purchase; get help!\n");
	    continue;
	}

	monleft = 0;
	price = trade.trd_price;
	natp = getnatp(trade.trd_maxbidder);
	if (natp->nat_money <= 0)
	    monleft = price;
	if (natp->nat_money < price && natp->nat_money > 0) {
	    monleft = price - (natp->nat_money - 1);
	    natp->nat_money = 1;
	    price = price - monleft;
	} else if (natp->nat_money > 0) {
	    monleft = 0;
	    natp->nat_money -= price;
	}

	subleft = monleft;

	if (opt_LOANS) {
	    /* Try to make a loan for the rest from the owner. */
	    if (monleft > 0 && natp->nat_money > 0) {
		if ((float)((float)price / (float)(price + monleft)) < 0.1) {
		    wu(0, trade.trd_maxbidder,
		       "You need at least 10 percent down to purchase something on credit.\n");
		} else {
		    couval = get_couval(trade.trd_maxbidder);
		    outstanding = get_outstand(trade.trd_maxbidder);
		    couval = couval - outstanding;
		    if (couval > monleft) {
			/*  Make the loan */
			foundloan = 0;
			for (j = 0; getloan(j, &loan); j++) {
			    if (loan.l_status != LS_FREE)
				continue;
			    foundloan = 1;
			    break;
			}
			if (!foundloan)
			    ef_extend(EF_LOAN, 1);
			loan.l_status = LS_SIGNED;
			loan.l_loner = trade.trd_owner;
			loan.l_lonee = trade.trd_maxbidder;
			loan.l_irate = 25;
			loan.l_ldur = 4;
			loan.l_amtpaid = 0;
			loan.l_amtdue = monleft;
			time(&loan.l_lastpay);
			loan.l_duedate =
			    (loan.l_ldur * SECS_PER_DAY) + loan.l_lastpay;
			loan.l_uid = j;
			if (!putloan(j, &loan))
			    logerror("Error writing to the loan file.\n");
			else
			    monleft = 0;
			nreport(trade.trd_maxbidder, N_FIN_TROUBLE,
				trade.trd_owner, 1);
			wu(0, trade.trd_maxbidder,
			   "You just took loan #%d for $%.2f to cover the cost of your purchase.\n",
			   j, (float)loan.l_amtdue);
			wu(0, trade.trd_owner,
			   "You just extended loan #%d to %s to help with the purchase cost.\n",
			   j, cname(trade.trd_maxbidder));
		    } else {
			nreport(trade.trd_maxbidder, N_CREDIT_JUNK,
				trade.trd_owner, 1);
			wu(0, trade.trd_maxbidder,
			   "You don't have enough credit to get a loan.\n");
			wu(0, trade.trd_owner,
			   "You just turned down a loan to %s.\n",
			   cname(trade.trd_maxbidder));
		    }
		}
	    }
	}
	if (monleft > 0) {
	    nreport(trade.trd_maxbidder, N_WELCH_DEAL, trade.trd_owner, 1);
	    wu(0, trade.trd_owner,
	       "%s tried to buy a %s #%d from you for $%.2f\n",
	       cname(trade.trd_maxbidder), trade_nameof(&trade, &tg),
	       saveid, (float)(price * tradetax));
	    wu(0, trade.trd_owner, "   but couldn't afford it.\n");
	    wu(0, trade.trd_owner,
	       "   Your item was taken off the market.\n");
	    wu(0, trade.trd_maxbidder,
	       "You tried to buy %s #%d from %s for $%.2f\n",
	       trade_nameof(&trade, &tg), saveid, cname(trade.trd_owner),
	       (float)(price * tradetax));
	    wu(0, trade.trd_maxbidder, "but couldn't afford it.\n");
	    continue;
	}

/* If we get this far, the sale will go through. */
/* Only pay tax on the part you actually get cash for.  As a break,
   we don't tax the part you have to give a loan on. */

	putnat(natp);
	natp = getnatp(trade.trd_owner);
	/* Make sure we subtract the extra amount */
	natp->nat_money += (roundavg(price * tradetax) - subleft);
	putnat(natp);
	switch (trade.trd_type) {
	case EF_NUKE:
	    tg.nuk.nuk_x = trade.trd_x;
	    tg.nuk.nuk_y = trade.trd_y;
	    makelost(EF_NUKE, tg.nuk.nuk_own, tg.nuk.nuk_uid, tg.nuk.nuk_x,
		     tg.nuk.nuk_y);
	    tg.nuk.nuk_own = trade.trd_maxbidder;
	    makenotlost(EF_NUKE, tg.nuk.nuk_own, tg.nuk.nuk_uid,
			tg.nuk.nuk_x, tg.nuk.nuk_y);
	    break;
	case EF_PLANE:
	    if ((tg.pln.pln_flags & PLN_LAUNCHED) == 0) {
		tg.pln.pln_x = trade.trd_x;
		tg.pln.pln_y = trade.trd_y;
	    }
	    makelost(EF_PLANE, tg.pln.pln_own, tg.pln.pln_uid,
		     tg.pln.pln_x, tg.pln.pln_y);
	    tg.pln.pln_own = trade.trd_maxbidder;
	    makenotlost(EF_PLANE, tg.pln.pln_own, tg.pln.pln_uid,
			tg.pln.pln_x, tg.pln.pln_y);
	    tg.pln.pln_wing = ' ';
	    /* no cheap version of fly */
	    if (opt_MOB_ACCESS) {
		tg.pln.pln_mobil = -(etu_per_update / sect_mob_neg_factor);
	    } else {
		tg.pln.pln_mobil = 0;
	    }
	    tg.pln.pln_mission = 0;
	    tg.pln.pln_harden = 0;
	    time(&tg.pln.pln_access);
	    tg.pln.pln_ship = -1;
	    tg.pln.pln_land = -1;
	    break;
	case EF_SHIP:
	    takeover_ship(&tg.shp, trade.trd_maxbidder, 0);
	    break;
	case EF_LAND:
	    tg.lnd.lnd_x = trade.trd_x;
	    tg.lnd.lnd_y = trade.trd_y;
	    if (tg.lnd.lnd_ship >= 0) {
		struct shpstr ship;
		getship(tg.lnd.lnd_ship, &ship);
		ship.shp_nland--;
		putship(ship.shp_uid, &ship);
	    }
	    makelost(EF_LAND, tg.lnd.lnd_own, tg.lnd.lnd_uid, tg.lnd.lnd_x,
		     tg.lnd.lnd_y);
	    tg.lnd.lnd_own = trade.trd_maxbidder;
	    makenotlost(EF_LAND, tg.lnd.lnd_own, tg.lnd.lnd_uid,
			tg.lnd.lnd_x, tg.lnd.lnd_y);
	    tg.lnd.lnd_army = ' ';
	    /* no cheap version of fly */
	    if (opt_MOB_ACCESS) {
		tg.lnd.lnd_mobil = -(etu_per_update / sect_mob_neg_factor);
	    } else {
		tg.lnd.lnd_mobil = 0;
	    }
	    tg.lnd.lnd_harden = 0;
	    time(&tg.lnd.lnd_access);
	    tg.lnd.lnd_mission = 0;
	    /* Drop any land units this unit was carrying */
	    snxtitem_xy(&ni, EF_LAND, tg.lnd.lnd_x, tg.lnd.lnd_y);
	    while (nxtitem(&ni, (s_char *)&land)) {
		if (land.lnd_land != tg.lnd.lnd_uid)
		    continue;
		land.lnd_land = -1;
		wu(0, land.lnd_own, "unit #%d dropped in %s\n",
		   land.lnd_uid,
		   xyas(land.lnd_x, land.lnd_y, land.lnd_own));
		putland(land.lnd_uid, &land);
	    }
	    /* Drop any planes this unit was carrying */
	    snxtitem_xy(&ni, EF_PLANE, tg.lnd.lnd_x, tg.lnd.lnd_y);
	    while (nxtitem(&ni, (s_char *)&plane)) {
		if (plane.pln_flags & PLN_LAUNCHED)
		    continue;
		if (plane.pln_land != land.lnd_uid)
		    continue;
		plane.pln_land = -1;
		wu(0, plane.pln_own, "plane #%d dropped in %s\n",
		   plane.pln_uid,
		   xyas(plane.pln_x, plane.pln_y, plane.pln_own));
		putplane(plane.pln_uid, &plane);
	    }
	    tg.lnd.lnd_ship = -1;
	    tg.lnd.lnd_land = -1;
	    break;
	default:
	    logerror("Bad trade type %d in trade\n", trade.trd_type);
	    break;
	}
	if (!ef_write(trade.trd_type, saveid, (char *)&tg)) {
	    logerror("Couldn't write unit to disk; seek help.\n");
	    continue;
	}
	nreport(trade.trd_owner, N_MAKE_SALE, trade.trd_maxbidder, 1);
	wu(0, trade.trd_owner, "%s bought a %s #%d from you for $%.2f\n",
	   cname(trade.trd_maxbidder), trade_nameof(&trade, &tg),
	   saveid, (float)(price * tradetax));
	wu(0, trade.trd_maxbidder,
	   "The bidding is over & you bought %s #%d from %s for $%.2f\n",
	   trade_nameof(&trade, &tg), saveid, cname(trade.trd_owner),
	   (float)price);
    }
/*    logerror("Done checking the trades.\n");*/
    return RET_OK;
}

int
ontradingblock(int type, int *ptr)
	  /* Generic pointer */
{
    struct trdstr trade;
    union trdgenstr tg;
    int n;

    for (n = 0; gettrade(n, &trade); n++) {
	if (trade.trd_unitid < 0)
	    continue;
	if (!trade_getitem(&trade, &tg))
	    continue;
	if (trade.trd_type != type)
	    continue;
	if (tg.gen.trg_uid == ((struct genstr *)ptr)->trg_uid)
	    return 1;
    }
    return 0;
}

void
trdswitchown(int type, int *ptr, int newown)
	  /* Generic pointer */
{
    struct trdstr trade;
    union trdgenstr tg;
    int n;

    for (n = 0; gettrade(n, &trade); n++) {
	if (trade.trd_unitid < 0)
	    continue;
	if (!trade_getitem(&trade, &tg))
	    continue;
	if (trade.trd_type != type)
	    continue;
	if (tg.gen.trg_uid != ((struct genstr *)ptr)->trg_uid)
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
