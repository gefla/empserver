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
 *  bdes.c: Change bmap designations
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "map.h"
#include "path.h"

int
bdes(void)
{
    char *p, d;
    struct nstr_sect nstr;
    struct sctstr sect;
    char prompt[128];
    char buf[1024];
    int changed = 0;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    while (!player->aborted && nxtsct(&nstr, &sect)) {
	if ((nstr.ncond > 0) && (sect.sct_own != player->cnum))
	    continue;
	d = player->map[sect.sct_uid];
	sprintf(prompt, "%s '%c'  desig? ",
		xyas(nstr.x, nstr.y, player->cnum),
		d ? d : ' ');
	if ((p = getstarg(player->argp[2], prompt, buf)) == 0)
	    continue;
	if (!isprint(*p)) {
	    if (*p)
		pr("Bad character. Must be printable!\n");
	    continue;
	}
	changed |= map_set(player->cnum, nstr.x, nstr.y, *p, 2);
    }
    if (changed)
	writebmap(player->cnum);
    return RET_OK;
}
