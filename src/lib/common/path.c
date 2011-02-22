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
 *  path.c: Path finding interface code
 *
 *  Known contributors to this file:
 *     Phil Lapsley, 1991
 *     Dave Pare, 1991
 *     Thomas Ruschak, 1993
 *     Steve McClure, 1997
 *     Markus Armbruster, 2011
 */

#include <config.h>

#include <string.h>
#include "file.h"
#include "optlist.h"
#include "path.h"
#include "sect.h"
#include "xy.h"

char *
BestLandPath(char *path,
	     struct sctstr *from,
	     struct sctstr *to, double *cost, int mob_type)
{
    double c;
    size_t len;

    *cost = 0.0;
    *path = 0;

    /*
     * Note: passing from->sct_own for actor is funny, but works: its
     * only effect is to confine the search to that nation's land.  It
     * doesn't affect mobility costs.  The real actor is different for
     * marching in allied land, and passing it would break path
     * finding there.
     */
    c = path_find(from->sct_x, from->sct_y, to->sct_x, to->sct_y,
		  from->sct_own, mob_type);
    if (c < 0)
	return NULL;
    len = path_find_route(path, 1024,
			  from->sct_x, from->sct_y,
			  to->sct_x, to->sct_y);
    if (len + 1 >= 1024)
	return NULL;
    strcpy(path + len, "h");
    *cost = c;
    return path;
}

char *
BestDistPath(char *path,
	     struct sctstr *from,
	     struct sctstr *to, double *cost)
{
    return BestLandPath(path, from, to, cost, MOB_MOVE);
}

char *
BestShipPath(char *path, int fx, int fy, int tx, int ty, int owner)
{
    size_t len;

    if (path_find(fx, fy, tx, ty, owner, MOB_SAIL) < 0)
	return NULL;
    len = path_find_route(path, 100, fx, fy, tx, ty);
    if (len >= 100)
	return NULL;
    if (len == 0)
	strcpy(path, "h");
    return path;
}

char *
BestAirPath(char *path, int fx, int fy, int tx, int ty)
{
    size_t len;

    if (path_find(fx, fy, tx, ty, 0, MOB_FLY) < 0)
	return NULL;
    len = path_find_route(path, 100, fx, fy, tx, ty);
    if (len >= 100)
	return NULL;
    if (len == 0)
	strcpy(path, "h");
    return path;
}
