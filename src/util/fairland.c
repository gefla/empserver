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
 * add one sector to each continent in turn, obeying the minimum
 * distance between continents, until they have the specified size.
 *
 * The kind of shape they grow into is determined by the "spike
 * percentage" --- the higher the spike, the more spindly they will
 * be.  If you lower the spike, the continents will be more round.
 *
 * If growing fails due to lack of room, start over.  If it fails too
 * many times, give up and terminate unsuccessfully.
 *
 * 3. Place and grow additional islands
 *
 * Place and grow islands one after the other.  Place the first sector
 * randomly, pick an island size, then grow the island to that size.
 *
 * Growing works as for continents, except the minimum distance for
 * additional islands applies, and growing simply stops when there is
 * no room.
 *
 * 4. Compute elevation
 *
 * Elevate islands one after the other.
 *
 * First, place the specified number of mountains randomly.
 * Probability increases with distance to sea.
 *
 * Last, elevate mountains and the capitals.  Pick coastal mountain
 * elevation randomly from an interval of medium elevations reserved
 * for them.  Pick non-coastal mountain elevation randomly from an
 * interval of high elevation reserved for them.  Set capital
 * elevation to a fixed, medium value.
 *
 * In between, elevate the remaining land one by one, working from
 * mountains towards the sea, and from the elevation just below the
 * non-coastal mountains' interval linearly down to 1, avoiding the
 * coastal mountains' interval.
 *
 * This gives islands of the same size the same set of elevations,
 * except for mountains.
 *
 * Elevate sea: pick a random depth from an interval that deepens with
 * the distance to land.
 *
 * 5. Set resources
 *
 * Sector resources are simple functions of elevation.  You can alter
 * macros OIL_MAX, IRON_MIN, GOLD_MIN, FERT_MAX, and URAN_MIN to
 * customize them.
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

/* The following five numbers refer to elevation under which (in the case of
   fertility or oil) or over which (in the case of iron, gold, and uranium)
   sectors with that elevation will contain that resource.  Elevation ranges
   from 0 to 100 */

/* raise FERT_MAX for more fertility */
#define FERT_MAX   56

/* raise OIL_MAX for more oil */
#define OIL_MAX	   33

/* lower IRON_MIN for more iron */
#define IRON_MIN   22

/* lower GOLD_MIN for more gold */
#define GOLD_MIN   36

/* lower URAN_MIN for more uranium */
#define URAN_MIN   56

/* do not change these 4 defines */
#define LANDMIN		1	/* plate altitude for normal land */
#define HILLMIN		34	/* plate altitude for hills */
#define PLATMIN		36	/* plate altitude for plateau */
#define HIGHMIN		98	/* plate altitude for mountains */

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
#define INFINITE_ELEVATION 999

/* these defines prevent infinite loops:
*/

#define COAST_SEARCH_MAX 200	/* how many times do we look for a coast sector
				   when growing continents and islands */
#define DRIFT_BEFORE_CHECK ((WORLD_X + WORLD_Y)/2)
#define DRIFT_MAX ((WORLD_X + WORLD_Y)*2)
#define MOUNTAIN_SEARCH_MAX 1000	/* how long do we try to place mountains */

/* handy macros:
*/

#define new_x(newx) (((newx) + WORLD_X) % WORLD_X)
#define new_y(newy) (((newy) + WORLD_Y) % WORLD_Y)

static int ctot;		/* total number of continents and islands grown */
static int *isecs;		/* array of how large each island is */

static int *capx, *capy;	/* location of the nc capitals */
static int dirx[] = { -2, -1, 1, 2, 1, -1 }; /* gyujnb */
static int diry[] = { 0, -1, -1, 0, 1, 1 };

static int **own;		/* owner of the sector.  -1 means water */

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

static int **elev;		/* elevation of the sectors */
static int **sectx, **secty;	/* the sectors for each continent */
static int **sectc;		/* which sectors are on the coast? */
static int *weight;		/* used for placing mountains */
static int *dsea, *dmoun;	/* the dist to the ocean and mountain */

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
static void elevate_land(void);
static void elevate_sea(void);
static void set_coastal_flags(void);

