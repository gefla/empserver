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
 *  fairland.c: Create a nice, new world
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2020
 */

/*
 * How fairland works
 *
 * 1. Place capitals
 *
 * Place the capitals on the torus in such a way so as to maximize
 * their distances from one another.  This uses the perturbation
 * technique of calculus of variations.
 *
 * 2. Grow start islands ("continents")
 *
 * For all continents, add the first sector at the capital's location,
 * and the second right to it.  These are the capital sectors.  Then
 * add one sector to each continent in turn, until they have the
 * specified size.
 *
 * Growth uses weighted random sampling to pick one sector from the
 * set of adjacent sea sectors that aren't too close to another
 * continent.  Growth operates in spiking mode with a chance given by
 * the spike percentage.  When "spiking", a sector's weight increases
 * with number of adjacent sea sectors.  This directs the growth away
 * from land, resulting in spikes.  When not spiking, the weight
 * increases with the number of adjacent land sectors.  This makes the
 * island more rounded.
 *
 * If growing fails due to lack of room, start over.  If it fails too
 * many times, give up and terminate unsuccessfully.
 *
 * 3. Place and grow additional islands
 *
 * Each continent has a "sphere of influence": the set of sectors
 * closer to it than to any other continent.  Each island is entirely
 * in one such sphere, and each sphere contains the same number of
 * islands with the same sizes.
 *
 * First, split the specified number of island sectors per continent
 * randomly into the island sizes.  Sort by size so that larger
 * islands are grown before smaller ones, to give the large ones the
 * best chance to grow to their planned size.
 *
 * Then place one island's first sector into each sphere, using
 * weighted random sampling with weights favoring sectors away from
 * land and other spheres.  Add one sector to each island in turn,
 * until they have the intended size.  Repeat until the specified
 * number of islands has been grown.
 *
 * If placement fails due to lack of room, start over, just like for
 * continents.
 *
 * Growing works as for continents, except the minimum distance for
 * additional islands applies, and growing simply stops when any of
 * the islands being grown lacks the room to grow further.  The number
 * of sectors not grown carries over to the next island size.
 *
 * 4. Compute elevation
 *
 * First, use a simple random hill algorithm to assign raw elevations:
 * initialize elevation to zero, then randomly raise circular hills on
 * land / lower circular depressions at sea.  Their size and height
 * depends on the distance to the coast.
 *
 * Then, elevate islands one after the other.
 *
 * Set the capitals' elevation to a fixed value.  Process the
 * remaining sectors in order of increasing raw elevation, first
 * non-mountains, then mountains.  Non-mountain elevation starts at 1,
 * and increases linearly to just below "high" elevation.  Mountain
 * elevation starts at "high" elevation, and increases linearly.
 *
 * This gives islands of the same size the same set of elevations.
 * Larger islands get more and taller mountains.
 *
 * Finally, elevate sea: normalize the raw elevations to [-127:-1].
 *
 * 5. Set resources
 *
 * Sector resources are simple functions of elevation.  You can alter
 * iron_conf[], gold_conf[], fert_conf[], oil_conf[], and uran_conf[]
 * to customize them.
 */

#include <config.h>

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include "chance.h"
#include "optlist.h"
#include "path.h"
#include "prototypes.h"
#include "sect.h"
#include "version.h"
#include "xy.h"

/* do not change these defines */
#define LANDMIN		1	/* plate altitude for normal land */
#define PLATMIN		36	/* plate altitude for plateau */
#define HIGHMIN		98	/* plate altitude for mountains */

/*
 * Resource configuration

 * Resources are determined by elevation.  The map from elevation to
 * resource is defined as a linear interpolation of resource data
 * points (elev, res) defined in the tables below.  Elevations range
 * from -127 to 127, and resource values from 0 to 100.
 */

struct resource_point {
    int elev, res;
};

struct resource_point iron_conf[] = {
    { -127, 0 },
    { 21, 0 },
    { 85, 100 },
    { HIGHMIN - 1, 100 },
    { HIGHMIN , 0 },
    { 127, 0 } };

struct resource_point gold_conf[] = {
    { -127, 0 },
    { 35, 0 },
    { HIGHMIN - 1, 80 },
    { HIGHMIN, 80 },
    { 127, 85 } };

struct resource_point fert_conf[] = {
    { -127, 100 },
    { -59, 100 },
    { LANDMIN - 1, 41 },
    { LANDMIN, 100 },
    { 10, 100 },
    { 56, 0 },
    { 127, 0 } };

struct resource_point oil_conf[] = {
    { -127, 100 },
    { -49, 100 },
    { LANDMIN - 1, 2 },
    { LANDMIN, 100 },
    { 6, 100 },
    { 34, 0 },
    { 127, 0 } };

struct resource_point uran_conf[] = {
    { -127, 0 },
    { 55, 0 },
    { 90, 100 },
    { 97, 100 },
    { 98, 0 },
    { 127, 0 } };

static void qprint(const char * const fmt, ...)
    ATTRIBUTE((format (printf, 1, 2)));

/*
 * Program arguments and options
 */
static char *program_name;
static int nc, sc;		/* number and size of continents */
static int ni, is;		/* number and size of islands */
#define DEFAULT_SPIKE 10
static int sp = DEFAULT_SPIKE;	/* spike percentage */
#define DEFAULT_MOUNTAIN 0
static int pm = DEFAULT_MOUNTAIN; /* mountain percentage */
#define DEFAULT_CONTDIST 2
static int di = DEFAULT_CONTDIST; /* min. distance between continents */
#define DEFAULT_ISLDIST 1
static int id = DEFAULT_ISLDIST;  /* ... continents and islands */
/* don't let the islands crash into each other.
   1 = don't merge, 0 = merge. */
static int DISTINCT_ISLANDS = 1;
static int quiet;
#define DEFAULT_OUTFILE_NAME "newcap_script"
static const char *outfile = DEFAULT_OUTFILE_NAME;

