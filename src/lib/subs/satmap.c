/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  satmap.c: Do a satellite map given an x,y location, effic and other
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#include <config.h>

#include <stdlib.h>
#include "file.h"
#include "land.h"
#include "map.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

static char **rad;
static char *radbuf;

void
satmap(int x, int y, int eff, int range, int flags, int type)
{
    struct sctstr sect;
    struct shpstr ship;
    struct lndstr land;
    int count;
    struct nstr_item ni;
    struct nstr_sect ns;
    int rx, ry;
    int row;
    int n;
    int changed = 0;
    long crackle;
    signed char noise[100];
    char selection[1024];

    if (!eff)
	return;

    if (!radbuf)
	radbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!rad) {
	rad = malloc(WORLD_Y * sizeof(char *));
	if (rad && radbuf) {
	    for (rx = 0; rx < WORLD_Y; rx++)
		rad[rx] = &radbuf[(WORLD_X + 1) * rx];
	}
    }

    if (!radbuf || !rad) {
	pr("Memory error in satmap, tell the deity.\n");
	return;
    }

    range = range * (eff / 100.0);
    pr("%s efficiency %d%%, max range %d\n",
       xyas(x, y, player->cnum), eff, range);
    memset(noise, 0, sizeof(noise));
    if (eff < 100) {
	pr("Some noise on the transmission...\n");
	for (n = 0; n < (100 - eff); ++n)
	    noise[100 * n / (100 - eff)] = 1;
    }

    /* Have to convert to player coords, since it gets converted
       back from there */
    sprintf(selection, "@%s:%d", xyas(x, y, player->cnum), range);

    if (type == EF_BAD || type == EF_SECTOR) {
	if (type == EF_SECTOR)	/* Use ?conditionals */
	    snxtsct(&ns, selection);
	else
	    snxtsct_dist(&ns, x, y, range);

	blankfill(radbuf, &ns.range, 1);
	if (flags & P_S) {
	    pr("Satellite sector report\n");
	    prdate();
	    sathead();
	}
	crackle = count = 0;
	while (nxtsct(&ns, &sect)) {
	    if (++crackle == 100)
		crackle = 0;
	    if (noise[crackle])
		continue;
	    if (flags & P_S) {
		if (sect.sct_own && sect.sct_own != player->cnum) {
		    satdisp_sect(&sect, (flags & P_I) ? 5 : 50);
		    ++count;
		    if (opt_HIDDEN)
			setcont(player->cnum, sect.sct_own, FOUND_FLY);
		}
	    }
	    if ((flags & P_I) ||
		sect.sct_type == SCT_WATER || sect.sct_type == SCT_MOUNT) {
		rad[ns.dy][ns.dx] = dchr[sect.sct_type].d_mnem;
	    } else
		rad[ns.dy][ns.dx] = '?';
	    changed +=
		map_set(player->cnum, ns.x, ns.y, rad[ns.dy][ns.dx], 0);
	}
	if (changed)
	    writemap(player->cnum);
	if (flags & P_S)
	    pr("  %d sectors\n\n", count);
    }

    if ((type == EF_BAD || type == EF_SHIP) &&
	(flags & P_S || flags & P_I)) {
	if (type == EF_SHIP)
	    snxtitem(&ni, EF_SHIP, selection, NULL);
	else
	    snxtitem_dist(&ni, EF_SHIP, x, y, range);

	crackle = count = 0;
	if (flags & P_S) {
	    pr("Satellite ship report\n");
	    prdate();
	    pr(" own  shp# ship type                                  sector   eff\n");
	}
	while (nxtitem(&ni, &ship)) {
	    if (ship.shp_own == 0)
		continue;
	    if ((mchr[(int)ship.shp_type].m_flags & M_SUB) &&
		((flags & (P_S | P_I)) != (P_S | P_I)))
		continue;
	    if (++crackle == 100)
		crackle = 0;
	    if (noise[crackle])
		continue;
	    if (flags & P_S) {
		pr("%4d %4d %-16.16s %-25.25s ",
		   ship.shp_own, ship.shp_uid,
		   mchr[(int)ship.shp_type].m_name, ship.shp_name);
		prxy("%4d,%-4d ", ship.shp_x, ship.shp_y, player->cnum);
		pr("%3d%%\n", ship.shp_effic);
		++count;
		if (opt_HIDDEN)
		    setcont(player->cnum, ship.shp_own, FOUND_FLY);
	    }
	    /* If we are imaging *and* drawing the map */
	    if ((flags & P_I) && (type == EF_BAD)) {
		rx = deltx(&ns.range, ship.shp_x);
		ry = delty(&ns.range, ship.shp_y);
		/* &~0x20 makes it a cap letter */
		rad[ry][rx] = (*mchr[(int)ship.shp_type].m_name) & ~0x20;
	    }
	}
	if (flags & P_S)
	    pr("  %d ships\n\n", count);
    }

    if ((type == EF_BAD || type == EF_LAND) &&
	(flags & P_S || flags & P_I)) {
	if (type == EF_LAND)
	    snxtitem(&ni, EF_LAND, selection, NULL);
	else
	    snxtitem_dist(&ni, EF_LAND, x, y, range);

	crackle = count = 0;
	if (flags & P_S) {
	    pr("Satellite unit report\n");
	    prdate();
	    pr(" own  lnd# unit type         sector   eff\n");
	}
	while (nxtitem(&ni, &land)) {
	    if (land.lnd_own == 0)
		continue;
	    if (lchr[(int)land.lnd_type].l_flags & L_SPY)
		continue;
	    if (!chance(land.lnd_effic / 20.0))
		continue;
	    if (++crackle == 100)
		crackle = 0;
	    if (noise[crackle])
		continue;
	    if (flags & P_S) {
		pr("%4d %4d %-16.16s ",
		   land.lnd_own, land.lnd_uid,
		   lchr[(int)land.lnd_type].l_name);
		prxy("%4d,%-4d", land.lnd_x, land.lnd_y, player->cnum);
		pr("%3d%%\n", land.lnd_effic);
		++count;
		if (opt_HIDDEN)
		    setcont(player->cnum, land.lnd_own, FOUND_FLY);
	    }
	    /* If we are imaging *and* drawing the map */
	    if ((flags & P_I) && (type == EF_BAD)) {
		rx = deltx(&ns.range, land.lnd_x);
		ry = delty(&ns.range, land.lnd_y);
		/* &~0x20 makes it a cap letter */
		rad[ry][rx] = (*lchr[(int)land.lnd_type].l_name) & ~0x20;
	    }
	}
	if (flags & P_S)
	    pr("  %d units\n\n", count);
    }

    /* Ok, have we made the map?  If so, display it */
    if (type == EF_BAD) {
	/*
	 * print out the map
	 * We have to make the center a '0' for ve
	 * ve needs a garbage line to terminate the map
	 */
	rad[delty(&ns.range, y)][deltx(&ns.range, y)] = '0';

	pr("Satellite radar report\n");
	n = ns.range.height;
	for (row = 0; row < n; row++)
	    pr("%s\n", rad[row]);
	pr("\n(c) 1989 Imaginative Images Inc.\n");
    }
}

