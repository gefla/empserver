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
 *  pathfind.c: Find cheapest paths
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2011
 */

#include <config.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "nat.h"
#include "optlist.h"
#include "prototypes.h"
#include "path.h"
#include "sect.h"

#ifdef PATH_FIND_DEBUG
#define DPRINTF(fmt, ...) ((void)printf(fmt , ## __VA_ARGS__))
#else
#define DPRINTF(fmt, ...) ((void)0)
#endif

static char *bufrotate(char *buf, size_t bufsz, size_t i);

/*
 * A* algorithm.  Refer to your graph algorithm textbook for how it
 * works.  Implementation is specialized to hex maps.
 *
 * Define PATH_FIND_STATS for performance statistics on stdout.
 */

/*
 * Array of sector data, indexed by sector uid
 *
 * We need to store a few values per sector visited by the path
 * search.  An array is the stupidest data structure that could
 * possibly work.
 *
 * Extra benefit: it works really well for distribution in a
 * continental game, where we visit most sectors.  That's our most
 * demanding use of path search, and its performance has noticable
 * impact on the update.
 *
 * Island game distribution is much less demanding.  The array may not
 * be the best choice here, but it's plainly good enough.  Same for
 * path searches outside the update.
 */

struct pf_map {
    /*
     * visit < pf_visit      : unvisited, remaining members invalid
     * visit == pf_visit     : open, dir & cost tentative, heapi used
     * visit == pf_visit + 1 : closed, dir & cost final, heapi unused
     */
    unsigned short visit;
    signed char dir;		/* cheapest direction to source */
    int heapi;			/* index in heap, valid if open */
    double cost;		/* cost from source */
};

static unsigned short pf_visit;
static struct pf_map *pf_map;

/*
 * Binary heap, cost priority queue of all open sectors
 *
 * Again, we use the stupidest data structure that could possibly
 * work: an array.  And we make it so large it can hold *all* sectors.
 * In practice, we need much less space, but a tighter upper bound is
 * not obvious to me right now.
 */

struct pf_heap {
    int uid;			/* sector uid and */
    coord x, y;			/* coordinates, uid == XYOFFSET(x, y) */
    double f;			/* estimated cost from source to dest */
};

static int pf_nheap;		/* #entries in pf_nheap[] */
static struct pf_heap *pf_heap;

/*
 * Source and costs
 */
static coord pf_sx, pf_sy;
static int pf_suid;
static natid pf_actor;
static double (*pf_sct_cost)(natid, int);
static double (*pf_h)(coord, coord, coord, coord, double);

/*
 * Performance statistics
 */
#ifdef PATH_FIND_STATS
static unsigned pf_nsearch, pf_nsource, pf_nopen, pf_nreopen, pf_nclose;
static unsigned pf_nheap_max, pf_noway;
static double pf_sumcost;
#define STAT_INC(v) ((void)((v)++))
#define STAT_INCBY(v, i) ((void)((v) += i))
#define STAT_HIMARK(v, h) ((void)((v) < (h) ? (v) = (h) : (h)))
#else	/* !PATH_FIND_STATS */
#define STAT_INC(v) ((void)0)
#define STAT_INCBY(v, i) ((void)0)
#define STAT_HIMARK(v, h) ((void)0)
#endif	/* !PATH_FIND_STATS */

/* Is sector with uid UID open?  */
static int
pf_is_open(int uid)
{
    return pf_map[uid].visit == pf_visit;
}

/* Is sector with uid UID closed?  */
static int
pf_is_closed(int uid)
{
    /*
     * optimization: check > pf_visit instead of == pf_visit + 1
     * works because pf_map[uid].visit <= pf_visit + 1
     */
    return pf_map[uid].visit > pf_visit;
}

/* Is sector with uid UID unvisited?  */
static int
pf_is_unvisited(int uid)
{
    return pf_map[uid].visit < pf_visit;
}

