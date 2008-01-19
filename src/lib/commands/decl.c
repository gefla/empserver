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
 *  decl.c: Declare war, neutrality, alliance, hostile or friendly
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"

int
decl(void)
{
    struct natstr nat, *natp;
    int rel;
    int who;
    struct nstr_item ni;
    char *p;
    char buf[1024];

    if (!(p = getstarg(player->argp[1],
		       "alliance, friendly, neutrality, hostility, or war? ",
		       buf)))
	return RET_SYN;
    switch (*p) {
    case 'a':
	rel = ALLIED;
	break;
    case 'f':
	rel = FRIENDLY;
	break;
    case 'n':
	rel = NEUTRAL;
	break;
    case 'h':
	rel = HOSTILE;
	break;
    case 'm':
	if (!opt_SLOW_WAR)
	    return RET_SYN;
	if (!player->god)
	    return RET_SYN;
	rel = MOBILIZATION;
	break;
    case 's':
	if (!opt_SLOW_WAR)
	    return RET_SYN;
	if (!player->god)
	    return RET_SYN;
	rel = SITZKRIEG;
	break;
    case 'w':
	rel = AT_WAR;
	break;
    default:
	pr("That's no declaration!\n");
	return RET_SYN;
    }

    if (!snxtitem(&ni, EF_NATION, player->argp[2]))
	return RET_SYN;
    who = player->cnum;
    if (player->god) {
	who = natarg(player->argp[3], "for which country? ");
	if (who < 0)
	    return RET_SYN;
    }

    natp = getnatp(who);
    while (nxtitem(&ni, &nat)) {
	if (nat.nat_stat == STAT_UNUSED)
	    continue;
	if (who == (natid)ni.cur)
	    continue;
	if (opt_HIDDEN) {
	    if (!player->god && !getcontact(natp, ni.cur)) {
		pr("You haven't contacted country #%d yet\n", ni.cur);
		continue;
	    }
	}
	if (getrel(natp, (natid)ni.cur) == rel)
	    continue;
	setrel(who, (natid)ni.cur, rel);
    }
    return RET_OK;
}
