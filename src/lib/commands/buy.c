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
 *  buy.c: Buy commodities from other nations
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Pat Loney, 1992
 *     Steve McClure, 1996-2000
 */

#include "misc.h"
#include "xy.h"
#include "file.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "news.h"
#include "nsc.h"
#include "item.h"
#include "deity.h"
#include "land.h"
#include "commodity.h"
#include "plane.h"
#include "nuke.h"
#include "ship.h"
#include "trade.h"
#include "player.h"
#include "loan.h"
#include "commands.h"
#include "optlist.h"

extern int MARK_DELAY;

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
    struct comstr ncomm;
    struct comstr comt;
    struct trdstr tmpt;
    struct ichrstr *ip;
    int qty;
    int o, n, q;
    coord x, y;
    char *p;
    float bid;
    time_t now;
    double tally;
    double canspend;
    extern double buytax;
    extern double tradetax;
    s_char buf[1024];

    if (!opt_MARKET) {
	pr("The market is disabled.\n");
	return RET_FAIL;
    }
    natp = getnatp(player->cnum);
    display_mark(player->argp[1]);
    pr("\n");
    p = getstarg(player->argp[2], "Which lot are you bidding on: ", buf);
    if (p == 0)
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
    if (player->argp[1] && *(player->argp[1]) &&
	comm.com_type != player->argp[1][0]) {
	pr("That lot is not of the type you specified.\n");
	return RET_OK;
    }
    if (comm.com_owner == player->cnum) {
	pr("You can't bid on your own lot.\n");
	return RET_OK;
    }
    pr("WARNING!  This market issues credit.  If you make more\n");
    pr("  bids than your treasury can cover at the time of sale,\n");
    pr("  you can potentially go into financial ruin, and see no\n");
    pr("  gains.  You have been warned.\n\n");
    if ((p = getstarg(player->argp[3], "How much per unit: ", buf)) == 0)
	return RET_SYN;
    bid = atof(p);
    if (bid <= 0)
	return RET_FAIL;
    if (natp->nat_money < (bid * comm.com_amount * buytax)) {
	pr("This purchase would cost %.2f, %.2f more than you have.\n",
	   bid * comm.com_amount * buytax,
	   bid * comm.com_amount * buytax - natp->nat_money);
	return RET_FAIL;
    }
