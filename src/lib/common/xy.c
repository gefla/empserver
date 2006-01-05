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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  xy.c: x-y related conversion routines
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include "misc.h"
#include "xy.h"
#include "nat.h"
#include "sect.h"
#include "file.h"
#include "common.h"
#include "optlist.h"

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
    else if (x < -WORLD_X / 2)
	x += WORLD_X;
    return x;
}

coord
yrel(struct natstr *np, coord absy)
{
    coord y;

    y = YNORM(absy - np->nat_yorg);
    if (y >= WORLD_Y / 2)
	y -= WORLD_Y;
    else if (y < -WORLD_Y / 2)
	y += WORLD_Y;
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

/*
 * Convert initial part of STR to normalized x-coordinate.
 * Return -1 on error.  This works, as normalized coordinates are
 * non-negative.
 * Assign pointer to first character after the coordinate to *END,
 * unless END is a null pointer.
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
 * Convert initial part of STR to normalized y-coordinate.
 * Return -1 on error.  This works, as normalized coordinates are
 * non-negative.
 * Assign pointer to first character after the coordinate to *END,
 * unless END is a null pointer.
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
    if ((x + y) & 01) {
	logerror("%d,%d is an invalid sector specification!\n", x, y);
	return -1;
    }
    return (YNORM(y) * WORLD_X + XNORM(x)) / 2;
}

coord
xnorm(coord x)
{
    if (x < 0)
	x = WORLD_X - (-x % WORLD_X);
    return x % WORLD_X;
}

coord
ynorm(coord y)
{
    if (y < 0)
	y = WORLD_Y - (-y % WORLD_Y);
    return y % WORLD_Y;
}

int
xyinrange(coord x, coord y, struct range *rp)
{
    if (rp->lx < rp->hx) {
	/* xrange doesn't wrap */
	if (x < rp->lx || x > rp->hx)
	    return 0;
    } else {
	if (x < rp->lx && x > rp->hx)
	    return 0;
    }
    if (rp->ly < rp->hy) {
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