void
sathead(void)
{
    pr("                    sct rd  rl  def\n");
    pr("   sect   type own  eff eff eff eff  civ  mil  shl  gun iron  pet  food\n");
}

void
satdisp_sect(struct sctstr *sp, int acc)
{
    prxy("%4d,%-4d   ", sp->sct_x, sp->sct_y, player->cnum);
    pr("%c  %3d  %3d %3d %3d %3d %4d %4d %4d %4d %4d %4d %5d\n",
       dchr[sp->sct_type].d_mnem,
       sp->sct_own, roundintby((int)sp->sct_effic, acc / 2),
       roundintby((int)sp->sct_road, acc / 2),
       opt_RAILWAYS ? !!sct_rail_track(sp) : roundintby(sp->sct_rail, acc / 2),
       roundintby((int)sp->sct_defense, acc / 2),
       roundintby(sp->sct_item[I_CIVIL], acc),
       roundintby(sp->sct_item[I_MILIT], acc),
       roundintby(sp->sct_item[I_SHELL], acc),
       roundintby(sp->sct_item[I_GUN], acc),
       roundintby(sp->sct_item[I_IRON], acc),
       roundintby(sp->sct_item[I_PETROL], acc),
       roundintby(sp->sct_item[I_FOOD], acc));
    map_set(player->cnum, sp->sct_x, sp->sct_y, dchr[sp->sct_type].d_mnem,
	    0);
}

void
satdisp_units(coord x, coord y)
{
    int first;
    struct nstr_item ni;
    struct shpstr ship;
    struct lndstr land;

    snxtitem_xy(&ni, EF_SHIP, x, y);
    first = 1;
    while (nxtitem(&ni, &ship)) {
	if (ship.shp_own == 0)
	    continue;
	if (mchr[(int)ship.shp_type].m_flags & M_SUB)
	    continue;
	if (first) {
	    pr("\t own  shp# ship type                                  sector   eff\n");
	    first = 0;
	}
	pr("\t%4d %4d %-16.16s %-25.25s ",
	   ship.shp_own, ship.shp_uid,
	   mchr[(int)ship.shp_type].m_name, ship.shp_name);
	prxy("%4d,%-4d ", ship.shp_x, ship.shp_y, player->cnum);
	pr("%3d%%\n", ship.shp_effic);
    }

    if (!first)
	pr("\n");

    snxtitem_xy(&ni, EF_LAND, x, y);
    first = 1;

    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == 0)
	    continue;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    continue;
	if (lchr[(int)land.lnd_type].l_flags & L_SPY)
	    continue;
	if (!chance(land.lnd_effic / 20.0))
	    continue;

	if (first) {
	    pr("\t own  lnd# unit type       sector   eff\n");
	    first = 0;
	}

	pr("\t%4d %4d %-16.16s ",
	   land.lnd_own, land.lnd_uid, lchr[(int)land.lnd_type].l_name);
	prxy("%4d,%-4d ", land.lnd_x, land.lnd_y, player->cnum);
	pr("%3d%%\n", land.lnd_effic);
    }

    if (!first)
	pr("\n");
}
