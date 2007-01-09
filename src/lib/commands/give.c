/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  give.c: give stuff to countries (deity)
 * 
 *  Known contributors to this file:
 *     David Muir Sharnoff
 *     Steve McClure, 1997
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "news.h"
#include "optlist.h"

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
	if (p == 0 || *p == '\0')
	    return RET_SYN;
	if ((amt = atoi(p)) == 0)
	    return RET_SYN;
	n = sect.sct_item[ip->i_uid];
	if (amt < 0 && -amt > n) {
	    m = 0;
	} else if (amt > 0 && amt + n > ITEM_MAX) {
	    m = ITEM_MAX;
	} else
	    m = n + amt;
	sect.sct_item[ip->i_uid] = m;
	putsect(&sect);
	if (sect.sct_own != 0 && m != n) {
	    if (m > n) {
		if (opt_GODNEWS)
		    nreport(player->cnum, N_GIFT, sect.sct_own, 1);
		wu(player->cnum, sect.sct_own, "%s gave you %d %s in %s\n",
		   cname(player->cnum), m - n, ip->i_name,
		   xyas(sect.sct_x, sect.sct_y, sect.sct_own));
	    } else {
		if (opt_GODNEWS)
		    nreport(sect.sct_own, N_TAKE, player->cnum, 1);
		wu(player->cnum, sect.sct_own, "%s stole %d %s from %s\n",
		   cname(player->cnum), n - m, ip->i_name,
		   xyas(sect.sct_x, sect.sct_y, sect.sct_own));
	    }
	}
	pr("%d %s in %s\n", m, ip->i_name,
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
    }
    return RET_OK;
}
