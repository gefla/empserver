/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  setsect.c: Give resources to a country
 *
 *  Known contributors to this file:
 *     David Muir Sharnoff
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2013
 */

#include <config.h>

#include "commands.h"
#include "news.h"
#include "optlist.h"


/*
 * format: setres thing <sect> <#>
 */
int
setsector(void)
{
    struct sctstr sect;
    char *what;
    int amt, current;
    char *p;
    struct nstr_sect nstr;
    char buf[1024];
    char char0, char1;

    what = getstarg(player->argp[1],
		    "Give what (iron, gold, oil, uranium, fertility, owner, eff., mob., work, avail., oldown, mines)? ",
		    buf);
    if (!what)
	return RET_SYN;
    char0 = what[0];
    char1 = what[1];

    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    while (nxtsct(&nstr, &sect) > 0) {
	p = getstarg(player->argp[3], "What value : ", buf);
	if (!p || !*p)
	    return RET_SYN;
	amt = atoi(p);
	if (!check_sect_ok(&sect))
	    return RET_FAIL;
	switch (char0) {
	case 'i':
	    current = sect.sct_min;
	    current += amt;
	    current = LIMIT_TO(current, 0, 100);
	    if (sect.sct_own != 0)
		resnoise(&sect, "Iron ore content", sect.sct_min, current);
	    sect.sct_min = (unsigned char)current;
	    break;
	case 'g':
	    current = sect.sct_gmin;
	    current += amt;
	    current = LIMIT_TO(current, 0, 100);
	    if (sect.sct_own != 0)
		resnoise(&sect, "Gold content", sect.sct_gmin, current);
	    sect.sct_gmin = (unsigned char)current;
	    break;
	case 'o':
	    switch (char1) {
	    case 'i':
		current = sect.sct_oil;
		current += amt;
		current = LIMIT_TO(current, 0, 100);
		if (sect.sct_own != 0)
		    resnoise(&sect, "Oil content", sect.sct_oil, current);
		sect.sct_oil = (unsigned char)current;
		break;
	    case 'w':
		if ((amt < 0) || (amt > MAXNOC - 1))
		    return RET_SYN;
		pr("Owner of %s changed from %s to %s.\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum),
		   prnatid(sect.sct_own), prnatid(amt));
		if (sect.sct_own) {
		    wu(0, sect.sct_own,
		       "Sector %s taken from you by an act of %s!\n",
		       xyas(sect.sct_x, sect.sct_y, sect.sct_own),
		       cname(player->cnum));
		}
		if (amt)
		    wu(0, amt,
		       "Sector %s given to you by an act of %s!\n",
		       xyas(sect.sct_x, sect.sct_y, amt),
		       cname(player->cnum));
		sect.sct_own = (natid)amt;
		break;
	    case 'l':
		if ((amt < 0) || (amt > MAXNOC - 1))
		    return RET_SYN;
		pr("Old owner of %s changed from %s to %s.\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum),
		   prnatid(sect.sct_oldown), prnatid(amt));
		sect.sct_oldown = (natid)amt;
		break;
	    default:
		pr("huh?\n");
		return RET_SYN;
	    }
	    break;
	case 'e':
	    current = sect.sct_effic;
	    current += amt;
	    current = LIMIT_TO(current, 0, 100);
	    pr("Efficiency in %s changed to %d.\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), current);
	    sect.sct_effic = current;
	    break;
	case 'm':
	    switch (char1) {
	    case 'i':
		current = sect.sct_mines;
		current += amt;
		current = LIMIT_TO(current, 0, MINES_MAX);
		if (sect.sct_own != 0 && sect.sct_own == sect.sct_oldown)
		    resnoise(&sect, "Mines", sect.sct_mines, current);
		sect.sct_mines = current;
		break;
	    case 'o':
		current = sect.sct_mobil;
		current += amt;
		current = LIMIT_TO(current, -127, 127);
		pr("Mobility in %s changed to %d.\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum), current);
		sect.sct_mobil = current;
		break;
	    default:
		pr("huh?\n");
		return RET_SYN;
	    }
	    break;
	case 'a':
	    current = sect.sct_avail;
	    current += amt;
	    current = LIMIT_TO(current, 0, 9999);
	    pr("Available in %s changed to %d.\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), current);
	    sect.sct_avail = (short)current;
	    break;
	case 'w':
	    current = sect.sct_work;
	    current += amt;
	    current = LIMIT_TO(current, 0, 100);
	    pr("Work in %s changed to %d.\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), current);
	    sect.sct_work = (unsigned char)current;
	    break;
	case 'f':
	    current = sect.sct_fertil;
	    current += amt;
	    current = LIMIT_TO(current, 0, 100);
	    if (sect.sct_own != 0)
		resnoise(&sect, "Fertility content", sect.sct_fertil, current);
	    sect.sct_fertil = (unsigned char)current;
	    break;
	case 'u':
	    current = sect.sct_uran;
	    current += amt;
	    current = LIMIT_TO(current, 0, 100);
	    if (sect.sct_own != 0)
		resnoise(&sect, "Uranium content", sect.sct_uran, current);
	    sect.sct_uran = (unsigned char)current;
	    break;
	default:
	    pr("huh?\n");
	    return RET_SYN;
	}
	putsect(&sect);
    }
    return RET_OK;
}

static void
resbenefit(natid who, int good)
{
    if (!opt_GODNEWS)
	return;

    if (good) {
	if (who)
	    nreport(player->cnum, N_AIDS, who, 1);
    } else {
	if (who)
	    nreport(player->cnum, N_HURTS, who, 1);
    }
}

void
resnoise(struct sctstr *sptr, char *name, int old, int new)
{
    pr("%s of %s changed from %d to %d\n",
       name, xyas(sptr->sct_x, sptr->sct_y, player->cnum), old, new);
    if (sptr->sct_own)
	wu(0, sptr->sct_own,
	   "%s in %s was changed from %d to %d by an act of %s\n",
	   name, xyas(sptr->sct_x, sptr->sct_y, sptr->sct_own),
	   old, new, cname(player->cnum));
    resbenefit(sptr->sct_own, (old < new));
}
