/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nuke.c: Display a listing of your nuclear arsenal
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "nuke.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "commands.h"

int
nuke(void)
{
    register int first_line = 0;
    register int first_nuke;
    register int show_comm;
    register int i;
    struct nstr_item nstr;
    struct nukstr nuk;
    struct sctstr sect;

    if (!snxtitem(&nstr, EF_NUKE, player->argp[1]))
	return RET_SYN;

    while (nxtitem(&nstr, (s_char *)&nuk)) {
	if (!player->god && !player->owner)
	    continue;
	if (nuk.nuk_own == 0)
	    continue;
	if (first_line++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("  sect        eff num nuke-type         lcm   hcm   oil   rad avail\n");
	}
	getsect(nuk.nuk_x, nuk.nuk_y, &sect);
	first_nuke = 1;
	show_comm = 0;
	if (sect.sct_type == SCT_NUKE && sect.sct_effic >= 60)
	    show_comm = 1;
	for (i = 0; i < N_MAXNUKE; i++) {
	    if (nuk.nuk_types[i] > 0) {
		if (first_nuke) {
		    if (player->god)
			pr("%-3d ", nuk.nuk_own);
		    prxy("%4d,%-4d", sect.sct_x, sect.sct_y, player->cnum);
		    pr(" %c", dchr[sect.sct_type].d_mnem);
		    if (sect.sct_newtype != sect.sct_type)
			pr("%c", dchr[sect.sct_newtype].d_mnem);
		    else
			pr(" ");
		    pr("%4d%%", sect.sct_effic);
		    first_nuke = 0;
		} else {
		    pr("                 ");
		}

		pr("%3d ", nuk.nuk_types[i]);
		pr("%-16.16s ", nchr[i].n_name);

		if (show_comm) {
		    pr("%5d ", sect.sct_item[I_LCM]);
		    pr("%5d ", sect.sct_item[I_HCM]);
		    pr("%5d ", sect.sct_item[I_OIL]);
		    pr("%5d ", sect.sct_item[I_RAD]);
		    pr("%5d", sect.sct_avail);
		    show_comm = 0;
		}
		pr("\n");
	    }
	}
    }
    return RET_OK;
}
