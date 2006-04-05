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
 *  path.c: Routines associated with paths, directions, etc.
 * 
 *  Known contributors to this file:
 *   
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "path.h"
#include "nat.h"
#include "sect.h"
#include "file.h"
#include "prototypes.h"

int
chkdir(char dir_char, int min_dir, int max_dir)
{
    int i;

    for (i = min_dir; i <= max_dir; i++)
	if (dir_char == dirch[i])
	    return i;
    return -1;
}

void
direrr(char *stop_msg, char *view_msg, char *map_msg)
{
    pr("Legal directions are:\n");
    pr(" %c %c\n", dirch[DIR_UL], dirch[DIR_UR]);
    pr("%c   %c\n", dirch[DIR_L], dirch[DIR_R]);
    pr(" %c %c\n", dirch[DIR_DL], dirch[DIR_DR]);
    if (stop_msg != 0)
	pr(stop_msg, dirch[DIR_STOP]);
    if (view_msg != 0)
	pr(view_msg, dirch[DIR_VIEW]);
    if (map_msg != 0)
	pr(map_msg, dirch[DIR_MAP]);
}

/*
 * Map direction DIR to a direction index DIR_STOP..DIR_LAST.
 * DIR must be a valid direction.
 */
int
diridx(char dir)
{
    unsigned i = dir - 'a';

    if (CANT_HAPPEN(i >= sizeof(dirindex) / sizeof(*dirindex)
		    || dirindex[i] < 0))
	return DIR_STOP;
    return dirindex[i];
}

/*
 * return pointer to path; prompt user until a stop char
 * or a bomb char have been entered.  A "bomb" char in
 * this context is actually "execute" for the partial
 * move commands, and isn't valid for those commands
 * which do not accept partial moves.
 */
char *
getpath(char *buf, char *arg, coord x, coord y, int onlyown,
	int showdes, enum p_mode destinations)
{
    char buf2[1024];
    char *p = buf;
    char *bp;
    char prompt[128];
    coord dx, dy;
    struct sctstr sect, dsect;
    coord nx, ny;
    int dir;
    double mv_cost;

    if (arg) {
	strncpy(buf, arg, MAX_PATH_LEN - 1);
	buf[MAX_PATH_LEN - 1] = 0;
    } else {
	*p = 0;
    }

    getsect(x, y, &sect);
    nx = x;
    ny = y;

  more:
    while (*p) {
	if (sarg_xy(p, &dx, &dy)) {
	    bp = NULL;
	    switch (destinations) {
	    case P_NONE:
		pr("Destination sectors not allowed here!\n");
		break;
	    case P_WALKING:
		if (getsect(dx, dy, &dsect))
		    bp = BestLandPath(buf2, &sect, &dsect,
				      &mv_cost, MOB_ROAD);
		break;
	    case P_FLYING:
		bp = BestAirPath(buf2, nx, ny, dx, dy);
		break;
	    case P_SAILING:
		bp = BestShipPath(buf2, nx, ny, dx, dy, player->cnum);
		break;
	    }
	    if (bp && p + strlen(bp) + 1 < buf + MAX_PATH_LEN) {
		strcpy(p, bp);
		pr("Using best path  '%s'\n", p);
		pr("Using total path '%s'\n", buf);
		return buf;
	    } else {
		pr("Can't get to %s from here!\n",
		   xyas(dx, dy, player->cnum));
	    }
	    break;
	}
	dir = chkdir(*p, DIR_STOP, DIR_LAST);
	if (dir < 0) {
	    pr("\"%c\" is not legal...", *p);
	    direrr("'%c' to stop\n", NULL, NULL);
	    break;
	}
	nx = x + diroff[dir][0];
	ny = y + diroff[dir][1];
	getsect(nx, ny, &sect);
	if (onlyown && sect.sct_own != player->cnum) {
	    pr("You don't own %s; you can't go there!\n",
	       xyas(nx, ny, player->cnum));
	    break;
	}
	if (dir == DIR_STOP || dir == DIR_MAP) {
	    p[1] = 0;
	    return buf;
	}
	++p;
	x = nx;
	y = ny;
    }
    fly_map(x, y);
    if (showdes) {
	getsect(x, y, &sect);
	sprintf(prompt, "<%c: %s> ", dchr[sect.sct_type].d_mnem,
		xyas(x, y, player->cnum));
    } else {
	sprintf(prompt, "<%d: %s> ", (int)(p - buf),
		xyas(x, y, player->cnum));
    }
    bp = getstring(prompt, buf2);
    if (bp && p + strlen(bp) + 1 >= buf + MAX_PATH_LEN) {
	pr("Path length may not exceed %d.\n", MAX_PATH_LEN);
	pr("Aborting...\n");
	bp = NULL;
    }
    if (!bp)
	return NULL;
    strcpy(p, bp);
    if (*bp)
	goto more;
    return buf;
}

/*
 * fly move cost
 */
/* ARGSUSED */
double
fcost(struct sctstr *sp, natid own)
{
    return 1.0;
}

/*
 * nav move cost
 */
/* ARGSUSED */
double
ncost(struct sctstr *sp, natid own)
{
    return 1.0;
}

/*
 * return end x,y of path, and the base
 * movement cost it takes to get there.
 */
double
pathtoxy(char *path, coord *xp, coord *yp,
	 double (*cost)(struct sctstr *, natid))
{
    struct sctstr s;
    char *pp;
    coord x;
    coord y;
    int val;
    double m;

    x = *xp;
    y = *yp;
    m = 0.0;
    for (pp = path; *pp; pp++) {
	if ((val = diridx(*pp)) == DIR_STOP)
	    break;
	x += diroff[val][0];
	y += diroff[val][1];
	if (!getsect(x, y, &s))
	    return -1.0;
	m += cost(&s, s.sct_own);
    }
    *xp = xnorm(x);
    *yp = ynorm(y);
    return m;
}

void
pathrange(coord cx, coord cy, char *pp, int border, struct range *range)
{
    int dir;

    range->lx = cx;
    range->hx = cx;
    range->ly = cy;
    range->hy = cy;
    range->width = 0;
    range->height = 0;
    for (; *pp; pp++) {
	dir = diridx(*pp);
	if (dir == DIR_STOP)
	    break;
	cx += diroff[dir][0];
	cy += diroff[dir][1];
	if (cx < range->lx)
	    range->lx = cx;
	if (cx > range->hx)
	    range->hx = cx;
	if (cy < range->ly)
	    range->ly = cy;
	if (cy > range->hy)
	    range->hy = cy;
    }
    range->lx = xnorm(range->lx - border * 2);
    range->ly = ynorm(range->ly - border);
    range->hx = xnorm(range->hx + border * 2 + 1);
    range->hy = ynorm(range->hy + border + 1);
    xysize_range(range);
}
