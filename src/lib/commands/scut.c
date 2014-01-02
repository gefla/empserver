/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  scut.c: Scuttle ships, planes or land units
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2012
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "news.h"
#include "optlist.h"
#include "unit.h"

int
scut(void)
{
    struct nstr_item ni;
    union empobj_storage item;
    int type, n;
    struct mchrstr *mp;
    char *p;
    char prompt[128];
    char buf[1024];
    struct sctstr sect;

    if (!(p = getstarg(player->argp[1], "Ship, land, or plane? ", buf)))
	return RET_SYN;
    switch (*p) {
    case 's':
	type = EF_SHIP;
	break;
    case 'p':
	type = EF_PLANE;
	break;
    case 'l':
	type = EF_LAND;
	break;
    default:
	pr("Ships, land units, or planes only! (s, l, p)\n");
	return RET_SYN;
    }

    if (!snxtitem(&ni, type, player->argp[2], NULL))
	return RET_SYN;
    n = 0;
    while (nxtitem(&ni, &item)) {
	if (!player->owner)
	    continue;
	n++;
    }
    snprintf(prompt, sizeof(prompt), "Really scuttle %d %s%s [n]? ",
			    n, ef_nameof(type), splur(n));
    if (!confirm(prompt))
	return RET_FAIL;

    snxtitem_rewind(&ni);
    while (nxtitem(&ni, &item)) {
	if (!player->owner)
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(type, &item.ship)) {
		pr("You cannot scuttle an item on the trading block!\n");
		continue;
	    }
	}

	if (type == EF_SHIP) {
	    mp = &mchr[(int)item.ship.shp_type];
	    if (mp->m_flags & M_TRADE) {
		if (!scuttle_tradeship(&item.ship, 1))
		    continue;
	    }
	} else if (type == EF_LAND) {
	    if (item.land.lnd_ship >= 0) {
		pr("%s is on a ship, and cannot be scuttled!\n",
		   prland(&item.land));
		continue;
	    }
	}
	pr("%s scuttled in %s\n",
	   unit_nameof(&item.gen),
	   xyas(item.gen.x, item.gen.y, player->cnum));
	getsect(item.gen.x, item.gen.y, &sect);
	if (sect.sct_own == item.gen.own)
	    unit_drop_cargo(&item.gen, sect.sct_own);
	item.gen.effic = 0;
	put_empobj(type, item.gen.uid, &item.gen);
    }

    return RET_OK;
}

int
scuttle_tradeship(struct shpstr *sp, int interactive)
{
    float cash = 0;
    float ally_cash = 0;
    int dist = -1;
    struct sctstr sect;
    struct mchrstr *mp;
    struct natstr *np;
    char buf[512];
    struct natstr *natp;

    mp = &mchr[(int)sp->shp_type];
    getsect(sp->shp_x, sp->shp_y, &sect);
    if (sect.sct_own && sect.sct_type == SCT_HARBR && sect.sct_effic >= 2
	&& relations_with(sect.sct_own, sp->shp_own) >= FRIENDLY) {
	dist = mapdist(sp->shp_x, sp->shp_y,
		       sp->shp_orig_x, sp->shp_orig_y);
	/* Don't disclose distance to to pirates */
	if (sp->shp_own == sp->shp_orig_own)
	    mpr(sp->shp_own, "%s has gone %d sects\n", prship(sp), dist);
	if (dist < trade_1_dist)
	    cash = 0;
	else if (dist < trade_2_dist)
	    cash = 1.0 + trade_1 * dist;
	else if (dist < trade_3_dist)
	    cash = 1.0 + trade_2 * dist;
	else
	    cash = 1.0 + trade_3 * dist;
	cash *= mp->m_cost;
	cash *= sp->shp_effic / 100.0;

	if (sect.sct_own != sp->shp_own) {
	    ally_cash = cash * trade_ally_cut;
	    cash *= (1.0 + trade_ally_bonus);
	}
    }

    if (!cash && (dist < 0 || sp->shp_own == sp->shp_orig_own)) {
	if (interactive) {
	    pr("You won't get any money if you scuttle in %s!",
	       xyas(sp->shp_x, sp->shp_y, player->cnum));
	    sprintf(buf, "Are you sure you want to scuttle %s? ", prship(sp));
	    return confirm(buf);
	} else
	    return 0;
    }

    if (interactive) {
	player->dolcost -= cash;
    } else {
	natp = getnatp(sp->shp_own);
	natp->nat_money += cash;
	putnat(natp);
	wu(0, sp->shp_own, "You just made $%d.\n", (int)cash);
    }

    if (ally_cash) {
	np = getnatp(sect.sct_own);
	np->nat_money += ally_cash;
	putnat(np);
	wu(0, sect.sct_own,
	   "Trade with %s nets you $%d at %s\n",
	   cname(sp->shp_own),
	   (int)ally_cash, xyas(sect.sct_x, sect.sct_y, sect.sct_own));
	if (sp->shp_own != sp->shp_orig_own)
	    nreport(sp->shp_own, N_PIRATE_TRADE, sp->shp_orig_own, 1);
	else
	    nreport(sp->shp_own, N_TRADE, sect.sct_own, 1);
    } else if (sp->shp_own != sp->shp_orig_own)
	nreport(sp->shp_own, N_PIRATE_KEEP, sp->shp_orig_own, 1);

    return 1;
}
