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
 *  land.c: New version of empcre - Create the land masses in the game
 * 
 *  Known contributors to this file:
 *     James Anderson, 1987
 *     Dave Pare, 1989
 */

#include <stdio.h>
#include <fcntl.h>
#include "var.h"
#include "misc.h"
#include "power.h"
#include "nat.h"
#include "sect.h"
#include "gamesdef.h"
#include "file.h"
#include "xy.h"
#include "prototypes.h"

#define rnd(x) (random() % (x))

#define XPLATES		((WORLD_X) / 2)	/* basically world x-y size */
#define YPLATES		(WORLD_Y)
#define	XSIZE		(XPLATES)
#define YSIZE		(YPLATES)

#define BIGV		256		/* used in making altitude */
#define SMALLV		128		/* ex-ocean: rnd(SMALLV) - rnd(BIGV) */

typedef enum plates {
	OCEAN, ISLAND, CONTINENT
} plate_e;

#define LANDMIN		1		/* plate altitude for normal land */
#define HILLMIN		34		/* plate altitude for hills */
#define PLATMIN		36		/* plate altitude for plateau */
#define HIGHMIN		98		/* plate altitude for mountains */

#define LANDCH		60		    /* land plate percentage */
#define NUMLAND		(YPLATES * XPLATES * LANDCH)/100
#define NUMISLE		NUMLAND/5	    /* 1 isle for 5 land */
#define NUMWATER	(XPLATES * YPLATES) - (NUMLAND + NUMISLE)

#define SECTRANGE	3		/* smoothing area */
#define MINCONTDIST	2		/* minimum continent distance */
#define CHUNKSIZE	2		/* basic land block size */
#define NEWCONTDIST	(rnd(mincontdist) + mincontdist)
					/* dist away from others for newcont */
#define NUMCHUNKS	70		/* number of CHUNKS per cont */

struct	sctstr sects[YSIZE][XSIZE];

plate_e	plates[YPLATES][XPLATES];
int	world[YSIZE][XSIZE];

int	sectrange = SECTRANGE;
int	mincontdist = MINCONTDIST;
int	chunksize = CHUNKSIZE;
int	numchunks = NUMCHUNKS;
int	numisle = NUMISLE;

static void initworld();
static void growcont();
static void newcont();
static int verify();
static int makeland();
static void make_altitude();
static int total_land();
static void make_sects();
static void land_sct_init(coord x, coord y, s_char *ptr);


int
main(argc, argv)
	int	argc;
	s_char	**argv;
{
	extern	struct empfile empfile[];
	register int n;
	int	x, y;
	int     i, j;
	time_t  now;
	int     fd;
	int	left;
	int	big;

	if (argc > 1 && argc != 6) {
		printf("usage: %s sectrange mincontdist chunksize numchunks numisle\n",
			*argv);
		return -1;
	}
	if (argc == 6) {
		sectrange = atoi(argv[1]);
		mincontdist = atoi(argv[2]);
		chunksize = atoi(argv[3]);
		numchunks = atoi(argv[4]);
		numisle = atoi(argv[5]);
	}
	printf("sectrange: %d\n", sectrange);
	printf("mincontdist: %d\n", mincontdist);
	printf("chunksize: %d\n", chunksize);
	printf("numchunks: %d\n", numchunks);
	printf("numisle: %d\n", numisle);
	fd = open(empfile[EF_SECTOR].file, O_RDWR|O_CREAT|O_TRUNC, 0660);
	if (fd < 0) {
		perror(empfile[EF_SECTOR].file);
		return -1;
	}
	time(&now);
	srandom(now+getpid());
	initworld((plate_e *)plates);
	left = NUMLAND;
	printf("Creating continents");
	while (left > 0) {
		big = (left / (numchunks * 3 * chunksize*chunksize)) + 1;
		for (n=0; n<big; n++) {
			newcont(plates, &x, &y, NEWCONTDIST);
			left -= makeland(plates, x, y, chunksize, CONTINENT);
		}
		for (n=0; n < big * numchunks; n++) {
			growcont(plates, &x, &y);
			left -= makeland(plates, x, y, chunksize, CONTINENT);
			if ((n % numchunks) == 0) {
				printf(".");
				fflush(stdout);
			}
		}
	}
	printf("\n");
	printf("Creating islands");
	for (n = 0; n < numisle; n++) {
		/* find an open spot */
		while (plates[(i = rnd(YPLATES))][(j = rnd(XPLATES))] != OCEAN)
			;
		plates[i][j] = ISLAND;
	}
	printf("\n");
	printf("Making altitude\n");
	make_altitude(plates, world);
	printf("Creating sectors\n");
	make_sects(world, sects);
	printf("Writing sectors\n");
	n = write(fd, sects, sizeof(sects));
	if (n < 0) {
		perror(empfile[EF_SECTOR].file);
		return -1;
	}
	if (n != sizeof(sects)) {
		printf("%s: partial write\n", empfile[EF_SECTOR].file);
		return -1;
	}
	close(fd);
	exit(0);
}

static void
initworld(plates)
	register plate_e *plates;
{
	register int i;

	for (i=0; i<XPLATES*YPLATES; i++)
		*plates++ = OCEAN;
}

static void
growcont(plates, xp, yp)
	register plate_e plates[YPLATES][XPLATES];
	int	*xp;
	int	*yp;
{
	int	x, y;

	/* merge with another one */
	while (plates[(y=rnd(YPLATES))][(x=rnd(XPLATES))] == OCEAN)
		;
	*xp = x;
	*yp = y;
}

