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
 *  terr.c: Designate territory of sectors
 * 
 *  Known contributors to this file:
 *     Edward M. Rynes Esq.
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "commands.h"

int
terr(void)
{
    struct sctstr sect;
    s_char *p;
    int terr_n;
    int field;
    struct nstr_sect nstr;
    s_char prompt[128];
    s_char buf[1024];

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    if (player->argp[3] && isdigit(*(player->argp[3]))) {
	field = atoi(player->argp[3]);
    } else {
	field = 0;
    }
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	sprintf(prompt, "%s %d%% %s  territory? ",
		xyas(nstr.x, nstr.y, player->cnum),
		sect.sct_effic, dchr[sect.sct_type].d_name);
	if ((p = getstarg(player->argp[2], prompt, buf)) == 0)
	    return RET_FAIL;
	if (!check_sect_ok(&sect))
	    return RET_FAIL;
	if (*p == 0)
	    continue;
	terr_n = atoi(p);
	while (terr_n < 0 || terr_n > 99 || *p < '0' || *p > '9') {
	    pr("Enter a number between 0 and 99!\n");
	    sprintf(prompt, "%s %d%% %s  territory? ",
		    xyas(nstr.x, nstr.y, player->cnum),
		    sect.sct_effic, dchr[sect.sct_type].d_name);
	    if ((p = getstarg((s_char *)0, prompt, buf)) == 0)
		return RET_FAIL;
	    if (!check_sect_ok(&sect))
		return RET_FAIL;
	    if (*p == 0)
		break;
	    terr_n = atoi(p);
	}
	if (*p == 0)
	    continue;
	switch (field) {
	case 1:
	    sect.sct_terr1 = terr_n;
	    break;
	case 2:
	    sect.sct_terr2 = terr_n;
	    break;
	case 3:
	    sect.sct_terr3 = terr_n;
	    break;
	default:
	    sect.sct_terr = terr_n;
	}
	putsect(&sect);
    }
    return RET_OK;
}
