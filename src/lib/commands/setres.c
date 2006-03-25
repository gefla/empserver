/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  setres.c: Set resources of a sector
 * 
 *  Known contributors to this file:
 *     David Muir Sharnoff
 *     Karl Hagen
 *     Steve McClure, 1998
 */

#include <config.h>

#include <stdio.h>
#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "nat.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

/*
 * format: setres resource <amt>  <sect>
 */
int
setres(void)
{
    struct sctstr sect;
    s_char *what;
    int amt;
    s_char *p;
    struct nstr_sect nstr;
    s_char buf[1024];

    if ((what = getstarg(player->argp[1],
			 "Set What (iron, gold, oil, uranium, fertility)? ",
			 buf)) == 0)
	return RET_SYN;
    switch (what[0]) {
    case 'i':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    if (amt > 100)
		amt = 100;
	    if (amt < 0)
		amt = 0;
	    if (sect.sct_own != 0)
		resnoise(&sect, 1, "Iron ore content",
			 (int)sect.sct_min, amt);
	    sect.sct_min = (unsigned char)amt;
	    putsect(&sect);
	}
	break;
    case 'g':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    if (amt > 100)
		amt = 100;
	    if (amt < 0)
		amt = 0;
	    if (sect.sct_own != 0)
		resnoise(&sect, 1, "Gold content",
			 (int)sect.sct_gmin, amt);
	    sect.sct_gmin = (unsigned char)amt;
	    putsect(&sect);
	}
	break;
    case 'o':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    if (amt > 100)
		amt = 100;
	    if (amt < 0)
		amt = 0;
	    if (sect.sct_own != 0)
		resnoise(&sect, 1, "Oil content", (int)sect.sct_oil, amt);
	    sect.sct_oil = (unsigned char)amt;
	    putsect(&sect);
	}
	break;
    case 'f':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    if (amt > 100)
		amt = 100;
	    if (amt < 0)
		amt = 0;
	    if (sect.sct_own != 0)
		resnoise(&sect, 1, "Fertility content",
			 (int)sect.sct_fertil, amt);
	    sect.sct_fertil = (unsigned char)amt;
	    putsect(&sect);
	}
	break;
    case 'u':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    if (amt > 100)
		amt = 100;
	    if (amt < 0)
		amt = 0;
	    if (sect.sct_own != 0)
		resnoise(&sect, 1, "Uranium content",
			 (int)sect.sct_uran, amt);
	    sect.sct_uran = (unsigned char)amt;
	    putsect(&sect);
	}
	break;
    default:
	pr("huh?\n");
	return RET_SYN;
    }
    return RET_OK;
}