#define STABLE_CYCLE 4		/* stability required for perterbed capitals */
#define DRIFT_BEFORE_CHECK ((WORLD_X + WORLD_Y)/2)
#define DRIFT_MAX ((WORLD_X + WORLD_Y)*2)

/* handy macros:
*/

#define new_x(newx) (((newx) + WORLD_X) % WORLD_X)
#define new_y(newy) (((newy) + WORLD_Y) % WORLD_Y)

/*
 * Island sizes
 * isecs[i] is the size of the i-th island.
 */
static int *isecs;

static int *capx, *capy;	/* location of the nc capitals */

/*
 * Island at x, y
 * own[XYOFFSET(x, y)] is x,y's island number, -1 if water.
 */
static short *own;

/*
 * Adjacent land sectors
 * adj_land[XYOFFSET(x, y)] bit d is set exactly when the sector next
 * to x, y in direction d is land.
 */
static unsigned char *adj_land;

/*
 * Elevation at x,y
 * elev[XYOFFSET(x, y)] is x,y's elevation.
 */
static short *elev;

/*
 * Exclusive zones
 * Each island is surrounded by an exclusive zone where only it may
 * grow.  The width of the zone depends on minimum distances.
 * While growing continents, it is @di sectors wide.
 * While growing additional islands, it is @id sectors wide.
 * DISTINCT_ISLANDS nullifies the exclusive zone then.
 * xzone[XYOFFSET(x, y)] is -1 when the sector is in no exclusive
 * zone, a (non-negative) island number when it is in that island's
 * exclusive zone and no other, and -2 when it is in multiple
 * exclusive zones.
 */
static short *xzone;

/*
 * Set of sectors seen already
 * Increment @cur_seen to empty the set of sectors seen, set
 * seen[XYOFFSET(x, y)] to @cur_seen to add x,y to the set.
 */
static unsigned *seen;
static unsigned cur_seen;

/*
 * Closest continent and "distance"
 * closest[XYOFFSET(x, y)] is the closest continent's number.
 * distance[] is complicated; see init_spheres_of_influence() and
 * init_distance_to_coast().
 */
static natid *closest;
static unsigned short *distance;

/*
 * Queue for breadth-first search
 */
static int *bfs_queue;
static int bfs_queue_head, bfs_queue_tail;

static int **sectx, **secty;	/* the sectors for each continent */

#define NUMTRIES 10		/* keep trying to grow this many times */

static const char *numletter =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static void help(char *);
static void usage(void);
static void parse_args(int argc, char *argv[]);
static void allocate_memory(void);
static void init(void);
static int drift(void);
static int grow_continents(void);
static void create_elevations(void);
static void write_sects(void);
static void output(void);
static int write_newcap_script(void);
static int stable(int);
static void elevate_prep(void);
static void elevate_land(void);
static void elevate_sea(void);

static void print_vars(void);
static void fl_move(int);
static int grow_islands(void);

/* Debugging aids: */
void print_own_map(void);
void print_xzone_map(void);
void print_closest_map(void);
void print_distance_map(void);
void print_elev_map(void);

/****************************************************************************
  MAIN
****************************************************************************/

int
main(int argc, char *argv[])
{
    int opt;
    char *config_file = NULL;
    int try, done;
    unsigned rnd_seed = 0;
    int seed_set = 0;

    program_name = argv[0];

    while ((opt = getopt(argc, argv, "e:hiqR:s:v")) != EOF) {
	switch (opt) {
	case 'e':
	    config_file = optarg;
	    break;
	case 'i':
	    DISTINCT_ISLANDS = 0;
	    break;
	case 'q':
	    quiet = 1;
	    break;
	case 'R':
	    rnd_seed = strtoul(optarg, NULL, 10);
	    seed_set = 1;
	    break;
	case 's':
	    outfile = optarg;
	    break;
	case 'h':
	    usage();
	    exit(0);
	case 'v':
	    printf("%s\n\n%s", version, legal);
	    exit(0);
	default:
	    help(NULL);
	    exit(1);
	}
    }

    if (!seed_set)
	rnd_seed = pick_seed();
    seed_prng(rnd_seed);
    empfile_init();
    if (emp_config(config_file) < 0)
	exit(1);
    empfile_fixup();

    parse_args(argc - optind, argv + optind);

    allocate_memory();
    print_vars();

    qprint("\n        #*# ...fairland rips open a rift in the datumplane... #*#\n\n");
    qprint("seed is %u\n", rnd_seed);
    try = 0;
    do {
	init();
	if (try)
	    qprint("\ntry #%d (out of %d)...\n", try + 1, NUMTRIES);
	qprint("placing capitals...\n");
	if (!drift())
	    qprint("unstable drift\n");
	qprint("growing continents...\n");
	done = grow_continents();
	if (!done)
	    continue;
	qprint("growing islands:");
	done = grow_islands();
    } while (!done && ++try < NUMTRIES);
    if (!done) {
	fprintf(stderr, "%s: world not large enough for this much land\n",
		program_name);
	exit(1);
    }
    qprint("elevating land...\n");
    create_elevations();

    qprint("writing to sectors file...\n");
    if (!write_newcap_script())
	exit(1);
    if (chdir(gamedir)) {
	fprintf(stderr, "%s: can't chdir to %s (%s)\n",
		program_name, gamedir, strerror(errno));
	exit(1);
    }
    if (!ef_open(EF_SECTOR, EFF_MEM | EFF_NOTIME))
	exit(1);
    write_sects();
    if (!ef_close(EF_SECTOR))
	exit(1);

    output();
    qprint("\n\nA script for adding all the countries can be found in \"%s\".\n",
	   outfile);
    exit(0);
}

