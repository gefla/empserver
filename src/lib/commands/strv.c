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
 *  star.c: Do a starvation report
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "deity.h"
#include "path.h"
#include "file.h"
#include <math.h>
#include "item.h"
#include "ship.h"
#include "optlist.h"
#include "land.h"
#include "commands.h"

static void starv_sects(s_char *range);
static void starv_ships(s_char *range);
static void starv_units(s_char *range);
static void sect_hdr(void);
static void ship_hdr(void);
static void unit_hdr(void);

int
starve(void)
{
    struct nstr_sect nstr;
    int do_sects = 0;
    int do_ships = 0;
    int do_units = 0;
    s_char range[1024];

    if (opt_NOFOOD) {		/* no food - no work! */
	pr("No food is required in this game\n");
	return RET_OK;
    }

    strcpy(range, "*");
    if (!player->argp[1]) {
	do_sects = do_ships = do_units = 1;
    } else if (*(player->argp[1]) == 's') {
	do_ships = 1;
	if (player->argp[2])
	    strcpy(range, player->argp[2]);
    } else if (*(player->argp[1]) == 'l') {
	do_units = 1;
	if (player->argp[2])
	    strcpy(range, player->argp[2]);
    } else {
	if (!snxtsct(&nstr, player->argp[1]))
	    return RET_SYN;
	do_sects = 1;
	strcpy(range, player->argp[1]);
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
starv_sects(s_char *range)
{
    struct nstr_sect nstr;
    struct sctstr sect;
    int nsect = 0;
    int vec[I_MAX + 1], s, needed;

    if (!snxtsct(&nstr, range))
	return;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (sect.sct_type == SCT_SANCT)
	    continue;

	getvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
/* This next 2 lines were added to overcompensate for the needy */
	if (vec[I_FOOD])
	    vec[I_FOOD]--;
	s = feed_people(vec, etu_per_update, &needed);

	if (s == 0)
	    continue;
	if (nsect++ == 0)
	    sect_hdr();
	if (player->god)
	    pr("%3d ", sect.sct_own);
	prxy("%4d,%-4d", nstr.x, nstr.y, player->cnum);
	pr(" %c", dchr[sect.sct_type].d_mnem);
	pr(" %c", (sect.sct_own != sect.sct_oldown ? '*' : ' '));
	if (sect.sct_newtype != sect.sct_type)
	    pr("%c", dchr[sect.sct_newtype].d_mnem);
	else
	    pr(" ");
	pr("%4d%%", sect.sct_effic);
	pr(" will starve %d people. %d more food needed\n", s, needed);
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
starv_ships(s_char *range)
{
    struct nstr_item ni;
    struct shpstr ship;
    int nship = 0;
    int vec[I_MAX + 1], s, needed;

    if (!snxtitem(&ni, EF_SHIP, range))
	return;

    while (nxtitem(&ni, (s_char *)&ship)) {
	if (!player->owner || !ship.shp_own)
	    continue;

	getvec(VT_ITEM, vec, (s_char *)&ship, EF_SHIP);
	s = feed_ship(&ship, vec, etu_per_update, &needed, 0);

	if (s == 0)
	    continue;
	if (nship++ == 0)
	    ship_hdr();
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%5d ", ship.shp_uid);
	pr("%-16.16s ", mchr[(int)ship.shp_type].m_name);
	pr(" will starve %d people. %d more food needed\n", s, needed);
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
starv_units(s_char *range)
{
    struct nstr_item ni;
    struct lndstr land;
    int nunit = 0;
    int vec[I_MAX + 1], s, needed;

    if (!snxtitem(&ni, EF_LAND, range))
	return;

    while (nxtitem(&ni, (s_char *)&land)) {
	if (!player->owner || !land.lnd_own)
	    continue;

	getvec(VT_ITEM, vec, (s_char *)&land, EF_LAND);
	s = feed_land(&land, vec, etu_per_update, &needed, 0);

	if (s == 0)
	    continue;
	if (nunit++ == 0)
	    unit_hdr();
	if (player->god)
	    pr("%3d ", land.lnd_own);
	pr("%5d ", land.lnd_uid);
	pr("%-16.16s ", lchr[(int)land.lnd_type].l_name);
	pr(" will starve %d mil. %d more food needed\n", s, needed);
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
