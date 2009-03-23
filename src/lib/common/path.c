/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  path.c: Empire/A* Interface code.
 *          Define AS_STATS for A* statistics.
 *
 *  Known contributors to this file:
 *     Phil Lapsley, 1991
 *     Dave Pare, 1991
 *     Thomas Ruschak, 1993
 *     Steve McClure, 1997
 */

#include <config.h>

#include <stdio.h>
#include "../as/as.h"
#include "file.h"
#include "misc.h"
#include "optlist.h"
#include "path.h"
#include "prototypes.h"
#include "sect.h"
#include "xy.h"

#define	BP_ASHASHSIZE	128	/* A* queue hash table size */
#define	BP_NEIGHBORS	6	/* max number of neighbors */

struct bestp {
    int sctcache_hits;
    int sctcache_misses;
    int bp_mobtype;
    struct as_data *adp;
};

static int bp_path(struct as_path *pp, char *buf);
static int bp_neighbors(struct as_coord c, struct as_coord *cp, void *);
static double bp_lbcost(struct as_coord from, struct as_coord to, void *);
static double bp_realcost(struct as_coord from, struct as_coord to, void *);
static double bp_seccost(struct as_coord from, struct as_coord to, void *);
static int bp_coord_hash(struct as_coord c);

/* We use this for caching neighbors.  It never changes except
 * at reboot time (maybe) so we never need to free it */
static struct sctstr **neighsects;

static struct bestp *
bp_init(void)
{
    struct bestp *bp;

    bp = malloc(sizeof(*bp));
    memset(bp, 0, sizeof(*bp));
    bp->adp = as_init(BP_NEIGHBORS, BP_ASHASHSIZE, bp_coord_hash,
		      bp_neighbors, bp_lbcost, bp_realcost,
		      bp_seccost, bp);

    if (bp->adp == NULL)
	return NULL;

    if (neighsects == NULL)
	neighsects = calloc(WORLD_SZ() * 6, sizeof(struct sctstr *));

    return bp;
}

/*
 * Find the best path from sector to to sector, and put the Empire movement
 * string in path.  Return 0 on success, -1 on error.
 */
static int
best_path(struct sctstr *from, struct sctstr *to, char *path,
	  int mob_type)
{
    static struct bestp *mybestpath;
    struct as_data *adp;
    struct as_path *ap;

    if (!mybestpath)
	mybestpath = bp_init();
    adp = mybestpath->adp;
    ap = as_find_cachepath(from->sct_x, from->sct_y, to->sct_x, to->sct_y);
    if (ap == NULL) {
	adp->from.x = from->sct_x;
	adp->from.y = from->sct_y;
	adp->to.x = to->sct_x;
	adp->to.y = to->sct_y;
	mybestpath->bp_mobtype = mob_type;

	if (as_search(adp) < 0)
	    return -1;
	ap = adp->path;
    }

    if (bp_path(ap, path) < 0)
	return -1;

#ifdef AS_STATS
    as_stats(adp, stderr);
#endif /* AS_STATS */
#ifdef BP_STATS
    fprintf(stderr, "best path %s\n", path);
    fprintf(stderr, "cache hits/misses: %d/%d\n",
	    bp->sctcache_hits, bp->sctcache_misses);
#endif /* BP_STATS */
    return 0;
}

/*
 * Translate an A* path into an empire movement string.  Return 0 on
 * success, -1 on failure.
 */
static int
bp_path(struct as_path *pp, char *buf)
{
    struct as_path *np;
    char *cp = buf;
    int dx, dy;
    int n;

    np = pp->next;
    while (np) {
	dx = np->c.x - pp->c.x;
	/* deal with wraparound from non-neg coords */
	if (dx < -2)
	    dx += WORLD_X;
	else if (dx > 2)
	    dx -= WORLD_X;
	dy = np->c.y - pp->c.y;
	if (dy < -1)
	    dy += WORLD_Y;
	else if (dy > 1)
	    dy -= WORLD_Y;
	for (n = 1; n <= 6; n++)
	    if (dx == diroff[n][0] && dy == diroff[n][1])
		break;
	if (n > 6)
	    return -1;

	*cp++ = dirch[n];
	pp = np;
	np = np->next;
    }
    *cp = '\0';
    return 0;
}

/*
 * Find coords neighboring this sector; return number of such
 * coords, and coordinartes themselves in an array pointed
 * to by *cpp.
 * XXX need to check ownership, sector types, etc.
 */