static void
print_vars(void)
{
    if (quiet)
	return;
    puts("Creating a planet with:\n");
    printf("%d continents\n", nc);
    printf("continent size: %d\n", sc);
    printf("number of islands: %d\n", ni);
    printf("average size of islands: %d\n", is);
    printf("spike: %d%%\n", sp);
    printf("%d%% of land is mountain (each continent will have %d mountains)\n",
	   pm, (pm * sc) / 100);
    printf("minimum distance between continents: %d\n", di);
    printf("minimum distance from islands to continents: %d\n", id);
    printf("World dimensions: %dx%d\n", WORLD_X, WORLD_Y);
}

static void
help(char *complaint)
{
    if (complaint)
	fprintf(stderr, "%s: %s\n", program_name, complaint);
    fprintf(stderr, "Try -h for help.\n");
}

static void
usage(void)
{
    printf("Usage: %s [OPTION]... NC SC [NI] [IS] [SP] [PM] [DI] [ID]\n"
	   "  -e CONFIG-FILE  configuration file\n"
	   "                  (default %s)\n"
	   "  -i              islands may merge\n"
	   "  -q              quiet\n"
	   "  -R SEED         seed for random number generator\n"
	   "  -s SCRIPT       name of script to create (default %s)\n"
	   "  -h              display this help and exit\n"
	   "  -v              display version information and exit\n"
	   "  NC              number of continents\n"
	   "  SC              continent size\n"
	   "  NI              number of islands (default NC)\n"
	   "  IS              average island size (default SC/2)\n"
	   "  SP              spike percentage: 0 = round, 100 = snake (default %d)\n"
	   "  PM              percentage of land that is mountain (default %d)\n"
	   "  DI              minimum distance between continents (default %d)\n"
	   "  ID              minimum distance from islands to continents (default %d)\n",
	   program_name, dflt_econfig, DEFAULT_OUTFILE_NAME,
	   DEFAULT_SPIKE, DEFAULT_MOUNTAIN, DEFAULT_CONTDIST, DEFAULT_ISLDIST);
}

static void
parse_args(int argc, char *argv[])
{
    int dist_max = mapdist(0, 0, WORLD_X / 2, WORLD_Y / 2);

    if (argc < 2) {
	help("missing arguments");
	exit(1);
    }
    if (argc > 8) {
	help("too many arguments");
	exit(1);
    }
    nc = atoi(argv[0]);
    if (nc < 1) {
	fprintf(stderr, "%s: number of continents must be > 0\n",
		program_name);
	exit(1);
    }

    sc = atoi(argv[1]);
    if (sc < 2) {
	fprintf(stderr, "%s: size of continents must be > 1\n",
		program_name);
	exit(1);
    }

    ni = nc;
    is = sc / 2;

    if (argc > 2)
	ni = atoi(argv[2]);
    if (ni < 0) {
	fprintf(stderr, "%s: number of islands must be >= 0\n",
		program_name);
	exit(1);
    }
    if (ni % nc) {
	fprintf(stderr, "%s: number of islands must be a multiple of"
		" the number of continents\n",
		program_name);
	exit(1);
    }

    if (argc > 3)
	is = atoi(argv[3]);
    if (is < 1) {
	fprintf(stderr, "%s: size of islands must be > 0\n",
		program_name);
	exit(1);
    }

    if (argc > 4)
	sp = atoi(argv[4]);
    if (sp < 0 || sp > 100) {
	fprintf(stderr,
		"%s: spike percentage must be between 0 and 100\n",
		program_name);
	exit(1);
    }

    if (argc > 5)
	pm = atoi(argv[5]);
    if (pm < 0 || pm > 100) {
	fprintf(stderr,
		"%s: mountain percentage must be between 0 and 100\n",
		program_name);
	exit(1);
    }

    if (argc > 6)
	di = atoi(argv[6]);
    if (di < 0) {
	fprintf(stderr, "%s: distance between continents must be >= 0\n",
		program_name);
	exit(1);
    }
    if (di > dist_max) {
	fprintf(stderr, "%s: distance between continents too large\n",
		program_name);
	exit(1);
    }

    if (argc > 7)
	id = atoi(argv[7]);
    if (id < 0) {
	fprintf(stderr,
		"%s: distance from islands to continents must be >= 0\n",
		program_name);
	exit(1);
    }
    if (id > dist_max) {
	fprintf(stderr,
		"%s: distance from islands to continents too large\n",
		program_name);
	exit(1);
    }
}

/****************************************************************************
  VARIABLE INITIALIZATION
****************************************************************************/

static void
allocate_memory(void)
{
    int i;

    capx = calloc(nc, sizeof(int));
    capy = calloc(nc, sizeof(int));
    own = malloc(WORLD_SZ() * sizeof(*own));
    adj_land = malloc(WORLD_SZ() * sizeof(*adj_land));
    elev = calloc(WORLD_SZ(), sizeof(*elev));
    xzone = malloc(WORLD_SZ() * sizeof(*xzone));
    seen = calloc(WORLD_SZ(), sizeof(*seen));
    closest = malloc(WORLD_SZ() * sizeof(*closest));
    distance = malloc(WORLD_SZ() * sizeof(*distance));
    bfs_queue = malloc(WORLD_SZ() * sizeof(*bfs_queue));
    sectx = calloc(nc + ni, sizeof(int *));
    secty = calloc(nc + ni, sizeof(int *));
    isecs = calloc(nc + ni, sizeof(int));
    for (i = 0; i < nc; ++i) {
	sectx[i] = calloc(sc, sizeof(int));
	secty[i] = calloc(sc, sizeof(int));
    }
    for (i = nc; i < nc + ni; ++i) {
	sectx[i] = calloc(is * 2, sizeof(int));
	secty[i] = calloc(is * 2, sizeof(int));
    }

}

static void
init(void)
{
    int i;

    for (i = 0; i < WORLD_SZ(); i++)
	own[i] = -1;
    memset(adj_land, 0, WORLD_SZ() * sizeof(*adj_land));
}

