/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  coun.c: Do a country roster
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "sect.h"
#include "nat.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include <fcntl.h>
#include <ctype.h>
#include "commands.h"
#include "optlist.h"

static void coun_list(struct natstr *natp);

int
coun(void)
{
    struct nstr_item ni;
    struct natstr nat;

    if (!snxtitem(&ni, EF_NATION, player->argp[1]))
	return RET_SYN;
    prdate();
    pr("  #   last access                         %sstatus     country name\n",
       player->god ? "BTU  " : "");
    while (nxtitem(&ni, &nat)) {
	if ((nat.nat_stat & STAT_INUSE) == 0)
	    continue;
	coun_list(&nat);
    }
    return RET_OK;
}

static void
coun_list(struct natstr *natp)
{
    char *status;
    natid cn = natp->nat_cnum;

    pr("%3d  ", cn);

    if (getplayer(cn)
	&& (player->god
	    || (natp->nat_stat & STAT_GOD)
	    || cn == player->cnum || getrel(natp, player->cnum) == ALLIED))
        pr(" Now logged on                     ");
    else if (player->god) {
	if (natp->nat_last_login == 0)
	    pr(" Never logged on                   ");
	else {
	    pr("%.16s - ", ctime(&natp->nat_last_login));
	    pr("%-16.16s",
	       natp->nat_last_login <= natp->nat_last_logout
	       ? ctime(&natp->nat_last_logout) : "?");
	}
    } else
	pr(" Unknown                           ");

    if (player->god)
	pr(" %4d", natp->nat_btu);

    if (natp->nat_stat & STAT_GOD)
	status = "DEITY";
    else if (natp->nat_stat & STAT_NEW)
	status = "New";
    else if (natp->nat_stat & STAT_SANCT)
	status = "Sanctuary";
    else if (natp->nat_stat & STAT_NORM) {
	status = "Active";
	if (!opt_HIDDEN || player->god) {
	    if (influx(natp))
		status = "In flux";
	    else if (natp->nat_money < 0)
		status = "Broke";
	}
    } else {
	status = "Visitor";
    }
    pr("  %-9.9s  %s\n", status, natp->nat_cnam);
}
