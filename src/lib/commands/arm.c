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
 *  arm.c: Arm planes (missiles) with nuclear devices
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 2000
 */

#include <config.h>

#include <string.h>
#include "misc.h"
#include "player.h"
#include "nuke.h"
#include "plane.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"

int
arm(void)
{
    struct nchrstr *ncp;
    struct plchrstr *plc;
    struct plnstr pl;
    struct nukstr nuke;
    char *p;
    int pno, nukno;
    struct nstr_item ni;
    char buf[1024];
    char prompt[128];

    if (!snxtitem(&ni, EF_PLANE, player->argp[1]))
	return RET_SYN;
    while (nxtitem(&ni, &pl)) {
	if (!player->owner)
	    continue;
	plc = &plchr[(int)pl.pln_type];
	if ((plc->pl_flags & (P_O | P_M)) == (P_O | P_M)) {
	    pr("A %s cannot carry nuclear devices!\n", plc->pl_name);
	    return RET_FAIL;
	}
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, &pl)) {
		pr("You cannot disarm %s while it is on the trading block!\n",
		   prplane(&pl));
		return RET_FAIL;
	    }
	}
	if (pl.pln_nuketype < 0) {
	    sprintf(prompt, "Nuclear device for %s: ", prplane(&pl));
	    p = getstarg(player->argp[2], prompt, buf);
	    if (!p || !*p)
		return RET_SYN;
	    if (!check_plane_ok(&pl))
		return RET_FAIL;
	    nukno = atoi(p);
	    if (!getnuke(nukno, &nuke) || !player->owner)
		return RET_FAIL;
	} else {
	    if (nuk_on_plane(&nuke, pl.pln_uid) < 0) {
		CANT_REACH();
		continue;
	    }
	}
	ncp = &nchr[nuke.nuk_type];
	if (pl.pln_load < ncp->n_weight) {
	    pr("A %s cannot carry %s devices!\n",
	       plc->pl_name, ncp->n_name);
	    return RET_FAIL;
	}
	p = getstarg(player->argp[3], "Airburst [n]? ", buf);

	if (!check_plane_ok(&pl) || !check_nuke_ok(&nuke))
	    return RET_FAIL;

	if (p && (*p == 'y' || *p == 'Y'))
	    pl.pln_flags |= PLN_AIRBURST;
	else
	    pl.pln_flags &= ~PLN_AIRBURST;

	pl.pln_nuketype = nuke.nuk_type;
	nuke.nuk_plane = pl.pln_uid;
	putplane(pl.pln_uid, &pl);
	putnuke(nuke.nuk_uid, &nuke);
	pr("%s armed with %s.\n", prplane(&pl), prnuke(&nuke));
	pr("Warhead on %s is programmed to %s\n",
	   prplane(&pl),
	   pl.pln_flags & PLN_AIRBURST ? "airburst" : "groundburst");
    }

    return RET_OK;
}

int
disarm(void)
{
    struct plnstr pl;
    struct nukstr nuke;
    struct nstr_item ni;

    if (!snxtitem(&ni, EF_PLANE, player->argp[1]))
	return RET_SYN;
    while (nxtitem(&ni, &pl)) {
	if (!player->owner)
	    continue;
	if (pl.pln_nuketype == -1)
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, &pl)) {
		pr("You cannot disarm %s while it is on the trading block!\n",
		   prplane(&pl));
		return RET_FAIL;
	    }
	}
	if (nuk_on_plane(&nuke, pl.pln_uid) < 0) {
	    CANT_REACH();
	    continue;
	}
	nuke.nuk_plane = -1;
	pl.pln_nuketype = -1;
	pl.pln_flags &= ~PLN_AIRBURST;
	putplane(pl.pln_uid, &pl);
	putnuke(nuke.nuk_uid, &nuke);
	pr("%s removed from %s and added to %s\n",
	   prnuke(&nuke), prplane(&pl),
	   xyas(pl.pln_x, pl.pln_y, player->cnum));
    }
    return RET_OK;
}