#ifdef PATH_FIND_DEBUG
static void
pf_check(void)
{
    int i, uid, c;

    for (i = 0; i < pf_nheap; i++) {
	uid = pf_heap[i].uid;
	assert(0 <= uid && uid < WORLD_SZ());
	assert(pf_map[uid].heapi == i);
	assert(pf_map[uid].visit == pf_visit);
	assert(pf_map[uid].cost <= pf_heap[i].f);
	c = 2 * i + 1;
	assert(c >= pf_nheap || pf_heap[i].f <= pf_heap[c].f);
	c++;
	assert(c >= pf_nheap || pf_heap[i].f <= pf_heap[c].f);
    }

    for (uid = 0; uid < WORLD_SZ(); uid++) {
	assert(pf_map[uid].visit <= pf_visit + 1);
	if (pf_map[uid].visit == pf_visit) {
	    i = pf_map[uid].heapi;
	    assert(0 <= i && i < pf_nheap && pf_heap[i].uid == uid);
	}
    }
}
#else
#define pf_check() ((void)0)
#endif

/* Swap pf_heap's I-th and J-th elements.  */
static void
pf_heap_swap(int i, int j)
{
    struct pf_heap tmp;

    assert(0 <= i && i < pf_nheap);
    assert(0 <= j && j < pf_nheap);
    tmp = pf_heap[i];
    pf_heap[i] = pf_heap[j];
    pf_heap[j] = tmp;
    pf_map[pf_heap[i].uid].heapi = i;
    pf_map[pf_heap[j].uid].heapi = j;
}

/* Restore heap property after N-th element's cost increased.  */
static void
pf_sift_down(int n)
{
    int r, c;

    assert(0 <= n && n < pf_nheap);
    for (r = n; (c = 2 * r + 1) < pf_nheap; r = c) {
	if (c + 1 < pf_nheap && pf_heap[c].f > pf_heap[c + 1].f)
	    c++;
	if (pf_heap[r].f < pf_heap[c].f)
	    break;
	pf_heap_swap(r, c);
    }
}

/* Restore heap property after N-th element's cost decreased.  */
static void
pf_sift_up(int n)
{
    int c, p;

    assert(0 <= n && n < pf_nheap);
    for (c = n; (p = (c - 1) / 2), c > 0; c = p) {
	if (pf_heap[p].f < pf_heap[c].f)
	    break;
	pf_heap_swap(p, c);
    }
}

/*
 * Open the unvisited sector X,Y.
 * UID is sector uid, it equals XYOFFSET(X,Y).
 * Cheapest path from source comes from direction DIR and has cost COST.
 * H is the estimated cost from X,Y to the destination.
 */
static void
pf_open(int uid, coord x, coord y, int dir, double cost, double h)
{
    int i;

    if (pf_is_open(uid)) {
	STAT_INC(pf_nreopen);
	i = pf_map[uid].heapi;
	DPRINTF("pf: reopen %d,%d %g %c %d\n", x, y, cost, dirch[dir], i);
    } else {
	assert(pf_is_unvisited(uid));
	STAT_INC(pf_nopen);
	i = pf_nheap++;
	STAT_HIMARK(pf_nheap_max, (unsigned)pf_nheap);
	DPRINTF("pf: open %d,%d %g %c %d\n", x, y, cost, dirch[dir], i);
	pf_map[uid].heapi = i;
	pf_map[uid].visit = pf_visit;
	pf_heap[i].uid = uid;
	pf_heap[i].x = x;
	pf_heap[i].y = y;
    }
    pf_map[uid].dir = dir;
    pf_map[uid].cost = cost;
    pf_heap[i].f = cost + h;

    pf_sift_up(i);
    pf_check();
}

/*
 * Close the sector at the top of the heap.
 */
static void
pf_close(void)
{
    int uid = pf_heap[0].uid;

    STAT_INC(pf_nclose);
    DPRINTF("pf: close %d,%d %d\n", pf_heap[0].x, pf_heap[0].y, pf_nheap);
    assert(pf_is_open(uid));
    if (--pf_nheap) {
	pf_heap[0] = pf_heap[pf_nheap];
	pf_map[pf_heap[0].uid].heapi = 0;
	pf_sift_down(0);
    }
    pf_map[uid].visit = pf_visit + 1;
    pf_check();
}

/* silence "not used" warning */
#ifdef PATH_FIND_DEBUG
/*
 * Return cost from source to sector with uid UID.
 * It must be open (cost is an estimate) or closed (cost is exact).
 */
static double
pf_cost(int uid)
{
    assert(!pf_is_unvisited(uid));
    return pf_map[uid].cost;
}
#endif

static coord
x_in_dir(coord x, int dir)
{
    int xx;

    assert(0 <= x && x < WORLD_X);
    assert(0 <= dir && dir <= DIR_LAST);
    xx = x + diroff[dir][0];
    if (xx < 0)
	return xx + WORLD_X;
    if (xx >= WORLD_X)
	return xx - WORLD_X;
    return xx;
}

