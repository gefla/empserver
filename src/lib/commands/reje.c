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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  reje.c: Refuse telegrams/annos/treaties/loans from countries
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "nat.h"
#include "file.h"
#include "xy.h"
#include "nsc.h"
#include "commands.h"
#include "optlist.h"

int
reje(void)
{
    register s_char *p;
    int rel;
    int do_undo;
    struct natstr nat;
    struct nstr_item ni;
    s_char buf[1024];

    if ((p = getstarg(player->argp[1], "reject or accept? ", buf)) == 0)
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
    if ((p =
	 getstarg(player->argp[2],
		  "mail, treaties, loans, or announcements? ", buf)) == 0)
	return RET_SYN;
    switch (*p) {
    case 'a':
	rel = REJ_ANNO;
	break;
    case 'l':
	rel = REJ_LOAN;
	break;
    case 'm':
	rel = REJ_TELE;
	break;
    case 't':
	rel = REJ_TREA;
	break;
    default:
	pr("That's not one of the choices!\n");
	return RET_SYN;
    }
    if (!snxtitem(&ni, EF_NATION, player->argp[3]))
	return RET_SYN;
    while (nxtitem(&ni, &nat)) {
	if (nat.nat_stat == STAT_GOD) {
	    pr("You may not reject/accept stuff from %s\nbecause they are a deity.\n", nat.nat_cnam);
	    continue;
	}
	if (opt_HIDDEN) {
	    if (!getcontact(getnatp(player->cnum), ni.cur))
		continue;
	}
	switch (rel) {
	case REJ_ANNO:
	    pr("%s annos from %s\n",
	       (do_undo == 1 ? "Rejecting" : "Accepting"), nat.nat_cnam);
	    break;
	case REJ_LOAN:
	    pr("%s loans from %s\n",
	       (do_undo == 1 ? "Rejecting" : "Accepting"), nat.nat_cnam);
	    break;
	case REJ_TELE:
	    pr("%s teles from %s\n",
	       (do_undo == 1 ? "Rejecting" : "Accepting"), nat.nat_cnam);
	    break;
	case REJ_TREA:
	    pr("%s treaties from %s\n",
	       (do_undo == 1 ? "Rejecting" : "Accepting"), nat.nat_cnam);
	    break;
	}
	setrej(player->cnum, (natid)ni.cur, do_undo, rel);
    }
    return RET_OK;
}