/****************************************************************************
  DRIFT THE CAPITALS UNTIL THEY ARE AS FAR AWAY FROM EACH OTHER AS POSSIBLE
****************************************************************************/

/*
 * How isolated is capital @j at @newx,@newy?
 * Return the distance to the closest other capital.
 */
static int
iso(int j, int newx, int newy)
{
    int d = INT_MAX;
    int i, md;

    for (i = 0; i < nc; ++i) {
	if (i == j)
	    continue;
	md = mapdist(capx[i], capy[i], newx, newy);
	if (md < d)
	    d = md;
    }

    return d;
}

/*
 * Drift the capitals
 * Return 1 for a stable drift, 0 for an unstable one.
 */
static int
drift(void)
{
    int turns, i;

    for (i = 0; i < nc; i++) {
	capy[i] = (2 * i) / WORLD_X;
	capx[i] = (2 * i) % WORLD_X + capy[i] % 2;
	if (capy[i] >= WORLD_Y) {
	    fprintf(stderr,
		    "%s: world not big enough for all the continents\n",
		    program_name);
	    exit(1);
	}
    }

    for (turns = 0; turns < DRIFT_MAX; ++turns) {
	if (stable(turns))
	    return 1;
	for (i = 0; i < nc; ++i)
	    fl_move(i);
    }
    return 0;
}

/*
 * Has the drift stabilized?
 * @turns is the number of turns so far.
 */
static int
stable(int turns)
{
    static int mc[STABLE_CYCLE];
    int i, isod, d = 0, stab = 1;

    if (!turns) {
	for (i = 0; i < STABLE_CYCLE; i++)
	    mc[i] = i;
    }

    if (turns <= DRIFT_BEFORE_CHECK)
	return 0;

    for (i = 0; i < nc; ++i) {
	isod = iso(i, capx[i], capy[i]);
	if (isod > d)
	    d = isod;
    }

    for (i = 0; i < STABLE_CYCLE; ++i)
	if (d != mc[i])
	    stab = 0;

    mc[turns % STABLE_CYCLE] = d;
    return stab;
}

/* This routine does the actual drifting
*/

static void
fl_move(int j)
{
    int dir, i, newx, newy;

    dir = DIR_L + roll0(6);
    for (i = 0; i < 6; i++) {
	if (dir > DIR_LAST)
	    dir -= 6;
	newx = new_x(capx[j] + diroff[dir][0]);
	newy = new_y(capy[j] + diroff[dir][1]);
	dir++;
	if (iso(j, newx, newy) >= iso(j, capx[j], capy[j])) {
	    capx[j] = newx;
	    capy[j] = newy;
	    return;
	}
    }
}

/****************************************************************************
  GROW THE CONTINENTS
****************************************************************************/

static int
is_coastal(int x, int y)
{
    return adj_land[XYOFFSET(x, y)]
	!= (1u << (DIR_LAST + 1)) - (1u << DIR_FIRST);
}

struct hexagon_iter {
    int dir, i, n;
};

/*
 * Start iterating around @x0,@y0 at distance @d.
 * Set *x,*y to coordinates of the first sector.
 */
static inline void
hexagon_first(struct hexagon_iter *iter, int x0, int y0, int n,
	      int *x, int *y)
{
    *x = new_x(x0 - 2 * n);
    *y = y0;
    iter->dir = DIR_FIRST;
    iter->i = 0;
    iter->n = n;
}

/*
 * Continue iteration started with hexagon_first().
 * Set *x,*y to coordinates of the next sector.
 * Return whether we're back at the first sector, i.e. iteration is
 * complete.
 */
static inline int
hexagon_next(struct hexagon_iter *iter, int *x, int *y)
{
    *x = new_x(*x + diroff[iter->dir][0]);
    *y = new_y(*y + diroff[iter->dir][1]);
    iter->i++;
    if (iter->i == iter->n) {
	iter->i = 0;
	iter->dir++;
    }
    return iter->dir <= DIR_LAST;
}

/*
 * Is @x,@y in no exclusive zone other than perhaps @c's?
 */
static int
xzone_ok(int c, int x, int y)
{
    int off = XYOFFSET(x, y);

    return xzone[off] == c || xzone[off] == -1;
}

/*
 * Add sectors within distance @dist of @x,@y to @c's exclusive zone.
 */
static void
xzone_around_sector(int c, int x, int y, int dist)
{
    int d, x1, y1, off;
    struct hexagon_iter hexit;

    assert(xzone_ok(c, x, y));

    xzone[XYOFFSET(x, y)] = c;
    for (d = 1; d <= dist; d++) {
	hexagon_first(&hexit, x, y, d, &x1, &y1);
	do {
	    off = XYOFFSET(x1, y1);
	    if (xzone[off] == -1)
		xzone[off] = c;
	    else if (xzone[off] != c)
		xzone[off] = -2;
	} while (hexagon_next(&hexit, &x1, &y1));
    }
}

/*
 * Add sectors within distance @dist to island @c's exclusive zone.
 */
static void
xzone_around_island(int c, int dist)
{
    int i;

    for (i = 0; i < isecs[c]; i++)
	xzone_around_sector(c, sectx[c][i], secty[c][i], dist);
}

/*
 * Initialize exclusive zones around @n islands.
 */
static void
xzone_init(int n)
{
    int i, c;

    for (i = 0; i < WORLD_SZ(); i++)
	xzone[i] = -1;

    for (c = 0; c < n; c++)
	xzone_around_island(c, id);
}

/*
 * Initialize breadth-first search.
 */
static void
bfs_init(void)
{
    int i;

    for (i = 0; i < WORLD_SZ(); i++) {
	closest[i] = -1;
	distance[i] = USHRT_MAX;
    }

    bfs_queue_head = bfs_queue_tail = 0;
}

