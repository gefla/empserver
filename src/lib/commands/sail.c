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
 *  sail.c: Set sail path for leaders
 * 
 *  Known contributors to this file:
 *     Robert Forsman
 */

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "var.h"
#include "ship.h"
#include "path.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "deity.h"
#include "commands.h"
#include "optlist.h"

static int
show_sail(struct nstr_item *nstr)
{
    register int count = 0;
    struct shpstr ship;

    while (nxtitem(nstr, (s_char *)&ship)) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (ship.shp_type < 0 || ship.shp_type > shp_maxno) {
	    pr("bad ship type %d (#%d)\n", ship.shp_type, nstr->cur);
	    continue;
	}
	if (count++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("shp#     ship type       x,y    ");
	    pr("mobil mobquota follows path\n");
	}
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%4d ", ship.shp_uid);
	pr("%-16.16s ", mchr[(int)ship.shp_type].m_name);
	prxy("%4d,%-4d ", ship.shp_x, ship.shp_y, player->cnum);
	pr("%3d  ", ship.shp_mobil);
	pr("   %3d     ", ship.shp_mobquota);
	pr("   %3d   ", ship.shp_follow);
	if (ship.shp_path[0]) {
	    pr(ship.shp_path);
	} else if ((ship.shp_autonav & AN_AUTONAV)) {
	    pr("Has orders");
	}
	pr("\n");
	if (opt_SHIPNAMES) {
	    if (ship.shp_name[0] != 0) {
		if (player->god)
		    pr("    ");
		pr("       %s\n", ship.shp_name);
	    }
	}
    }
    if (count == 0) {
	if (player->argp[1])
	    pr("%s: No ship(s)\n", player->argp[1]);
	else
	    pr("%s: No ship(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d ship%s\n", count, splur(count));
    return RET_OK;
}

static int
cmd_unsail_ship(struct nstr_item *nstr)
{
    struct shpstr ship;
    int count = 0;

    while (nxtitem(nstr, (s_char *)&ship)) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (ship.shp_type < 0 || ship.shp_type > shp_maxno) {
	    pr("bad ship type %d (#%d)\n", ship.shp_type, nstr->cur);
	    continue;
	}
	if (ship.shp_path[0]) {
	    pr("Ship #%d unsailed\n", ship.shp_uid);
	    count++;
	    ship.shp_path[0] = 0;
	    putship(ship.shp_uid, &ship);
	}
    }
    return RET_OK;
}

static int
cmd_sail_ship(struct nstr_item *nstr)
{
    s_char *cp;
    struct shpstr ship;
    char navpath[MAX_PATH_LEN];

    while (!player->aborted && nxtitem(nstr, (s_char *)&ship)) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (ship.shp_type < 0 || ship.shp_type > shp_maxno) {
	    pr("bad ship type %d (#%d)\n", ship.shp_type, nstr->cur);
	    continue;
	}
	if ((ship.shp_autonav & AN_AUTONAV) &&
	    !(ship.shp_autonav & AN_STANDBY)) {
	    pr("Ship #%d has other orders!\n", ship.shp_uid);
	    continue;
	}

	pr("Ship #%d at %s\n", ship.shp_uid,
	   xyas(ship.shp_x, ship.shp_y, ship.shp_own));
	cp = getpath(navpath, player->argp[2],
		     ship.shp_x, ship.shp_y, 0, 0, 0, P_SAILING);
	if (!check_ship_ok(&ship))
	    continue;
	if (!player->aborted) {
	    bzero(ship.shp_path, sizeof(ship.shp_path));
	    strncpy(ship.shp_path, cp, sizeof(ship.shp_path) - 2);
	    ship.shp_mission = 0;
	    putship(ship.shp_uid, &ship);
	}
    }
    return RET_OK;
}

int
sail(void)
{
    s_char *cp;
    struct nstr_item nstr;

    if (!opt_SAIL) {
	pr("The SAIL option is not enabled, so this command is not valid.\n");
	return RET_FAIL;
    }
    if (!snxtitem(&nstr, EF_SHIP, player->argp[1]))
	return RET_SYN;
    cp = player->argp[2];
    if ((*player->argp[0] == 'q') /*qsail command */ ||(cp && *cp == 'q')) {
	return (show_sail(&nstr));
    } else if (*player->argp[0] == 'u'	/*unsail command */
	       || (cp && *cp == '-')) {
	return (cmd_unsail_ship(&nstr));
    } else
	return (cmd_sail_ship(&nstr));
}
