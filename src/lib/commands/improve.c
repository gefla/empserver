/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  improve.c: Improve the infrastructure of a sector
 *
 *  Known contributors to this file:
 *     Steve McClure, 1996-2000
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "chance.h"
#include "commands.h"

int
improve(void)
{
    struct sctintrins *incp;
    struct sctstr sect;
    int nsect;
    struct nstr_sect nstr;
    char *p;
    char buf[1024];
    char prompt[128];
    int type;
    int value;
    int ovalue;
    int maxup, i, lim;
    struct natstr *natp;
    int wanted;

    p = getstarg(player->argp[1],
		 "Improve what ('road', 'rail' or 'defense')? ",
		 buf);
    if (!p || !*p)
	return RET_SYN;
    if (!strncmp(p, "ro", 2))
	type = INT_ROAD;
    else if (!strncmp(p, "ra", 2))
	type = INT_RAIL;
    else if (!strncmp(p, "de", 2)) {
	type = INT_DEF;
    } else
	return RET_SYN;

    incp = &intrchr[type];
    if (!incp->in_enable) {
	pr("%s improvement is disabled.\n", incp->in_name);
	return RET_FAIL;
    }

    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (type == INT_ROAD)
	    value = sect.sct_road;
	else if (type == INT_RAIL)
	    value = sect.sct_rail;
	else /* type == INT_DEF */
	    value = sect.sct_defense;
	sprintf(prompt, "Sector %s has a %s of %d%%.  Improve how much? ",
		xyas(sect.sct_x, sect.sct_y, player->cnum),
		incp->in_name, value);
	p = getstarg(player->argp[3], prompt, buf);
	if (!p || !*p)
	    continue;
	if (!check_sect_ok(&sect))
	    continue;
	maxup = 100 - value;
	wanted = atoi(p);
	if (wanted < 0)
	    continue;
	if (wanted < maxup)
	    maxup = wanted;
	if (!maxup)
	    continue;

	lim = (sect. sct_mobil - 1) * 100 / incp->in_bmobil;
	if (lim <= 0) {
	    pr("Not enough mobility in %s\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	    continue;
	}
	if (maxup > lim)
	    maxup = lim;

	for (i = I_NONE + 1; i <= I_MAX; i++) {
	    if (!incp->in_mat[i])
		continue;
	    lim = sect.sct_item[i] * 100 / incp->in_mat[i];
	    if (lim <= 0) {
		pr("Not enough %s in %s\n",
		   ichr[i].i_name,
		   xyas(sect.sct_x, sect.sct_y, player->cnum));
	    }
	    if (maxup > lim)
		maxup = lim;
	}
	if (maxup <= 0)
	    continue;

	natp = getnatp(player->cnum);
	lim = (natp->nat_money - player->dolcost) * 100 / incp->in_cost;
	if (lim <= 0) {
	    pr("Not enough money left to improve %s by %d%%\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), maxup);
	    break;
	}
	if (maxup > lim)
	    maxup = lim;

	for (i = I_NONE + 1; i <= I_MAX; i++)
	    sect.sct_item[i] -= roundavg(maxup * incp->in_mat[i] / 100.0);
	sect.sct_mobil -= roundavg(maxup * incp->in_bmobil / 100.0);
	player->dolcost += maxup * incp->in_cost / 100.0;
	ovalue = value;
	value += maxup;
	if (CANT_HAPPEN(value > 100))
	    value = 100;
	pr("Sector %s %s increased from %d%% to %d%%\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum),
	   incp->in_name, ovalue, value);
	if (type == INT_ROAD)
	    sect.sct_road = value;
	else if (type == INT_RAIL)
	    sect.sct_rail = value;
	else if (type == INT_DEF)
	    sect.sct_defense = value;
	putsect(&sect);
	nsect++;
    }
    if (nsect == 0) {
	if (player->argp[2])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return 0;
}
