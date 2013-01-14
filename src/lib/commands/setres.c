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
    int amt;
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
	amt = atoi(p);
	amt = LIMIT_TO(amt, 0, 100);
	if (!check_sect_ok(&sect))
	    return RET_FAIL;
	switch (char0) {
	case 'i':
	    if (sect.sct_own != 0)
		resnoise(&sect, "Iron ore content", sect.sct_min, amt);
	    sect.sct_min = (unsigned char)amt;
	    break;
	case 'g':
	    if (sect.sct_own != 0)
		resnoise(&sect, "Gold content", sect.sct_gmin, amt);
	    sect.sct_gmin = (unsigned char)amt;
	    break;
	case 'o':
	    if (sect.sct_own != 0)
		resnoise(&sect, "Oil content", sect.sct_oil, amt);
	    sect.sct_oil = (unsigned char)amt;
	    break;
	case 'f':
	    if (sect.sct_own != 0)
		resnoise(&sect, "Fertility content", sect.sct_fertil, amt);
	    sect.sct_fertil = (unsigned char)amt;
	    break;
	case 'u':
	    if (sect.sct_own != 0)
		resnoise(&sect, "Uranium content", sect.sct_uran, amt);
	    sect.sct_uran = (unsigned char)amt;
	    break;
	default:
	    pr("huh?\n");
	    return RET_SYN;
	}
	putsect(&sect);
    }
    return RET_OK;
}
