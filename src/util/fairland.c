/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fairland.c: Create a nice, new world
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include <config.h>

/* define ORE 1 to add resources, define ORE 0 if you want to use another
   program to add the resources */
static int ORE = 1;
static int quiet = 0;

/* If you don't specify these command line arguments, then these are the
   defaults */
#define DEFAULT_SPIKE 10
#define DEFAULT_MOUNTAIN 0
#define DEFAULT_CONTDIST 2
#define DEFAULT_ISLDIST 1

/* The following five numbers refer to elevation under which (in the case of
   fertility or oil) or over which (in the case of iron, gold, and uranium)
   sectors with that elevation will contain that resource.  Elevation ranges
   from 0 to 100 */

/* raise FERT_MAX for more fertility */
#define FERT_MAX   56

/* raise OIL_MAX for more oil */
#define OIL_MAX    33

/* lower IRON_MIN for more iron */
#define IRON_MIN   22

/* lower GOLD_MIN for more gold */
#define GOLD_MIN   36

/* lower URAN_MIN for more uranium */
#define URAN_MIN   56

#if defined(_WIN32)
#include "../lib/gen/getopt.h"
#else
#include <unistd.h>
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include "file.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "power.h"
#include "prototypes.h"
#include "sect.h"
#include "version.h"
#include "xy.h"

/* do not change these 4 defines */
#define LANDMIN		1	/* plate altitude for normal land */
#define HILLMIN		34	/* plate altitude for hills */
#define PLATMIN		36	/* plate altitude for plateau */
#define HIGHMIN		98	/* plate altitude for mountains */

static void qprint(const char * const fmt, ...)
    ATTRIBUTE((format (printf, 1, 2)));

#define DEFAULT_OUTFILE_NAME "newcap_script"
static const char *outfile = DEFAULT_OUTFILE_NAME;
/* mark the continents with a * so you can tell them
   from the islands 1 = mark, 0 = don't mark. */
static int AIRPORT_MARKER = 0;

/* don't let the islands crash into each other.
   1 = don't merge, 0 = merge. */
static int DISTINCT_ISLANDS = 1;

static char *program_name;

#define XSIZE           ((WORLD_X) / 2)	/* basically world x-y size */
#define YSIZE           (WORLD_Y)
#define STABLE_CYCLE 4		/* stability required for perterbed capitals */
#define INFINITY        999	/* a number which means "BIG" */

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
#define rnd(x) (random() % (x))

int secs;			/* number of sectors grown */
int ctot;			/* total number of continents and islands grown */
int *isecs;			/* array of how large each island is */

int nc, sc, di, sp, pm, ni, is, id;	/* the 8 arguments to this program */
unsigned long rnd_seed;		/* optional seed can be passed as an argument */
int *capx, *capy;		/* location of the nc capitals */
int *mc, mcc;			/* array and counter used for stability
				   check when perturbing */
int spike;			/* are we spiking? */
int mind;			/* the final distance between capitals that
				   we achieved */
int dirx[] = { -2, -1, 1, 2, 1, -1 };	/* gyujnb */
int diry[] = { 0, -1, -1, 0, 1, 1 };

int **own;			/* owner of the sector.  -1 means water */
int **elev;			/* elevation of the sectors */
int **sectx, **secty;		/* the sectors for each continent */
int **sectc;			/* which sectors are on the coast? */
int *vector;			/* used for measuring distances */
int *weight;			/* used for placing mountains */
int *dsea, *dmoun;		/* the dist to the ocean and mountain */
FILE *sect_fptr;			/* the file we write everything to */
struct sctstr **sects;
struct sctstr *sectsbuf;
int fl_status;			/* is anything wrong? */
#define STATUS_NO_ROOM 1	/* there was no room to grow */
#define NUMTRIES 10		/* keep trying to grow this many times */

