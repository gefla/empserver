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
 *  path.c: Empire/A* Interface code.  Provides callbacks for A* code and
 *          a sector cache to speed things up.  Define BP_STATS for sector
 *          cache statistics, AS_STATS for A* statistics.
 * 
 *  Known contributors to this file:
 *     Phil Lapsley, 1991
 *     Dave Pare, 1991
 *     Thomas Ruschak, 1993
 *     Steve McClure, 1997
 */

#include <stdio.h>
#include <stdlib.h>
#include "../as/as.h"
#include "misc.h"
#include "path.h"
#include "xy.h"
#include "sect.h"
#include "file.h"
#include "common.h"
#include "gen.h"
#include "optlist.h"

/* STM - The server is now reliant on the sector file being
 * memory mapped for other things.  So, this code has been
 * setup to have the sector hashing #ifdef'd instead so that
 * we don't have to do runtime checking.  If someone moves
 * the sector file to me non-memory mapped, they have larger
 * problems than this, and can just turn this back on.  Then
 * again, their performance will be so bad going to a file
 * all the time, it won't matter. */

/*#define DO_EFF_MEM_CHECKING*/


/* XXX won't need sector hash when sect file is memory mapped */

#define	BP_SCTHASHSIZE	128		/* sector cache hash table size */
#define	BP_ASHASHSIZE	128		/* A* queue hash table size */
#define	BP_NEIGHBORS	6		/* max number of neighbors */

struct	sctcache {
	coord	x, y;
	struct	sctstr *sp;
	struct	sctcache *next;
};

struct bestp {
	struct	sctcache *sctcachetab[BP_SCTHASHSIZE];
	int	sctcache_hits;
	int	sctcache_misses;
	int     bp_mobtype;
	struct	as_data *adp;
};

#ifdef DO_EFF_MEM_CHECKING

static	struct sctstr *bp_getsect(struct bestp *bp, coord x, coord y);
static	struct sctstr *bp_sctcache_get(struct bestp *bp, coord x, coord y);
static	void bp_sctcache_set(struct bestp *bp, coord x, coord y, struct sctstr *sp);
static	void bp_sctcache_zap(struct bestp *bp);

#endif /* DO_EFF_MEM_CHECKING */

static	int bp_path(struct as_path *pp, s_char *buf);
static	int bp_neighbors(struct as_coord c, struct as_coord *cp, s_char *pp);
static	double bp_lbcost(struct as_coord from, struct as_coord to, s_char *pp);
static	double bp_realcost(struct as_coord from, struct as_coord to, s_char *pp);
static	double bp_seccost(struct as_coord from, struct as_coord to, s_char *pp);
static	int bp_coord_hash(struct as_coord c);

struct empfile *ep;

/* We use this for caching neighbors.  It never changes except
 * at reboot time (maybe) so we never need to free it */
struct sctstr **neighsects = (struct sctstr **)0;

s_char *
bp_init(void)
{
	struct	bestp *bp;

	ep = &empfile[EF_SECTOR];

	bp = (struct bestp *) malloc(sizeof(*bp));
	bzero((s_char *)bp, sizeof(*bp));
	bp->adp = as_init(BP_NEIGHBORS, BP_ASHASHSIZE, bp_coord_hash,
		bp_neighbors, bp_lbcost, bp_realcost,
		bp_seccost, (s_char *)bp);

	if (bp->adp == NULL)
		return NULL;

	if (neighsects == (struct sctstr **)0)
	  neighsects = (struct sctstr **)calloc(1, (sizeof(struct sctstr *) *
						((WORLD_X * WORLD_Y)/2)*6));

	return (s_char *) bp;
}

/*
 * Find the best path from sector to to sector, and put the Empire movement
 * string in path.  Return 0 on success, -1 on error.
 */
