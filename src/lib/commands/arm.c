/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  arm.c: Arm planes (missiles) with nuclear devices
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 2000
 */

#include <string.h>
#include "misc.h"
#include "player.h"
#include "sect.h"
#include "nuke.h"
#include "plane.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "commands.h"
#include "optlist.h"

int
arm(void)
{
    struct nchrstr *ncp;
    struct plchrstr *plc;
    struct plnstr pl;
    struct plnstr start;	/* Used for sanity checking */
    struct nukstr nuke;
    s_char *p;
    int i;
    int pno;
    int nuketype;
    int nukenum;
    int len;
    struct nstr_item ni;
    s_char buf[1024];
    int disarm = **player->argp == 'd';
    s_char *prompt = disarm ? "Disarm plane: " : "Arm plane: ";

    if (!(p = getstarg(player->argp[1], prompt, buf)) || !*p)
	return RET_SYN;
    pno = atoi(p);
    if (pno < 0 || !getplane(pno, &pl) || pl.pln_own != player->cnum)
	return RET_FAIL;
    memcpy(&start, &pl, sizeof(struct plnstr));
    plc = &plchr[(int)pl.pln_type];
    if ((plc->pl_flags & (P_O | P_M)) == (P_O | P_M)) {
	pr("A %s cannot carry nuclear devices!\n", plc->pl_name);
	return RET_FAIL;
    }
    if (opt_MARKET) {
	if (ontradingblock(EF_PLANE, (int *)&pl)) {
	    pr("You cannot arm/disarm an item on the trading block!\n");
	    return RET_FAIL;
	}
    }
    if (pl.pln_nuketype == -1) {
	if (disarm) {
	    pr("%s is not carrying any nuclear devices\n", prplane(&pl));
	    return RET_FAIL;
	}
	if ((p = getstarg(player->argp[2], "Device type: ", buf)) == 0)
	    return RET_SYN;
	if (!check_plane_ok(&start))
	    return RET_FAIL;
	len = strlen(p);
	for (i = 0, ncp = nchr; i < N_MAXNUKE; i++, ncp++) {
	    if (strncmp(ncp->n_name, p, len) == 0)
		break;
	}
	if (i >= N_MAXNUKE) {
	    pr("No such nuke type!\n");
	    return RET_SYN;
	}
	nuketype = i;
	nukenum = -1;
	snxtitem_all(&ni, EF_NUKE);
	while (nxtitem(&ni, (s_char *)&nuke)) {
	    if (nuke.nuk_own != player->cnum)
		continue;
	    if (nuke.nuk_x != pl.pln_x || nuke.nuk_y != pl.pln_y)
		continue;
	    nukenum = ni.cur;
	    break;
	}
	if (nukenum < 0) {
	    pr("You don't own any nukes in that sector.\n");
	    return RET_FAIL;
	}
	if (nuke.nuk_types[nuketype] == 0) {
	    pr("No nukes of that type in that sector.\n");
	    return RET_FAIL;
	}
	if (pl.pln_load < ncp->n_weight) {
	    pr("A %s cannot carry %s devices!\n", plc->pl_name,
	       ncp->n_name);
	    return RET_FAIL;
	}
	p = getstarg(player->argp[3], "Airburst [n]? ", buf);

	if (!check_plane_ok(&start))
	    return RET_FAIL;

	if (p && (*p == 'y' || *p == 'Y'))
	    pl.pln_flags |= PLN_AIRBURST;
	else
	    pl.pln_flags &= ~PLN_AIRBURST;

	pl.pln_nuketype = nuketype;
	nuk_delete(&nuke, nuketype, 1);
    } else if (!disarm) {
	pr("%s already carrying a warhead.\n", prplane(&pl));
    }
    if (disarm) {
	pr("%s warhead removed from %s and added to %s\n",
	   nchr[(int)pl.pln_nuketype].n_name,
	   prplane(&pl), xyas(pl.pln_x, pl.pln_y, player->cnum));
	nuk_add(pl.pln_x, pl.pln_y, pl.pln_nuketype, 1);
	pl.pln_nuketype = -1;
	pl.pln_flags &= ~PLN_AIRBURST;
    } else {
	pr("%s armed with a %s warhead.\n", prplane(&pl),
	   nchr[(int)pl.pln_nuketype].n_name);
	pr("Warhead on %s is programmed to %s\n",
	   prplane(&pl),
	   pl.pln_flags & PLN_AIRBURST ? "airburst" : "groundburst");
    }

    putplane(pl.pln_uid, &pl);
    return RET_OK;
}
