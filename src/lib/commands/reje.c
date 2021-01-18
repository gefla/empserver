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
 *  reje.c: Refuse telegrams/annos/loans from countries
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2016
 */

#include <config.h>

#include "commands.h"

int
c_reject(void)
{
    static char *what[] = { "teles", "annos", "loans" };
    char *p;
    enum rej_comm rej;
    int do_undo;
    struct natstr nat;
    struct nstr_item ni;
    char buf[1024];

    if (!(p = getstarg(player->argp[1], "reject or accept? ", buf)))
	return RET_SYN;
    switch (*p) {
    case 'r':
	do_undo = 1;
	break;
    case 'a':
	do_undo = 0;
	break;
    default:
	pr("That's not one of the choices!\n");
	return RET_SYN;
    }
    p = getstarg(player->argp[2],
		 "mail, loans, or announcements? ", buf);
    if (!p)
	return RET_SYN;
    switch (*p) {
    case 'a':
	rej = REJ_ANNO;
	break;
    case 'l':
	rej = REJ_LOAN;
	break;
    case 'm':
	rej = REJ_TELE;
	break;
    default:
	pr("That's not one of the choices!\n");
	return RET_SYN;
    }
    if (!snxtitem(&ni, EF_NATION, player->argp[3], NULL))
	return RET_SYN;
    while (nxtitem(&ni, &nat)) {
	if (nat.nat_stat == STAT_GOD) {
	    pr("You may not reject/accept stuff from %s\n"
	       "because they are a deity.\n",
	       nat.nat_cnam);
	    continue;
	}
	if (nat.nat_stat == STAT_UNUSED)
	    continue;
	pr("%s %s from %s\n",
	   (do_undo == 1 ? "Rejecting" : "Accepting"),
	   what[rej], nat.nat_cnam);
	setrej(player->cnum, (natid)ni.cur, do_undo, rej);
    }
    return RET_OK;
}
