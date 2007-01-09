/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  atta.c: Attack another sector
 * 
 *  Known contributors to this file:
 *     Dave Pare
 *     Ken Stevens, 1995
 *     Steve McClure, 1996
 */

#include <config.h>

#include "combat.h"
#include "commands.h"
#include "land.h"
#include "mission.h"
#include "path.h"
#include "ship.h"

int
atta(void)
{
    struct combat off[6];	/* attacking sectors */
    struct combat def[1];	/* defending sector */
    coord newx, newy;
    struct sctstr sect;
    int fort_sup, ship_sup, land_sup, plane_sup;
    struct emp_qelem olist;	/* attacking units */
    struct emp_qelem dlist;	/* defending units */
    int ototal;			/* total attacking strength */
    int a_engineer = 0;		/* attacker engineers are present */
    int a_spy = 0;		/* the best attacker scout */
    double osupport = 1.0;	/* attack support */
    double dsupport = 1.0;	/* defense support */
    int last, n;
    char *p;
    char buf[1024];
    int rel;

    att_combat_init(def, EF_SECTOR);
    /*
     * Collect input from the attacker
     */

    /* What are we attacking? */

    if (!(p = getstarg(player->argp[1], "Sector :  ", buf)))
	return RET_SYN;
    if (!sarg_xy(p, &def->x, &def->y))
	return RET_SYN;
    if (att_abort(A_ATTACK, 0, def))
	return RET_FAIL;

    /* Show what we're attacking, and check treaties */

    if (att_show(def))
	return RET_FAIL;

    /* Ask about offensive support */

    att_ask_support(2, &fort_sup, &ship_sup, &land_sup, &plane_sup);
    if (att_abort(A_ATTACK, 0, def)) {
	att_empty_attack(A_ATTACK, 0, def);
	return RET_OK;
    }

    /* initialize the off[] array */

    for (n = 0, last = -1; n < 6; ++n) {	/* Directions */
	newx = def->x + diroff[n + 1][0];
	newy = def->y + diroff[n + 1][1];
	getsect(newx, newy, &sect);	/* incase cross world boundary */
	rel = getrel(getnatp(sect.sct_own), player->cnum);
	if (!player->owner && rel != ALLIED)
	    continue;
	att_combat_init(&off[++last], EF_SECTOR);
	off[last].x = sect.sct_x;
	off[last].y = sect.sct_y;
    }
    off->last = last;

    /* Ask the player what he wants to attack with */

    att_ask_offense(A_ATTACK, off, def, &olist, &a_spy, &a_engineer);
    if (att_abort(A_ATTACK, off, def)) {
	pr("Attack aborted\n");
	att_empty_attack(A_ATTACK, 0, def);
	return att_free_lists(&olist, 0);
    }

    ototal = att_get_offense(A_ATTACK, off, &olist, def);
    if (att_abort(A_ATTACK, off, def)) {
	pr("Attack aborted\n");
	att_empty_attack(A_ATTACK, 0, def);
	return att_free_lists(&olist, 0);
    }

    /*
     * We have now got all the answers from the attacker.  From this point
     * forward, we can assume that this battle is the _only_ thing
     * happening in the game.
     */

    /* Get the real defense */

    att_get_defense(&olist, def, &dlist, a_spy, ototal);

    /* Get attacker and defender support */

    att_get_support(A_ATTACK, fort_sup, ship_sup, land_sup, plane_sup,
		    &olist, off, &dlist, def, &osupport, &dsupport,
		    a_engineer);
    if (att_abort(A_ATTACK, off, def)) {
	pr("Attack aborted\n");
	att_empty_attack(A_ATTACK, 0, def);
	return att_free_lists(&olist, &dlist);
    }

    /*
     * Death, carnage, and destruction.
     */

    att_fight(A_ATTACK, off, &olist, osupport, def, &dlist, dsupport);

    return RET_OK;
}
