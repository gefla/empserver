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
 *  bestpath.c: Find the best path between sectors
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2006
 */

/*
 * IMPORTANT: These routines are very selectively used in the server.
 *
 * "bestownedpath" is only used to determine paths for ships and planes.
 *
 * Callers should not be calling these directly anymore. They should use
 * the "BestShipPath", "BestAirPath", "BestLandPath" and "BestDistPath"
 * functions.  Note that those last two use the A* algorithms to find
 * information.
 */

#include <config.h>

#include "file.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "path.h"
#include "prototypes.h"
#include "sect.h"
#include "xy.h"

static int owned_and_navigable(char *, int, int, int);

#define MAXROUTE	100
#define valid(x,y)	((((x) ^ (y)) & 1) == 0)

/*
 * Ok, note that here we malloc some buffers.  BUT, we never
 * free them.  Why, you may ask?  Because we want to allocate
 * them based on world size which is now (or soon to be) dynamic,
 * but we don't want to allocate each and every time, since that
 * would be slow.  And, since world size only changes at init
 * time, we can do this safely.
 */
static unsigned short *mapbuf;
static unsigned short **mapindex;

/*
 * Find passable path from X, Y to EX, EY for nation OWN.
 * BPATH is a buffer capable of holding at least MAXROUTE characters.
 * If BIGMAP is null, all sectors are passable (useful for flying).
 * Else it is taken to be a bmap.
 * Sectors owned by or allied to OWN are then passable according to
 * the usual rules.
 * Other sectors are assumed to be passable when BIGMAP shows '.' or
 * nothing.
 * Return a path if found, else a null pointer.
 * Wart: the path isn't terminated with 'h', except when if X,Y equals
 * EX,EY.
 */
char *
bestownedpath(char *bpath, char *bigmap,
	      int x, int y, int ex, int ey, int own)
{
    int i, j, tx, ty, markedsectors;
    int minx, maxx, miny, maxy, scanx, scany;
    unsigned routelen;

    if (!mapbuf)
	mapbuf = malloc(WORLD_X * WORLD_Y * sizeof(*mapbuf));
    if (!mapbuf)
	return NULL;
    if (!mapindex) {
	mapindex = malloc(WORLD_X * sizeof(*mapindex));
	if (mapindex) {
	    /* Setup the map pointers */
	    for (i = 0; i < WORLD_X; i++)
		mapindex[i] = &mapbuf[WORLD_Y * i];
	}
    }
    if (!mapindex)
	return NULL;

    x = XNORM(x);
    y = YNORM(y);
    ex = XNORM(ex);
    ey = YNORM(ey);

    if (x == ex && y == ey)
	return "h";

    if (!valid(x, y) || !valid(ex, ey))
	return NULL;
    if (!owned_and_navigable(bigmap, ex, ey, own))
	return NULL;

    for (i = 0; i < WORLD_X; i++)
	for (j = 0; j < WORLD_Y; j++)
	    mapindex[i][j] = 0xFFFF;	/* clear the workspace  */

    routelen = 0;		/* path length is now 0 */
    mapindex[x][y] = 0;		/* mark starting spot   */
    minx = x - 2;		/* set X scan bounds    */
    maxx = x + 2;
    miny = y - 1;		/* set Y scan bounds    */
    maxy = y + 1;

    do {
	if (++routelen == MAXROUTE)
	    return NULL;
	markedsectors = 0;
	for (scanx = minx; scanx <= maxx; scanx++) {
	    x = XNORM(scanx);
	    for (scany = miny; scany <= maxy; scany++) {
		y = YNORM(scany);
		if (!valid(x, y))
		    continue;
		if (((mapindex[x][y] & 0x1FFF) == routelen - 1)) {
		    for (i = DIR_FIRST; i <= DIR_LAST; i++) {
			tx = x + diroff[i][0];
			ty = y + diroff[i][1];
			tx = XNORM(tx);
			ty = YNORM(ty);
			if (mapindex[tx][ty] == 0xFFFF) {
			    if (owned_and_navigable(bigmap, tx, ty, own)) {
				if (CANT_HAPPEN(i < DIR_FIRST || i > DIR_LAST))
				    i = DIR_STOP;
				mapindex[tx][ty] =
				    ((i - DIR_FIRST + 1) << 13) + routelen;
				markedsectors++;
			    }
			}
			if (tx == ex && ty == ey) {
			    bpath[routelen] = 0;
			    while (routelen--) {
				i = (mapindex[tx][ty] >> 13)
				    - 1 + DIR_FIRST;
				bpath[routelen] = dirch[i];
				tx = tx - diroff[i][0];
				ty = ty - diroff[i][1];
				tx = XNORM(tx);
				ty = YNORM(ty);
			    }
			    return bpath;
			}
		    }
		}
	    }
	}
	miny--;
	maxy++;
	minx -= 2;
	maxx += 2;
    } while (markedsectors);

    return NULL;		/* no route possible    */
}

/*
 * Return non-zero if sector X, Y is passable.
 * If BIGMAP is null, all sectors are passable (useful for flying).
 * Else it is taken to be a bmap.
 * Sectors owned by or allied to OWN are checked according to the
 * usual rules, and the result is correct.
 * Other sectors are assumed to be passable when BIGMAP shows '.' or
 * nothing.
 */
static int
owned_and_navigable(char *bigmap, int x, int y, int own)
{
    char mapspot;
    struct sctstr sect;

    if (!bigmap)
	return 1;

    /* Owned or allied sector?  Check the real sector.  */
    getsect(x, y, &sect);
    if (sect.sct_own && relations_with(sect.sct_own, own) == ALLIED) {
	/* FIXME duplicates shp_check_nav() logic */
	switch (dchr[sect.sct_type].d_nav) {
	case NAVOK:
	    return 1;
	case NAV_CANAL:
	    /* FIXME return 1 when all ships have M_CANAL */
	    return 0;
	case NAV_02:
	    return sect.sct_effic >= 2;
	case NAV_60:
	    return sect.sct_effic >= 60;
	default:
	    return 0;
	}
    }

    /* Can only check bigmap */
    mapspot = bigmap[sect.sct_uid];
    return mapspot == '.' || mapspot == ' ' || mapspot == 0;
}
