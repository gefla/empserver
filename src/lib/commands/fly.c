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
 *  fly.c: fly a plane
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 2000
 *     Markus Armbruster, 2004-2012
 */

#include <config.h>

#include "commands.h"
#include "empobj.h"
#include "item.h"
#include "path.h"
#include "plane.h"

int
c_fly(void)
{
    coord tx, ty;
    coord ax, ay;
    int ap_to_target;
    struct ichrstr *ip;
    char flightpath[MAX_PATH_LEN];
    int cno;
    struct nstr_item ni_bomb;
    struct nstr_item ni_esc;
    union empobj_storage target;
    struct emp_qelem bomb_list;
    struct emp_qelem esc_list;
    int wantflags;
    struct sctstr ap_sect;
    char buf[1024];

    wantflags = 0;
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
    pr("Ending sector is %s\n", xyas(tx, ty, player->cnum));
    ip = whatitem(player->argp[5], "transport what? ");
    if (player->aborted)
	return RET_SYN;

    if (pln_where_to_land(tx, ty, &target, &wantflags) < 0)
	return RET_SYN;
    cno = target.gen.ef_type == EF_SHIP ? target.gen.uid : -1;

    if (ip && ip->i_uid == I_CIVIL) {
	if (target.gen.own != player->cnum) {
	    pr("Your civilians refuse to board a flight abroad!\n");
	    return RET_FAIL;
	}
	if (target.gen.ef_type == EF_SECTOR
	    && target.sect.sct_own != target.sect.sct_oldown) {
	    pr("Can't fly civilians into occupied sectors.\n");
	    return RET_FAIL;
	}
    }

    ap_to_target = strlen(flightpath);
    pr("range to target is %d\n", ap_to_target);
    /*
     * select planes within range
     */
    pln_sel(&ni_bomb, &bomb_list, &ap_sect, ap_to_target, 1,
	    wantflags, P_M | P_O);
    pln_sel(&ni_esc, &esc_list, &ap_sect, ap_to_target, 1,
	    wantflags | P_ESC | P_F, P_M | P_O);
    if (cno >= 0
	&& !pln_can_land_on_carrier(&bomb_list, &esc_list, &target.ship)) {
	pr("Not enough room on ship #%d!\n", cno);
	return RET_FAIL;
    }
    /*
     * now arm and equip the bombers, transports, whatever.
     */
    pln_arm(&bomb_list, ap_to_target, 't', ip);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    pln_arm(&esc_list, ap_to_target, 'e', NULL);
    ac_encounter(&bomb_list, &esc_list, ax, ay, flightpath, 0);
    if (QEMPTY(&bomb_list)) {
	pr("No planes got through fighter defenses\n");
    } else {
	pln_dropoff(&bomb_list, ip, tx, ty, cno);
	pln_newlanding(&bomb_list, tx, ty, cno);
	pln_newlanding(&esc_list, tx, ty, cno);
    }
    pln_put(&bomb_list);
    pln_put(&esc_list);
    return RET_OK;
}
