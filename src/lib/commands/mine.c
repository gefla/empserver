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
 *  mine.c: Lay mines from ships or units
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "ship.h"
#include "land.h"
#include "sect.h"
#include "nat.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

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

    if (!snxtitem(&ni, EF_SHIP, player->argp[1]))
	return RET_SYN;
    mines = onearg(player->argp[2],
		   "Drop how many mines from each ship?  ");
    if (mines <= 0)
	return RET_SYN;
    while (nxtitem(&ni, (s_char *)&ship)) {
	if (!player->owner)
	    continue;
	mp = &mchr[(int)ship.shp_type];
	if ((mp->m_flags & M_MINE) == 0)
	    continue;
	if ((shells = ship.shp_item[I_SHELL]) == 0)
	    continue;
	mines_avail = min(shells, mines);
	if (getsect(ship.shp_x, ship.shp_y, &sect) == 0 ||
	    (sect.sct_type != SCT_WATER && sect.sct_type != SCT_BSPAN)) {
	    pr("You can't lay mines there!!\n");
	    continue;
	}
	sect.sct_mines = min(sect.sct_mines + mines_avail, MINES_MAX);
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
    struct lchrstr *lp;
    struct nstr_item ni;
    int shells;
    int mines_wanted;
    int mines_laid;
    int total_mines_laid;
    s_char prompt[128];

    if (!snxtitem(&ni, EF_LAND, player->argp[1]))
	return RET_SYN;
    while (nxtitem(&ni, (s_char *)&land)) {
	if (!player->owner)
	    continue;
	lp = &lchr[(int)land.lnd_type];
	if (!(lp->l_flags & L_ENGINEER))
	    continue;
	if (land.lnd_mobil < 1) {
	    pr("Unit %d is out of mobility\n", land.lnd_uid);
	    continue;
	}
	resupply_commod(&land, I_SHELL);
	putland(land.lnd_uid, &land);
	if (!(shells = land.lnd_item[I_SHELL]))
	    continue;
	shells = min(shells, land.lnd_mobil);
	if (!getsect(land.lnd_x, land.lnd_y, &sect) ||
	    sect.sct_type == SCT_WATER || sect.sct_type == SCT_BSPAN) {
	    pr("You can't lay mines there!!\n");
	    continue;
	}
	if (sect.sct_own == sect.sct_oldown)
	    pr("There are currently %d mines in %s\n",
	       sect.sct_mines, xyas(sect.sct_x, sect.sct_y, player->cnum));
	sprintf(prompt, "Drop how many mines from %s?  ", prland(&land));
	mines_wanted = onearg(player->argp[2], prompt);
	if (!check_land_ok(&land))
	    continue;
	if (mines_wanted <= 0)
	    continue;
	land.lnd_mission = 0;
	total_mines_laid = 0;
	while (shells > 0 && total_mines_laid < mines_wanted) {
	    mines_laid = min(shells, mines_wanted - total_mines_laid);
	    land.lnd_item[I_SHELL] = shells - mines_laid;
	    land.lnd_mobil -= mines_laid;
	    putland(land.lnd_uid, &land);
	    resupply_commod(&land, I_SHELL);	/* Get more shells */
	    putland(land.lnd_uid, &land);
	    total_mines_laid += mines_laid;
	    shells = land.lnd_item[I_SHELL];
	    shells = min(shells, land.lnd_mobil);
	}
	getsect(sect.sct_x, sect.sct_y, &sect);
	sect.sct_mines = min(sect.sct_mines + total_mines_laid, MINES_MAX);
	putsect(&sect);
	if (total_mines_laid == mines_wanted) {
	    pr("%s laid a total of %d mines in %s",
	       prland(&land), total_mines_laid,
	       xyas(sect.sct_x, sect.sct_y, land.lnd_own));
	    if (!shells)
	    	pr(" but is now out of supply\n");
	    else
		pr("\n");
	} else
	    pr("%s ran out of %s before it could finish the job\nOnly %d mines were laid in %s\n", prland(&land), land.lnd_mobil > 0 ? "supply" : "mobility", total_mines_laid, xyas(sect.sct_x, sect.sct_y, land.lnd_own));
    }
    return RET_OK;
}
