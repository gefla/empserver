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
 *  bdes.c: Change bmap designations
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "sect.h"
#include "item.h"
#include "xy.h"
#include "path.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "commands.h"

int
bdes(void)
{
    s_char *p, d;
    struct nstr_sect nstr;
    struct sctstr sect;
    s_char prompt[128];
    s_char buf[1024];
    int changed = 0;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    while (!player->aborted && nxtsct(&nstr, &sect)) {
	if ((nstr.ncond > 0) && (sect.sct_own != player->cnum))
	    continue;
	d = player->map[sctoff(nstr.x, nstr.y)];
	sprintf(prompt, "%s '%c'  desig? ",
		xyas(nstr.x, nstr.y, player->cnum), d);
	if ((p = getstarg(player->argp[2], prompt, buf)) == 0)
	    continue;
	if (!isprint(*p)) {
	    pr("Bad character. Must be printable!\n");
	    continue;
	}
	changed |= map_set(player->cnum, nstr.x, nstr.y, *p, 2);
    }
    if (changed)
	writebmap(player->cnum);
    return RET_OK;
}
