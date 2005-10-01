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
 *  reco.c: Fly a recon mission
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "sect.h"
#include "ship.h"
#include "plane.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "path.h"
#include "commands.h"

int
reco(void)
{
    int mission_flags;
    coord tx, ty;
    coord ax, ay;
    int ap_to_target;
    s_char flightpath[MAX_PATH_LEN];
    int cno;
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
    getsect(tx, ty, &target);
    cno = -1;
    mission_flags = 0;
    if (pln_onewaymission(&target, &cno, &wantflags) < 0)
	return RET_SYN;
    ap_to_target = strlen(flightpath);
    if (*(flightpath + strlen(flightpath) - 1) == 'h')
	ap_to_target--;
    pr("range to target is %d\n", ap_to_target);
    /*
     * select planes within range
     */
    pln_sel(&ni_bomb, &bomb_list, &ap_sect, ap_to_target,
	    1, wantflags, P_M | P_O);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    wantflags |= P_F;
    wantflags |= P_ESC;
    pln_sel(&ni_esc, &esc_list, &ap_sect, ap_to_target,
	    1, wantflags, P_M | P_O);
    if (cno >= 0 && !pln_oneway_to_carrier_ok(&bomb_list, &esc_list, cno)) {
	pr("Not enough room on ship #%d!\n", cno);
	return RET_FAIL;
    }
    /*
     * now arm and equip the bombers, transports, whatever.
     */
    mission_flags |= P_X;	/* stealth (shhh) */
    mission_flags |= P_H;	/* gets turned off if not all choppers */
    mission_flags |= P_A;
    mission_flags = pln_arm(&bomb_list, ap_to_target, 'r',
			    0, P_S | P_I, mission_flags);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    mission_flags = pln_arm(&esc_list, ap_to_target, 'r',
			    0, P_F | P_ESC, mission_flags);
    mission_flags |= PM_R;

    if (*player->argp[0] == 's')
	mission_flags |= PM_S;

    ac_encounter(&bomb_list, &esc_list, ax, ay, flightpath, mission_flags,
		 0, 0, 0);
    if (QEMPTY(&bomb_list)) {
	pr("No planes got through fighter defenses\n");
    } else {
	getsect(tx, ty, &target);
	pln_newlanding(&bomb_list, tx, ty, cno);
	pln_newlanding(&esc_list, tx, ty, cno);
    }
    pln_put(&bomb_list);
    pln_put(&esc_list);
    return RET_OK;
}