static coord
y_in_dir(coord y, int dir)
{
    int yy;

    assert(0 <= y && y < WORLD_Y);
    assert(0 <= dir && dir <= DIR_LAST);
    yy = y + diroff[dir][1];
    if (yy < 0)
	return yy + WORLD_Y;
    if (yy >= WORLD_Y)
	return yy - WORLD_Y;
    return yy;
}

static int
rev_dir(int dir)
{
    assert(DIR_FIRST <= dir && dir <= DIR_LAST);
    return dir >= DIR_FIRST + 3 ? dir - 3 : dir + 3;
}

/*
 * Set the current source and cost function.
 * SX,SY is the source.
 * The cost to enter the sector with uid u is COST(ACTOR, u).
 * Negative value means the sector can't be entered.
 * Optional H() is the heuristic: the cost from x,y to the destination
 * dx,dy is at least H(x, y, dx, dy, c1d), where c1d is the cost to
 * enter dx,dy.  The closer H() is to the true cost, the more
 * efficient this function works.
 * With a null H(), A* degenerates into Dijkstra's algorithm.  You can
 * then call path_find_to() multiple times for the same source.  This
 * is faster when you need many destinations.
 */
static void
pf_set_source(coord sx, coord sy, natid actor, double (*cost) (natid, int),
	      double (*h) (coord, coord, coord, coord, double))
{
    STAT_INC(pf_nsource);
    DPRINTF("pf: source %d,%d\n", sx, sy);
    pf_sx = sx;
    pf_sy = sy;
    pf_suid = XYOFFSET(sx, sy);
    pf_actor = actor;
    pf_sct_cost = cost;
    pf_h = h;

    if (!pf_map) {
	pf_map = calloc(WORLD_SZ(), sizeof(*pf_map));
	pf_heap = malloc(WORLD_SZ() * sizeof(*pf_heap));
	pf_visit = 1;
    } else if ((unsigned short)(pf_visit + 3) < pf_visit) {
	DPRINTF("pf: visit wrap-around\n");
	memset(pf_map, 0, WORLD_SZ() * sizeof(*pf_map));
	pf_visit = 1;
    } else
	pf_visit += 2;

    pf_nheap = 0;
}

/*
 * Find cheapest path from current source to DX,DY, return its cost.
 */
double
path_find_to(coord dx, coord dy)
{
    int duid;
    int uid, nuid, i;
    double c1d, cost, c1;
    coord x, y, nx, ny;

    STAT_INC(pf_nsearch);
    DPRINTF("pf: dest %d,%d\n", dx, dy);
    duid = XYOFFSET(dx, dy);
    if (pf_is_closed(duid)) {
	DPRINTF("pf: done old %g\n", pf_map[duid].cost);
	STAT_INCBY(pf_sumcost, pf_map[duid].cost);
	return pf_map[duid].cost;
    }

    c1d = pf_sct_cost(pf_actor, duid);
    if (c1d < 0) {
	DPRINTF("pf: done new %g\n", -1.0);
	STAT_INC(pf_noway);
	return -1;
    }

    if (pf_is_unvisited(pf_suid))
	/* first search from this source */
	pf_open(pf_suid, pf_sx, pf_sy, DIR_STOP, 0.0,
		pf_h ? pf_h(pf_sx, pf_sy, dx, dy, c1d) : 0.0);
    else
	assert(!pf_h);		/* multiple searches only w/o heuristic */

    while (pf_nheap > 0 && (uid = pf_heap[0].uid) != duid) {
	x = pf_heap[0].x;
	y = pf_heap[0].y;
	pf_close();
	cost = pf_map[uid].cost;

	for (i = 0; i < 6; i++) { /* for all neighbors */
	    nx = x_in_dir(x, DIR_FIRST + i);
	    ny = y_in_dir(y, DIR_FIRST + i);
	    nuid = XYOFFSET(nx, ny);
	    if (pf_h ? pf_is_closed(nuid) : !pf_is_unvisited(nuid))
		continue;
	    c1 = pf_sct_cost(pf_actor, nuid);
	    if (c1 < 0)
		continue;
	    if (pf_is_unvisited(nuid) || cost + c1 < pf_map[nuid].cost)
		pf_open(nuid, nx, ny, DIR_FIRST + i, cost + c1,
			pf_h ? pf_h(nx, ny, dx, dy, c1d) : 0);
	}
    }

    DPRINTF("pf: done new %g\n", !pf_nheap ? -1.0 : pf_map[duid].cost);
    if (!pf_nheap) {
	STAT_INC(pf_noway);
	return -1.0;
    }
    STAT_INCBY(pf_sumcost, pf_map[duid].cost);
    return pf_map[duid].cost;
}

