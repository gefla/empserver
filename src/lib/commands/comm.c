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
 *  comm.c: Do a commodity report
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "commands.h"
#include "path.h"

static void prthresh(int val);

int
comm(void)
{
    struct sctstr sect;
    char dirstr[12];
    int nsect;
    int n;
    struct nstr_sect nstr;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    prdate();
    (void)strcpy(dirstr, ".      $");
    n = 1;
    for (n = 1; n <= 6; n++)
	dirstr[n] = dirch[n];
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (nsect++ == 0) {
	    if (player->god)
		pr("   ");
	    pr("COMMODITIES deliver--  distribute\n");
	    if (player->god)
		pr("   ");
	    pr("  sect      sgpidbolhr sgpidbolhr  sh gun  pet iron dust bar  oil  lcm  hcm rad\n");
	}
	if (player->god)
	    pr("%3d", sect.sct_own);
	prxy("%4d,%-4d", nstr.x, nstr.y, player->cnum);
	pr(" %c", dchr[sect.sct_type].d_mnem);
	pr(" %c", dirstr[sect.sct_del[I_SHELL] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_GUN] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_PETROL] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_IRON] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_DUST] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_BAR] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_OIL] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_LCM] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_HCM] & 0x7]);
	pr("%c ", dirstr[sect.sct_del[I_RAD] & 0x7]);
	prthresh(sect.sct_dist[I_SHELL]);
	prthresh(sect.sct_dist[I_GUN]);
	prthresh(sect.sct_dist[I_PETROL]);
	prthresh(sect.sct_dist[I_IRON]);
	prthresh(sect.sct_dist[I_DUST]);
	prthresh(sect.sct_dist[I_BAR]);
	prthresh(sect.sct_dist[I_OIL]);
	prthresh(sect.sct_dist[I_LCM]);
	prthresh(sect.sct_dist[I_HCM]);
	prthresh(sect.sct_dist[I_RAD]);
	pr("%4d", sect.sct_item[I_SHELL]);
	pr("%4d", sect.sct_item[I_GUN]);
	pr("%5d", sect.sct_item[I_PETROL]);
	pr("%5d", sect.sct_item[I_IRON]);
	pr("%5d", sect.sct_item[I_DUST]);
	pr("%4d", sect.sct_item[I_BAR]);
	pr("%5d", sect.sct_item[I_OIL]);
	pr("%5d", sect.sct_item[I_LCM]);
	pr("%5d", sect.sct_item[I_HCM]);
	pr("%4d", sect.sct_item[I_RAD]);
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
prthresh(int val)
{
    if (val >= 1000)
	val = 'a';
    else if (val > 0)
	val = val / 100 + '0';
    else
	val = '.';
    pr("%c", val);
}
