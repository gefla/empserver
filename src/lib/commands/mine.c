/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mine.c: Lay mines from ships or units
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2010
 */

#include <config.h>

#include "commands.h"
#include "land.h"
#include "map.h"
#include "ship.h"

/*
 * format: mine <SHIPS> <NUMBER MINES>
 */
int
mine(void)
{
    struct shpstr ship;
    struct sctstr sect;
    struct mchrstr *mp;
    struct nstr_item ni;
    int mines;
    int shells;
    int mines_avail;

    if (!snxtitem(&ni, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;
    mines = onearg(player->argp[2],
		   "Drop how many mines from each ship? ");
    if (mines <= 0)
	return RET_SYN;
    while (nxtitem(&ni, &ship)) {
	if (!player->owner)
	    continue;
	mp = &mchr[(int)ship.shp_type];
	if ((mp->m_flags & M_MINE) == 0)
	    continue;
	if ((shells = ship.shp_item[I_SHELL]) == 0)
	    continue;
	mines_avail = MIN(shells, mines);
	if (getsect(ship.shp_x, ship.shp_y, &sect) == 0 ||
	    sect.sct_type != SCT_WATER) {
	    pr("You can't lay mines there!!\n");
	    continue;
	}
	sect.sct_mines = MIN(sect.sct_mines + mines_avail, MINES_MAX);
	ship.shp_item[I_SHELL] = shells - mines_avail;
	putsect(&sect);
	ship.shp_mission = 0;
	putship(ship.shp_uid, &ship);
	pr("Laying %d mines from %s\n", mines_avail, prship(&ship));
	if (mines_avail &&
	    map_set(player->cnum, sect.sct_x, sect.sct_y, 'X', 0))
	    writemap(player->cnum);
    }
    return RET_OK;
}

/*
 * format: landmine <UNITS> <NUMBER MINES>
 */
int
landmine(void)
{
    struct lndstr land;
    struct sctstr sect;
    struct nstr_item ni;
    int todo;
    int mines_wanted;
    int mines_laid;
    int total_mines_laid;
    char prompt[128];

    if (!snxtitem(&ni, EF_LAND, player->argp[1], NULL))
	return RET_SYN;
    while (nxtitem(&ni, &land)) {
	if (!player->owner)
	    continue;
	if (!(lchr[land.lnd_type].l_flags & L_ENGINEER))
	    continue;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0) {
	    pr("%s is on a %s\n", prland(&land),
	       land.lnd_ship >= 0 ? "ship" : "land unit");
	    continue;
	}
	if (land.lnd_mobil < 1) {
	    pr("%s is out of mobility\n", prland(&land));
	    continue;
	}
	if (!getsect(land.lnd_x, land.lnd_y, &sect)
	    || SCT_MINES_ARE_SEAMINES(&sect)
	    || sect.sct_own != land.lnd_own) {
	    pr("You can't lay mines there!!\n");
	    continue;
	}
	if (sect.sct_own == sect.sct_oldown)
	    pr("There are currently %d mines in %s\n",
	       sect.sct_mines, xyas(sect.sct_x, sect.sct_y, player->cnum));
	sprintf(prompt, "Drop how many mines from %s? ", prland(&land));
	mines_wanted = onearg(player->argp[2], prompt);
	if (mines_wanted < 0)
	    return RET_FAIL;
	if (mines_wanted == 0)
	    continue;
	if (!check_land_ok(&land) || !check_sect_ok(&sect))
	    continue;
	land.lnd_mission = 0;
	todo = MIN(mines_wanted, land.lnd_mobil);
	total_mines_laid = 0;
	do {
	    lnd_supply(&land, I_SHELL, todo);
	    mines_laid = MIN(todo, land.lnd_item[I_SHELL]);
	    land.lnd_item[I_SHELL] -= mines_laid;
	    land.lnd_mobil -= mines_laid;
	    putland(land.lnd_uid, &land);
	    total_mines_laid += mines_laid;
	    todo -= mines_laid;
	} while (todo && mines_laid);
	lnd_supply_all(&land);
	getsect(sect.sct_x, sect.sct_y, &sect);
	sect.sct_mines = MIN(sect.sct_mines + total_mines_laid, MINES_MAX);
	putsect(&sect);
	if (total_mines_laid == mines_wanted) {
	    pr("%s laid a total of %d mines in %s",
	       prland(&land), total_mines_laid,
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	    if (!land.lnd_item[I_SHELL])
		pr(" but is now out of shells\n");
	    else
		pr("\n");
	} else
	    pr("%s ran out of %s before it could finish the job\n"
	       "Only %d mines were laid in %s\n",
	       prland(&land),
	       land.lnd_mobil > 0 ? "shells" : "mobility",
	       total_mines_laid,
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
    }
    return RET_OK;
}