/*
 * Write route from SX,SY to DX,DY to BUF[BUFSIZ], return its length.
 * If the route is longer than BUFSIZ-1 characters, it's truncated.
 * You must compute path cost first, with path_find_to().
 * SX,SY must be on a shortest path from the current source to DX,DY.
 */
size_t
path_find_route(char *buf, size_t bufsz,
		coord sx, coord sy, coord dx, coord dy)
{
    coord x, y;
    size_t i, len;
    int suid, uid, d;

    suid = XYOFFSET(sx, sy);
    assert(bufsz > 0 && !pf_is_unvisited(suid));

    i = bufsz;
    buf[--i] = 0;
    len = 0;

    x = dx;
    y = dy;
    for (;;) {
	DPRINTF("pf: %d,%d %.*s%.*s\n",
		x, y,
		(int)(bufsz - i), buf + i,
		len >= bufsz ? (int)i : 0, buf);
	uid = XYOFFSET(x, y);
	assert(!pf_is_unvisited(uid));
	d = pf_map[uid].dir;
	if (d == DIR_STOP || uid == suid)
	    break;
	if (!i)
	    i = bufsz;
	buf[--i] = dirch[d];
	len++;
	x = x_in_dir(x, rev_dir(d));
	y = y_in_dir(y, rev_dir(d));
    }

    assert(x == sx && y == sy);
    if (len >= bufsz)
	bufrotate(buf, bufsz, i);
    else {
	assert(i + len < bufsz);
	memmove(buf, buf + i, len + 1);
    }
    return len;
}

/*
 * Rotate BUF[BUFSZ] to put BUF[I] into BUF[0], and zero-terminate.
 */
static char *
bufrotate(char *buf, size_t bufsz, size_t i)
{
    char tmp[64];
    size_t n;

    while (i) {
	n = MIN(i, sizeof(tmp));
	memcpy(tmp, buf, n);
	memcpy(buf, buf + n, bufsz - n);
	memcpy(buf + bufsz - n, tmp, n);
	i -= n;
    }
    buf[bufsz - 1] = 0;
    return buf;
}

#ifdef PATH_FIND_DEBUG
void
path_find_visualize(coord sx, coord sy, coord dx, coord dy)
{
    int uid;
    int xmin, xmax, ymin, ymax, x, y, odd, ch;
    double c, u, cost;
    char buf[1024];

    assert(pf_cost(XYOFFSET(sx, sy)) == 0.0);
    c = pf_cost(XYOFFSET(dx, dy));
    u = c / 10.0;

    /* find bounding box */
    xmin = xmax = 0;
    ymin = ymax = 0;
    for (y = -WORLD_Y / 2; y < WORLD_Y / 2; y++) {
	odd = ((sx + -WORLD_X / 2) ^ (sy + y)) & 1;
	for (x = -WORLD_X / 2 + odd; x < WORLD_X / 2; x += 2) {
	    uid = XYOFFSET(XNORM(sx + x), YNORM(sy + y));
	    if (pf_is_unvisited(uid))
		continue;
	    if (xmin > x)
		xmin = x;
	    if (xmax < x)
		xmax = x;
	    if (ymin > y)
		ymin = y;
	    if (ymax < y)
		ymax = y;
	}
    }
    printf("bbox %d:%d,%d:%d origin %d,%d\n",
	   xmin, xmax, ymin, ymax, sx, sy);

    for (y = ymin; y <= ymax; y++) {
	odd = ((sx + xmin) ^ (sy + y)) & 1;
	if (odd)
	    printf(" ");
	for (x = xmin + odd; x <= xmax; x += 2) {
	    uid = XYOFFSET(XNORM(sx + x), YNORM(sy + y));
	    if (pf_is_unvisited(uid))
		ch = ' ';
	    else if (uid == XYOFFSET(dx, dy))
		ch = 'D';
	    else if (uid == XYOFFSET(sx, sy))
		ch = 'S';
	    else {
		cost = pf_cost(uid);
		ch = cost > c ? '+' : '0' + (int)(10 * (cost / c));
	    }
	    printf(" %c", ch);
	}
	printf("\n");
    }
    path_find_route(buf, sizeof(buf), sx, sy, dx, dy);
    printf("%s %g\n", buf, pf_cost(XYOFFSET(dx, dy)));
}
#endif

