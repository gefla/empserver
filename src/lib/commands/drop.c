/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  drop.c: Air-drop commodities
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Markus Armbruster, 2004-2012
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "path.h"
#include "plane.h"

int
drop(void)
{
    coord tx, ty;
    coord ax, ay;
    int ap_to_target;
    struct ichrstr *ip;
    char flightpath[MAX_PATH_LEN];
    struct nstr_item ni_bomb;
    struct nstr_item ni_esc;
    struct sctstr target;
    struct emp_qelem bomb_list;
    struct emp_qelem esc_list;
    int wantflags;
    struct sctstr ap_sect;
    char buf[1024];

    if (get_planes(&ni_bomb, &ni_esc, player->argp[1], player->argp[2]) < 0)
	return RET_SYN;
    if (!get_assembly_point(player->argp[3], &ap_sect, buf))
	return RET_SYN;
    ax = ap_sect.sct_x;
    ay = ap_sect.sct_y;
    if (!getpath(flightpath, player->argp[4], ax, ay, 0, 0, MOB_FLY)
	|| *flightpath == 0)
	return RET_SYN;
    tx = ax;
    ty = ay;
    (void)pathtoxy(flightpath, &tx, &ty, fcost);
    pr("target is %s\n", xyas(tx, ty, player->cnum));
    if (!(ip = whatitem(player->argp[5], "Drop off what? ")))
	return RET_SYN;
    getsect(tx, ty, &target);

    if (relations_with(target.sct_own, player->cnum) == ALLIED) {
	/* own or allied sector: cargo drop */
	if (ip->i_uid == I_CIVIL) {
	    if (target.sct_own != player->cnum) {
		pr("Your civilians refuse to board a flight abroad!\n");
		return RET_FAIL;
	    }
	    if (target.sct_own != target.sct_oldown) {
		pr("Can't drop civilians into occupied sectors.\n");
		return RET_FAIL;
	    }
	}
	wantflags = P_C;
    } else {
	/* into the unknown... */
	if (ip->i_uid != I_SHELL) {
	    pr("You don't own %s!\n", xyas(tx, ty, player->cnum));
	    return RET_FAIL;
	}
	/* mine drop */
	wantflags = P_MINE;
    }

    ap_to_target = strlen(flightpath);
    if (flightpath[ap_to_target - 1] == 'h')
	ap_to_target--;
    pr("range to target is %d\n", ap_to_target);
    /*
     * select planes within range
     */
    pln_sel(&ni_bomb, &bomb_list, &ap_sect, ap_to_target, 2,
	    wantflags, P_M | P_O);
    pln_sel(&ni_esc, &esc_list, &ap_sect, ap_to_target, 2,
	    P_ESC | P_F, P_M | P_O);
    /*
     * now arm and equip the bombers, transports, whatever.
     */
    pln_arm(&bomb_list, 2 * ap_to_target,
	    wantflags & P_MINE ? 'm' : 'd',
	    ip);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    pln_arm(&esc_list, 2 * ap_to_target, 'e', NULL);
    ac_encounter(&bomb_list, &esc_list, ax, ay, flightpath, 0);
    if (QEMPTY(&bomb_list)) {
	pr("No planes got through fighter defenses\n");
    } else {
	if (wantflags & P_MINE)
	    pln_mine(&bomb_list, tx, ty);
	else
	    pln_dropoff(&bomb_list, ip, tx, ty, -1);
    }
    pln_put(&bomb_list);
    pln_put(&esc_list);
    return RET_OK;
}