static void print_vars(void);
static void fl_move(int);
static void grow_islands(void);

/* Debugging aids: */
void print_own_map(void);
void print_xzone_map(void);
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
    } while (!done && ++try < NUMTRIES);
    if (!done) {
	fprintf(stderr, "%s: world not large enough to hold continents\n",
		program_name);
	exit(1);
    }
    qprint("growing islands:");
    grow_islands();
    qprint("\nelevating land...\n");
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
    own = calloc(WORLD_X, sizeof(int *));
    xzone = malloc(WORLD_SZ() * sizeof(*xzone));
    elev = calloc(WORLD_X, sizeof(int *));
    for (i = 0; i < WORLD_X; ++i) {
	own[i] = calloc(WORLD_Y, sizeof(int));
	elev[i] = calloc(WORLD_Y, sizeof(int));
    }
    sectx = calloc(nc + ni, sizeof(int *));
    secty = calloc(nc + ni, sizeof(int *));
    sectc = calloc(nc + ni, sizeof(int *));
    isecs = calloc(nc + ni, sizeof(int));
    weight = calloc(MAX(sc, is * 2), sizeof(int));
    dsea = calloc(MAX(sc, is * 2), sizeof(int));
    dmoun = calloc(MAX(sc, is * 2), sizeof(int));
    for (i = 0; i < nc; ++i) {
	sectx[i] = calloc(sc, sizeof(int));
	secty[i] = calloc(sc, sizeof(int));
	sectc[i] = calloc(sc, sizeof(int));
    }
    for (i = nc; i < nc + ni; ++i) {
	sectx[i] = calloc(is * 2, sizeof(int));
	secty[i] = calloc(is * 2, sizeof(int));
	sectc[i] = calloc(is * 2, sizeof(int));
    }

}

