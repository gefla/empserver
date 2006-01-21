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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  move.c: Move something somewhere.
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "sect.h"
#include "item.h"
#include "file.h"
#include "xy.h"
#include "path.h"
#include "nat.h"
#include "map.h"
#include "nsc.h"
#include "damage.h"
#include "prototypes.h"

static int move_map(s_char *what, coord curx, coord cury, s_char *arg);

int
move_ground(s_char *what, struct sctstr *start, struct sctstr *end,
	    double weight, s_char *path,
	    int (*map)(s_char *, coord, coord, s_char *), int exploring,
	    int *dam)
{
    struct sctstr sect, ending_sect;
    struct sctstr next, dsect;
    coord curx, cury, oldx, oldy;
    coord tmpx, tmpy;
    coord dx, dy;
    s_char *movstr;
    double sect_mcost;
    double total_mcost;
    double mv_cost;
    double mobility = (double)start->sct_mobil;
    int dir;
    int intcost;
    int takedam = (*dam), out = 0;
    s_char bpath[512];
    s_char buf2[512];
    s_char prompt[128];
    s_char buf[1024];

    *end = *start;
    if (mobility <= 0.0)
	return -1;
    *dam = 0;
    if (path && sarg_xy(path, &dx, &dy) && getsect(dx, dy, &ending_sect)) {
	if ((ending_sect.sct_x == start->sct_x) &&
	    (ending_sect.sct_y == start->sct_y)) {
	    pr("Start sector is ending sector!\n");
	    return -1;
	}
	pr("Looking for best path to %s\n", path);
	path = BestLandPath(buf2, start, &ending_sect, &total_mcost,
			    MOB_ROAD);
	if (exploring && (path != (s_char *)0))	/* take off the 'h' */
	    *(path + strlen(path) - 1) = '\0';
	if (path == (s_char *)0)
	    pr("No owned path exists!\n");
	else {
	    pr("Using best path '%s', movement cost %1.3f\n",
	       path, total_mcost);
	    strncpy(bpath, path, sizeof(bpath));
	    path = bpath;
	}
	if ((total_mcost * weight) > mobility) {
	    pr("Not enough mobility to go all the way. Nothing moved.\n");
	    return -1;
	}
    }
    movstr = path;
    curx = start->sct_x;
    cury = start->sct_y;
    total_mcost = 0.0;
    if (getsect(curx, cury, &sect) < 0) {
	logerror("move_path: getsect %d,%d", curx, cury);
	return -1;
    }
    for (;;) {
	oldx = curx;
	oldy = cury;
	if (movstr == 0 || *movstr == 0) {
	    if (exploring) {
		map(what, curx, cury, (s_char *)0);
	    } else {
		move_map(what, curx, cury, (s_char *)0);
	    }
	    sprintf(prompt, "<%.1f: %c %s> ", mobility,
		    dchr[sect.sct_type].d_mnem,
		    xyas(sect.sct_x, sect.sct_y, player->cnum));
	    movstr = getstring(prompt, buf);
	}
	if (movstr && sarg_xy(movstr, &dx, &dy)) {
	    if (getsect(dx, dy, &dsect)) {
		movstr = BestLandPath(buf2, &sect, &dsect, &mv_cost,
				      MOB_ROAD);
	    } else {
		pr("Invalid destination sector!\n");
		movstr = (s_char *)0;
	    }

	    if (movstr == (s_char *)0) {
		pr("Can't get to %s from here!\n",
		   xyas(dx, dy, player->cnum));
		movstr = (s_char *)0;
	    } else {
		if ((mv_cost * weight) > mobility) {
		    pr("Not enough mobility to go all the way. Nothing moved.\n");
		    movstr = (s_char *)0;
		} else {
		    pr("Using best path '%s', movement cost %1.3f\n",
		       movstr, mv_cost);
		    strncpy(bpath, movstr, sizeof(bpath));
		    movstr = bpath;
		}
	    }
	}
	if (movstr == 0 || *movstr == 0) {
	    buf2[0] = dirch[DIR_STOP];
	    buf2[1] = 0;
	    movstr = buf2;
	}
	if ((dir = chkdir(*movstr, DIR_STOP, DIR_MAP)) < 0) {
	    pr("\"%c\" is not legal...", *movstr);
	    direrr("'%c' to stop ", "'%c' to view ", "& '%c' to map\n");
	    *movstr = 0;
	    continue;
	}
	movstr++;
	if (dir == DIR_MAP) {
	    if (!exploring)
		map(what, curx, cury, movstr + 1);
	    *movstr = 0;
	    continue;
	} else if (dir == DIR_STOP)
	    break;
	else if (dir == DIR_VIEW) {
	    pr("%d%% %s with %d civilians.\n", sect.sct_effic,
	       dchr[sect.sct_type].d_name,
	       sect.sct_item[I_CIVIL]);
	    continue;
	}
	/*
	 * now see if we can move into the
	 * next sector.  Mobility, terrain,
	 * or ownership may prevent us.
	 */
	tmpx = curx + diroff[dir][0];
	tmpy = cury + diroff[dir][1];
	if (getsect(tmpx, tmpy, &next) < 0) {
	    pr("You can't go there...\n");
	    *movstr = 0;
	    continue;
	}
	if (!player->god) {
	    if ((next.sct_type == SCT_SANCT) &&
		(next.sct_own != player->cnum)) {
		pr("Converts, huh?\n");
		*movstr = 0;
		continue;
	    }
	    sect_mcost = sector_mcost(&next, MOB_ROAD);
	    if ((!player->owner && (!exploring
				    || next.sct_item[I_MILIT]
				    || next.sct_item[I_CIVIL]))
		|| sect_mcost == -1.0) {
		/* already-owned, or prohibited terrain */
		pr("You can't go there...\n");
		*movstr = 0;
		continue;
	    }
	    sect_mcost *= weight;
	    if (sect_mcost > mobility) {
		pr("Not enough mobility.  ");
		pr("You can't go there...\n");
		*movstr = 0;
		continue;
	    }
	    mobility -= sect_mcost;
	    total_mcost += sect_mcost;
	}
	curx = tmpx;
	cury = tmpy;
	if (cury != start->sct_y)
	    out = 1;
	if (curx != start->sct_x)
	    out = 1;

	sect = next;

	if (takedam)
	    *dam += check_lmines(sect.sct_x, sect.sct_y, weight);
	if (*dam >= 100)
	    break;
	/*
	 * Check and see if anyone will interdict us
	 */
	if (takedam && chance(weight / 100.0) &&
	    ((curx != oldx) || (cury != oldy)))
	    (*dam) += ground_interdict(curx, cury, player->cnum,
				       "commodities");
	if (*dam >= 100)
	    break;
    }
    *end = sect;
    intcost = (int)total_mcost;
    if (intcost < 0)
	return -1;
    if ((start->sct_x == end->sct_x) && (start->sct_y == end->sct_y)
	&& !out)
	return -1;

    if (chance(total_mcost - intcost))
	intcost++;
    return intcost;
}


