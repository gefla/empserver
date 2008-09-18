/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  star.c: Do a starvation report
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include <math.h>
#include "commands.h"
#include "item.h"
#include "land.h"
#include "optlist.h"
#include "path.h"
#include "ship.h"

static void starv_sects(char *range);
static void starv_ships(char *range);
static void starv_units(char *range);
static void sect_hdr(void);
static void ship_hdr(void);
static void unit_hdr(void);

int
starve(void)
{
    int do_sects = 0;
    int do_ships = 0;
    int do_units = 0;
    char *range;

    if (opt_NOFOOD) {		/* no food - no work! */
	pr("No food is required in this game\n");
	return RET_OK;
    }

    range = "*";
    if (!player->argp[1]) {
	do_sects = do_ships = do_units = 1;
    } else if (*(player->argp[1]) == 's') {
	do_ships = 1;
	if (player->argp[2])
	    range = player->argp[2];
    } else if (*(player->argp[1]) == 'l') {
	do_units = 1;
	if (player->argp[2])
	    range = player->argp[2];
    } else {
	do_sects = 1;
	range = player->argp[1];
    }
    player->simulation = 1;
    prdate();
    if (do_sects)
	starv_sects(range);
    if (do_ships)
	starv_ships(range);
    if (do_units)
	starv_units(range);
    player->simulation = 0;
    return RET_OK;
}

static void
starv_people(short *vec, int victims)
{
    pr(" will starve %d people. %.0f more food needed\n", victims,
       ceil(food_needed(vec, etu_per_update) - vec[I_FOOD]));
}

static void
starv_sects(char *range)
{
    struct nstr_sect nstr;
    struct sctstr sect;
    int nsect = 0;
    int s, save;

    if (!snxtsct(&nstr, range))
	return;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (sect.sct_type == SCT_SANCT)
	    continue;

	/*
	 * Check for starvation.  Suppress complaints about tiny
	 * population without food by adding 1f just for the check.
	 * That's okay because growfood() will grow at least that much
	 * anyway.
	 */
	save = sect.sct_item[I_FOOD];
	if (sect.sct_item[I_FOOD] == 0)
	    sect.sct_item[I_FOOD] = 1;
	s = famine_victims(sect.sct_item, etu_per_update);
	sect.sct_item[I_FOOD] = save;
	if (s == 0)
	    continue;

	if (nsect++ == 0)
	    sect_hdr();
	if (player->god)
	    pr("%3d ", sect.sct_own);
	prxy("%4d,%-4d", nstr.x, nstr.y, player->cnum);
	pr(" %c", dchr[sect.sct_type].d_mnem);
	pr(" %c", sect.sct_own != sect.sct_oldown ? '*' : ' ');
	if (sect.sct_newtype != sect.sct_type)
	    pr("%c", dchr[sect.sct_newtype].d_mnem);
	else
	    pr(" ");
	pr("%4d%%", sect.sct_effic);
	starv_people(sect.sct_item, s);
    }
    if (nsect == 0) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return;
}

static void
sect_hdr(void)
{
    if (player->god)
	pr("    ");
    pr("Starvation               \n");
    if (player->god)
	pr("own ");
    pr("  sect         eff ");
    pr("\n");
}

static void
starv_ships(char *range)
{
    struct nstr_item ni;
    struct shpstr ship;
    int nship = 0;
    int s;

    if (!snxtitem(&ni, EF_SHIP, range, NULL))
	return;

    while (nxtitem(&ni, &ship)) {
	if (!player->owner || !ship.shp_own)
	    continue;

	s = famine_victims(ship.shp_item, etu_per_update);
	if (s == 0)
	    continue;
	if (nship++ == 0)
	    ship_hdr();
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%5d ", ship.shp_uid);
	pr("%-16.16s ", mchr[(int)ship.shp_type].m_name);
	starv_people(ship.shp_item, s);
    }
    if (nship == 0) {
	if (range)
	    pr("%s: No ship(s)\n", range);
	else
	    pr("%s: No ship(s)\n", "");
	return;
    } else
	pr("%d ship%s\n", nship, splur(nship));
    return;
}

static void
ship_hdr(void)
{
    if (player->god)
	pr("    ");
    pr("Starvation               \n");
    if (player->god)
	pr("own ");
    pr(" shp#     ship type       \n");
}

static void
starv_units(char *range)
{
    struct nstr_item ni;
    struct lndstr land;
    int nunit = 0;
    int s;

    if (!snxtitem(&ni, EF_LAND, range, NULL))
	return;

    while (nxtitem(&ni, &land)) {
	if (!player->owner || !land.lnd_own)
	    continue;

	s = famine_victims(land.lnd_item, etu_per_update);
	if (s == 0)
	    continue;
	if (nunit++ == 0)
	    unit_hdr();
	if (player->god)
	    pr("%3d ", land.lnd_own);
	pr("%5d ", land.lnd_uid);
	pr("%-16.16s ", lchr[(int)land.lnd_type].l_name);
	starv_people(land.lnd_item, s);
    }
    if (nunit == 0) {
	if (range)
	    pr("%s: No unit(s)\n", range);
	else
	    pr("%s: No unit(s)\n", "");
	return;
    } else
	pr("%d unit%s\n", nunit, splur(nunit));
    return;
}

static void
unit_hdr(void)
{
    if (player->god)
	pr("    ");
    pr("Starvation               \n");
    if (player->god)
	pr("own ");
    pr(" lnd#     unit type       \n");
}