int
best_path(struct sctstr *from, struct sctstr *to, s_char *path, int mob_type)
{
	static struct bestp *mybestpath;
	struct	as_data *adp;
	struct as_path *ap;

	if (mybestpath == 0) 
		mybestpath = (struct bestp *)bp_init();
	adp = mybestpath->adp;
#ifdef DO_EFF_MEM_CHECKING
	bp_sctcache_zap(mybestpath);
#endif
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
bp_path(struct as_path *pp, s_char *buf)
{
	struct	as_path *np;
	s_char	*cp = buf;
	int	dx, dy;
	int	n;

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
		for (n=1;n<=6;n++)
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
bp_neighbors(struct as_coord c, struct as_coord *cp, s_char *pp)
{
#ifdef DO_EFF_MEM_CHECKING
	struct	bestp *bp = (struct bestp *) pp;
#endif /* DO_EFF_MEM_CHECKING */
	coord	x, y;
	coord	nx, ny;
	int	n = 0, q;
	struct	sctstr *sp, *from, **ssp;
	/* Six pointers, just in case our cache isn't there */
	struct  sctstr *tsp[] = { 0, 0, 0, 0, 0, 0 };
	int	sx, sy, offset;

	x = c.x;
	y = c.y;
#ifdef DO_EFF_MEM_CHECKING
	if ((ep->flags & EFF_MEM) == 0) {
		from = bp_getsect(bp, x, y);
	} else {
#endif /* DO_EFF_MEM_CHECKING */
	    sx = XNORM(x);
	    sy = YNORM(y);
	    offset = (sy * WORLD_X + sx) / 2; 
	    from = (struct sctstr *) (ep->cache + ep->size * offset);
#ifdef DO_EFF_MEM_CHECKING
	}
#endif /* DO_EFF_MEM_CHECKING */

	if (neighsects == (struct sctstr **)0)
	    ssp = (struct sctstr **)&tsp[0];
	else
	    ssp = (struct sctstr **)&neighsects[offset * 6];
	for (q = 1; q <= 6; q++, ssp++) {
	    if (*ssp == (struct sctstr *)0) {
		/* We haven't cached this neighbor yet */
		nx = x + diroff[q][0];
		ny = y + diroff[q][1];
		sx = XNORM(nx);
		sy = YNORM(ny);
#ifdef DO_EFF_MEM_CHECKING
		if ((ep->flags & EFF_MEM) == 0) {
			sp = bp_getsect(bp, nx, ny);
		} else {
#endif /* DO_EFF_MEM_CHECKING */
			offset = (sy * WORLD_X + sx) / 2; 
			sp = (struct sctstr *) (ep->cache + ep->size * offset);
			/* We can only save in our neighbor cache if the
			   sector file is in memory */
			*ssp = sp;
#ifdef DO_EFF_MEM_CHECKING
		}
#endif /* DO_EFF_MEM_CHECKING */
	    } else {
		sp = *ssp;
		sx = XNORM(sp->sct_x);
		sy = YNORM(sp->sct_y);
	    }
	    /* No need to calculate cost each time, just make sure we can
	       move through it.  We calculate it later. */
	    if (dchr[sp->sct_type].d_mcst == 0)
		continue;
	    if (sp->sct_own != from->sct_own)
		continue;
	    cp[n].x = sx;
	    cp[n].y = sy;
	    n++;
	}
	return (n);
}

/*
 * Compute a lower-bound on the cost from "from" to "to".
 */
/*ARGSUSED*/
static double
bp_lbcost(struct as_coord from, struct as_coord to, s_char *pp)
{
	struct	bestp *bp = (struct bestp *) pp;
	struct	sctstr *ts;
	float	cost;
	int x, y, sx, sy, offset;

#ifdef DO_EFF_MEM_CHECKING
	if ((ep->flags & EFF_MEM) == 0) {
		ts = bp_getsect(bp, (coord)to.x, (coord)to.y);
	} else {
#endif /* DO_EFF_MEM_CHECKING */
	    x = to.x;
	    y = to.y;
	    sx = XNORM(x);
	    sy = YNORM(y);
	    offset = (sy * WORLD_X + sx) / 2;
	    ts = (struct sctstr *)(ep->cache + ep->size * offset);
#ifdef DO_EFF_MEM_CHECKING
	}
#endif /* DO_EFF_MEM_CHECKING */
	cost = sector_mcost(ts, bp->bp_mobtype);
	return (cost);
}

/*
 * Compute the real cost to move from "from" to "to".
 */
static double
bp_realcost(struct as_coord from, struct as_coord to, s_char *pp)
{
	return (bp_lbcost(from, to, pp));
}

/*
 * Tie breaker secondary metric (only used when lower bound costs
 * are equal).
 */
/*ARGSUSED*/
static double
bp_seccost(struct as_coord from, struct as_coord to, s_char *pp)
{
	return ((double) mapdist((coord)from.x, (coord)from.y,
		(coord)to.x, (coord)to.y));
}

#ifdef DO_EFF_MEM_CHECKING

/*
 * Get a sector from the cache.  If it's not in the cache,
 * get it from disk and add it to the cache.
 */
static struct sctstr *
bp_getsect(struct bestp *bp, coord x, coord y)
{
	struct	sctstr *sp;

	sp = bp_sctcache_get(bp, x, y);
	if (sp == NULL) {
		sp = (struct sctstr *) malloc(sizeof(*sp));
		getsect(x, y, sp);
		bp_sctcache_set(bp, x, y, sp);
		bp->sctcache_misses++;
	} else {
		bp->sctcache_hits++;
	}
	return (sp);
}

/*
 * Get a sector from the cache; return NULL if it's not there.
 */
static struct sctstr *
bp_sctcache_get(struct bestp *bp, coord x, coord y)
{
	int 	hashval;
	struct	as_coord c;
	struct	sctcache *hp;

	c.x = x;
	c.y = y;
	hashval = bp_coord_hash(c) % BP_SCTHASHSIZE;
	for (hp = bp->sctcachetab[hashval]; hp; hp = hp->next) {
		if (hp->x == x && hp->y == y)
			return (hp->sp);
	}
	return (NULL);
}

/*
 * Put a sector in the cache.
 */
static void
bp_sctcache_set(struct bestp *bp, coord x, coord y, struct sctstr *sp)
{
	int	hashval;
	struct	as_coord c;
	struct	sctcache *hp;

	hp = (struct sctcache *) calloc(1, sizeof(*hp));
	hp->x = x;
	hp->y = y;
	hp->sp = sp;
	c.x = x;
	c.y = y;
	hashval = bp_coord_hash(c) % BP_SCTHASHSIZE;
	hp->next = bp->sctcachetab[hashval];
	bp->sctcachetab[hashval] = hp;
}

/*
 * Zap the cache and reset statistics.
 */
static void
bp_sctcache_zap(struct bestp *bp)
{
	register struct sctcache *hp;
	register struct sctcache *np;
	register int i;

	for (i = 0; i < BP_SCTHASHSIZE; i++) {
		for (hp = bp->sctcachetab[i]; hp; hp = np) {
			np = hp->next;
			free(hp->sp);
			free(hp);
		}
		bp->sctcachetab[i] = NULL;
	}
	bp->sctcache_hits = 0;
	bp->sctcache_misses = 0;
}

#endif /* DO_EFF_MEM_CHECKING */

/*
 * Hash a coordinate into an integer.
 */
static int
bp_coord_hash(struct as_coord c)
{
	return ((abs(c.x) + 1) << 3) ^ abs(c.y);
}

void
bp_enable_cachepath()
{
    as_enable_cachepath();
}

void
bp_disable_cachepath()
{
    as_disable_cachepath();
}

void
bp_clear_cachepath()
{
    as_clear_cachepath();
}

double
pathcost(struct sctstr *start, s_char *path, int mob_type)
{
	register int o;
	register int cx, cy;
	double	cost = 0.0;
	struct	sctstr *sp;
	int sx, sy, offset;

	cx = start->sct_x;
	cy = start->sct_y;

	while (*path) {
	    if (*path == 'h') {
		path++;
		continue;
	    }
	    o = dirindex[(int)((*path) - 'a')];
	    cx += diroff[o][0];
	    cy += diroff[o][1];
	    sx = XNORM(cx);
	    sy = YNORM(cy);
	    offset = (sy * WORLD_X + sx) / 2; 
	    sp = (struct sctstr *)(ep->cache + ep->size * offset);
	    cost += sector_mcost(sp, mob_type);
	    path++;
	}

	return cost;
}

s_char * 
BestDistPath(s_char *path,
	     struct sctstr *from,
	     struct sctstr *to,
	     double *cost,
	     int mob_type)
{
    return BestLandPath(path, from, to, cost, mob_type);
}

s_char *
BestLandPath(s_char *path,
	     struct sctstr *from,
	     struct sctstr *to,
	     double *cost, 
	     int mob_type)
{
    int length;

    *path = 0;
    *cost = 0.0;
    if (best_path(from, to, path, mob_type) < 0)
	return (s_char *)0;
    *cost = pathcost(from, path, mob_type);
    length = strlen(path);
    path[length] = 'h';
    path[length + 1] = '\0';
    return path;
}

s_char *
BestShipPath(s_char *path,
	     int fx,
	     int fy,
	     int tx,
	     int ty,
	     int owner)
{
    s_char *map;

    /* need to make sector database available to bestpath */
    map = ef_ptr(EF_BMAP, owner);

    return (bestownedpath(path, map, fx, fy, tx, ty, ".=h", owner));
}

s_char *
BestAirPath(s_char *path,
	     int fx,
	     int fy,
	     int tx,
	     int ty)
{
    return (bestownedpath(path, 0, fx, fy, tx, ty, "", -1));
    /*    return (bestpath(path, fx, fy, tx, ty, ""));*/
}