static void
init(void)
{
    int i, j;

    for (i = 0; i < WORLD_X; ++i) {
	for (j = 0; j < WORLD_Y; ++j) {
	    own[i][j] = -1;
	}
    }
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
    int i, n, newx, newy;

    for (i = roll0(6), n = 0; n < 6; i = (i + 1) % 6, ++n) {
	newx = new_x(capx[j] + dirx[i]);
	newy = new_y(capy[j] + diry[i]);
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

/* Look for a coastal sector of continent c
*/

static void
find_coast(int c)
{
    int i, j;

    for (i = 0; i < isecs[c]; ++i) {
	sectc[c][i] = 0;
	for (j = 0; j < 6; ++j)
	    if (own[new_x(sectx[c][i] + dirx[j])][new_y(secty[c][i] + diry[j])] == -1)
		sectc[c][i] = 1;
    }
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
 * Can island @c grow at @x,@y?
 */
static int
can_grow_at(int c, int x, int y)
{
    return own[x][y] == -1 && xzone_ok(c, x, y);
}

static int
try_to_grow(int c, int newx, int newy, int extra_dist)
{
    int d = c < nc ? di : id;
    int i, px, py;
    struct hexagon_iter hexit;

    if (!can_grow_at(c, newx, newy))
	return 0;

    for (i = 1; i <= extra_dist; i++) {
	hexagon_first(&hexit, newx, newy, d + i, &px, &py);
	do {
	    if (own[px][py] != -1 &&
		own[px][py] != c &&
		(DISTINCT_ISLANDS || own[px][py] < nc))
		return 0;
	} while (hexagon_next(&hexit, &px, &py));
    }

    xzone_around_sector(c, newx, newy,
			c < nc ? di : DISTINCT_ISLANDS ? id : 0);
    sectx[c][isecs[c]] = newx;
    secty[c][isecs[c]] = newy;
    isecs[c]++;
    own[newx][newy] = c;
    return 1;
}

/* Move along the coast in a clockwise direction.
*/

static void
next_coast(int c, int x, int y, int *xp, int *yp)
{
    int i, nx, ny, wat = 0;

    if (isecs[c] == 1) {
	*xp = x;
	*yp = y;
	return;
    }

    for (i = 0; i < 12; ++i) {
	nx = new_x(x + dirx[i % 6]);
	ny = new_y(y + diry[i % 6]);
	if (own[nx][ny] == -1)
	    wat = 1;
	if (wat && own[nx][ny] == c) {
	    *xp = nx;
	    *yp = ny;
	    return;
	}
    }
}

/* Choose a sector to grow from
*/

static int
new_try(int c, int spike)
{
    int secs = isecs[c];
    int i, starti;

    if (secs == 1) {
	if (sectc[c][0])
	    return 0;
    } else {
	i = starti = (spike && sectc[c][secs - 1]) ? secs - 1 : roll0(secs);
	do {
	    if (sectc[c][i])
		return i;
	    i = (i + 1) % secs;
	} while (i != starti);
	assert(c >= nc);
	return -1;
    }
    return -1;
}

/* Grow continent c by 1 sector
*/

static int
grow_one_sector(int c)
{
    int spike = roll0(100) < sp;
    int done, coast_search, try1, x, y, newx, newy, i, n, sx, sy;

    if ((try1 = new_try(c, spike)) == -1)
	return 0;
    x = sx = sectx[c][try1];
    y = sy = secty[c][try1];
    coast_search = 0;
    done = 0;
    do {
	if (spike) {
	    for (i = roll0(6), n = 0; n < 12 && !done; i = (i + 1) % 6, ++n) {
		newx = new_x(x + dirx[i]);
		newy = new_y(y + diry[i]);
		if (n > 5 ||
		    (own[new_x(x+dirx[(i+5)%6])][new_y(y+diry[(i+5)%6])] == -1 &&
		     own[new_x(x+dirx[(i+1)%6])][new_y(y+diry[(i+1)%6])] == -1))
		    if (try_to_grow(c, newx, newy, 0))
			done = 1;
	    }
	} else
	    for (i = roll0(6), n = 0; n < 6 && !done; i = (i + 1) % 6, ++n) {
		newx = new_x(x + dirx[i]);
		newy = new_y(y + diry[i]);
		if (try_to_grow(c, newx, newy, 0))
		    done = 1;
	    }
	next_coast(c, x, y, &x, &y);
	++coast_search;
    } while (!done && coast_search < COAST_SEARCH_MAX &&
	     (isecs[c] == 1 || x != sx || y != sy));
    return done;
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

    ctot = 0;
    xzone_init(0);

    for (c = 0; c < nc; ++c) {
	isecs[c] = 0;
	if (!try_to_grow(c, capx[c], capy[c], 0)
	    || !try_to_grow(c, new_x(capx[c] + 2), capy[c], 0)) {
	    done = 0;
	    continue;
	}
    }

    if (!done) {
	qprint("No room for continents\n");
	return 0;
    }

    for (secs = 2; secs < sc && done; secs++) {
	for (c = 0; c < nc; ++c) {
	    find_coast(c);
	    if (!grow_one_sector(c))
		done = 0;
	}
    }

    for (c = 0; c < nc; ++c)
	find_coast(c);

    if (!done)
	qprint("Only managed to grow %d out of %d sectors.\n",
	       secs - 1, sc);
    ctot = nc;
    return done;
}

/****************************************************************************
  GROW THE ISLANDS
****************************************************************************/

/* Choose a place to start growing an island from
*/
static int
place_island(int c, int *xp, int *yp)
{
    int d, sx, sy;
    int ssy = roll0(WORLD_Y);
    int ssx = new_x(roll0(WORLD_X / 2) * 2 + ssy % 2);

    if (ssx > WORLD_X - 2)
	ssx = new_x(ssx + 2);
    for (d = di; d >= 0; --d) {
	sx = ssx;
	sy = ssy;
	*xp = new_x(sx + 2);
	for (*yp = sy; *xp != sx || *yp != sy; *xp += 2) {
	    if (*xp >= WORLD_X) {
		*yp = new_y(*yp + 1);
		*xp = *yp % 2;
		if (*xp == sx && *yp == sy)
		    break;
	    }
	    if (try_to_grow(c, *xp, *yp, d))
		return 1;
	}
    }
    return 0;
}

/* Grow all the islands
*/

static void
grow_islands(void)
{
    int stunted_islands = 0;
    int c, secs, x, y, isiz;

    xzone_init(nc);

    for (c = nc; c < nc + ni; ++c) {
	if (!place_island(c, &x, &y)) {
	    qprint("\nNo room for island #%d", c - nc + 1);
	    break;
	}

	isiz = roll(is) + roll0(is);
	for (secs = 1; secs < isiz; secs++) {
	    find_coast(c);
	    if (!grow_one_sector(c)) {
		stunted_islands++;
		break;
	    }
	}

	find_coast(c);
	qprint(" %d(%d)", c - nc + 1, secs);
	ctot++;
    }

    if (stunted_islands)
	qprint("\n%d stunted island%s",
	       stunted_islands, splur(stunted_islands));
}

/****************************************************************************
  CREATE ELEVATIONS
****************************************************************************/
static void
create_elevations(void)
{
    int i, j;

    for (i = 0; i < WORLD_X; i++) {
	for (j = 0; j < WORLD_Y; j++)
	    elev[i][j] = -INFINITE_ELEVATION;
    }
    elevate_land();
    elevate_sea();
}

/* Generic function for finding the distance to the closest sea, land, or
   mountain
*/
static int
distance_to_what(int x, int y, int flag)
{
    int d, px, py;
    struct hexagon_iter hexit;

    for (d = 1; d < 5; ++d) {
	hexagon_first(&hexit, x, y, d, &px, &py);
	do {
	    switch (flag) {
	    case 0:		/* distance to sea */
		if (own[px][py] == -1)
		    return d;
		break;
	    case 1:		/* distance to land */
		if (own[px][py] != -1)
		    return d;
		break;
	    case 2:		/* distance to mountain */
		if (elev[px][py] == INFINITE_ELEVATION)
		    return d;
		break;
	    }
	} while (hexagon_next(&hexit, &px, &py));
    }
    return d;
}

#define ELEV elev[sectx[c][i]][secty[c][i]]
#define distance_to_sea() (sectc[c][i]?1:distance_to_what(sectx[c][i], secty[c][i], 0))
#define distance_to_mountain() distance_to_what(sectx[c][i], secty[c][i], 2)

/* Decide where the mountains go
*/
static void
elevate_land(void)
{
    int i, mountain_search, k, c, total, ns, nm, highest, where, h, newk,
	r, dk;

    for (c = 0; c < ctot; ++c) {
	total = 0;
	ns = isecs[c];
	nm = (pm * ns) / 100;

/* Place the mountains */

	for (i = 0; i < ns; ++i) {
	    dsea[i] = distance_to_sea();
	    weight[i] = (total += (dsea[i] * dsea[i]));
	}

	for (k = nm, mountain_search = 0;
	     k && mountain_search < MOUNTAIN_SEARCH_MAX;
	     ++mountain_search) {
	    r = roll0(total);
	    for (i = 0; i < ns; ++i)
		if (r < weight[i] && ELEV == -INFINITE_ELEVATION &&
		    (c >= nc ||
		     ((!(capx[c] == sectx[c][i] &&
			 capy[c] == secty[c][i])) &&
		      (!(new_x(capx[c] + 2) == sectx[c][i] &&
			 capy[c] == secty[c][i]))))) {
		    ELEV = INFINITE_ELEVATION;
		    break;
		}
	    --k;
	}

/* Elevate land that is not mountain and not capital */

	for (i = 0; i < ns; ++i)
	    dmoun[i] = distance_to_mountain();
	dk = (ns - nm - ((c < nc) ? 3 : 1) > 0) ?
	  (100 * (HIGHMIN - LANDMIN)) / (ns - nm - ((c < nc) ? 3 : 1)) :
	  100 * INFINITE_ELEVATION;
	for (k = 100 * (HIGHMIN - 1);; k -= dk) {
	    highest = 0;
	    where = -1;
	    for (i = 0; i < ns; ++i) {
		if (ELEV == -INFINITE_ELEVATION &&
		    (c >= nc || ((!(capx[c] == sectx[c][i] &&
				    capy[c] == secty[c][i])) &&
				 (!(new_x(capx[c] + 2) == sectx[c][i] &&
				    capy[c] == secty[c][i]))))) {
		    h = 3 * (5 - dmoun[i]) + dsea[i];
		    assert(h > 0);
		    if (h > highest) {
			highest = h;
			where = i;
		    }
		}
	    }
	    if (where == -1)
		break;
	    newk = k / 100;
	    if (newk >= HILLMIN && newk < PLATMIN)
		newk = PLATMIN;
	    if (newk < LANDMIN)
		newk = LANDMIN;
	    elev[sectx[c][where]][secty[c][where]] = newk;
	}

/* Elevate the mountains and capitals */

	for (i = 0; i < ns; ++i) {
	    if (ELEV == INFINITE_ELEVATION) {
		if (dsea[i] == 1)
		    ELEV = HILLMIN + roll0(PLATMIN - HILLMIN);
		else
		    ELEV = HIGHMIN + roll0((256 - HIGHMIN) / 2) +
		      roll0((256 - HIGHMIN) / 2);
	    } else if (c < nc &&
		       (((capx[c] == sectx[c][i] && capy[c] == secty[c][i])) ||
			((new_x(capx[c] + 2) == sectx[c][i] &&
			  capy[c] == secty[c][i]))))
		ELEV = PLATMIN;
	}
    }
}

#define distance_to_land() distance_to_what(x, y, 1)

static void
elevate_sea(void)
{
    int x, y;

    for (y = 0; y < WORLD_Y; ++y) {
	for (x = y % 2; x < WORLD_X; x += 2) {
	    if (elev[x][y] == -INFINITE_ELEVATION)
		elev[x][y] = -roll(distance_to_land() * 20 + 27);
	}
    }
}

static int
elev_to_sct_type(int elevation)
{
    if (elevation < LANDMIN)
	return SCT_WATER;
    if (elevation < HILLMIN)
	return SCT_RURAL;
    if (elevation < PLATMIN)
	return SCT_MOUNT;
    if (elevation < HIGHMIN)
	return SCT_RURAL;
    return SCT_MOUNT;
}

/****************************************************************************
  ADD THE RESOURCES
****************************************************************************/

static int
set_fert(int e)
{
    int fert = 0;
    if (e < LANDMIN)
	fert = LANDMIN - e + 40;
    else if (e < FERT_MAX)
	fert = (120 * (FERT_MAX - e)) / (FERT_MAX - LANDMIN);
    if (fert > 100)
	fert = 100;
    return fert;
}

static int
set_oil(int e)
{
    int oil = 0;
    if (e < LANDMIN)
	oil = (LANDMIN - e) * 2 + roll0(2);
    else if (e <= OIL_MAX)
	oil = (120 * (OIL_MAX - e + 1)) / (OIL_MAX - LANDMIN + 1);
    if (oil > 100)
	oil = 100;
    return oil;
}

static int
set_iron(int e)
{
    int iron = 0;
    if (e >= IRON_MIN && e < HIGHMIN)
	iron = (120 * (e - IRON_MIN + 1)) / (HIGHMIN - IRON_MIN);
    if (iron > 100)
	iron = 100;
    return iron;
}

static int
set_gold(int e)
{
    int gold = 0;
    if (e >= GOLD_MIN) {
	if (e < HIGHMIN)
	    gold = (80 * (e - GOLD_MIN + 1)) / (HIGHMIN - GOLD_MIN);
	else
	    gold = 100 - 20 * HIGHMIN / e;
    }
    if (gold > 100)
	gold = 100;
    return gold;
}

static int
set_uran(int e)
{
    int uran = 0;
    if (e >= URAN_MIN && e < HIGHMIN)
	uran = (120 * (e - URAN_MIN + 1)) / (HIGHMIN - URAN_MIN);
    if (uran > 100)
	uran = 100;
    return uran;
}

static void
add_resources(struct sctstr *sct)
{
    sct->sct_fertil = set_fert(sct->sct_elev);
    sct->sct_oil = set_oil(sct->sct_elev);
    sct->sct_min = set_iron(sct->sct_elev);
    sct->sct_gmin = set_gold(sct->sct_elev);
    sct->sct_uran = set_uran(sct->sct_elev);
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
	    sct->sct_elev = elev[x][y];
	    sct->sct_type = elev_to_sct_type(elev[x][y]);
	    sct->sct_newtype = sct->sct_type;
	    sct->sct_dterr = own[sct->sct_x][y] + 1;
	    add_resources(sct);
	}
    }
    set_coastal_flags();
}

/****************************************************************************
  PRINT A PICTURE OF THE MAP TO YOUR SCREEN
****************************************************************************/
static void
output(void)
{
    int sx, sy, x, y, c, type;

    if (quiet == 0) {
	for (sy = -WORLD_Y / 2; sy < WORLD_Y / 2; sy++) {
	    y = YNORM(sy);
	    puts("");
	    if (y % 2)
		printf(" ");
	    for (sx = -WORLD_X / 2 + y % 2; sx < WORLD_X / 2; sx += 2) {
		x = XNORM(sx);
		c = own[x][y];
		type = elev_to_sct_type(elev[x][y]);
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
 * Print a map to help visualize own[][].
 * This is for debugging.
 */
void
print_own_map(void)
{
    int sx, sy, x, y;

    for (sy = -WORLD_Y / 2; sy < WORLD_Y / 2; sy++) {
	y = YNORM(sy);
	printf("%4d ", sy);
	for (sx = -WORLD_X / 2; sx < WORLD_X / 2; sx++) {
	    x = XNORM(sx);
	    if ((x + y) & 1)
		putchar(' ');
	    else if (own[x][y] == -1)
		putchar('.');
	    else
		putchar(numletter[own[x][y] % 62]);
	}
	putchar('\n');
    }
}

/*
 * Print a map to help visualize elev[][].
 * This is for debugging.  It expects the terminal to understand
 * 24-bit color escape sequences \e[48;2;$red;$green;$blue;m.
 */
void
print_elev_map(void)
{
    int sx, sy, x, y, sat;

    for (sy = -WORLD_Y / 2; sy < WORLD_Y / 2; sy++) {
	y = YNORM(sy);
	printf("%4d ", sy);
	for (sx = -WORLD_X / 2; sx < WORLD_X / 2; sx++) {
	    x = XNORM(sx);
	    if ((x + y) & 1)
		putchar(' ');
	    else if (!elev[x][y])
		putchar(' ');
	    else if (elev[x][y] < 0) {
		sat = 256 + elev[x][y] * 2;
		printf("\033[48;2;%d;%d;%dm \033[0m", sat, sat, 255);
	    } else if (elev[x][y] < HIGHMIN / 2) {
		sat = (HIGHMIN / 2 - elev[x][y]) * 4;
		printf("\033[48;2;%d;%d;%dm \033[0m", sat, 255, sat);
	    } else if (elev[x][y] < HIGHMIN) {
		sat = 128 + (HIGHMIN - elev[x][y]) * 2;
		printf("\033[48;2;%d;%d;%dm \033[0m", sat, sat / 2, sat / 4);
	    } else {
		sat = 128 + (elev[x][y] - HIGHMIN) * 4 / 5;
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
	    else if (own[x][y] >= 0)
		putchar('-');
	    else if (xzone[off] >= 0)
		putchar(numletter[xzone[off] % 62]);
	    else {
		assert(own[x][y] == -1);
		putchar(xzone[off] == -1 ? '.' : '!');
	    }
	}
	putchar('\n');
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

static void
set_coastal_flags(void)
{
    int i, j;
    struct sctstr *sp;

    for (i = 0; i < nc + ni; ++i) {
	for (j = 0; j < isecs[i]; j++) {
	    sp = getsectp(sectx[i][j], secty[i][j]);
	    sp->sct_coastal = sectc[i][j];
	}
    }
}