/*  check to see if all of the bids that this player has out plus this new bid
    would make him go broke.  Ken, I ought to skin you alive for making me code
    this part up.*/
    tally = 0.0;
    for (q = 0; gettrade(q, &tmpt); q++) {
	if (tmpt.trd_maxbidder == player->cnum &&
	    tmpt.trd_unitid >= 0 && tmpt.trd_owner != player->cnum) {
	    tally += tmpt.trd_maxprice * tradetax;
	}
    }
    for (q = 0; getcomm(q, &comt); q++) {
	if (comt.com_maxbidder == player->cnum &&
	    comt.com_owner != 0 && comt.com_owner != player->cnum) {
	    tally += (comt.com_maxprice * comt.com_amount) * buytax;
	}
    }
    canspend = natp->nat_money - tally;
    getcomm(o, &comm);
    if ((bid * comm.com_amount * buytax) > canspend) {
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
    ip = whichitem(comm.com_type);
    n = getvar(ip->i_vtype, (char *)&sect, EF_SECTOR);
    qty = comm.com_amount;
    if (qty + n > 9990) {
	pr("That sector cannot hold %d more %s. It currently holds %d.\n",
	   qty, ip->i_name, n);
	return RET_FAIL;
    }
    /* First we check for room, then we yank back.  Probably not necessary. */
    if (putvar(ip->i_vtype, n + qty, (char *)&sect, EF_SECTOR) <= 0) {
	pr("No room to store %s in %s\n",
	   ip->i_name, xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_FAIL;
    }
    if (putvar(ip->i_vtype, n, (char *)&sect, EF_SECTOR) <= 0) {
	pr("Something weird just happened, tell the deity.\n");
	logerror("buy.c: putvar failed.\n");
	return RET_FAIL;
    }
    if ((bid * comm.com_amount) > natp->nat_money) {
	pr("You don't have that much to spend!\n");
	return RET_FAIL;
    }
    getcomm(o, &ncomm);
    if (!ncomm.com_owner) {
	pr("That lot has been taken off the market.\n");
	return RET_FAIL;
    }
    if (bid > 0.04 + comm.com_maxprice) {
	comm.com_maxprice = bid;
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
    extern double buytax;
    struct comstr comm;
    struct sctstr *sect;
    struct ichrstr *ip;
    struct natstr *natp;
    int m;
    int n;
    time_t now;
    double tmoney;
    double tleft;
    double subleft;
    double monleft;
    double gain;
    double price;
    struct lonstr loan;
    long outstanding;		/* Outstanding debt */
    long couval;		/* Value of country's goods */
    int foundloan;
    int j;

/*    logerror("Checking the market.\n");*/
    for (n = 0; getcomm(n, &comm); n++) {
	if (comm.com_maxbidder == comm.com_owner || comm.com_owner == 0)
	    continue;
	(void)time(&now);
	tleft = MARK_DELAY / 3600.0 - (now - comm.com_markettime) / 3600.0;
	if (tleft < 0)
	    tleft = 0;
	if (tleft > 0.0)
	    continue;
	ip = whichitem(comm.com_type);
	sect = getsectp(comm.com_x, comm.com_y);
	m = getvar(ip->i_vtype, (char *)sect, EF_SECTOR);

	monleft = 0;

	price = comm.com_maxprice * comm.com_amount * buytax;
	gain = comm.com_maxprice * comm.com_amount;

	natp = getnatp(comm.com_maxbidder);
	tmoney = natp->nat_money;
	if (tmoney <= 0)
	    monleft = price;
	if (tmoney < price && tmoney > 0) {
	    monleft = price - (tmoney - 1);
	    tmoney = 1;
	    price = price - monleft;
	} else if (tmoney > 0) {
	    monleft = 0;
	    tmoney = tmoney - price;
	}

	/* Subtract the amount of money that needs to come out in a loan. */
	subleft = monleft;

	if (opt_LOANS) {
	    /* Try to make a loan for the rest from the owner. */
	    if (monleft > 0 && tmoney > 0) {
		if ((float)((float)price / (float)(price + monleft)) < 0.1) {
		    wu(0, comm.com_maxbidder,
		       "You need at least 10 percent down to purchase something on credit.\n");
		} else {
		    couval = get_couval(comm.com_maxbidder);
		    outstanding = get_outstand(comm.com_maxbidder);
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
			loan.l_loner = comm.com_owner;
			loan.l_lonee = comm.com_maxbidder;
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
			nreport(comm.com_maxbidder, N_FIN_TROUBLE,
				comm.com_owner, 1);
			wu(0, comm.com_maxbidder,
			   "You just took loan #%d for $%.2f to cover the cost of your purchase.\n",
			   j, loan.l_amtdue);
			wu(0, comm.com_owner,
			   "You just extended loan #%d to %s to help with the purchase cose.\n",
			   j, cname(comm.com_maxbidder));
		    } else {
			nreport(comm.com_maxbidder, N_CREDIT_JUNK,
				comm.com_owner, 1);
			wu(0, comm.com_maxbidder,
			   "You don't have enough credit to get a loan.\n");
			wu(0, comm.com_owner,
			   "You just turned down a loan to %s.\n",
			   cname(comm.com_maxbidder));
		    }
		}
	    }
	}

	if (monleft > 0) {
	    nreport(comm.com_maxbidder, N_WELCH_DEAL, comm.com_owner, 1);
	    wu(0, comm.com_maxbidder,
	       "You didn't have enough cash/credit to cover the cost.\n");
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
	} else if (putvar(ip->i_vtype, m + comm.com_amount,
			  (char *)sect, EF_SECTOR) <= 0) {
	    wu(0, comm.com_maxbidder,
	       "Warehouse full,  sale #%d fell though.\n", n);
	    wu(0, comm.com_owner,
	       "Sale #%d fell through.  Goods remain on the market.\n", n);
	    comm.com_maxbidder = comm.com_owner;
	} else {
	    putsect(sect);
	    nreport(comm.com_owner, N_MAKE_SALE, comm.com_maxbidder, 1);
	    wu(0, comm.com_owner, "%s bought %d %c's from you for $%.2f\n",
	       cname(comm.com_maxbidder), comm.com_amount,
	       comm.com_type, gain);
	    wu(0, comm.com_maxbidder,
	       "You just bought %d %c's from %s for $%.2f\n",
	       comm.com_amount, comm.com_type, cname(comm.com_owner),
	       gain * buytax);
	    natp = getnatp(comm.com_owner);
	    /* Make sure we subtract the amount that came out in a loan */
	    natp->nat_money += (gain - subleft);
	    natp = getnatp(comm.com_maxbidder);
	    natp->nat_money = tmoney;
	    comm.com_owner = 0;
	}
	comm.com_owner = 0;
	putcomm(n, &comm);
    }
/*    logerror("Done checking the market.\n");*/
    return RET_OK;
}
