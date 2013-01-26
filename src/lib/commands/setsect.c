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
 *  setsect.c: Give resources to a country
 *
 *  Known contributors to this file:
 *     David Muir Sharnoff
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2013
 */

#include <config.h>

#include "commands.h"

/*
 * format: setres thing <sect> <#>
 */
int
setsector(void)
{
    struct sctstr sect;
    char *what;
    int amt, ret;
    char *p;
    struct nstr_sect nstr;
    char buf[1024];
    char char0, char1;

    what = getstarg(player->argp[1],
		    "Give what (iron, gold, oil, uranium, fertility, owner, eff., mob., work, avail., oldown, mines)? ",
		    buf);
    if (!what || !*what)
	return RET_SYN;
    char0 = what[0];
    char1 = what[1];

    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    while (nxtsct(&nstr, &sect) > 0) {
	p = getstarg(player->argp[3], "What value : ", buf);
	if (!p || !*p)
	    return RET_SYN;
	amt = atoi(p);
	if (!check_sect_ok(&sect))
	    return RET_FAIL;
	switch (char0) {
	case 'i':
	    ret = edit_sect_i(&sect, "i", sect.sct_min + amt);
	    break;
	case 'g':
	    ret = edit_sect_i(&sect, "g", sect.sct_gmin + amt);
	    break;
	case 'o':
	    switch (char1) {
	    case 'i':
		ret = edit_sect_i(&sect, "c", sect.sct_oil + amt);
		break;
		break;
	    case 'w':
		ret = edit_sect(&sect, "o", p);
		break;
	    case 'l':
		ret = edit_sect(&sect, "O", p);
		break;
	    default:
		pr("huh?\n");
		return RET_SYN;
	    }
	    break;
	case 'e':
	    ret = edit_sect_i(&sect, "e", sect.sct_effic + amt);
	    break;
	case 'm':
	    switch (char1) {
	    case 'i':
		ret = edit_sect_i(&sect, "M", sect.sct_mines + amt);
		break;
	    case 'o':
		ret = edit_sect_i(&sect, "m", sect.sct_mobil + amt);
		break;
	    default:
		pr("huh?\n");
		return RET_SYN;
	    }
	    break;
	case 'a':
	    ret = edit_sect_i(&sect, "a", sect.sct_avail + amt);
	    break;
	case 'w':
	    ret = edit_sect_i(&sect, "w", sect.sct_work + amt);
	    break;
	case 'f':
	    ret = edit_sect_i(&sect, "f", sect.sct_fertil + amt);
	    break;
	case 'u':
	    ret = edit_sect_i(&sect, "u", sect.sct_uran + amt);
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
