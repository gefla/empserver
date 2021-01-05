/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  para.c: Drop paratroopers onto a sector
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Markus Armbruster, 2004-2011
 */

#include <config.h>

#include "combat.h"
#include "commands.h"
#include "item.h"
#include "path.h"
#include "plane.h"
#include "ship.h"

static int paradrop(struct emp_qelem *list, coord x, coord y);

int
para(void)
{
    coord tx, ty;
    coord ax, ay;
    int ap_to_target;
    char flightpath[MAX_PATH_LEN];
    struct nstr_item ni_bomb;
    struct nstr_item ni_esc;
    struct sctstr target;
    struct emp_qelem bomb_list;
    struct emp_qelem esc_list;
    struct sctstr ap_sect;
    char buf[1024];

    if (get_planes(&ni_bomb, &ni_esc, player->argp[1], player->argp[2]) < 0)
	return RET_SYN;
    if (!get_assembly_point(player->argp[3], &ap_sect, buf))
	return RET_SYN;
    ax = ap_sect.sct_x;
    ay = ap_sect.sct_y;
    if (!getpath(flightpath, player->argp[4], ax, ay, 0, 0, MOB_FLY))
	return RET_SYN;
    tx = ax;
    ty = ay;
    (void)pathtoxy(flightpath, &tx, &ty, fcost);
    getsect(tx, ty, &target);
    pr("LZ is %s\n", xyas(tx, ty, player->cnum));
    ap_to_target = strlen(flightpath);
    pr("range to target is %d\n", ap_to_target);
    if (target.sct_own == player->cnum) {
	pr("You can't air-assault your own sector!\n");
	return RET_FAIL;
    }
    /*
     * select planes within range
     */
    pln_sel(&ni_bomb, &bomb_list, &ap_sect, ap_to_target, 2,
	    P_P | P_C, P_M | P_O);
    pln_sel(&ni_esc, &esc_list, &ap_sect, ap_to_target, 2,
	    P_ESC | P_F, P_M | P_O);
    /*
     * now arm and equip the bombers, transports, whatever.
     */
    pln_arm(&bomb_list, 2 * ap_to_target, 'a', NULL);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    pln_arm(&esc_list, 2 * ap_to_target, 'e', NULL);
    ac_encounter(&bomb_list, &esc_list, ax, ay, flightpath, 0);
    if (QEMPTY(&bomb_list)) {
	pr("No planes got through fighter defenses\n");
    } else {
	getsect(tx, ty, &target);
	paradrop(&bomb_list, tx, ty);
    }
    pln_put(&bomb_list);
    pln_put(&esc_list);
    return RET_OK;
}

static int
paradrop(struct emp_qelem *list, coord x, coord y)
{
    struct combat off[1];	/* assaulting ship or sector */
    struct combat def[1];	/* defending ship */
    struct emp_qelem olist;	/* assaulting units */
    struct emp_qelem dlist;	/* defending units */
    int ototal;			/* total assaulting strength */
    int a_engineer = 0;		/* assaulter engineers are present */
    int a_spy = 0;		/* the best assaulter scout */
    double osupport = 1.0;	/* assault support */
    double dsupport = 1.0;	/* defense support */
    struct plist *plp;
    struct emp_qelem *qp;

    /* Check for valid attack */

    att_combat_init(def, EF_SECTOR);
    def->x = x;
    def->y = y;
    if (att_abort(A_PARA, NULL, def))
	return RET_FAIL;

    /* Show what we're air-assaulting */
    att_show(def);

    /* set what we're air-assaulting with */

    emp_initque(&olist);
    att_combat_init(off, EF_PLANE);
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	off->troops += plp->load;
    }
    off->mil = off->troops;
    if (att_abort(A_PARA, off, def)) {
	pr("Air-Assault aborted\n");
	return RET_OK;
    }

    ototal = att_get_offense(A_PARA, off, &olist, def);
    if (att_abort(A_PARA, off, def)) {
	pr("Air-assault aborted\n");
	return RET_OK;
    }

    /* Get the defense */

    att_get_defense(&olist, def, &dlist, a_spy, ototal);

    /* Get defender support */

    att_get_support(A_PARA, 0, 0, 0, 0,
		    &olist, off, &dlist, def, &osupport, &dsupport,
		    a_engineer);

    if (att_abort(A_PARA, off, def)) {
	pr("Air-assault aborted\n");
	return RET_OK;
    }
    /*
     * Death, carnage, and destruction.
     */

    att_fight(A_PARA, off, &olist, osupport, def, &dlist, dsupport);

    return RET_OK;
}
