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
 *  assa.c: Hit the beaches!
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1997
 */

#include <math.h>
#include "misc.h"
#include "player.h"
#include "file.h"
#include "sect.h"
#include "path.h"
#include "news.h"
#include "treaty.h"
#include "nat.h"
#include "xy.h"
#include "land.h"
#include "nsc.h"
#include "mission.h"
#include "ship.h"
#include "combat.h"
#include "commands.h"

int
assa(void)
{
    struct combat off[1];	/* assaulting ship */
    struct combat def[1];	/* defending sector */
    int fort_sup, ship_sup, land_sup, plane_sup;
    struct emp_qelem olist;	/* assaulting units */
    struct emp_qelem dlist;	/* defending units */
    int ototal;			/* total assaulting strength */
    int a_engineer = 0;		/* assaulter engineers are present */
    int a_spy = 0;		/* the best assaulter scout */
    double osupport = 1.0;	/* assault support */
    double dsupport = 1.0;	/* defense support */
    s_char *p;
    s_char buf[1024];
    int n;
    int ourtotal;
    struct emp_qelem *qp, *next;
    struct llist *llp;
    int rel;

    att_combat_init(off, EF_SHIP);
    att_combat_init(def, EF_SECTOR);
    /*
     * Collect input from the assaulter
     */

    /* What are we assaulting? */

    if (!(p = getstarg(player->argp[1], "Sector :  ", buf)))
	return RET_SYN;
    if (!sarg_xy(p, &def->x, &def->y))
	return RET_SYN;
    if (att_abort(A_ASSAULT, 0, def))
	return RET_FAIL;

    /*
     * Ask the assaulter what he wants to assault with
     */

    if ((off->shp_uid =
	 onearg(player->argp[2], "Assault from ship #")) < 0) {
	pr("You may only assault from one ship!\n");
	return RET_FAIL;
    }
    if (att_abort(A_ASSAULT, off, def)) {
	pr("Assault aborted\n");
	return RET_OK;
    }

    /* Show what we're assaulting, and check treaties */

    if (att_show(def))
	return RET_FAIL;

    /* Ask about offensive support */

    att_ask_support(3, &fort_sup, &ship_sup, &land_sup, &plane_sup);
    if (att_abort(A_ASSAULT, off, def)) {
	att_empty_attack(A_ASSAULT, 0, def);
	return RET_OK;
    }

    /* Ask the player what he wants to assault with */

    att_ask_offense(A_ASSAULT, off, def, &olist, &a_spy, &a_engineer);
    if (att_abort(A_ASSAULT, off, def)) {
	pr("Assault aborted\n");
	att_empty_attack(A_ASSAULT, 0, def);
	return att_free_lists(&olist, 0);
    }

    /* If we're assaulting our own sector, end here */
    if (def->own == player->cnum) {
	if (off->troops)
	    pr("You reinforce %s with %d troops\n",
	       xyas(def->x, def->y, player->cnum), off->troops);
	if (off->troops || !QEMPTY(&olist))
	    att_move_in_off(A_ASSAULT, off, &olist, def);
	return RET_OK;
    }

    /*
     * Estimate the defense strength and give the player a chance to abort
     */

    ototal = att_estimate_defense(A_ASSAULT, off, &olist, def, a_spy);
    if (att_abort(A_ASSAULT, off, def)) {
	pr("Assault aborted\n");
	att_empty_attack(A_ASSAULT, 0, def);
	return att_free_lists(&olist, 0);
    }

    /*
     * We have now got all the answers from the assaulter.  From this point
     * forward, we can assume that this battle is the _only_ thing
     * happening in the game.
     */

    /* First, we check to see if the only thing we have are spies
     * assaulting.  If so, we try to sneak them on land.  If they
     * make it, the defenders don't see a thing.  If they fail, well,
     * the spies die, and the defenders see them. */

    ourtotal = 0;
    for (n = 0; n <= off->last; n++) {
	if (off[n].type == EF_BAD)
	    continue;
	ourtotal += off[n].troops * att_combat_eff(off + n);
    }
    for (qp = olist.q_forw; qp != &olist; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	if (lchr[(int)llp->land.lnd_type].l_flags & L_SPY)
	    continue;
	ourtotal++;
    }

    /* If no attacking forces (i.e. we got here with only spies)
     * then try to sneak on-land. */

    if (!ourtotal) {
	pr("Trying to sneak on shore...\n");

	for (qp = olist.q_forw; qp != &olist; qp = next) {
	    next = qp->q_forw;
	    llp = (struct llist *)qp;
	    rel = getrel(getnatp(def->own), player->cnum);
	    if (chance(0.10) || rel == ALLIED || !def->own) {
		pr("%s made it on shore safely.\n", prland(&llp->land));
		llp->land.lnd_x = def->x;
		llp->land.lnd_y = def->y;
		llp->land.lnd_ship = -1;
		putland(llp->land.lnd_uid, &llp->land);
	    } else {
		pr("%s was spotted", prland(&llp->land));
		if (rel == HOSTILE || rel == AT_WAR || rel == SITZKRIEG ||
		    rel == MOBILIZATION) {
		    wu(0, def->own, "%s spy shot and killed in %s.\n",
		       cname(player->cnum), xyas(def->x, def->y,
						 def->own));
		    pr(" and was killed in the attempt.\n");
		    llp->land.lnd_effic = 0;
		    putland(llp->land.lnd_uid, &llp->land);
		    lnd_delete(llp, "");
		} else {
		    wu(0, def->own, "%s spy spotted in %s.\n",
		       cname(player->cnum), xyas(def->x, def->y,
						 def->own));
		    pr(" but made it ok.\n");
		    llp->land.lnd_x = def->x;
		    llp->land.lnd_y = def->y;
		    llp->land.lnd_ship = -1;
		    putland(llp->land.lnd_uid, &llp->land);
		}
	    }
	}
	return RET_OK;
    }

    /* Get the real defense */

    att_get_defense(&olist, def, &dlist, a_spy, ototal);

    /* Get assaulter and defender support */

    att_get_support(A_ASSAULT, fort_sup, ship_sup, land_sup, plane_sup,
		    &olist, off, &dlist, def, &osupport, &dsupport,
		    a_engineer);
    if (att_abort(A_ASSAULT, off, def)) {
	pr("Assault aborted\n");
	att_empty_attack(A_ASSAULT, 0, def);
	return att_free_lists(&olist, &dlist);
    }

    /*
     * Death, carnage, and destruction.
     */

    att_fight(A_ASSAULT, off, &olist, osupport, def, &dlist, dsupport);

    return RET_OK;
}
