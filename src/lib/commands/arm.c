/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  arm.c: Arm planes (missiles) with nuclear devices
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 2000
 *     Markus Armbruster, 2006-2011
 */

#include <config.h>

#include <stdio.h>
#include "commands.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"

int
arm(void)
{
    struct nchrstr *ncp;
    struct plchrstr *plc;
    struct plnstr pl;
    struct nukstr nuke;
    char *p;
    int nukno;
    struct nstr_item ni;
    char buf[1024];
    char prompt[128];

    if (!snxtitem(&ni, EF_PLANE, player->argp[1], NULL))
	return RET_SYN;
    while (nxtitem(&ni, &pl)) {
	if (!player->owner
	    && relations_with(pl.pln_own, player->cnum) != ALLIED)
	    continue;
	plc = &plchr[(int)pl.pln_type];
	if ((plc->pl_flags & (P_O | P_N | P_MAR))
	    || (plc->pl_flags & (P_M | P_F)) == (P_M | P_F)) {
	    pr("A %s cannot carry nuclear devices!\n", plc->pl_name);
	    return RET_FAIL;
	}
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, &pl)) {
		pr("You cannot arm %s while it is on the trading block!\n",
		   prplane(&pl));
		return RET_FAIL;
	    }
	}
	if (!getnuke(nuk_on_plane(&pl), &nuke)) {
	    sprintf(prompt, "Nuclear device for %s: ", prplane(&pl));
	    p = getstarg(player->argp[2], prompt, buf);
	    if (!p || !*p)
		return RET_SYN;
	    if (!check_plane_ok(&pl))
		return RET_FAIL;
	    nukno = atoi(p);
	    if (!getnuke(nukno, &nuke) || !player->owner)
		return RET_FAIL;
	}
	ncp = &nchr[nuke.nuk_type];
	if (pln_load(&pl) < ncp->n_weight) {
	    pr("A %s cannot carry %s devices!\n",
	       plc->pl_name, ncp->n_name);
	    return RET_FAIL;
	}
	p = getstarg(player->argp[3], "Airburst [n]? ", buf);
	if (!p)
	    return RET_FAIL;

	if (!check_plane_ok(&pl) || !check_nuke_ok(&nuke))
	    return RET_FAIL;

	if (nuke.nuk_plane >= 0 && nuke.nuk_plane != pl.pln_uid) {
	    pr("%s is already armed on plane #%d!\n",
	       prnuke(&nuke), nuke.nuk_plane);
	    return RET_FAIL;
	}
	if (nuke.nuk_x != pl.pln_x || nuke.nuk_y != pl.pln_y) {
	    pr("%s isn't in the same sector as %s!\n",
	       prnuke(&nuke), prplane(&pl));
	    return RET_FAIL;
	}

	if (*p == 'y' || *p == 'Y')
	    pl.pln_flags |= PLN_AIRBURST;
	else
	    pl.pln_flags &= ~PLN_AIRBURST;
	pl.pln_mission = 0;

	snprintf(buf, sizeof(buf), "armed on your %s in %s",
		 prplane(&pl), xyas(pl.pln_x, pl.pln_y, pl.pln_own));
	gift(pl.pln_own, player->cnum, &nuke, buf);
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
    struct sctstr sect;
    char buf[128];

    if (!snxtitem(&ni, EF_PLANE, player->argp[1], NULL))
	return RET_SYN;
    while (nxtitem(&ni, &pl)) {
	if (!player->owner)
	    continue;
	if (!getnuke(nuk_on_plane(&pl), &nuke))
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, &pl)) {
		pr("You cannot disarm %s while it is on the trading block!\n",
		   prplane(&pl));
		return RET_FAIL;
	    }
	}
	getsect(nuke.nuk_x, nuke.nuk_y, &sect);
	if (!player->owner
	    && relations_with(sect.sct_own, player->cnum) != ALLIED) {
	    pr("Disarming %s in sector %s requires an alliance!\n",
	       prplane(&pl), xyas(sect.sct_x, sect.sct_y, player->cnum));
	    continue;
	}
	snprintf(buf, sizeof(buf), "unloaded in your %s at %s",
		 dchr[sect.sct_type].d_name,
		 xyas(sect.sct_x, sect.sct_y, sect.sct_own));
	gift(sect.sct_own, player->cnum, &nuke, buf);
	nuke.nuk_plane = -1;
	pl.pln_flags &= ~PLN_AIRBURST;
	putplane(pl.pln_uid, &pl);
	putnuke(nuke.nuk_uid, &nuke);
	pr("%s removed from %s and added to %s\n",
	   prnuke(&nuke), prplane(&pl),
	   xyas(pl.pln_x, pl.pln_y, player->cnum));
    }
    return RET_OK;
}
