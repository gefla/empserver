/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  deli.c: Set deliveries from a sector
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "path.h"

int
deli(void)
{
    struct sctstr sect;
    int dir, del;
    struct ichrstr *ich;
    int thresh;
    int sx, sy;
    struct nstr_sect nstr;
    char buf[1024];
    char prompt[128];
    char *p;

    if ((ich = whatitem(player->argp[1], "deliver what? ")) == 0)
	return RET_SYN;
    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;

    while (nxtsct(&nstr, &sect) > 0) {
	if (!player->owner)
	    continue;

	del = sect.sct_del[ich->i_uid];
	thresh = del & ~0x7;
	dir = del & 0x7;

	sprintf(prompt, "%s %s %s threshold or direction or 'query'? ",
		xyas(nstr.x, nstr.y, player->cnum),
		dchr[sect.sct_type].d_name, ich->i_name);
	if (!(p = getstarg(player->argp[3], prompt, buf)) || !*p)
	    return RET_SYN;
	if (*p != 'q') {
	    if (((*p >= '0') && (*p <= '9')) || *p == '+') {
		thresh = atoi(p) & ~0x7;
		if (*p == '+')
		    p = NULL;
		else {
		    sprintf(prompt, "%s %s %s direction? ",
			    xyas(nstr.x, nstr.y, player->cnum),
			    dchr[sect.sct_type].d_name, ich->i_name);
		    p = getstarg(player->argp[4], prompt, buf);
		    if (!p)
			return RET_SYN;
		}
	    }
	    if (p && *p) {
		dir = chkdir(*p, DIR_STOP, DIR_LAST);
		if (dir < 0) {
		    pr("'%c' is not a valid direction...\n", *p);
		    direrr(NULL, NULL, NULL);
		    return RET_SYN;
		}
	    }

	    if (!check_sect_ok(&sect))
		continue;

	    thresh = MIN(thresh, ITEM_MAX) & ~7;
	    del = thresh | dir;
	    sect.sct_del[ich->i_uid] = del;
	    putsect(&sect);
	}

	if (!del)
	    continue;

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
	    pr(" (cutoff %d)\n", thresh);
    }

    return RET_OK;
}
