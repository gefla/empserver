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
 *  cens.c: Do a census report
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "path.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"

static void cens_hdr(void);

int
cens(void)
{
    struct sctstr sect;
    int nsect;
    int n;
    struct nstr_sect nstr;
    s_char dirstr[20];

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    prdate();
    for (n = 1; n <= 6; n++)
	dirstr[n] = dirch[n];
    dirstr[0] = '.';
    dirstr[7] = '$';
    dirstr[8] = '\0';
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (nsect++ == 0)
	    cens_hdr();
	if (player->god)
	    pr("%3d ", sect.sct_own);
	prxy("%4d,%-4d", nstr.x, nstr.y, player->cnum);
	pr(" %c", dchr[sect.sct_type].d_mnem);
	if (sect.sct_newtype != sect.sct_type)
	    pr("%c", dchr[sect.sct_newtype].d_mnem);
	else
	    pr(" ");
	pr("%4d%%", sect.sct_effic);
	if (sect.sct_off)
	    pr(" no ");
	else
	    pr("    ");
	pr("%4d", sect.sct_mobil);

	pr(" %c", dirstr[sect.sct_del[I_UW] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_FOOD] & 0x7]);

	n = sect.sct_dist[I_UW] % 1000;
	pr(" %c", n == 0 ? '.' : '0' + (n / 100));
	n = sect.sct_dist[I_FOOD] % 1000;
	pr("%c ", n == 0 ? '.' : '0' + (n / 100));
	pr("%c", sect.sct_own != sect.sct_oldown ? '*' : ' ');

	pr("%5d", sect.sct_item[I_CIVIL]);
	pr("%5d", sect.sct_item[I_MILIT]);
	pr("%5d", sect.sct_item[I_UW]);
	pr("%5d", sect.sct_item[I_FOOD]);
	pr("%4d%%", sect.sct_work);
	pr("%5d", sect.sct_avail);
	if (!player->god) {
	    if (sect.sct_terr)
		pr("%4d", sect.sct_terr);
	    else
		pr("    ");
	}
	if (opt_FALLOUT) {
	    pr("%5d", sect.sct_fallout);
	}
	set_coastal(&sect);
	if (sect.sct_coastal)
	    pr("%4d", sect.sct_coastal);
	pr("\n");
    }
    if (nsect == 0) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return 0;
}

static void
cens_hdr(void)
{
    if (player->god)
	pr("    ");
    pr("CENSUS                   del dst\n");
    if (player->god)
	pr("own ");
    pr("  sect        eff ");
    pr("prd ");
    pr("mob uf uf *  civ  mil   uw food work avail ");
    if (!player->god)
	pr("ter ");
    if (opt_FALLOUT)
	pr("fall ");
    pr("coa");
    pr("\n");
}

void
set_coastal(struct sctstr *sp)
{
    int n;
    struct sctstr sect;
    u_char start_flags = sp->sct_coastal;

    /* It's already been set, it didn't change (you can't fill
       in water, even with bridge spans they are still coastal.) */
    if (sp->sct_coastal)
	return;
    for (n = 1; n <= 6; ++n) {	/* Directions */
	getsect(sp->sct_x + diroff[n][0], sp->sct_y + diroff[n][1], &sect);
	if (sect.sct_type == SCT_WATER ||
	    sect.sct_type == SCT_BTOWER || sect.sct_type == SCT_BSPAN) {
	    sp->sct_coastal = 1;
	    if (start_flags != sp->sct_coastal)
		putsect(sp);
	    return;
	}
    }
    sp->sct_coastal = 0;
    if (start_flags != sp->sct_coastal)
	putsect(sp);
}