#ifdef PATH_FIND_STATS
void
path_find_print_stats(void)
{
    printf("pathfind %u searches, %u sources,"
	   " %u opened, %u reopened, %u closed,"
	   " %u heap max, %zu bytes, %u noway, %g avg cost\n",
	   pf_nsearch, pf_nsource, pf_nopen, pf_nreopen, pf_nclose,
	   pf_nheap_max,
	   (WORLD_SZ() * (sizeof(*pf_map) + sizeof(*pf_heap))),
	   pf_noway, pf_nsearch ? pf_sumcost / pf_nsearch : 0.0);
}
#endif

/*
 * Empire interface glue
 */

static double
cost_land(natid actor, int uid, int mobtype)
{
    /*
     * Non-negative cost must not depend on ACTOR, see BestLandPath().
     */
    struct sctstr *sp = (void *)empfile[EF_SECTOR].cache;

    if (sp[uid].sct_own != actor)
	return -1.0;
    return sector_mcost(&sp[uid], mobtype);
}

static double
cost_move(natid actor, int uid)
{
    return cost_land(actor, uid, MOB_MOVE);
}

static double
cost_march(natid actor, int uid)
{
    return cost_land(actor, uid, MOB_MARCH);
}

static double
cost_rail(natid actor, int uid)
{
    return cost_land(actor, uid, MOB_RAIL);
}

static double
h_move(coord x, coord y, coord dx, coord dy, double c1d)
{
    int md = mapdist(x, y, dx, dy);
    return md ? c1d + (md - 1) * 0.001 : 0.0;
}

static double
h_march_rail(coord x, coord y, coord dx, coord dy, double c1d)
{
    int md = mapdist(x, y, dx, dy);
    return md ? c1d + (md - 1) * 0.02 : 0.0;
}

static double
cost_sail(natid actor, int uid)
{
    struct sctstr *sp = (void *)empfile[EF_SECTOR].cache;
    natid sctown = sp[uid].sct_own;
    char *bmap;

    if (sctown && relations_with(sctown, actor) == ALLIED) {
	/* FIXME duplicates shp_check_nav() logic */
	switch (dchr[sp[uid].sct_type].d_nav) {
	case NAVOK:
	    return 1.0;
	case NAV_CANAL:
	    /* FIXME return 1.0 when all ships have M_CANAL */
	    return -1.0;
	case NAV_02:
	    return sp[uid].sct_effic >= 2 ? 1.0 : -1.0;
	case NAV_60:
	    return sp[uid].sct_effic >= 60 ? 1.0 : -1.0;
	default:
	    return -1.0;
	}
    }

    bmap = ef_ptr(EF_BMAP, actor);
    return bmap[uid] == '.' || bmap[uid] == ' ' || bmap[uid] == 0
	? 1.0 : -1.0;
}

static double
cost_fly(natid actor, int uid)
{
    return 1.0;
}

static double
h_sail_fly(coord x, coord y, coord dx, coord dy, double c1d)
{
    return mapdist(x, y, dx, dy);
}

static double (*cost_tab[])(natid, int) = {
    cost_move, cost_march, cost_rail, cost_sail, cost_fly
};

static double (*h[])(coord, coord, coord, coord, double) = {
    h_move, h_march_rail, h_march_rail, h_sail_fly, h_sail_fly
};

/*
 * Start finding paths from SX,SY.
 * Use mobility costs for ACTOR and MOBTYPE.
 */
void
path_find_from(coord sx, coord sy, natid actor, int mobtype)
{
    pf_set_source(sx, sy, actor, cost_tab[mobtype], NULL);
}

/*
 * Find cheapest path from SX,SY to DX,DY, return its mobility cost.
 * Use mobility costs for ACTOR and MOBTYPE.
 */
double
path_find(coord sx, coord sy, coord dx, coord dy, natid actor, int mobtype)
{
    pf_set_source(sx, sy, actor, cost_tab[mobtype], h[mobtype]);
    return path_find_to(dx, dy);
}
