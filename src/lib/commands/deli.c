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
 *  deli.c: Set deliveries from a sector
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "var.h"
#include "sect.h"
#include "item.h"
#include "file.h"
#include "path.h"
#include "nsc.h"
#include "nat.h"
#include "commands.h"

int
deli(void)
{
    struct sctstr sect;
    register int dir, del;
    register struct ichrstr *ich;
    register int thresh = -1;
    int i_del;
    int sx, sy;
    int status;
    struct nstr_sect nstr;
    s_char buf[1024];
    s_char prompt[128];
    s_char *p;

    if ((ich = whatitem(player->argp[1], "deliver what? ")) == 0)
	return RET_SYN;
    /*
       if (ich->i_vtype == V_CIVIL || ich->i_vtype == V_MILIT) {
       pr("You cannot deliver people!\n");
       return RET_FAIL;
       }
     */
    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    i_del = V_DEL(ich - ichr);

    while (nxtsct(&nstr, &sect) > 0) {
	if (!player->owner)
	    continue;
	sprintf(prompt, "%s %s 'query' or %s threshold? ",
		xyas(nstr.x, nstr.y, player->cnum),
		dchr[sect.sct_type].d_name, ich->i_name);
	if (!(p = getstarg(player->argp[3], prompt, buf)) || !*p)
	    return RET_SYN;
	del = getvar(i_del, (s_char *)&sect, EF_SECTOR);
	if (((*p >= '0') && (*p <= '9')) || *p == '+') {
	    thresh = atoi(p) & ~0x7;
	    if (*p == '+'
		|| !(p = getstarg(player->argp[4], "Direction? ", buf))
		|| !*p)
		dir = del & 0x7;
	    else if ((dir = chkdir(*p, DIR_STOP, DIR_LAST)) < 0)
		return RET_SYN;
	} else if (*p != 'q')
	    return RET_SYN;

	if (!check_sect_ok(&sect))
	    continue;

	if (thresh >= 0) {
	    del = thresh + dir;
	    status = putvar(i_del, del, (s_char *)&sect, EF_SECTOR);
	    if (status < 0) {
		pr("No room for delivery path in %s\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum));
		continue;
	    } else if (!status) {
		/* Either nothing to set, or bogus amount. */
		continue;
	    } else
		putsect(&sect);
	}
	if (!del)
	    continue;
	dir = del & 0x7;
	sx = diroff[dir][0] + sect.sct_x;
	sy = diroff[dir][1] + sect.sct_y;
	pr("Deliver %s from %s @ %s to %s",
	   ich->i_name,
	   dchr[sect.sct_type].d_name,
	   xyas(sect.sct_x, sect.sct_y, player->cnum),
	   xyas(sx, sy, player->cnum));
	if (!(del & ~0x7))
	    pr("\n");
	else
	    pr(" (cutoff %d)\n", del & ~0x7);
    }
    return RET_OK;
}