/*
 * Add sector @x,@y to the BFS queue.
 * It's closest to @c, with distance @dist.
 */
static void
bfs_enqueue(int c, int x, int y, int dist)
{
    int off = XYOFFSET(x, y);

    assert(dist < distance[off]);
    closest[off] = c;
    distance[off] = dist;
    bfs_queue[bfs_queue_tail] = off;
    bfs_queue_tail++;
    if (bfs_queue_tail >= WORLD_SZ())
	bfs_queue_tail = 0;
    assert(bfs_queue_tail != bfs_queue_head);
}

/*
 * Search breadth-first until the queue is empty.
 */
static void
bfs_run_queue(void)
{
    int off, dist, i, noff, nx, ny;
    coord x, y;

    while (bfs_queue_head != bfs_queue_tail) {
	off = bfs_queue[bfs_queue_head];
	bfs_queue_head++;
	if (bfs_queue_head >= WORLD_SZ())
	    bfs_queue_head = 0;
	dist = distance[off] + 1;
	sctoff2xy(&x, &y, off);
	for (i = DIR_FIRST; i <= DIR_LAST; i++) {
	    nx = new_x(x + diroff[i][0]);
	    ny = new_y(y + diroff[i][1]);
	    noff = XYOFFSET(nx, ny);
	    if (dist < distance[noff]) {
		bfs_enqueue(closest[off], nx, ny, dist);
	    } else if (distance[noff] == dist) {
		if (closest[off] != closest[noff])
		    closest[noff] = (natid)-1;
	    } else
		assert(distance[noff] < dist);
	}
    }
}

/*
 * Add island @c's coastal sectors to the BFS queue, with distance 0.
 */
static void
bfs_enqueue_island(int c)
{
    int i;

    for (i = 0; i < isecs[c]; i++) {
	if (is_coastal(sectx[c][i], secty[c][i]))
	    bfs_enqueue(c, sectx[c][i], secty[c][i], 0);
    }
}

/*
 * Enqueue spheres of influence borders for breadth-first search.
 */
static void
bfs_enqueue_border(void)
{
    int x, y, off, dir, nx, ny, noff;

    for (y = 0; y < WORLD_Y; y++) {
	for (x = y % 2; x < WORLD_X; x += 2) {
	    off = XYOFFSET(x, y);
	    if (distance[off] <= id + 1)
		continue;
	    if (closest[off] == (natid)-1)
		continue;
	    for (dir = DIR_FIRST; dir <= DIR_LAST; dir++) {
		nx = new_x(x + diroff[dir][0]);
		ny = new_y(y + diroff[dir][1]);
		noff = XYOFFSET(nx, ny);
		if (closest[noff] != closest[off]) {
		    bfs_enqueue(closest[off], x, y, id + 1);
		    break;
		}
	    }
	}
    }
}

/*
 * Compute spheres of influence
 * A continent's sphere of influence is the set of sectors closer to
 * it than to any other continent.
 * Set closest[XYOFFSET(x, y)] to the closest continent's number,
 * -1 if no single continent is closest.
 * Set distance[XYOFFSET(x, y)] to the minimum of the distance to the
 * closest coastal land sector and the distance to just outside the
 * sphere of influence plus @id.  For sea sectors within a continent's
 * sphere of influence, distance[off] - id is the distance to the
 * border of the area where additional islands can be placed.
 */
static void
init_spheres_of_influence(void)
{
    int c;

    bfs_init();
    for (c = 0; c < nc; c++)
	bfs_enqueue_island(c);
    bfs_run_queue();
    bfs_enqueue_border();
    bfs_run_queue();
}

/*
 * Precompute distance to coast
 * Set distance[XYOFFSET(x, y)] to the distance to the closest coastal
 * land sector.
 * Set closest[XYOFFSET(x, y)] to the closest continent's number,
 * -1 if no single continent is closest.
 */
static void
init_distance_to_coast(void)
{
    int c;

    bfs_init();
    for (c = 0; c < nc + ni; c++)
	bfs_enqueue_island(c);
    bfs_run_queue();
}

/*
 * Is @x,@y in the same sphere of influence as island @c?
 * Always true when @c is a continent.
 */
static int
is_in_sphere(int c, int x, int y)
{
    return c < nc || closest[XYOFFSET(x, y)] == c % nc;
}

/*
 * Can island @c grow at @x,@y?
 */
static int
can_grow_at(int c, int x, int y)
{
    return own[XYOFFSET(x, y)] == -1 && xzone_ok(c, x, y)
	&& is_in_sphere(c, x, y);
}

static void
adj_land_update(int x, int y)
{
    int is_land = own[XYOFFSET(x, y)] != -1;
    int dir, nx, ny, noff;

    for (dir = DIR_FIRST; dir <= DIR_LAST; dir++) {
	nx = new_x(x + diroff[dir][0]);
	ny = new_y(y + diroff[dir][1]);
	noff = XYOFFSET(nx, ny);
	if (is_land)
	    adj_land[noff] |= 1u << DIR_BACK(dir);
	else
	    adj_land[noff] &= ~(1u << DIR_BACK(dir));
    }
}

static void
add_sector(int c, int x, int y)
{
    int off = XYOFFSET(x, y);

    assert(own[off] == -1);
    xzone_around_sector(c, x, y, c < nc ? di : DISTINCT_ISLANDS ? id : 0);
    sectx[c][isecs[c]] = x;
    secty[c][isecs[c]] = y;
    isecs[c]++;
    own[off] = c;
    adj_land_update(x, y);
}

static int grow_weight(int c, int x, int y, int spike)
{
    int n, b;

    /*
     * #Land neighbors is #bits set in adj_land[].
     * Count them Brian Kernighan's way.
     */
    n = 0;
    for (b = adj_land[XYOFFSET(x, y)]; b; b &= b - 1)
	n++;
    assert(n > 0 && n < 7);

    if (spike)
	return (6 - n) * (6 - n);

    return n * n * n;
}