/*ARGSUSED*/
static int
move_map(s_char *what, coord curx, coord cury, s_char *arg)
{
    struct nstr_sect ns;
    struct natstr *np;
    struct sctstr sect;
    coord rel_x, rel_y;
    s_char range[128];
    s_char view[7];
    int i;
    int changed = 0;

    np = getnatp(player->cnum);
    rel_x = xrel(np, curx);
    rel_y = yrel(np, cury);
    sprintf(range, "%d:%d,%d:%d", rel_x - 2, rel_x + 2, rel_y - 1,
	    rel_y + 1);
    player->condarg = 0;
    /* This is necessary, otherwise move_map would attempt to pay */
    /* attention to the conditional arguments left behind by such */
    /* a command as "tran p -1,-1 ?eff=100".. It'd then only see  */
    /* 100% efficienct sects, and get all screwed up         --ts */
    if (!snxtsct(&ns, range))
	return RET_FAIL;
    i = 0;
    while (i < 7 && nxtsct(&ns, &sect)) {
	view[i] = dchr[sect.sct_type].d_mnem;
	switch (sect.sct_type) {
	case SCT_WATER:
	case SCT_RURAL:
	case SCT_MOUNT:
	case SCT_WASTE:
	case SCT_PLAINS:
	    break;
	default:
	    if (sect.sct_own != player->cnum && !player->god)
		view[i] = '?';
	    break;
	}
	changed += map_set(player->cnum, ns.x, ns.y, view[i], 0);
	i++;
    }
    if (changed)
	writemap(player->cnum);
    if (!getsect(curx, cury, &sect))
	return RET_FAIL;
    pr("    %c %c      eff   mob   civ  mil   uw food  work  avail\n",
       view[0], view[1]);
    pr("   %c %c %c     %3d   %3d  %4d %4d %4d %4d   %3d   %3d\n",
       view[2], view[3], view[4],
       sect.sct_effic, sect.sct_mobil,
       sect.sct_item[I_CIVIL], sect.sct_item[I_MILIT], sect.sct_item[I_UW],
       sect.sct_item[I_FOOD], sect.sct_work, sect.sct_avail);
    pr("    %c %c\n", view[5], view[6]);
    return RET_OK;
}

int
fly_map(coord curx, coord cury)
{
    struct nstr_sect ns;
    struct natstr *np;
    struct sctstr sect;
    coord rel_x, rel_y;
    s_char view[7];
    int i;
    s_char range[128];

    np = getnatp(player->cnum);
    rel_x = xrel(np, curx);
    rel_y = yrel(np, cury);
    sprintf(range, "%d:%d,%d:%d", rel_x - 2, rel_x + 2, rel_y - 1,
	    rel_y + 1);
    player->condarg = 0;
    /* This is necessary, otherwise move_map would attempt to pay */
    /* attention to the conditional arguments left behind by such */
    /* a command as "tran p -1,-1 ?eff=100".. It'd then only see  */
    /* 100% efficienct sects, and get all screwed up         --ts */

    if (!snxtsct(&ns, range))
	return RET_FAIL;
    i = 0;
    while (i < 7 && nxtsct(&ns, &sect)) {
	if (!(view[i] = player->bmap[sctoff(ns.x, ns.y)]))
	    view[i] = ' ';
	i++;
    }

    pr("    %c %c\n", view[0], view[1]);
    pr("   %c %c %c\n", view[2], view[3], view[4]);
    pr("    %c %c\n", view[5], view[6]);
    return RET_OK;
}

int
check_lmines(coord x, coord y, double weight)
{
    struct sctstr sect;
    int dam = 0;

    getsect(x, y, &sect);
    if (sect.sct_mines > 0 &&
	sect.sct_oldown != player->cnum &&
	chance(DMINE_LHITCHANCE(sect.sct_mines)) && chance(weight / 100.0)) {
	pr_beep();
	pr("Blammo! Landmines detected! in %s  ",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	dam = roll(20);
	--sect.sct_mines;
	putsect(&sect);
	pr("%d damage sustained.\n", dam);
    }
    return dam;
}
