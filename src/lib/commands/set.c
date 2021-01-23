/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  set.c: Place units/ships/planes/nukes up for sale.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 *     Markus Armbruster, 2004-2018
 */

#include <config.h>

#include "commands.h"
#include "empobj.h"
#include "land.h"
#include "optlist.h"
#include "plane.h"
#include "ship.h"
#include "trade.h"

/*
 * format: set <type> <SHIP/NUKE> <PRICE>
 */
int
c_set(void)
{
    static int ef_saleable[] = { EF_SHIP, EF_PLANE, EF_LAND, EF_NUKE, EF_BAD };
    char *p;
    int type;
    int price;
    char prompt[80];
    struct trdstr trade;
    struct nstr_item ni;
    struct nstr_item ni_trade;
    union empobj_storage item;
    struct sctstr sect;
    int freeslot;
    int foundslot;
    int id;
    time_t now;
    char buf[1024];

    if (!opt_MARKET) {
	pr("The market is disabled.\n");
	return RET_FAIL;
    }
    check_market();
    check_trade();

    p = getstarg(player->argp[1], "Ship, plane, land unit or nuke? ", buf);
    if (!p)
	return RET_SYN;
    if ((type = ef_byname_from(p, ef_saleable)) < 0) {
	pr("You can sell only ships, planes, land units or nukes\n");
	return RET_SYN;
    }
    if (!snxtitem(&ni, type, player->argp[2], NULL))
	return RET_SYN;
    while (nxtitem(&ni, &item)) {
	if (!player->owner && !player->god)
	    continue;
	getsect(item.gen.x, item.gen.y, &sect);
	if (!military_control(&sect)) {
	    pr("Military control required to sell goods.\n");
	    return RET_FAIL;
	}
	trade.trd_type = type;
	sprintf(prompt, "%s #%d; price? ",
		trade_nameof(&trade, &item.gen), ni.cur);
	if (!(p = getstarg(player->argp[3], prompt, buf)))
	    return RET_FAIL;
	if (!check_obj_ok(&item.gen))
	    return RET_FAIL;
	if ((price = atoi(p)) < 0)
	    continue;
	foundslot = -1;
	freeslot = -1;
	snxtitem_all(&ni_trade, EF_TRADE);
	while (nxtitem(&ni_trade, &trade)) {
	    if (!trade.trd_owner) {
		if (freeslot < 0)
		    freeslot = ni_trade.cur;
		continue;
	    }
	    if (trade.trd_unitid == ni.cur && trade.trd_type == type) {
		foundslot = ni_trade.cur;
		break;
	    }
	}
	if (price <= 0) {
	    if (foundslot >= 0) {
		pr("%s #%d (lot #%d) removed from trading\n",
		   trade_nameof(&trade, &item.gen), ni.cur, foundslot);
		trade.trd_owner = 0;
		puttrade(ni_trade.cur, &trade);
	    }
	} else {
	    if (trade_has_unsalable_cargo(&item.gen, 1))
		return RET_FAIL;
	    if (foundslot >= 0)
		id = foundslot;
	    else if (freeslot >= 0)
		id = freeslot;
	    else
		id = ni_trade.cur;
	    ef_blank(EF_TRADE, id, &trade);
	    trade.trd_x = 1;
	    trade.trd_y = 0;
	    trade.trd_type = type;
	    trade.trd_owner = player->cnum;
	    trade.trd_unitid = ni.cur;
	    trade.trd_price = price;
	    (void)time(&now);
	    trade.trd_markettime = now;
	    trade.trd_maxbidder = player->cnum;
	    puttrade(id, &trade);
	    pr("%s #%d (lot #%d) price %s to $%d\n",
	       trade_nameof(&trade, &item.gen), ni.cur,
	       id, foundslot >= 0 ? "reset" : "set", price);
	}
    }
    return RET_OK;
}