static int
grow_one_sector(int c)
{
    int spike = roll0(100) < sp;
    int wsum, newx, newy, i, x, y, off, dir, nx, ny, noff, w;

    assert(cur_seen < UINT_MAX);
    cur_seen++;
    wsum = 0;
    newx = newy = -1;

    for (i = 0; i < isecs[c]; i++) {
	x = sectx[c][i];
	y = secty[c][i];
	off = XYOFFSET(x, y);

	for (dir = DIR_FIRST; dir <= DIR_LAST; dir++) {
	    if (adj_land[off] & (1u << dir))
		continue;
	    nx = new_x(x + diroff[dir][0]);
	    ny = new_y(y + diroff[dir][1]);
	    noff = XYOFFSET(nx, ny);
	    if (seen[noff] == cur_seen)
		continue;
	    assert(seen[noff] < cur_seen);
	    seen[noff] = cur_seen;
	    if (!can_grow_at(c, nx, ny))
		continue;
	    w = grow_weight(c, nx, ny, spike);
	    assert(wsum < INT_MAX - w);
	    wsum += w;
	    if (roll0(wsum) < w) {
		newx = nx;
		newy = ny;
	    }
	}
    }

    if (!wsum)
	return 0;

    add_sector(c, newx, newy);
    return 1;
}

/*
 * Grow the continents.
 * Return 1 on success, 0 on error.
 */
static int
grow_continents(void)
{
    int done = 1;
    int c, secs;

    xzone_init(0);

    for (c = 0; c < nc; ++c) {
	isecs[c] = 0;
	if (!can_grow_at(c, capx[c], capy[c])
	    || !can_grow_at(c, new_x(capx[c] + 2), capy[c])) {
	    done = 0;
	    continue;
	}
	add_sector(c, capx[c], capy[c]);
	add_sector(c, new_x(capx[c] + 2), capy[c]);
    }

    if (!done) {
	qprint("No room for continents\n");
	return 0;
    }

    for (secs = 2; secs < sc && done; secs++) {
	for (c = 0; c < nc; ++c) {
	    if (!grow_one_sector(c))
		done = 0;
	}
    }

    if (!done)
	qprint("Only managed to grow %d out of %d sectors.\n",
	       secs - 1, sc);
    return done;
}

/****************************************************************************
  GROW THE ISLANDS
****************************************************************************/

/*
 * Place additional island @c's first sector.
 * Return 1 on success, 0 on error.
 */
static int
place_island(int c, int isiz)
{
    int n, x, y, d, w, newx, newy;

    n = 0;

    for (y = 0; y < WORLD_Y; y++) {
	for (x = y % 2; x < WORLD_X; x += 2) {
	    if (can_grow_at(c, x, y)) {
		d = distance[XYOFFSET(x, y)];
		assert(d > id);
		w = (d - id) * (d - id);
		n += MIN(w, (isiz + 2) / 3);
		if (roll0(n) < w) {
		    newx = x;
		    newy = y;
		}
	    }
	}
    }

    if (n)
	add_sector(c, newx, newy);
    return n;
}

static int
int_cmp(const void *a, const void *b)
{
    return *(int *)b - *(int *)a;
}

static int *
size_islands(void)
{
    int n = ni / nc;
    int *isiz = malloc(n * sizeof(*isiz));
    int r0, r1, i;

    isiz[0] = n * is;
    r1 = roll0(is);
    for (i = 1; i < n; i++) {
	r0 = r1;
	r1 = roll0(is);
	isiz[i] = is + r1 - r0;
	isiz[0] -= isiz[i];
    }

    qsort(isiz, n, sizeof(*isiz), int_cmp);
    return isiz;
}

/*
 * Grow the additional islands.
 * Return 1 on success, 0 on error.
 */
static int
grow_islands(void)
{
    int *island_size = size_islands();
    int xzone_valid = 0;
    int carry = 0;
    int i, j, c, done, secs, isiz, x, y;

    init_spheres_of_influence();

    for (i = 0; i < ni / nc; i++) {
	c = nc + i * nc;

	if (!xzone_valid)
	    xzone_init(c);

	carry += island_size[i];
	isiz = MIN(2 * is, carry);

	for (j = 0; j < nc; j++) {
	    isecs[c + j] = 0;
	    if (!place_island(c + j, isiz)) {
		qprint("\nNo room for island #%d\n", c - nc + j + 1);
		free(island_size);
		return 0;
	    }
	}

	done = 1;
	for (secs = 1; secs < isiz && done; secs++) {
	    for (j = 0; j < nc; j++) {
		if (!grow_one_sector(c + j))
		    done = 0;
	    }
	}

	if (!done) {
	    secs--;
	    for (j = 0; j < nc; j++) {
		if (isecs[c + j] != secs) {
		    isecs[c + j]--;
		    assert(isecs[c + j] == secs);
		    x = sectx[c + j][secs];
		    y = secty[c + j][secs];
		    own[XYOFFSET(x, y)] = -1;
		    adj_land_update(x, y);
		}
	    }
	    xzone_valid = 0;
	}

	for (j = 0; j < nc; j++)
	    qprint(" %d(%d)", c - nc + j + 1, isecs[c + j]);

	carry -= secs;
    }

    free(island_size);
    qprint("\n");

    if (carry)
	qprint("Only managed to grow %d out of %d island sectors.\n",
	       is * ni - carry * nc, is * ni);

    return 1;
}

/****************************************************************************
  CREATE ELEVATIONS
****************************************************************************/
static void
create_elevations(void)
{
    elevate_prep();
    elevate_land();
    elevate_sea();
}

static int
elev_cmp(const void *p, const void *q)
{
    int a = *(int *)p;
    int b = *(int *)q;
    int delev = elev[a] - elev[b];

    return delev ? delev : a - b;
}

