/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  setres.c: Set resources of a sector
 *
 *  Known contributors to this file:
 *     David Muir Sharnoff
 *     Karl Hagen
 *     Steve McClure, 1998
 *     Markus Armbruster, 2010-2013
 */

#include <config.h>

#include "commands.h"

/*
 * format: setres resource <amt>  <sect>
 */
int
setres(void)
{
    struct sctstr sect;
    char *what;
    int ret;
    char *p;
    struct nstr_sect nstr;
    char buf[1024];
    char char0;

    what = getstarg(player->argp[1],
		    "Set what (iron, gold, oil, uranium, fertility)? ",
		    buf);
    if (!what)
	return RET_SYN;
    char0 = what[0];

    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    while (nxtsct(&nstr, &sect) > 0) {
	p = getstarg(player->argp[3], "What value : ", buf);
	if (!p || !*p)
	    return RET_SYN;
	if (!check_sect_ok(&sect))
	    return RET_FAIL;
	switch (char0) {
	case 'i':
	    ret = edit_sect(&sect, "i", p);
	    break;
	case 'g':
	    ret = edit_sect(&sect, "g", p);
	    break;
	case 'o':
	    ret = edit_sect(&sect, "c", p);
	    break;
	case 'f':
	    ret = edit_sect(&sect, "f", p);
	    break;
	case 'u':
	    ret = edit_sect(&sect, "u", p);
	    break;
	default:
	    pr("huh?\n");
	    ret = RET_SYN;
	}
	if (ret != RET_OK)
	    return ret;
	putsect(&sect);
    }
    return RET_OK;
}
