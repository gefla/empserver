/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  path.c: Routines associated with paths, directions, etc.
 * 
 *  Known contributors to this file:
 *   
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "path.h"
#include "nat.h"
#include "sect.h"
#include "file.h"
#include "prototypes.h"

int
getdir(s_char *prompt, s_char *stop_msg, s_char *view_msg,
       s_char *bomb_msg)
{
    register int max_dir;
    register int min_dir;
    register int dir_num;
    s_char buf[1024];

    if (stop_msg != 0)
	min_dir = DIR_STOP;
    else
	min_dir = DIR_FIRST;
    if (view_msg == 0)
	max_dir = DIR_LAST;
    else
	max_dir = DIR_VIEW;
    while (1) {
	if (getstring(prompt, buf) == 0 || *buf == 0)
	    return -1;
	dir_num = chkdir(buf[0], min_dir, max_dir);
	if (dir_num >= min_dir)
	    break;
	direrr(stop_msg, view_msg, bomb_msg);
    }
    return dir_num;
}

int
chkdir(s_char dir_char, int min_dir, int max_dir)
{
    register int i;

    for (i = min_dir; i <= max_dir; i++)
	if (dir_char == dirch[i])
	    return i;
    return -1;
}

void
direrr(s_char *stop_msg, s_char *view_msg, s_char *map_msg)
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
 * return pointer to path; prompt user until a stop char
 * or a bomb char have been entered.  A "bomb" char in
 * this context is actually "execute" for the partial
 * move commands, and isn't valid for those commands
 * which do not accept partial moves.
 */
s_char *
getpath(s_char *buf, s_char *arg, coord x, coord y, int onlyown,
	int showdes, int showxy, int destinations)
{
    s_char *p = buf;
    s_char *bp;
    s_char prompt[128];
    coord dx, dy;
    struct sctstr sect, dsect;
    coord nx, ny;
    int dir;
    s_char *execute;
    double mv_cost;

    if (arg) {
	strncpy(buf, arg, MAX_PATH_LEN - 1);
	buf[MAX_PATH_LEN - 1] = 0;
    } else {
	*p = 0;
    }

    if (showxy)
	execute = " & '%c' to execute\n";
    else
	execute = "\n";

    getsect(x, y, &sect);
    nx = x;
    ny = y;

  more:
    while (*p) {
	if (sarg_xy(p, &dx, &dy)) {
	    bp = 0;
	    if (destinations == P_NONE) {
		pr("Destination sectors not allowed here!\n");
		*p = 0;
	    }
	    if (getsect(dx, dy, &dsect)) {
		if (destinations == P_WALKING) {
		    bp = BestLandPath(p, &sect, &dsect,
				      &mv_cost, MOB_ROAD);
		} else if (destinations == P_FLYING) {
		    bp = BestAirPath(p, nx, ny, dx, dy);
		}
	    } else {
		pr("Invalid destination sector!\n");
		*p = 0;
	    }
	    if (bp) {
		pr("Using best path  '%s'\n", p);
		pr("Using total path '%s'\n", buf);
		return buf;
	    } else {
		pr("Can't get to %s from here!\n",
		   xyas(nx, ny, player->cnum));
		*p = 0;
	    }
	    break;
	}
	dir = chkdir(*p, DIR_STOP, DIR_LAST);
	if (dir < 0) {
	    pr("\"%c\" is not legal...", *p);
	    direrr("'%c' to stop", (s_char *)0, execute);
	    *p = 0;
	    break;
	}
	nx = x + diroff[dir][0];
	ny = y + diroff[dir][1];
	getsect(nx, ny, &sect);
	if (onlyown && sect.sct_own != player->cnum) {
	    *p = 0;
	    pr("You don't own %s; you can't go there!\n",
	       xyas(nx, ny, player->cnum));
	    break;
	}
	if (dir == DIR_STOP || dir == DIR_MAP) {
	    p[1] = 0;
	    return buf;
	}
	if (++p - buf == MAX_PATH_LEN) {
	    pr("Path length may not exceed %d.\n", MAX_PATH_LEN);
	    pr("Aborting...\n");
	    *buf = 0;
	    return buf;
	}
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
    if (!(bp = getstring(prompt, p)) || !*bp) {
	if (player->aborted)
	    *buf = 0;
	return buf;
    }
    goto more;
}

/* ARGSUSED */
double
mcost(struct sctstr *sp, int own)
{
    return sector_mcost(sp, MOB_ROAD);

/*
	int	m_cost;

	if (!(m_cost = dchr[sp->sct_type].d_mcst))
		return -1.0;
	if (m_cost < 25)
		return (200.0 + (m_cost - 3.0) * sp->sct_effic) / 500.0;
	else
		return (m_cost * 5.0 - (double)sp->sct_effic) / 25.0;
 */
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
pathtoxy(s_char *path, coord *xp, coord *yp,
	 double (*cost) (struct sctstr *, natid))
{
    struct sctstr s;
    s_char *pp;
    coord x;
    coord y;
    int val;
    double m;
    int c;

    x = *xp;
    y = *yp;
    m = 0.0;
    for (pp = path; *pp; pp++) {
	if ((val = chkdir(*pp, DIR_STOP, DIR_LAST)) == 0)
	    break;
	x += diroff[val][0];
	y += diroff[val][1];
	c = dirch[val];
	if (c == DIR_STOP)
	    break;
	if (!getsect(x, y, &s))
	    return -1.0;
	m += cost(&s, s.sct_own);
    }
    *xp = xnorm(x);
    *yp = ynorm(y);
    return m;
}

/*
 * return true if "who" owns the path starting at x,y
 */
int
chkpath(natid who, s_char *path, coord x, coord y)
{
    s_char *pp;
    int val;
    struct sctstr sect;

    for (pp = path; *pp; pp++) {
	if ((val = chkdir(*pp, DIR_STOP, DIR_LAST)) == 0)
	    break;
	x += diroff[val][0];
	y += diroff[val][1];
	if (!getsect(x, y, &sect) || sect.sct_own != who)
	    return 0;
    }
    return 1;
}

void
pathrange(register coord cx, register coord cy, register s_char *pp,
	  int border, struct range *range)
{
    int dir;

    range->lx = cx;
    range->hx = cx;
    range->ly = cy;
    range->hy = cy;
    range->width = 0;
    range->height = 0;
    while ((dir = chkdir(*pp, DIR_FIRST, DIR_LAST)) >= 0) {
	pp++;
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