static void
newcont(plates, xp, yp, dist)
	register plate_e plates[YPLATES][XPLATES];
	int	*xp;
	int	*yp;
	int	dist;
{
	register int x, y;
	int	i;

	for (i=0; i < 30; i++) { 
		y = rnd(YPLATES);
		x = rnd(XPLATES);
		if (verify(plates, x, y, OCEAN, dist))
			break;
	}
	if (i == 30) {
		growcont(plates, xp, yp);
	} else {
		*xp = x;
		*yp = y;
	}
}

/*
 * verify that with "dist", there are only type "what" sectors
 * returns 0 if fail, 1 if success.
 */
static int
verify(plates, x, y, what, dist)
	register plate_e plates[YPLATES][XPLATES];
	int	x;
	int	y;
	int	what;
	int	dist;
{
	register int xbase, ybase;
	register int x1, y1;

	for (ybase = y - dist; ybase <= y + dist; ybase++) {
		for (xbase = x - dist; xbase <= x + dist; xbase++) {
			/* normalize to world coords */
			y1 = ybase < 0 ? ybase+YPLATES : ybase % YPLATES;
			x1 = xbase < 0 ? xbase+XPLATES : xbase % XPLATES;
			if (plates[y1][x1] != what)
				return 0;
		}
	}
	return 1;
}

static int
makeland(plates, x, y, dist, what)
	register plate_e plates[YPLATES][XPLATES];
	int	x;
	int	y;
	int	dist;
	int	what;
{
	register int xbase, ybase;
	register int xfail, yfail;
	register int x1, y1;
	int	created;

	created = 0;
	for (ybase = y - dist; ybase <= y + dist; ybase++) {
		yfail = y - ybase;
		if (yfail < 0)
			yfail = -yfail;
		y1 = ybase < 0 ? ybase+YPLATES : ybase % YPLATES;
		for (xbase = x - dist; xbase <= x + dist; xbase++) {
			x1 = xbase < 0 ? xbase+XPLATES : xbase % XPLATES;
			if (plates[y1][x1] != OCEAN)
				continue;
			xfail = x - x1;
			if (xfail < 0)
				xfail = -xfail;
			if (xfail < yfail)
				xfail = yfail;
			if (xfail < dist-1 || !rnd(xfail + 1) ||
			    !rnd(xfail + 1)) {
				plates[y1][x1] = what;
				created++;
			}
		}
	}
	return created;
}

static void
make_altitude(plates, world)
	register plate_e plates[YPLATES][XPLATES];
	register int world[YSIZE][XSIZE];
{
	register int x, y;

	for (y = 0; y < YPLATES; y++) {
		for (x = 0; x < XPLATES; x++) {
			switch (plates[y][x]) {
			case OCEAN:
				/*-BIGV, -SMALLV/2, SMALLV*/
				world[y][x] = rnd(SMALLV) - rnd(BIGV);
				break;
			case ISLAND:
				/*-BIGV, 0, BIGV*/
				world[y][x] = rnd(BIGV) - rnd(BIGV) + 2;
				break;
			case CONTINENT:
				/*-SMALLV, SMALLV/2, BIGV*/
				world[y][x] = rnd(BIGV) - rnd(SMALLV);
			}
		}
	}
}

static int
total_land(world, xbase, ybase, range)
	register int world[YSIZE][XSIZE];
	register int xbase;
	int ybase;
	register int range;
{
	register int x;
	register int xmax;
	register int total;
	register int *row;
	int	y;
	int	ymax;

	total = 0;
	xmax = xbase + range;
	ymax = ybase + range;
	for (y = ybase; y < ymax; y++) {
		row = world[y % YSIZE];
		for (x = xbase; x < xmax; x++)
			total += row[x % XSIZE];
	}
	return total;
}

static void
make_sects(world, sects)
	register int world[YSIZE][XSIZE];
	struct sctstr *sects;
{
	register struct sctstr *sct;
	register int i;
	register int x, y;
	int	elev[12+12+3]; /* # sects from -12 to 12 in steps of 10 elev */
	int	range;
	int	rangesq;
	int	total;
	int	sum;

	for (i = 0; i < 12+12+3; i++)
		elev[i] = 0;
	sum = 0;
	sct = sects;
	for (y = 0; y < YSIZE; y++) {
		for (x = 0; x < XSIZE; x++, sct++) {
			land_sct_init(x*2 + (y & 01), y, (s_char *)sct);
			range = 3 + rnd(sectrange);
			rangesq = range * range;
			total = total_land(world, x, y, range) / rangesq;
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
			sum += total;
			if (total < -129)
				elev[0]++;
			else
				if (total > 129)
					elev[26]++;
				else
					elev[13+total/10]++;
		}
	}
	for (i = 0; i < 12+12+3; i++)
		if (elev[i] != 0)
			printf("%4d sectors elevation %4d to %4d\n",
				elev[i], 10*i - 140, 10*i - 130);
}

static void
land_sct_init(coord x, coord y, s_char *ptr)
{
	struct	sctstr *sp = (struct sctstr *) ptr;

	sp->ef_type = EF_SECTOR;
	sp->sct_x = x;
	sp->sct_y = y;
	sp->sct_dist_x = x;
	sp->sct_dist_y = y;
}
