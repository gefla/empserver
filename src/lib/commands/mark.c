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
 *  mark.c: Display report for commodities
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#include "misc.h"
#include "nat.h"
#include "var.h"
#include "item.h"
#include "deity.h"
#include "commodity.h"
#include "player.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"

int
mark(void)
{
    if (!opt_MARKET) {
	pr("The market is disabled.\n");
	return RET_FAIL;
    }
    if (player->argp[1] && *(player->argp[1]))
	return display_mark(player->argp[1]);
    else
	return display_mark("   ");
}

void
pr_mark(struct comstr *comm)
{
    time_t now;
    double tleft;
    extern int MARK_DELAY;

    (void)time(&now);
    tleft = MARK_DELAY / 3600.0 - (now - comm->com_markettime) / 3600.0;
    if (tleft < 0.0)
	tleft = 0.0;
    pr(" %3d  $%12.2f  %2d  %5.2f hrs  (%3d)   %c    %6d  ",
       comm->com_uid,
       comm->com_maxprice,
       comm->com_maxbidder,
       tleft, comm->com_owner, comm->com_type, comm->com_amount);
    if (comm->com_owner == player->cnum || player->god)
	pr("%s", xyas(comm->sell_x, comm->sell_y, player->cnum));
    pr("\n");
}

int
display_mark(s_char *arg)
{
    struct comstr comm;
    struct comstr comm2;
    int sellers = 0;
    int cnt = 0;
    char c;
    s_char *p;
    struct ichrstr *ip;
    s_char buf[1024];
    int cheapest_items[I_MAX + 2];
    int i;
    int all = 0;

/* First, we execute all trades, so that we can only buy what is available. */
    check_market();
    check_trade();

    p = getstarg(arg, "What commodity (or 'all')? ", buf);
    c = (char)0;
    if (p && *p)
	c = *p;
    for (ip = &ichr[0]; ip && ip->i_mnem; ip++)
	if (ip->i_mnem == c)
	    break;
    c = ip->i_mnem;

    pr("\n     Empire Market Report\n   ");
    prdate();
    pr(" lot  high bid/unit  by  time left  owner  item  amount  sector\n");
    pr(" ---  -------------  --  ---------  -----  ----  ------  ------\n");

    if (arg) {
	if (strcmp(arg, "all"))
	    all = 1;
    }
    if (all && !c) {
	/* Ok, just printing the lowest of all of them */
	for (i = 0; i < I_MAX + 2; i++)
	    cheapest_items[i] = -1;
	for (sellers = 0; getcomm(sellers, &comm); sellers++) {
	    if (comm.com_owner == 0)
		continue;
	    for (i = 0, ip = &ichr[0]; ip && ip->i_mnem; ip++, i++)
		if (ip->i_mnem == comm.com_type)
		    break;
	    if (!ip->i_mnem)
		continue;
	    if (cheapest_items[i] != -1) {
		getcomm(cheapest_items[i], &comm2);
		if (comm.com_maxprice < comm2.com_maxprice) {
		    cheapest_items[i] = sellers;
		}
	    } else {
		cheapest_items[i] = sellers;
	    }
	}
	for (i = 0; i < I_MAX + 2; i++) {
	    if (cheapest_items[i] == -1)
		continue;
	    getcomm(cheapest_items[i], &comm);
	    cnt = 1;
	    pr_mark(&comm);
	}
    } else {
	/* Ok, print them all, or all of this type */
	for (sellers = 0; getcomm(sellers, &comm); sellers++) {
	    if (comm.com_owner == 0)
		continue;
	    if (c && comm.com_type != c)
		continue;
	    cnt = 1;
	    pr_mark(&comm);
	}
    }
    if (cnt <= 0)
	pr("\nHmmmm, the market seems to be empty today.\n");
    else
	pr("\nLooks just like Christmas at K-mart, doesn't it!\n");
    return RET_OK;
}