static void
elevate_prep(void)
{
    int n = WORLD_SZ() * 8;
    int off0, r, sign, elevation, d, x1, y1, off1;
    coord x0, y0;
    struct hexagon_iter hexit;

    init_distance_to_coast();

    while (n > 0) {
	off0 = roll0(WORLD_SZ());
	sctoff2xy(&x0, &y0, off0);
	if (own[off0] == -1) {
	    r = roll(MIN(3, distance[off0]));
	    sign = -1;
	} else {
	    r = roll(MIN(3, distance[off0]) + 1);
	    sign = 1;
	}
	elevation = elev[off0] + sign * r * r;
	elev[off0] = LIMIT_TO(elevation, SHRT_MIN, SHRT_MAX);
	n--;
	for (d = 1; d < r; d++) {
	    hexagon_first(&hexit, x0, y0, d, &x1, &y1);
	    do {
		off1 = XYOFFSET(x1, y1);
		elevation = elev[off1] + sign * (r * r - d * d);
		elev[off1] = LIMIT_TO(elevation, SHRT_MIN, SHRT_MAX);
		n--;
	    } while (hexagon_next(&hexit, &x1, &y1));
	}
    }
}

static void
elevate_land(void)
{
    int *off = malloc(MAX(sc, is * 2) * sizeof(*off));
    int max_nm = (pm * MAX(sc, is * 2)) / 100;
    int c, nm, i0, n, i;
    double elevation, delta;

    for (c = 0; c < nc + ni; c++) {
	nm = (pm * isecs[c]) / 100;
	i0 = c < nc ? 2 : 0;
	n = isecs[c] - i0;
	for (i = 0; i < i0; i++)
	    elev[XYOFFSET(sectx[c][i], secty[c][i])] = PLATMIN;
	for (i = 0; i < n; i++)
	    off[i] = XYOFFSET(sectx[c][i0 + i], secty[c][i0 + i]);
	qsort(off, n, sizeof(*off), elev_cmp);
	delta = (double)(HIGHMIN - LANDMIN - 1) / (n - nm - 1);
	elevation = LANDMIN;
	for (i = 0; i < n - nm; i++) {
	    elev[off[i]] = (int)(elevation + 0.5);
	    elevation += delta;
	}
	elevation = HIGHMIN;
	delta = (127.0 - HIGHMIN) / max_nm;
	for (; i < n; i++) {
	    elevation += delta;
	    elev[off[i]] = (int)(elevation + 0.5);
	}
    }

    free(off);
}

static void
elevate_sea(void)
{
    int i, min;

    min = 0;
    for (i = 0; i < WORLD_SZ(); i++) {
	if (elev[i] < min)
	    min = elev[i];
    }

    for (i = 0; i < WORLD_SZ(); i++) {
	if (elev[i] < 0)
	    elev[i] = -1 - 126 * elev[i] / min;
    }
}

static int
elev_to_sct_type(int elevation)
{
    if (elevation < LANDMIN)
	return SCT_WATER;
    if (elevation < HIGHMIN)
	return SCT_RURAL;
    return SCT_MOUNT;
}

/****************************************************************************
  ADD THE RESOURCES
****************************************************************************/

/*
 * Map elevation @elev to a resource value according to @conf.
 * This is a linear interpolation on the data points in @conf.
 */
static int
elev_to_resource(int elev, struct resource_point conf[])
{
    int i, elev1, elev2, delev, res1, res2, dres;

    for (i = 1; elev > conf[i].elev; i++) ;
    assert(conf[i - 1].elev <= elev);

    elev1 = conf[i - 1].elev;
    elev2 = conf[i].elev;
    delev = elev2 - elev1;
    res1 = conf[i - 1].res;
    res2 = conf[i].res;
    dres = res2 - res1;
    return (int)(res1 + (double)((elev - elev1) * dres) / delev);
}

static void
add_resources(struct sctstr *sct)
{
    sct->sct_min = elev_to_resource(sct->sct_elev, iron_conf);
    sct->sct_gmin = elev_to_resource(sct->sct_elev, gold_conf);
    sct->sct_fertil = elev_to_resource(sct->sct_elev, fert_conf);
    sct->sct_oil = elev_to_resource(sct->sct_elev, oil_conf);
    sct->sct_uran = elev_to_resource(sct->sct_elev, uran_conf);
}

/****************************************************************************
  DESIGNATE THE SECTORS
****************************************************************************/

static void
write_sects(void)
{
    struct sctstr *sct;
    int x, y;

    for (y = 0; y < WORLD_Y; y++) {
	for (x = y % 2; x < WORLD_X; x += 2) {
	    sct = getsectp(x, y);
	    sct->sct_elev = elev[sct->sct_uid];
	    sct->sct_type = elev_to_sct_type(sct->sct_elev);
	    sct->sct_newtype = sct->sct_type;
	    sct->sct_dterr = own[sct->sct_uid] + 1;
	    sct->sct_coastal = is_coastal(sct->sct_x, sct->sct_y);
	    add_resources(sct);
	}
    }
}

/****************************************************************************
  PRINT A PICTURE OF THE MAP TO YOUR SCREEN
****************************************************************************/
static void
output(void)
{
    int sx, sy, x, y, off, c, type;

    if (quiet == 0) {
	for (sy = -WORLD_Y / 2; sy < WORLD_Y / 2; sy++) {
	    y = YNORM(sy);
	    puts("");
	    if (y % 2)
		printf(" ");
	    for (sx = -WORLD_X / 2 + y % 2; sx < WORLD_X / 2; sx += 2) {
		x = XNORM(sx);
		off = XYOFFSET(x, y);
		c = own[off];
		type = elev_to_sct_type(elev[off]);
		if (type == SCT_WATER)
		    printf(". ");
		else if (type == SCT_MOUNT)
		    printf("^ ");
		else if (c >= nc)
		    printf("%% ");
		else {
		    assert(0 <= c && c < nc);
		    if ((x == capx[c] || x == new_x(capx[c] + 2))
			&& y == capy[c])
			printf("%c ", numletter[c % 62]);
		    else
			printf("# ");
		}
	    }
	}
    }
}

