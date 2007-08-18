/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  setsect.c: Give resources to a country
 * 
 *  Known contributors to this file:
 *     David Muir Sharnoff
 *     Steve McClure, 1998
 */

#include <config.h>

#include "commands.h"
#include "lost.h"
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

    if ((what = getstarg(player->argp[1],
			 "Give What (iron, gold, oil, uranium, fertility, owner, eff., mob., work, avail., oldown, mines)? ",
			 buf)) == 0)
	return RET_SYN;
    char0 = what[0];
    char1 = what[1];

    switch (char0) {
    case 'i':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    current = sect.sct_min;
	    current += amt;
	    if (current < 0)
		current = 0;
	    if (current > 100)
		current = 100;
	    if (sect.sct_own != 0)
		resnoise(&sect, 1, "Iron ore content",
			 (int)sect.sct_min, current);
	    sect.sct_min = (unsigned char)current;
	    putsect(&sect);
	}
	break;
    case 'g':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    current = sect.sct_gmin;
	    current += amt;
	    if (current < 0)
		current = 0;
	    if (current > 100)
		current = 100;
	    if (sect.sct_own != 0)
		resnoise(&sect, 1, "Gold content",
			 (int)sect.sct_gmin, current);
	    sect.sct_gmin = (unsigned char)current;
	    putsect(&sect);
	}
	break;
    case 'o':
	switch (char1) {
	case 'i':
	    if (!snxtsct(&nstr, player->argp[2]))
		return RET_SYN;
	    while (nxtsct(&nstr, &sect) > 0) {
		if (!(p = getstarg(player->argp[3], "What value : ", buf))
		    || (*p == '\0'))
		    return RET_SYN;
		amt = atoi(p);
		current = sect.sct_oil;
		current += amt;
		if (current < 0)
		    current = 0;
		if (current > 100)
		    current = 100;
		if (sect.sct_own != 0)
		    resnoise(&sect, 1, "Oil content",
			     (int)sect.sct_oil, current);
		sect.sct_oil = (unsigned char)current;
		putsect(&sect);
	    }
	    break;
	case 'w':
	    if (!snxtsct(&nstr, player->argp[2]))
		return RET_SYN;
	    while (nxtsct(&nstr, &sect) > 0) {
		if (!(p = getstarg(player->argp[3], "What value : ", buf))
		    || (*p == '\0'))
		    return RET_SYN;
		amt = atoi(p);
		if ((amt < 0) || (amt > MAXNOC - 1))
		    return RET_SYN;
		pr("Owner of %s changed from %s (#%d) to %s (#%d).\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum),
		   cname(sect.sct_own), sect.sct_own, cname(amt), amt);
		if (sect.sct_own) {
		    wu(player->cnum, sect.sct_own,
		       "Sector %s lost to deity intervention\n",
		       xyas(sect.sct_x, sect.sct_y, sect.sct_own));
		}
		if (amt)
		    wu(player->cnum, amt,
		       "Sector %s gained from deity intervention\n",
		       xyas(sect.sct_x, sect.sct_y, amt));
		makelost(EF_SECTOR, sect.sct_own, 0,
			 sect.sct_x, sect.sct_y);
		makenotlost(EF_SECTOR, amt, 0, sect.sct_x, sect.sct_y);
		sect.sct_own = (natid)amt;
		putsect(&sect);
	    }
	    break;
	case 'l':
	    if (!snxtsct(&nstr, player->argp[2]))
		return RET_SYN;
	    while (nxtsct(&nstr, &sect) > 0) {
		if (!(p = getstarg(player->argp[3], "What value : ", buf))
		    || (*p == '\0'))
		    return RET_SYN;
		amt = atoi(p);
		if ((amt < 0) || (amt > MAXNOC - 1))
		    return RET_SYN;
		pr("Old owner of %s changed from %s (#%d) to %s (#%d).\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum),
		   cname(sect.sct_oldown),
		   sect.sct_oldown, cname(amt), amt);
		sect.sct_oldown = (natid)amt;
		putsect(&sect);
	    }
	    break;
	default:
	    pr("huh?\n");
	    return RET_SYN;
	}
	break;
    case 'e':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    current = sect.sct_effic;
	    current += amt;
	    if (current < 0)
		current = 0;
	    if (current > 100)
		current = 100;
	    pr("Efficiency in %s changed to %d.\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), current);
	    sect.sct_effic = (unsigned char)current;
	    putsect(&sect);
	}
	break;
    case 'm':
	switch (char1) {
	case 'i':
	    if (!snxtsct(&nstr, player->argp[2]))
		return RET_SYN;
	    while (nxtsct(&nstr, &sect) > 0) {
		if (!(p = getstarg(player->argp[3], "What value : ", buf))
		    || (*p == '\0'))
		    return RET_SYN;
		amt = atoi(p);
		current = sect.sct_mines;
		current += amt;
		if (current < 0)
		    current = 0;
		if (current > MINES_MAX)
		    current = MINES_MAX;
		if (sect.sct_own != 0)
		    resnoise(&sect, 1, "Mines", sect.sct_mines, current);
		sect.sct_mines = current;
		putsect(&sect);
	    }
	    break;
	case 'o':
	    if (!snxtsct(&nstr, player->argp[2]))
		return RET_SYN;
	    while (nxtsct(&nstr, &sect) > 0) {
		if (!(p = getstarg(player->argp[3], "What value : ", buf))
		    || (*p == '\0'))
		    return RET_SYN;
		amt = atoi(p);
		current = sect.sct_mobil;
		current += amt;
		if (current < -127)
		    current = -127;
		if (current > 127)
		    current = 127;
		pr("Mobility in %s changed to %d.\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum), current);
		sect.sct_mobil = (short)current;
		putsect(&sect);
	    }
	    break;
	default:
	    pr("huh?\n");
	    return RET_SYN;
	}
	break;
    case 'a':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    current = sect.sct_avail;
	    current += amt;
	    if (current < 0)
		current = 0;
	    if (current > 9999)
		current = 9999;
	    pr("Available in %s changed to %d.\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), current);
	    sect.sct_avail = (short)current;
	    putsect(&sect);
	}
	break;
    case 'w':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    current = sect.sct_work;
	    current += amt;
	    if (current < 0)
		current = 0;
	    if (current > 100)
		current = 100;
	    pr("Work in %s changed to %d.\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), current);
	    sect.sct_work = (unsigned char)current;
	    putsect(&sect);
	}
	break;
    case 'f':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    current = sect.sct_fertil;
	    current += amt;
	    if (current < 0)
		current = 0;
	    if (current > 120)
		current = 120;
	    if (sect.sct_own != 0)
		resnoise(&sect, 1, "Fertility content",
			 (int)sect.sct_fertil, current);
	    sect.sct_fertil = (unsigned char)current;
	    putsect(&sect);
	}
	break;
    case 'u':
	if (!snxtsct(&nstr, player->argp[2]))
	    return RET_SYN;
	while (nxtsct(&nstr, &sect) > 0) {
	    if (!(p = getstarg(player->argp[3], "What value : ", buf)) ||
		(*p == '\0'))
		return RET_SYN;
	    amt = atoi(p);
	    current = sect.sct_uran;
	    current += amt;
	    if (current < 0)
		current = 0;
	    if (current > 100)
		current = 100;
	    if (sect.sct_own != 0)
		resnoise(&sect, 1, "Uranium content",
			 (int)sect.sct_uran, current);
	    sect.sct_uran = (unsigned char)current;
	    putsect(&sect);
	}
	break;
    default:
	pr("huh?\n");
	return RET_SYN;
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
resnoise(struct sctstr *sptr, int public_amt, char *name, int old,
	 int new)
{
    char p[100];

    pr("%s of %s changed from %d to %d%%\n",
       name, xyas(sptr->sct_x, sptr->sct_y, player->cnum), old, new);
    if (public_amt)
	(void)sprintf(p, "changed from %d to %d", old, new);
    else
	(void)sprintf(p, "%s", old < new ? "increased" : "decreased");
    if (sptr->sct_own)
	wu(0, sptr->sct_own,
	   "%s in %s was %s by an act of %s\n",
	   name, xyas(sptr->sct_x, sptr->sct_y, sptr->sct_own),
	   p, cname(player->cnum));
    resbenefit(sptr->sct_own, (old < new));
}
