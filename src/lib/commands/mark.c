/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mark.c: Display report for commodities
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#include <config.h>

#include "misc.h"
#include "item.h"
#include "commodity.h"
#include "player.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"

int
mark(void)
{
    char buf[1024];
    char *p;
    struct ichrstr *ip;

    if (!opt_MARKET) {
	pr("The market is disabled.\n");
	return RET_FAIL;
    }

    if (player->argp[1] && player->argp[1]) {
	p = getstarg(player->argp[1], "What commodity (or 'all')? ", buf);
	if (!p)
	    return RET_SYN;
	if (!strcmp(p, "all"))
	    return display_mark(I_NONE, 0);
	else {
	    ip = item_by_name(p);
	    if (!ip)
		return RET_SYN;
	    return display_mark(ip->i_vtype, 0);
	}
    }
    return display_mark(I_NONE, 1);
}

static void
pr_mark(struct comstr *comm)
{
    time_t now;
    double tleft;

    (void)time(&now);
    tleft = MARK_DELAY / 3600.0 - (now - comm->com_markettime) / 3600.0;
    if (tleft < 0.0)
	tleft = 0.0;
    pr(" %3d  $%12.2f  %2d  %5.2f hrs  (%3d)   %c    %6d  ",
       comm->com_uid,
       comm->com_price,
       comm->com_maxbidder,
       tleft, comm->com_owner, ichr[comm->com_type].i_mnem, comm->com_amount);
    if (comm->com_owner == player->cnum || player->god)
	pr("%s", xyas(comm->sell_x, comm->sell_y, player->cnum));
    pr("\n");
}

int
display_mark(i_type only_itype, int only_cheapest)
{
    struct comstr comm;
    struct comstr comm2;
    int sellers = 0;
    int cnt = 0;
    int cheapest_items[I_MAX + 1];
    i_type i;

    /* Execute trades so report lists only lots that are still available.  */
    check_market();
    check_trade();

    pr("\n     Empire Market Report\n   ");
    prdate();
    pr(" lot  high bid/unit  by  time left  owner  item  amount  sector\n");
    pr(" ---  -------------  --  ---------  -----  ----  ------  ------\n");

    if (only_cheapest) {
	for (i = I_NONE + 1; i <= I_MAX; i++)
	    cheapest_items[i] = -1;
	for (sellers = 0; getcomm(sellers, &comm); sellers++) {
	    if (comm.com_owner == 0)
		continue;
	    if (CANT_HAPPEN(comm.com_type <= I_NONE || comm.com_type > I_MAX))
		continue;
	    if (cheapest_items[comm.com_type] != -1) {
		getcomm(cheapest_items[comm.com_type], &comm2);
		if (comm.com_price < comm2.com_price) {
		    cheapest_items[comm.com_type] = sellers;
		}
	    } else {
		cheapest_items[comm.com_type] = sellers;
	    }
	}
	CANT_HAPPEN(only_itype != I_NONE); /* not implemented */
	for (i = I_NONE + 1; i <= I_MAX; i++) {
	    if (cheapest_items[i] == -1)
		continue;
	    getcomm(cheapest_items[i], &comm);
	    cnt = 1;
	    pr_mark(&comm);
	}
    } else {
	for (sellers = 0; getcomm(sellers, &comm); sellers++) {
	    if (comm.com_owner == 0)
		continue;
	    if (only_itype != I_NONE && comm.com_type != only_itype)
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