/*
 * Print a map to help visualize own[].
 * This is for debugging.
 */
void
print_own_map(void)
{
    int sx, sy, x, y, off;

    for (sy = -WORLD_Y / 2; sy < WORLD_Y / 2; sy++) {
	y = YNORM(sy);
	printf("%4d ", sy);
	for (sx = -WORLD_X / 2; sx < WORLD_X / 2; sx++) {
	    x = XNORM(sx);
	    off = XYOFFSET(x, y);
	    if ((x + y) & 1)
		putchar(' ');
	    else if (own[off] == -1)
		putchar('.');
	    else
		putchar(numletter[own[off] % 62]);
	}
	putchar('\n');
    }
}

/*
 * Print a map to help visualize elev[].
 * This is for debugging.  It expects the terminal to understand
 * 24-bit color escape sequences \e[48;2;$red;$green;$blue;m.
 */
void
print_elev_map(void)
{
    int sx, sy, x, y, off, sat;

    for (sy = -WORLD_Y / 2; sy < WORLD_Y / 2; sy++) {
	y = YNORM(sy);
	printf("%4d ", sy);
	for (sx = -WORLD_X / 2; sx < WORLD_X / 2; sx++) {
	    x = XNORM(sx);
	    off = XYOFFSET(x, y);
	    if ((x + y) & 1)
		putchar(' ');
	    else if (!elev[off])
		putchar(' ');
	    else if (elev[off] < 0) {
		sat = 256 + elev[off] * 2;
		printf("\033[48;2;%d;%d;%dm \033[0m", sat, sat, 255);
	    } else if (elev[off] < HIGHMIN / 2) {
		sat = (HIGHMIN / 2 - elev[off]) * 4;
		printf("\033[48;2;%d;%d;%dm \033[0m", sat, 255, sat);
	    } else if (elev[off] < HIGHMIN) {
		sat = 128 + (HIGHMIN - elev[off]) * 2;
		printf("\033[48;2;%d;%d;%dm \033[0m", sat, sat / 2, sat / 4);
	    } else {
		sat = 128 + (elev[off] - HIGHMIN) * 2;
		printf("\033[48;2;%d;%d;%dm^\033[0m", sat, sat, sat);
	    }
	}
	putchar('\n');
    }
}

/*
 * Print a map to help visualize xzone[].
 * This is for debugging.
 */
void
print_xzone_map(void)
{
    int sx, sy, x, y, off;

    for (sy = -WORLD_Y / 2; sy < WORLD_Y / 2; sy++) {
	y = YNORM(sy);
	printf("%4d ", sy);
	for (sx = -WORLD_X / 2; sx < WORLD_X / 2; sx++) {
	    x = XNORM(sx);
	    off = XYOFFSET(x, y);
	    if ((x + y) & 1)
		putchar(' ');
	    else if (own[off] >= 0)
		putchar('-');
	    else if (xzone[off] >= 0)
		putchar(numletter[xzone[off] % 62]);
	    else {
		assert(own[off] == -1);
		putchar(xzone[off] == -1 ? '.' : '!');
	    }
	}
	putchar('\n');
    }
}

/*
 * Print a map to help visualize closest[].
 * This is for debugging.
 */
void
print_closest_map(void)
{
    int sx, sy, x, y, off;

    for (sy = -WORLD_Y / 2; sy < WORLD_Y / 2; sy++) {
	y = YNORM(sy);
	printf("%4d ", sy);
	for (sx = -WORLD_X / 2; sx < WORLD_X / 2; sx++) {
	    x = XNORM(sx);
	    off = XYOFFSET(x, y);
	    if ((x + y) & 1)
		putchar(' ');
	    else if (closest[off] == (natid)-1)
		putchar('.');
	    else if (!distance[off]) {
		assert(closest[off] == own[off]);
		putchar('-');
	    } else {
		putchar(numletter[closest[off] % 62]);
	    }
	}
	printf("\n");
    }
}

void
print_distance_map(void)
{
    int sx, sy, x, y, off;

    for (sy = -WORLD_Y / 2; sy < WORLD_Y / 2; sy++) {
	y = YNORM(sy);
	printf("%4d ", sy);
	for (sx = -WORLD_X / 2; sx < WORLD_X / 2; sx++) {
	    x = XNORM(sx);
	    off = XYOFFSET(x, y);
	    if ((x + y) & 1)
		putchar(' ');
	    else if (closest[off] == (natid)-1)
		putchar('.');
	    else if (!distance[off]) {
		assert(closest[off] == own[off]);
		putchar('-');
	    } else {
		putchar(numletter[distance[off] % 62]);
	    }
	}
	printf("\n");
    }
}


/***************************************************************************
  WRITE A SCRIPT FOR PLACING CAPITALS
****************************************************************************/
static int
write_newcap_script(void)
{
    int c;
    FILE *script = fopen(outfile, "w");

    if (!script) {
	fprintf(stderr, "%s: unable to write to %s (%s)\n",
		program_name, outfile, strerror(errno));
	return 0;
    }

    for (c = 0; c < nc; ++c) {
	fprintf(script, "add %d %d %d p\n", c + 1, c + 1, c + 1);
	fprintf(script, "newcap %d %d,%d\n", c + 1, capx[c], capy[c]);
    }
    fprintf(script, "add %d visitor visitor v\n", c + 1);
    fclose(script);
    return 1;
}

static void
qprint(const char *const fmt, ...)
{
    va_list ap;

    if (!quiet) {
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
    }
}
