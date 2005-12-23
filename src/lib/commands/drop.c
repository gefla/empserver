/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  drop.c: Air-drop commodities
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "sect.h"
#include "ship.h"
#include "item.h"
#include "plane.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "path.h"
#include "commands.h"

int
drop(void)
{
    int mission_flags;
    coord tx, ty;
    coord ax, ay;
    int ap_to_target;
    struct ichrstr *ip;
    s_char flightpath[MAX_PATH_LEN];
    struct nstr_item ni_bomb;
    struct nstr_item ni_esc;
    struct sctstr target;
    struct emp_qelem bomb_list;
    struct emp_qelem esc_list;
    int wantflags;
    struct sctstr ap_sect;
    s_char buf[1024];

    wantflags = 0;
    if (!snxtitem(&ni_bomb, EF_PLANE, player->argp[1]))
	return RET_SYN;
    if (!snxtitem(&ni_esc, EF_PLANE,
		  getstarg(player->argp[2], "escort(s)? ", buf)))
	pr("No escorts...\n");
    if (!get_assembly_point(player->argp[3], &ap_sect, buf))
	return RET_SYN;
    ax = ap_sect.sct_x;
    ay = ap_sect.sct_y;
    if (getpath(flightpath, player->argp[4], ax, ay, 0, 0, P_FLYING) == 0
	|| *flightpath == 0)
	return RET_SYN;
    tx = ax;
    ty = ay;
    (void)pathtoxy(flightpath, &tx, &ty, fcost);
    pr("target is %s\n", xyas(tx, ty, player->cnum));
    if ((ip = whatitem(player->argp[5], "Drop off what? ")) == 0)
	return RET_SYN;
    getsect(tx, ty, &target);

    if (target.sct_own == player->cnum
	|| getrel(getnatp(target.sct_own), player->cnum) == ALLIED) {
	if (ip->i_vtype == I_CIVIL && target.sct_own != target.sct_oldown) {
	    pr("Can't drop civilians into occupied sectors.\n");
	    return RET_FAIL;
	}
    } else {
	/* into the unknown... */
	if (ip->i_vtype != I_SHELL) {
	    pr("You don't own %s!\n", xyas(tx, ty, player->cnum));
	    return RET_FAIL;
	}
	wantflags = P_MINE;
    }

    ap_to_target = strlen(flightpath);
    if (*(flightpath + strlen(flightpath) - 1) == 'h')
	ap_to_target--;
    pr("range to target is %d\n", ap_to_target);
    /*
     * select planes within range
     */
    mission_flags = 0;
    pln_sel(&ni_bomb, &bomb_list, &ap_sect, ap_to_target,
	    2, wantflags, P_M | P_O);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    pln_sel(&ni_esc, &esc_list, &ap_sect, ap_to_target,
	    2, P_ESC | P_F, P_M | P_O);
    /*
     * now arm and equip the bombers, transports, whatever.
     */
    mission_flags |= P_X;	/* stealth (shhh) */
    mission_flags |= P_H;	/* gets turned off if not all choppers */
    mission_flags |= P_MINE;
    mission_flags = pln_arm(&bomb_list, 2 * ap_to_target,
			    wantflags & P_MINE ? 'm' : 'd',
			    ip, 0, mission_flags);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    mission_flags = pln_arm(&esc_list, 2 * ap_to_target, 'd',
			    ip, P_ESC | P_F, mission_flags);
    ac_encounter(&bomb_list, &esc_list, ax, ay, flightpath, mission_flags,
		 0, 0, 0);
    if (QEMPTY(&bomb_list)) {
	pr("No planes got through fighter defenses\n");
    } else {
	getsect(tx, ty, &target);
	if (wantflags & P_MINE)
	    pln_mine(&bomb_list, &target);
	else
	    pln_dropoff(&bomb_list, ip, tx, ty, &target, EF_SECTOR);
    }
    pln_put(&bomb_list);
    pln_put(&esc_list);
    return RET_OK;
}
