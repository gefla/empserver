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
 *  move.c: Move something somewhere.
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2011
 */

#include <config.h>

#include <ctype.h>
#include "damage.h"
#include "file.h"
#include "map.h"
#include "path.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"

static int move_map(coord curx, coord cury, char *arg);

int
move_ground(struct sctstr *start, struct sctstr *end,
	    double weight, char *path,
	    int (*map)(coord, coord, char *), int exploring,
	    int *dam)
{
    struct sctstr sect;
    struct sctstr next;
    coord curx, cury, oldx, oldy;
    coord tmpx, tmpy;
    coord dx, dy;
    char *movstr;
    double sect_mcost;
    double total_mcost;
    double mv_cost;
    size_t len;
    double mobility = start->sct_mobil;
    int dir;
    int intcost;
    int takedam = *dam;
    int out = 0;
    char prompt[128];
    char buf[1024];

    *end = *start;
    if (mobility <= 0.0)
	return -1;
    *dam = 0;
    if (path && sarg_xy(path, &dx, &dy)) {
	if (dx == start->sct_x && dy == start->sct_y) {
	    pr("Start sector is ending sector!\n");
	    return -1;
	}
	pr("Looking for best path to %s\n", path);
	total_mcost = path_find(start->sct_x, start->sct_y, dx, dy,
				player->cnum, MOB_MOVE);
	path = NULL;
	if (total_mcost < 0)
	    pr("No owned path exists!\n");
	else {
	    len = path_find_route(buf, sizeof(buf),
				  start->sct_x, start->sct_y, dx, dy);
	    if (!exploring) {
		if (len < sizeof(buf))
		    strcpy(buf + len, "h");
		len++;
	    }
	    if (len >= sizeof(buf))
		pr("Can't handle path to %s, it's too long, sorry.\n",
		   xyas(dx, dy, player->cnum));
	    else {
		path = buf;
		pr("Using best path '%s', movement cost %1.3f\n",
		   path, total_mcost);
		if (total_mcost * weight > mobility) {
		    pr("Not enough mobility to go all the way."
		       " Nothing moved.\n");
		    return -1;
		}
	    }
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
	if (!movstr || *movstr == 0) {
	    if (exploring) {
		map(curx, cury, NULL);
	    } else {
		move_map(curx, cury, NULL);
	    }
	    sprintf(prompt, "<%.1f: %c %s> ", mobility,
		    dchr[sect.sct_type].d_mnem,
		    xyas(sect.sct_x, sect.sct_y, player->cnum));
	    movstr = getstring(prompt, buf);
	}
	if (movstr && sarg_xy(movstr, &dx, &dy)) {
	    mv_cost = path_find(sect.sct_x, sect.sct_y, dx, dy,
				player->cnum, MOB_MOVE);
	    if (mv_cost < 0) {
		pr("Can't get to %s from here!\n",
		   xyas(dx, dy, player->cnum));
		movstr = NULL;
	    } else {
		len = path_find_route(buf, sizeof(buf),
				      sect.sct_x, sect.sct_y, dx, dy);
		if (len < sizeof(buf))
		    strcpy(buf + len, "h");
		len++;
		if (len >= sizeof(buf)) {
		    pr("Can't handle path to %s, it's too long, sorry.\n",
		       xyas(dx, dy, player->cnum));
		    movstr = NULL;
		} else {
		    if ((mv_cost * weight) > mobility) {
			pr("Not enough mobility to go all the way. Nothing moved.\n");
			movstr = NULL;
		    } else {
			movstr = buf;
			pr("Using best path '%s', movement cost %1.3f\n",
			   movstr, mv_cost);
		    }
		}
	    }
	}
	if (!movstr || *movstr == 0) {
	    buf[0] = dirch[DIR_STOP];
	    buf[1] = 0;
	    movstr = buf;
	}
	if ((dir = chkdir(*movstr, DIR_STOP, DIR_MAP)) < 0) {
	    pr("\"%c\" is not legal...", *movstr);
	    direrr("'%c' to stop ", "'%c' to view ", "& '%c' to map\n");
	    *movstr = 0;
	    continue;
	}
	do  movstr++; while (isspace(*movstr));
	if (dir == DIR_MAP) {
	    if (!exploring)
		map(curx, cury, movstr);
	    *movstr = 0;
	    continue;
	} else if (dir == DIR_STOP)
	    break;
	else if (dir == DIR_VIEW) {
	    pr("%d%% %s with %d civilians.\n", sect.sct_effic,
	       dchr[sect.sct_type].d_name, sect.sct_item[I_CIVIL]);
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
	    sect_mcost = sector_mcost(&next, MOB_MOVE);
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
	curx = next.sct_x;
	cury = next.sct_y;
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
	    *dam += ground_interdict(curx, cury, player->cnum,
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
move_map(coord curx, coord cury, char *arg)
{
    struct nstr_sect ns;
    struct sctstr sect;
    char view[7];
    int i;
    int changed = 0;

    snxtsct_dist(&ns, curx, cury, 1);
    i = 0;
    while (i < 7 && nxtsct(&ns, &sect)) {
	/* Nasty: this relies on the iteration order */
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
    struct sctstr sect;
    char view[7];
    int i;

    snxtsct_dist(&ns, curx, cury, 1);
    i = 0;
    while (i < 7 && nxtsct(&ns, &sect)) {
	/* Nasty: this relies on the iteration order */
	if (!(view[i] = player->bmap[sect.sct_uid]))
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
    if (SCT_LANDMINES(&sect) > 0 &&
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
