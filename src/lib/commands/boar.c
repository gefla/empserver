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
 *  boar.c: Board an enemy ship
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#include <config.h>

#include "combat.h"
#include "commands.h"
#include "land.h"
#include "retreat.h"
#include "ship.h"

int
boar(void)
{
    struct combat off[1];	/* boarding ship or sector */
    struct combat def[1];	/* defending ship */
    struct emp_qelem olist;	/* boarding units */
    struct emp_qelem dlist;	/* defending units */
    int ototal;			/* total boarding strength */
    int a_engineer = 0;		/* boarder engineers are present */
    int a_spy = 0;		/* the best boarder scout */
    struct shpstr ship;		/* for retreating */
    struct sctstr sect;
    struct lndstr land;
    struct nstr_item ni;
    int foundland;
    char *p;
    char buf[1024];

    att_combat_init(def, EF_SHIP);
    /*
     * Collect input from the boarder
     */

    /* What are we boarding? */
    p = getstarg(player->argp[1], "Victim ship #?  ", buf);
    if (!p || (def->shp_uid = atoi(p)) < 0)
	return RET_SYN;

    /*
     * Ask the boarder what he wants to board with
     */

    if (!(p = getstarg(player->argp[2], "Boarding party from? ", buf)))
	return RET_SYN;
    if (issector(p)) {
	att_combat_init(off, EF_SECTOR);
	if (!sarg_xy(p, &off->x, &off->y))
	    return RET_SYN;
	getsect(off->x, off->y, &sect);
	if (sect.sct_own != player->cnum) {
	    pr("You don't own %s!\n", xyas(off->x, off->y, player->cnum));
	    return RET_SYN;
	}
	if (sect.sct_mobil <= 0) {
	    /* Look for land units with mobility */
	    snxtitem_xy(&ni, EF_LAND, off->x, off->y);
	    foundland = 0;
	    while (nxtitem(&ni, &land)) {
		if (land.lnd_own != player->cnum)
		    continue;
		if (land.lnd_ship >= 0 || land.lnd_land >= 0)
		    continue;
		if (land.lnd_mobil <= 0)
		    continue;
		/* Only land units with assault can board */
		if (!(lchr[(int)land.lnd_type].l_flags & L_ASSAULT))
		    continue;
		foundland = 1;
	    }
	    if (!foundland) {
		pr("You don't have any mobility (sector or land units) in %s!\n",
		   xyas(off->x, off->y, player->cnum));
		return RET_SYN;
	    }
	}
    } else {
	att_combat_init(off, EF_SHIP);
	if ((off->shp_uid = atoi(p)) < 0)
	    return RET_SYN;
    }
    if (att_abort(A_BOARD, off, def)) {
	pr("Board aborted\n");
	return RET_OK;
    }

    /* Fire at the attacking ship */

    att_approach(off, def);
    if (att_abort(A_BOARD, off, def)) {
	pr("Board aborted\n");
	att_empty_attack(A_BOARD, 0, def);
	return RET_OK;
    }

    /* Show what we're boarding, and check treaties */

    if (att_show(def))
	return RET_FAIL;

    /* Ask the player what he wants to board with */

    att_ask_offense(A_BOARD, off, def, &olist, &a_spy, &a_engineer);
    if (att_abort(A_BOARD, off, def)) {
	pr("Board aborted\n");
	att_empty_attack(A_BOARD, 0, def);
	return att_free_lists(&olist, NULL);
    }

    ototal = att_get_offense(A_BOARD, off, &olist, def);
    if (att_abort(A_BOARD, off, def)) {
	pr("Board aborted\n");
	att_empty_attack(A_BOARD, 0, def);
	return att_free_lists(&olist, NULL);
    }

    /*
     * We have now got all the answers from the boarder.  From this point
     * forward, we can assume that this battle is the _only_ thing
     * happening in the game.
     */

    /* Get the real defense */

    att_get_defense(&olist, def, &dlist, a_spy, ototal);

    /*
     * Death, carnage, and destruction.
     */

    if (!(att_fight(A_BOARD, off, &olist, 1.0, def, &dlist, 1.0))) {
	getship(def->shp_uid, &ship);
	if (ship.shp_rflags & RET_BOARDED) {
	    retreat_ship(&ship, 'u');
	    putship(def->shp_uid, &ship);
	}
    }

    return RET_OK;
}
