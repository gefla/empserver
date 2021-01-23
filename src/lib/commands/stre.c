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
 *  stre.c: Calculate military strengths of sectors
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Markus Armbruster, 2004-2010
 */

#include <config.h>

#include "combat.h"
#include "commands.h"
#include "land.h"
#include "ship.h"

static double units_in_sector(struct combat *def);
static void stre_hdr(void);

int
stre(void)
{
    struct sctstr sect;
    int nsect = 0;
    struct nstr_sect nstr;
    double dtotal, r_total, eff;
    struct combat def[1];
    int dummy;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    prdate();
    nsect = 0;
    att_combat_init(def, EF_SECTOR);
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (nsect++ == 0)
	    stre_hdr();
	if (player->god)
	    pr("%3d ", sect.sct_own);
	prxy("%4d,%-4d", nstr.x, nstr.y);
	pr(" %c", dchr[sect.sct_type].d_mnem);
	pr("%4d%%", sect.sct_effic);
	def->x = nstr.x;
	def->y = nstr.y;
	def->set = 0;
	att_get_combat(def, 1);
	if (def->mil)
	    pr("%5d", def->mil);
	else
	    pr("%5s", "");
	dtotal = units_in_sector(def);
	if (dtotal > 0)
	    pr("%7d", (int)dtotal);
	else
	    pr("%7s", "");

	if (def->sct_type != SCT_MOUNT)
	    r_total = att_reacting_units(def, NULL, 0, &dummy, 9999999);
	else
	    r_total = 0.0;
	def->own = 0;
	eff = att_combat_eff(def);
	if (sect.sct_own == sect.sct_oldown || player->god) {
	    if (SCT_LANDMINES(&sect) > 0) {
		pr("%7d", sect.sct_mines);
		eff *= (1.0 + MIN(sect.sct_mines, 20) * 0.02);
	    } else
		pr("%7s", "");
	} else {
	    pr("%7s", "?");
	}
	pr("%6.2f", eff);
	pr("%9d", (int)((dtotal + def->mil) * eff));
	if (r_total > 0)
	    pr(" %9d", (int)(r_total * eff));
	else
	    pr(" %9s", "");
	pr("%9d\n", (int)((dtotal + def->mil + r_total) * eff));
	/*
	 * This command is quite compute intensive.  Yield the
	 * processor after every sector, to keep the game responsive
	 * for other players.
	 */
	empth_yield();
    }
    if (!nsect) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return 0;
}

static double
units_in_sector(struct combat *def)
{
    double d_unit;
    double dtotal = 0.0;
    struct nstr_item ni;
    struct lndstr land;

    snxtitem_xy(&ni, EF_LAND, def->x, def->y);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == 0)
	    continue;
	if (land.lnd_own != def->own)
	    continue;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    continue;
	d_unit = defense_val(&land);
	if (!lnd_could_be_supplied(&land))
	    d_unit /= 2.0;
	dtotal += d_unit;
    }
    return dtotal;
}

static void
stre_hdr(void)
{
    if (player->god)
	pr("    ");
    pr("DEFENSE STRENGTH               land  sect   sector  reacting    total\n");
    if (player->god)
	pr("own ");
    pr("  sect       eff  mil  units  mines  mult  defense     units  defense\n");
}