static int
bp_neighbors(struct as_coord c, struct as_coord *cp, void *pp)
{
    struct sctstr *sectp = (void *)empfile[EF_SECTOR].cache;
    struct bestp *bp = pp;
    coord x, y;
    coord nx, ny;
    int n = 0, q;
    struct sctstr *sp, *from, **ssp;
    /* Six pointers, just in case our cache isn't there */
    struct sctstr *tsp[] = { NULL, NULL, NULL, NULL, NULL, NULL };
    int sx, sy, offset;

    x = c.x;
    y = c.y;
    sx = XNORM(x);
    sy = YNORM(y);
    offset = XYOFFSET(sx, sy);
    from = &sectp[offset];

    if (neighsects == NULL)
	ssp = (struct sctstr **)&tsp[0];
    else
	ssp = (struct sctstr **)&neighsects[offset * 6];
    for (q = 1; q <= 6; q++, ssp++) {
	if (*ssp == NULL) {
	    /* We haven't cached this neighbor yet */
	    nx = x + diroff[q][0];
	    ny = y + diroff[q][1];
	    sx = XNORM(nx);
	    sy = YNORM(ny);
	    offset = XYOFFSET(sx, sy);
	    sp = &sectp[offset];
	    *ssp = sp;
	} else {
	    sp = *ssp;
	    sx = XNORM(sp->sct_x);
	    sy = YNORM(sp->sct_y);
	}
	/* No need to calculate cost each time, just make sure we can
	   move through it.  We calculate it later. */
	if (dchr[sp->sct_type].d_mob0 < 0)
	    continue;
	if (bp->bp_mobtype == MOB_RAIL && !SCT_HAS_RAIL(sp))
	    continue;
	if (sp->sct_own != from->sct_own)
	    continue;
	cp[n].x = sx;
	cp[n].y = sy;
	n++;
    }
    return n;
}

/*
 * Compute a lower-bound on the cost from "from" to "to".
 */
/*ARGSUSED*/
static double
bp_lbcost(struct as_coord from, struct as_coord to, void *pp)
{
    struct sctstr *sectp = (void *)empfile[EF_SECTOR].cache;
    struct bestp *bp = pp;
    int x, y, sx, sy, offset;

    x = to.x;
    y = to.y;
    sx = XNORM(x);
    sy = YNORM(y);
    offset = XYOFFSET(sx, sy);
    return sector_mcost(&sectp[offset], bp->bp_mobtype);
}

/*
 * Compute the real cost to move from "from" to "to".
 */
static double
bp_realcost(struct as_coord from, struct as_coord to, void *pp)
{
    return bp_lbcost(from, to, pp);
}

/*
 * Tie breaker secondary metric (only used when lower bound costs
 * are equal).
 */
/*ARGSUSED*/
static double
bp_seccost(struct as_coord from, struct as_coord to, void *pp)
{
    return mapdist((coord)from.x, (coord)from.y,
		   (coord)to.x, (coord)to.y);
}

/*
 * Hash a coordinate into an integer.
 */
static int
bp_coord_hash(struct as_coord c)
{
    return ((abs(c.x) + 1) << 3) ^ abs(c.y);
}

void
bp_enable_cachepath(void)
{
    as_enable_cachepath();
}

void
bp_disable_cachepath(void)
{
    as_disable_cachepath();
}

void
bp_clear_cachepath(void)
{
    as_clear_cachepath();
}

double
pathcost(struct sctstr *start, char *path, int mob_type)
{
    struct sctstr *sectp = (void *)empfile[EF_SECTOR].cache;
    unsigned i;
    int o;
    int cx, cy;
    double cost = 0.0;
    struct sctstr *sp;
    int sx, sy, offset;

    cx = start->sct_x;
    cy = start->sct_y;

    while (*path) {
	if (*path == 'h') {
	    path++;
	    continue;
	}
	i = *path - 'a';
	if (CANT_HAPPEN(i >= sizeof(dirindex) / sizeof(*dirindex)))
	    break;
	o = dirindex[i];
	if (CANT_HAPPEN(o < 0))
	    break;
	cx += diroff[o][0];
	cy += diroff[o][1];
	sx = XNORM(cx);
	sy = YNORM(cy);
	offset = XYOFFSET(sx, sy);
	sp = &sectp[offset];
	cost += sector_mcost(sp, mob_type);
	path++;
    }

    return cost;
}

char *
BestDistPath(char *path,
	     struct sctstr *from,
	     struct sctstr *to, double *cost)
{
    return BestLandPath(path, from, to, cost, MOB_MOVE);
}

char *
BestLandPath(char *path,
	     struct sctstr *from,
	     struct sctstr *to, double *cost, int mob_type)
{
    int length;

    *path = 0;
    *cost = 0.0;
    if (best_path(from, to, path, mob_type) < 0)
	return NULL;
    *cost = pathcost(from, path, mob_type);
    length = strlen(path);
    path[length] = 'h';
    path[length + 1] = '\0';
    return path;
}

char *
BestShipPath(char *path, int fx, int fy, int tx, int ty, int owner)
{
    char *map;

    map = ef_ptr(EF_BMAP, owner);
    if (!map)
	return NULL;
    return bestownedpath(path, map, fx, fy, tx, ty, owner);
}

char *
BestAirPath(char *path, int fx, int fy, int tx, int ty)
{
    return bestownedpath(path, NULL, fx, fy, tx, ty, -1);
}