const char *numletter =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static void help(char *);
static void usage(void);
static void parse_args(int argc, char *argv[]);
static int allocate_memory(void);
static void init(void);
static int drift(void);
static void grow_continents(void);
static void create_elevations(void);
static void write_sects(void);
static int write_file(void);
static void output(void);
static int write_newcap_script(void);
static int stable(void);
static void elevate_land(void);
static void elevate_sea(void);
static int map_symbol(int x, int y);
static void fl_sct_init(coord, coord, struct sctstr *, time_t timestamp);
static void set_coastal_flags(void);

static void print_vars(void);
static void fl_move(int);
static void next_coast(int c, int x, int y, int *xp, int *yp);
static void grow_islands(void);

/****************************************************************************
  MAIN
****************************************************************************/

int
main(int argc, char *argv[])
{
    int opt;
    char *config_file = NULL;
    int i = 0;

    program_name = argv[0];
    rnd_seed = time(NULL);

    while ((opt = getopt(argc, argv, "ae:hioqR:s:v")) != EOF) {
	switch (opt) {
	case 'a':
	    AIRPORT_MARKER = 1;
	    break;
	case 'e':
	    config_file = optarg;
	    break;
	case 'i':
	    DISTINCT_ISLANDS = 0;
	    break;
	case 'o':
	    ORE = 0;
	    break;
	case 'q':
	    quiet = 1;
	    break;
	case 'R':
	    rnd_seed = strtoul(optarg, NULL, 10);
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
    srandom(rnd_seed);
    if (emp_config(config_file))
	exit(1);

    parse_args(argc - optind, argv + optind);
    if (allocate_memory() == -1)
	exit(-1);
    print_vars();

    do {
	init();
	if (i)
	    qprint("\ntry #%d (out of %d)...", i + 1, NUMTRIES);
	qprint("\n\n        #*# ...fairland rips open a rift in the datumplane... #*#\n\n");
	qprint("seed is %lu\n", rnd_seed);
	qprint("placing capitals...\n");
	if (!drift())
	    qprint("fairland: unstable drift -- try increasisg DRIFT_MAX\n");
	qprint("growing continents...\n");
	grow_continents();
    } while (fl_status && ++i < NUMTRIES);
    if (fl_status) {
	fputs("ERROR: World not large enough to hold continents\n",
	      stderr);
	exit(1);
    }
    qprint("growing islands:");
    grow_islands();
    qprint("\nelevating land...\n");
    create_elevations();
    qprint("designating sectors...\n");
    if (ORE)
	qprint("adding resources...\n");
    write_sects();
    qprint("writing to sectors file...\n");
    if (write_file() == -1)
	exit(-1);
    output();
    write_newcap_script();
    if (!ORE)
	qprint("\t*** Resources have not been added ***\n");
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

static int
my_sqrt(int n)
{
    int i;

    for (i = 1; i * i < n * 10000; ++i) ;
    return (i + 50) / 100;
}

/****************************************************************************
  PARSE COMMAND LINE ARGUMENTS
****************************************************************************/

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
	   "  -a              airport marker for continents\n"
	   "  -e CONFIG-FILE  configuration file\n"
	   "                  (default %s)\n"
	   "  -h              display this help and exit\n"
	   "  -i              islands may merge\n"
	   "  -o              don't set resources\n"
	   "  -q              quiet\n"
	   "  -R SEED         seed for random number generator\n"
	   "  -s SCRIPT       name of script to create (default %s)\n"
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
	puts("fairland: error -- number of continents must be > 0");
	exit(1);
    }

    sc = atoi(argv[1]);
    if (sc < 1) {
	puts("fairland: error -- size of continents must be > 0");
	exit(1);
    }

    if (argc > 2)
	ni = atoi(argv[2]);
    else
	ni = nc;

    if (argc > 3)
	is = atoi(argv[3]);
    else
	is = sc / 2;
    if (is < 0)
	is = 0;

    if (argc > 4)
	sp = atoi(argv[4]);
    else
	sp = DEFAULT_SPIKE;
    if (sp < 0)
	sp = 0;
    if (sp > 100)
	sp = 100;

    if (argc > 5)
	pm = atoi(argv[5]);
    else
	pm = DEFAULT_MOUNTAIN;
    if (pm < 0)
	pm = 0;

    if (argc > 6)
	di = atoi(argv[6]);
    else
	di = DEFAULT_CONTDIST;

    if (di < 0) {
	puts("fairland: error -- distance between continents must be >= 0");
	exit(1);
    }
    if (di > WORLD_X / 2 || di > WORLD_Y / 2) {
	puts("fairland: error -- distance between continents too large");
	exit(1);
    }

    if (argc > 7)
	id = atoi(argv[7]);
    else
	id = DEFAULT_ISLDIST;
    if (id < 0) {
	puts("fairland: error -- distance from islands to continents must be >= 0");
	exit(1);
    }
    if (id > WORLD_X || id > WORLD_Y) {
	puts("fairland: error -- distance from islands to continents too large");
	exit(1);
    }
    if (nc * sc + nc * my_sqrt(sc) * 2 * (di + 1) > WORLD_X * WORLD_Y) {
	puts("fairland: error -- world not big enough to fit continents.");
	puts("arguments must satisfy:");
	puts("nc*sc*sc + nc*sqrt(sc)*2*(di+1) < WORLD_X * WORLD_Y");
	exit(1);
    }
}

/****************************************************************************
  VARIABLE INITIALIZATION
****************************************************************************/

static int
allocate_memory(void)
{
    int i;
    char *fname;

    fname = malloc(strlen(gamedir) + 1 + strlen(empfile[EF_SECTOR].file) + 1);
    sprintf(fname, "%s/%s", gamedir, empfile[EF_SECTOR].file);
    sect_fptr = fopen(fname, "wb");
    if (sect_fptr == NULL) {
	perror(fname);
	return -1;
    }
    free(fname);
    sectsbuf = calloc((YSIZE * XSIZE), sizeof(struct sctstr));
    sects = calloc(YSIZE, sizeof(struct sctstr *));
    for (i = 0; i < YSIZE; i++)
	sects[i] = &sectsbuf[XSIZE * i];
    capx = calloc(nc, sizeof(int));
    capy = calloc(nc, sizeof(int));
    vector = calloc(WORLD_X + WORLD_Y, sizeof(int));
    mc = calloc(STABLE_CYCLE, sizeof(int));
    own = calloc(WORLD_X, sizeof(int *));
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

    return 0;
}

static void
init(void)
{
    int i, j, xx = 0, yy = 0;

    mcc = 0;
    fl_status = 0;

    for (i = 0; i < WORLD_X; ++i) {
	for (j = 0; j < WORLD_Y; ++j) {
	    own[i][j] = -1;
	    elev[i][j] = -INFINITY;
	}
    }

    for (i = 0; i < nc; ++i, xx += 2) {
	if (xx >= WORLD_X) {
	    ++yy;
	    xx = yy % 2;
	    if (yy == WORLD_Y) {
		puts("fairland error: world not big enough for all the continents.\n");
		exit(1);
	    }
	}
	capx[i] = xx;
	capy[i] = yy;
    }
    for (i = 0; i < STABLE_CYCLE; ++i)
	mc[i] = i;
}

/****************************************************************************
  DRIFT THE CAPITALS UNTIL THEY ARE AS FAR AWAY FROM EACH OTHER AS POSSIBLE
****************************************************************************/

/* How isolated is capital j?
*/
static int
iso(int j, int newx, int newy)
{
    int i, md, d = WORLD_X + WORLD_Y;

    for (i = 0; i < nc; ++i) {
	if (i == j)
	    continue;
	md = mapdist(capx[i], capy[i], newx, newy);
	if (md < d)
	    d = md;
    }

    return d;
}

/* Drift all the capitals
*/
static int
drift(void)
{
    int i, turns;

    for (turns = 0; turns < DRIFT_MAX; ++turns) {
	if (turns > DRIFT_BEFORE_CHECK && (mind = stable()))
	    return 1;
	for (i = 0; i < nc; ++i)
	    fl_move(i);
    }
    return 0;
}

/* Check to see if we have stabilized--can we stop drifting the capitals?
*/

static int
stable(void)
{
    int i, isod, d = 0, stab = 1;

    for (i = 0; i < nc; ++i) {
	isod = iso(i, capx[i], capy[i]);
	if (isod > d)
	    d = isod;
    }
    for (i = 0; i < STABLE_CYCLE; ++i)
	if (d != mc[i])
	    stab = 0;
    mc[mcc] = d;
    mcc = (mcc + 1) % STABLE_CYCLE;
    return stab ? d : 0;
}

/* This routine does the actual drifting
*/

static void
fl_move(int j)
{
    int i, n, newx, newy;

    for (i = rnd(6), n = 0; n < 6; i = (i + 1) % 6, ++n) {
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

    for (i = 0; i < secs; ++i) {
	sectc[c][i] = 0;
	for (j = 0; j < 6; ++j)
	    if (own[new_x(sectx[c][i] + dirx[j])][new_y(secty[c][i] + diry[j])] == -1)
		sectc[c][i] = 1;
    }
}

/* Used for measuring distances
*/
static int
next_vector(int n)
{
    int i;

    if (n == 1) {
	vector[0] += 1;
	vector[0] %= 6;
	return vector[0];
    }
    for (i = 1; i < n && vector[i] == vector[i - 1]; ++i) ;
    vector[i - 1] += 1;
    vector[i - 1] %= 6;
    return i > 1 || vector[0] > 0;
}

/* Test to see if we're allowed to grow there: the arguments di and id
*/
static int
try_to_grow(int c, int newx, int newy, int d)
{
    int i, j, px, py;

    for (i = 1; i <= d; ++i) {
	for (j = 0; j < i; ++j)
	    vector[j] = 0;
	do {
	    px = newx;
	    py = newy;
	    for (j = 0; j < i; ++j) {
		px = new_x(px + dirx[vector[j]]);
		py = new_y(py + diry[vector[j]]);
	    }
	    if (own[px][py] != -1 &&
		own[px][py] != c &&
		(DISTINCT_ISLANDS || own[px][py] < nc))
		return 0;
	} while (next_vector(i));
    }
    sectx[c][secs] = newx;
    secty[c][secs] = newy;
    own[newx][newy] = c;
    return 1;
}

/* Move along the coast in a clockwise direction.
*/

static void
next_coast(int c, int x, int y, int *xp, int *yp)
{
    int i, nx, ny, wat = 0;

    if (secs == 1) {
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
new_try(int c)
{
    int i, starti;

    if (secs == 1) {
	if (sectc[c][0])
	    return 0;
    } else {
	i = starti = (spike && sectc[c][secs - 1]) ? secs - 1 : rnd(secs);
	do {
	    if (sectc[c][i])
		return i;
	    i = (i + 1) % secs;
	} while (i != starti);
	if (c < nc) {
	    printf("fairland: BUG -- couldn't find coast for continent %c, sector %d.\nPlease mail stevens@math.utoronto.ca.\n",
		   c + 'a', secs);
	    exit(1);
	} else
	    return -1;
    }
    return -1;
}

/* Grow continent c by 1 sector
*/

static int
grow_one_sector(int c)
{
    int done, coast_search, try1, x, y, newx, newy, i, n, sx, sy;

    spike = rnd(100) < sp;
    if ((try1 = new_try(c)) == -1)
	return 0;
    x = sx = sectx[c][try1];
    y = sy = secty[c][try1];
    coast_search = 0;
    done = 0;
    do {
	if (spike) {
	    for (i = rnd(6), n = 0; n < 12 && !done; i = (i + 1) % 6, ++n) {
		newx = new_x(x + dirx[i]);
		newy = new_y(y + diry[i]);
		if (own[newx][newy] == -1 &&
		    (n > 5 ||
		     (own[new_x(x+dirx[(i+5)%6])][new_y(y+diry[(i+5)%6])] == -1 &&
		      own[new_x(x+dirx[(i+1)%6])][new_y(y+diry[(i+1)%6])] == -1)))
		    if (try_to_grow(c, newx, newy, c < nc ? di : id))
			done = 1;
	    }
	} else
	    for (i = rnd(6), n = 0; n < 6 && !done; i = (i + 1) % 6, ++n) {
		newx = new_x(x + dirx[i]);
		newy = new_y(y + diry[i]);
		if (own[newx][newy] == -1)
		    if (try_to_grow(c, newx, newy, c < nc ? di : id))
			done = 1;
	    }
	next_coast(c, x, y, &x, &y);
	++coast_search;
    } while (!done && coast_search < COAST_SEARCH_MAX &&
	     (secs == 1 || x != sx || y != sy));
    if (!done && c < nc) {
	qprint("fairland: error -- continent %c had no room to grow!\n",
	       numletter[c % 62]);
	fl_status |= STATUS_NO_ROOM;
    }
    return done;
}

/* Grow all the continents
*/
static void
grow_continents(void)
{
    int c;

    for (c = 0; c < nc; ++c) {
	sectx[c][0] = capx[c];
	secty[c][0] = capy[c];
	own[sectx[c][0]][secty[c][0]] = c;
	sectx[c][1] = new_x(capx[c] + 2);
	secty[c][1] = capy[c];
	own[sectx[c][1]][secty[c][1]] = c;
    }

    for (secs = 2; secs < sc && !fl_status; ++secs) {
	for (c = 0; c < nc; ++c) {
	    find_coast(c);
	    grow_one_sector(c);
	}
    }
    for (c = 0; c < nc; ++c)
	find_coast(c);

    if (fl_status)
	qprint("Only managed to grow %d out of %d sectors.\n", secs, sc);
    ctot = nc;
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
    int ssy = rnd(WORLD_Y);
    int ssx = new_x(rnd(WORLD_X / 2) * 2 + ssy % 2);

    if (ssx > WORLD_X - 2)
	ssx = new_x(ssx + 2);
    for (d = di + id; d >= id; --d) {
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
	    if (own[*xp][*yp] == -1 && try_to_grow(c, *xp, *yp, d))
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
    int c, x, y, isiz;

    for (c = nc; c < nc + ni; ++c) {
	secs = 0;
	if (!place_island(c, &x, &y))
	    return;
	isiz = 1 + rnd(2 * is - 1);
	do {
	    ++secs;
	    find_coast(c);
	} while (secs < isiz && grow_one_sector(c));
	find_coast(c);
	qprint(" %d(%d)", c - nc + 1, secs);
	isecs[c] = secs;
	ctot = c;
    }
}

/****************************************************************************
  CREATE ELEVATIONS
****************************************************************************/
static void
create_elevations(void)
{
    elevate_land();
    elevate_sea();
}

/* Generic function for finding the distance to the closest sea, land, or
   mountain
*/
static int
distance_to_what(int x, int y, int flag)
{
    int j, d, px, py;

    for (d = 1; d < 5; ++d) {
	for (j = 0; j < d; ++j)
	    vector[j] = 0;
	do {
	    px = x;
	    py = y;
	    for (j = 0; j < d; ++j) {
		px = new_x(px + dirx[vector[j]]);
		py = new_y(py + diry[vector[j]]);
	    }
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
		if (elev[px][py] == INFINITY)
		    return d;
		break;
	    }
	} while (next_vector(d));
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
	ns = (c < nc) ? sc : isecs[c];
	nm = (pm * ns) / 100;

/* Place the mountains */

	for (i = 0; i < ns; ++i) {
	    dsea[i] = distance_to_sea();
	    weight[i] = (total += (dsea[i] * dsea[i]));
	}

	for (k = nm, mountain_search = 0;
	     k && mountain_search < MOUNTAIN_SEARCH_MAX;
	     ++mountain_search) {
	    r = rnd(total);
	    for (i = 0; i < ns; ++i)
		if (r < weight[i] && ELEV == -INFINITY &&
		    (c >= nc ||
		     ((!(capx[c] == sectx[c][i] &&
			 capy[c] == secty[c][i])) &&
		      (!(new_x(capx[c] + 2) == sectx[c][i] &&
			 capy[c] == secty[c][i]))))) {
		    ELEV = INFINITY;
		    break;
		}
	    --k;
	}

/* Elevate land that is not mountain and not capital */

	for (i = 0; i < ns; ++i)
	    dmoun[i] = distance_to_mountain();
	dk = (ns - nm - ((c < nc) ? 3 : 1) > 0) ?
	  (100 * (HIGHMIN - LANDMIN)) / (ns - nm - ((c < nc) ? 3 : 1)) :
	  100 * INFINITY;
	for (k = 100 * (HIGHMIN - 1);; k -= dk) {
	    highest = -INFINITY;
	    where = -1;
	    for (i = 0; i < ns; ++i) {
		if (ELEV != INFINITY &&
		    (c >= nc || ((!(capx[c] == sectx[c][i] &&
				    capy[c] == secty[c][i])) &&
				 (!(new_x(capx[c] + 2) == sectx[c][i] &&
				    capy[c] == secty[c][i]))))) {
		    h = 3 * (5 - dmoun[i]) + dsea[i];
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
	    dsea[where] = -INFINITY;
	    dmoun[where] = INFINITY;
	}

/* Elevate the mountains and capitals */

	for (i = 0; i < ns; ++i) {
	    if (ELEV == INFINITY) {
		if (dsea[i] == 1)
		    ELEV = HILLMIN + rnd(PLATMIN - HILLMIN);
		else
		    ELEV = HIGHMIN + rnd((256 - HIGHMIN) / 2) +
		      rnd((256 - HIGHMIN) / 2);
	    } else if ((c < nc &&
			((capx[c] == sectx[c][i] && capy[c] == secty[c][i]))) ||
		       ((new_x(capx[c] + 2) == sectx[c][i] &&
			 capy[c] == secty[c][i])))
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
	    if (elev[x][y] == -INFINITY)
		elev[x][y] = -rnd((distance_to_land() * 20 + 27)) - 1;
	}
    }
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
	fert = (140 * (FERT_MAX - e)) / (FERT_MAX - LANDMIN);
    if (fert > 120)
	fert = 120;
    return fert;
}

static int
set_oil(int e)
{
    int oil = 0;
    if (e < LANDMIN)
	oil = (LANDMIN - e) * 2 + rnd(2);
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
    int c, x, y, total;
    time_t current_time = time(NULL);

    /*  sct = &sects[0][0]; */
    sct = sectsbuf;
    for (y = 0; y < YSIZE; y++) {
	for (x = 0; x < XSIZE; x++, sct++) {
	    fl_sct_init(x * 2 + (y & 1), y, sct, current_time);
	    total = elev[sct->sct_x][y];
	    if (total < LANDMIN) {
		sct->sct_type = SCT_WATER;
	    } else if (total < HILLMIN)
		sct->sct_type = SCT_RURAL;
	    else if (total < PLATMIN)
		sct->sct_type = SCT_MOUNT;
	    else if (total < HIGHMIN)
		sct->sct_type = SCT_RURAL;
	    else
		sct->sct_type = SCT_MOUNT;
	    sct->sct_elev = total;
	    sct->sct_newtype = sct->sct_type;
	    if (ORE)
		add_resources(sct);
	}
    }
    if (AIRPORT_MARKER)
	for (c = 0; c < nc; ++c) {
	    sects[capy[c]][capx[c] / 2 + capy[c] % 2].sct_type = SCT_AIRPT;
	    sects[capy[c]][capx[c] / 2 + capy[c] % 2].sct_newtype = SCT_AIRPT;
	}
    set_coastal_flags();
}

/****************************************************************************
  WRITE ALL THIS STUFF TO THE FILE
****************************************************************************/
static int
write_file(void)
{
    int n;

    n = fwrite(sectsbuf, sizeof(struct sctstr), YSIZE * XSIZE, sect_fptr);
    if (n <= 0) {
	perror(empfile[EF_SECTOR].file);
	return -1;
    }
    if (n != YSIZE * XSIZE) {
	printf("%s:partial write\n", empfile[EF_SECTOR].file);
	return -1;
    }
    fclose(sect_fptr);
    return 0;
}

/****************************************************************************
  PRINT A PICTURE OF THE MAP TO YOUR SCREEN
****************************************************************************/
static void
output(void)
{
    int i, j;
    if (quiet == 0) {
	for (i = 0; i < WORLD_Y; ++i) {
	    puts("");
	    if (i % 2)
		printf(" ");
	    for (j = i % 2; j < WORLD_X; j += 2) {
		if (own[j][i] == -1)
		    printf(". ");
		else {
		    printf("%c ", map_symbol(j, i));
		}
	    }
	}
    }
    if (AIRPORT_MARKER)
	printf("\n\nEach continent is marked by a \"*\" on the map (to distinguish them from\nthe islands).  You can redesignate these airfields to wilderness sectors\none at a time, each time you add a new country to the game.\n");
}

static int
map_symbol(int x, int y)
{
    int c, iscap = 0;

    for (c = 0; c < nc; ++c)
	if ((x == capx[c] && y == capy[c])
	    || (x == new_x(capx[c] + 2) && y == capy[c]))
	    iscap = 1;
    if ((elev[x][y] >= HILLMIN && elev[x][y] < PLATMIN)
	|| elev[x][y] >= HIGHMIN)
	return '^';
    return own[x][y] >= nc ? '%' : iscap ? '#' : numletter[own[x][y] % 62];
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
	printf("fairland: error, unable to write to %s.\n", outfile);
	return -1;
    }

    for (c = 0; c < nc; ++c) {
	fprintf(script, "add %d %d %d n i\n", c + 1, c + 1, c + 1);
	if (AIRPORT_MARKER)
	    fprintf(script, "des %d,%d -\n", capx[c], capy[c]);
	fprintf(script, "newcap %d %d,%d\n", c + 1, capx[c], capy[c]);
    }
    fprintf(script, "add %d visitor visitor v i\n", c + 1);
    ++c;
    fclose(script);
    qprint("\n\nA script for adding all the countries can be found in \"%s\".\n",
	   outfile);
    return 0;
}

static void
qprint(const char * const fmt, ...)
{
    va_list ap;

    if (!quiet) {
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
    }
}

static void
fl_sct_init(coord x, coord y, struct sctstr *sp, time_t timestamp)
{
    sp->ef_type = EF_SECTOR;
    sp->sct_x = x;
    sp->sct_y = y;
    sp->sct_dist_x = x;
    sp->sct_dist_y = y;
    sp->sct_road = 0;
    sp->sct_rail = 0;
    sp->sct_defense = 0;
    sp->sct_timestamp = timestamp;
    sp->sct_coastal = 1;
}

static void
set_coastal_flags(void)
{
    int i, j;

    qprint("setting coastal flags...\n");
    for (i = 0; i < nc; ++i)
	for (j = 0; j < sc; j++)
	    sects[secty[i][j]][sectx[i][j] / 2].sct_coastal = sectc[i][j];
    for (i = nc; i < nc + ni; ++i)
	for (j = 0; j < isecs[i]; j++)
	    sects[secty[i][j]][sectx[i][j] / 2].sct_coastal = sectc[i][j];
}
