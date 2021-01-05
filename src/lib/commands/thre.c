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
 *  thre.c: Describe/set threshold for given commodity
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 */

#include <config.h>

#include "commands.h"
#include "item.h"

/*
 * threshold <COMM> <SECTS> <THRESH>
 */
int
thre(void)
{
    struct sctstr sect;
    struct nstr_sect nstr;
    int val;
    struct ichrstr *ip;
    char *p;
    int thresh;
    i_type type;
    char prompt[128];
    char buf[128];

    if (!(ip = whatitem(player->argp[1], "What commodity? ")))
	return RET_SYN;
    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    type = ip->i_uid;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	val = sect.sct_dist[type];
	if (val > 0)
	    sprintf(prompt, "%s %s  old threshold %d new? ",
		    xyas(nstr.x, nstr.y, player->cnum),
		    dchr[sect.sct_type].d_name, val);
	else
	    sprintf(prompt, "%s %s  threshold? ",
		    xyas(nstr.x, nstr.y, player->cnum),
		    dchr[sect.sct_type].d_name);
	if (!(p = getstarg(player->argp[3], prompt, buf)))
	    return RET_FAIL;
	if (!*p)
	    continue;
	if (!check_sect_ok(&sect))
	    return RET_FAIL;
	thresh = atoi(p);
	if (thresh < 0)
	    return RET_FAIL;
	if (thresh > ITEM_MAX)
	    thresh = ITEM_MAX;
	if ((val > 0) && (val == thresh)) {
	    pr("%s threshold unchanged (left at %d)\n",
	       xyas(nstr.x, nstr.y, player->cnum), val);
	    continue;
	}
	if (val > 0 && player->argp[3] && *player->argp[3])
	    pr("%s old threshold %d\n",
	       xyas(nstr.x, nstr.y, player->cnum), val);
	sect.sct_dist[type] = thresh;
	putsect(&sect);
    }
    return RET_OK;
}
