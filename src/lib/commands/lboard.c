/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  lboard.c: Board an enemy land unit (heavily borrowing from boar.c)
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#include <config.h>

#include "combat.h"
#include "commands.h"
#include "land.h"
#include "mission.h"
#include "path.h"
#include "ship.h"

int
lboa(void)
{
    struct combat off[1];	/* boarding sector */
    struct combat def[1];	/* defending land unit */
    struct emp_qelem olist;	/* boarding units */
    struct emp_qelem dlist;	/* defending units */
    int ototal;			/* total boarding strength */
    int a_engineer = 0;		/* boarder engineers are present */
    int a_spy = 0;		/* the best boarder scout */
    struct sctstr sect;
    struct lndstr land;
    char *p;
    char buf[1024];

    att_combat_init(def, EF_LAND);
    /*
     * Collect input from the boarder
     */

    /* What are we boarding? */

    if (!(p = getstarg(player->argp[1], "Victim land unit #?  ", buf)) ||
	(def->lnd_uid = atoi(p)) < 0)
	return RET_SYN;

    /*
     * Ask the boarder what sector they want to board with
     */

    /* Note: if we allow land units to board other land units, we need
     * to make sure the code will allow that */
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
	    pr("You don't have any mobility in %s!\n",
	       xyas(off->x, off->y, player->cnum));
	    return RET_SYN;
	}
    } else {
	pr("You can only board land units from a sector.\n");
	return RET_SYN;
    }
    if (att_abort(A_LBOARD, off, def)) {
	pr("Land unit boarding aborted\n");
	return RET_OK;
    }

    /* Show what we're boarding */

    if (att_show(def))
	return RET_FAIL;

    /* Ask the player what he wants to board with */

    att_ask_offense(A_LBOARD, off, def, &olist, &a_spy, &a_engineer);
    if (att_abort(A_LBOARD, off, def)) {
	pr("Land unit boarding aborted\n");
	att_empty_attack(A_LBOARD, 0, def);
	return att_free_lists(&olist, 0);
    }

    ototal = att_get_offense(A_LBOARD, off, &olist, def);
    if (att_abort(A_LBOARD, off, def)) {
	pr("Land unit boarding aborted\n");
	att_empty_attack(A_LBOARD, 0, def);
	return att_free_lists(&olist, 0);
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

    if (!(att_fight(A_LBOARD, off, &olist, 1.0, def, &dlist, 1.0))) {
	getland(def->lnd_uid, &land);
	/*
	 * What about retreat on RET_BOARDED?  Well, land units can't
	 * move when the boarder is hostile, and retreating when he
	 * isn't is not useful.
	 */
    }

    return RET_OK;
}
