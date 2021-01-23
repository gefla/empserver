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
 *  give.c: give stuff to countries (deity)
 *
 *  Known contributors to this file:
 *     David Muir Sharnoff
 *     Steve McClure, 1997
 *     Markus Armbruster, 2004-2013
 */

#include <config.h>

#include "actofgod.h"
#include "commands.h"
#include "item.h"

/*
 * format: give <commod> <sect> <amt>
 */
int
give(void)
{
    struct sctstr sect;
    int amt;
    struct ichrstr *ip;
    int m, n;
    char *p;
    struct nstr_sect nstr;
    char buf[1024];

    if (!(ip = whatitem(player->argp[1], "which commodity? ")))
	return RET_SYN;
    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    while (nxtsct(&nstr, &sect) > 0) {
	p = getstarg(player->argp[3], "how much : ", buf);
	if (!p || !*p)
	    return RET_SYN;
	if ((amt = atoi(p)) == 0)
	    return RET_SYN;
	if (!check_sect_ok(&sect))
	    return RET_FAIL;
	n = sect.sct_item[ip->i_uid];
	if (amt < 0 && n + amt < 0)
	    m = 0;
	else if (amt > 0 && n > ITEM_MAX - amt)
	    m = ITEM_MAX;
	else
	    m = n + amt;
	sect.sct_item[ip->i_uid] = m;
	putsect(&sect);
	report_divine_gift(sect.sct_own, ip, m - n,
			   xyas(sect.sct_x, sect.sct_y, sect.sct_own));
	if (m - n != amt)
	    pr("Only %d %s in %s\n", abs(m - n),
	       m - n >= 0 ? "given" : "taken",
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	pr("%d %s in %s\n", m, ip->i_name,
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
    }
    return RET_OK;
}
