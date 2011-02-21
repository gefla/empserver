/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nati.c: List nation information
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"

int
nati(void)
{
    struct natstr *natp;
    struct sctstr sect;
    double hap;
    int mil;
    int civ;
    int cnum;
    int poplimit, safepop, uwpop;
    double pfac;

    if (player->argp[1]) {
	if (!(natp = natargp(player->argp[1], NULL)))
	    return RET_SYN;
    } else
	natp = getnatp(player->cnum);

    cnum = natp->nat_cnum;
    if (!player->god && cnum != player->cnum) {
	pr("Only deities can request a nation report for another country.\n");
	return RET_FAIL;
    }

    pr("\n(#%i) %s Nation Report\t", cnum, cname(cnum));
    prdate();
    pr("Nation status is %s", natstate(natp));
    pr("     Bureaucratic Time Units: %d\n", natp->nat_btu);
    if (natp->nat_stat != STAT_UNUSED) {
	getsect(natp->nat_xcap, natp->nat_ycap, &sect);
	if (influx(natp))
	    pr("No capital (was at %s).\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	else {
	    civ = sect.sct_item[I_CIVIL];
	    mil = sect.sct_item[I_MILIT];
	    pr("%d%% eff %s at %s has %d civilian%s & %d military\n",
	       sect.sct_effic,
	       sect.sct_type == SCT_CAPIT ? "capital" : "mountain capital",
	       xyas(sect.sct_x, sect.sct_y, player->cnum),
	       civ, splur(civ), mil);
	}
    }
    pr(" The treasury has $%.2f", (double)natp->nat_money);
    pr("     Military reserves: %ld\n", natp->nat_reserve);
    pr("Education..........%6.2f       Happiness.......%6.2f\n",
       natp->nat_level[NAT_ELEV], natp->nat_level[NAT_HLEV]);
    pr("Technology.........%6.2f       Research........%6.2f\n",
       natp->nat_level[NAT_TLEV], natp->nat_level[NAT_RLEV]);
    pr("Technology factor :%6.2f%%", tfact(cnum, 100.));

    if (opt_NO_PLAGUE)
	pfac = 0.0;
    else
	pfac = (natp->nat_level[NAT_TLEV] + 100.0) /
	    (natp->nat_level[NAT_RLEV] + 100.0);
    pr("     Plague factor : %6.2f%%\n", pfac);
    pr("\n");

    poplimit = max_population(natp->nat_level[NAT_RLEV], SCT_MINE, 0);
    pr("Max population : %d\n", poplimit);

    safepop = (int)(poplimit / (1.0 + obrate * (double)etu_per_update));
    uwpop = (int)(poplimit / (1.0 + uwbrate * (double)etu_per_update));
    pr("Max safe population for civs/uws: %d/%d\n", safepop, uwpop);

    hap = hap_req(natp);
    if (hap > 0.0)
	pr("Happiness needed is %.2f\n", hap);
    else
	pr("No happiness needed\n");

    return RET_OK;
}
