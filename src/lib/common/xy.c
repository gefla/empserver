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
 *  xy.c: x-y related conversion routines
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Markus Armbruster, 2004-2013
 */

#include <config.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "prototypes.h"
#include "sect.h"
#include "xy.h"

/*
 * return pointer to a string containing the x,y
 * coords as desired by a particular target country.
 */
char *
xyas(coord x, coord y, natid country)
{
    struct natstr *np;

    np = getnatp(country);
    return prbuf("%d,%d", xrel(np, x), yrel(np, y));
}

char *
ownxy(struct sctstr *sp)
{
    return xyas(sp->sct_x, sp->sct_y, sp->sct_own);
}

coord
xrel(struct natstr *np, coord absx)
{
    coord x;

    x = XNORM(absx - np->nat_xorg);
    if (x >= WORLD_X / 2)
	x -= WORLD_X;
    return x;
}

coord
yrel(struct natstr *np, coord absy)
{
    coord y;

    y = YNORM(absy - np->nat_yorg);
    if (y >= WORLD_Y / 2)
	y -= WORLD_Y;
    return y;
}

void
xyrelrange(struct natstr *np, struct range *src, struct range *dst)
{
    dst->lx = xrel(np, src->lx);
    dst->hx = xrel(np, src->hx);
    dst->ly = yrel(np, src->ly);
    dst->hy = yrel(np, src->hy);
    dst->width = src->width;
    dst->height = src->height;
}

void
xyabsrange(struct natstr *np, struct range *src, struct range *dst)
{
    dst->lx = xabs(np, src->lx);
    dst->hx = xabs(np, src->hx);
    dst->ly = yabs(np, src->ly);
    dst->hy = yabs(np, src->hy);
    dst->width = src->width;
    dst->height = src->height;
}

void
xysize_range(struct range *rp)
{
    if (rp->lx > rp->hx)
	rp->width = WORLD_X + rp->hx + 1 - rp->lx;
    else
	rp->width = rp->hx + 1 - rp->lx;
    if (CANT_HAPPEN(rp->width > WORLD_X))
	rp->width = WORLD_X;
    if (rp->ly > rp->hy)
	rp->height = WORLD_Y + rp->hy + 1 - rp->ly;
    else
	rp->height = rp->hy + 1 - rp->ly;
    if (CANT_HAPPEN(rp->height > WORLD_Y))
	rp->height = WORLD_Y;
}

void
xydist_range(coord x, coord y, int dist, struct range *rp)
{
    if (4 * dist + 1 < WORLD_X) {
	rp->lx = xnorm(x - 2 * dist);
	rp->hx = xnorm(x + 2 * dist);
	rp->width = 4 * dist + 1;
    } else {
	rp->lx = xnorm(x - WORLD_X / 2);
	rp->hx = xnorm(rp->lx + WORLD_X - 1);
	rp->width = WORLD_X;
    }

    if (2 * dist + 1 < WORLD_Y) {
	rp->ly = ynorm(y - dist);
	rp->hy = ynorm(y + dist);
	rp->height = 2 * dist + 1;
    } else {
	rp->ly = ynorm(y - WORLD_Y / 2);
	rp->hy = ynorm(rp->ly + WORLD_Y - 1);
	rp->height = WORLD_Y;
    }
}

/*
 * Convert initial part of @str to normalized x-coordinate.
 * Return -1 on error.  This works, as normalized coordinates are
 * non-negative.
 * Assign pointer to first character after the coordinate to *@end,
 * unless @end is a null pointer.
 */
coord
strtox(char *str, char **end)
{
    long l;

    errno = 0;
    l = strtol(str, end, 10);
    if (*end == str || errno != 0)
	return -1;
    return XNORM(l);
}

/*
 * Convert initial part of @str to normalized y-coordinate.
 * Return -1 on error.  This works, as normalized coordinates are
 * non-negative.
 * Assign pointer to first character after the coordinate to *@end,
 * unless @end is a null pointer.
 */
coord
strtoy(char *str, char **end)
{
    long l;

    errno = 0;
    l = strtol(str, end, 10);
    if (*end == str || errno != 0)
	return -1;
    return YNORM(l);
}

coord
xabs(struct natstr *np, coord relx)
{
    relx += np->nat_xorg;
    return XNORM(relx);
}

coord
yabs(struct natstr *np, coord rely)
{
    rely += np->nat_yorg;
    return YNORM(rely);
}

int
sctoff(coord x, coord y)
{
    if (CANT_HAPPEN((x + y) & 1))
	return -1;
    return XYOFFSET(XNORM(x), YNORM(y));
}

void sctoff2xy(coord *x, coord *y, int off)
{
    *y = off * 2 / WORLD_X;
    *x = off * 2 % WORLD_X + *y % 2;
}

coord
xnorm(coord x)
{
    return XNORM(x);
}

coord
ynorm(coord y)
{
    return YNORM(y);
}

int
xyinrange(coord x, coord y, struct range *rp)
{
    if (rp->lx <= rp->hx) {
	/* xrange doesn't wrap */
	if (x < rp->lx || x > rp->hx)
	    return 0;
    } else {
	if (x < rp->lx && x > rp->hx)
	    return 0;
    }
    if (rp->ly <= rp->hy) {
	/* yrange doesn't wrap */
	if (y < rp->ly || y > rp->hy)
	    return 0;
    } else {
	if (y < rp->ly && y > rp->hy)
	    return 0;
    }
    return 1;
}


char *
prbuf(char *format, ...)
{
    static int nbuf = -1;
    static char buf[20][1024];
    va_list ap;

    if (++nbuf > 19)
	nbuf = 0;

    va_start(ap, format);
    (void)vsprintf(buf[nbuf], format, ap);
    va_end(ap);

    return buf[nbuf];
}
