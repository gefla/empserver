/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fly.c: fly a plane
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 2000
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "ship.h"
#include "item.h"
#include "plane.h"
#include "nuke.h"
#include "xy.h"
#include "nsc.h"
#include "news.h"
#include "file.h"
#include "nat.h"
#include "path.h"
#include "commands.h"

int
fly(void)
{
    s_char *p;
    int mission_flags;
    int tech;
    coord tx, ty;
    coord ax, ay;
    int ap_to_target;
    struct ichrstr *ip;
    s_char flightpath[MAX_PATH_LEN];
    struct shpstr ship;
    int cno;
    struct nstr_item ni_bomb;
    struct nstr_item ni_esc;
    coord x, y;
    struct sctstr target;
    struct emp_qelem bomb_list;
    struct emp_qelem esc_list;
    int wantflags;
    struct sctstr ap_sect;
    int dst_type;
    s_char *dst_ptr;
    s_char buf[1024];

    wantflags = 0;
    if (!snxtitem(&ni_bomb, EF_PLANE, player->argp[1]))
	return RET_SYN;
    if (!snxtitem
	(&ni_esc, EF_PLANE, getstarg(player->argp[2], "escort(s)? ", buf)))
	pr("No escorts...\n");
    if ((p = getstarg(player->argp[3], "assembly point? ", buf)) == 0
	|| *p == 0)
	return RET_SYN;
    if (!sarg_xy(p, &x, &y) || !getsect(x, y, &ap_sect))
	return RET_SYN;
    if (ap_sect.sct_own && ap_sect.sct_own != player->cnum &&
	getrel(getnatp(ap_sect.sct_own), player->cnum) != ALLIED) {
	pr("Assembly point not owned by you or an ally!\n");
	return RET_SYN;
    }
    ax = x;
    ay = y;
    if (getpath(flightpath, player->argp[4], ax, ay, 0, 0,
		0, P_FLYING) == 0 || *flightpath == 0)
	return RET_SYN;
    tx = ax;
    ty = ay;
    (void)pathtoxy(flightpath, &tx, &ty, fcost);
    pr("Ending sector is %s\n", xyas(tx, ty, player->cnum));
    getsect(tx, ty, &target);
    cno = -1;
    ip = whatitem(player->argp[5], "transport what? ");
    mission_flags = 0;
    if (pln_onewaymission(&target, &cno, &wantflags) < 0)
	return RET_SYN;
    if (cno < 0) {
	dst_ptr = (s_char *)&target;
	dst_type = EF_SECTOR;
    } else {
	getship(cno, &ship);
	dst_ptr = (s_char *)&ship;
	dst_type = EF_SHIP;
    }
    ap_to_target = strlen(flightpath);
    if (*(flightpath + strlen(flightpath) - 1) == 'h')
	ap_to_target--;
    pr("range to target is %d\n", ap_to_target);
    /*
     * select planes within range
     */
    pln_sel(&ni_bomb, &bomb_list, &ap_sect, ap_to_target,
	    1, wantflags, P_M | P_O);
    wantflags |= P_F;
    wantflags |= P_ESC;
    pln_sel(&ni_esc, &esc_list, &ap_sect, ap_to_target,
	    1, wantflags, P_M | P_O);
    /*
     * now arm and equip the bombers, transports, whatever.
     * tech is stored in high 16 bits of mission_flags.
     * yuck.
     */
    tech = 0;
    mission_flags |= P_X;	/* stealth (shhh) */
    mission_flags |= P_H;	/* gets turned off if not all choppers */
    mission_flags =
	pln_arm(&bomb_list, ap_to_target, 't', ip, 0, mission_flags,
		&tech);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    mission_flags =
	pln_arm(&esc_list, ap_to_target, 't', ip, P_ESC | P_F,
		mission_flags, &tech);
    ac_encounter(&bomb_list, &esc_list, ax, ay, flightpath, mission_flags,
		 0, 0, 0);
    if (QEMPTY(&bomb_list)) {
	pr("No planes got through fighter defenses\n");
    } else {
	getsect(tx, ty, &target);
	pln_dropoff(&bomb_list, ip, tx, ty, dst_ptr, dst_type);
	pln_newlanding(&bomb_list, tx, ty, cno);
	pln_newlanding(&esc_list, tx, ty, cno);
    }
    pln_put(&bomb_list);
    pln_put(&esc_list);
    return RET_OK;
}
