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
 *  ore.c: Create a world
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak
 */

/*
 * In the oceans, it puts oil = (2d6-4)*10+d10, fert = (2d6-2)*10+d10
 *                        (max of 127 in both cases)
 *
 * For islands smaller than ISLANDSIZE, it puts in normal
 *     resources. (These are assumed to be small non-start islands)
 * For islands at least ISLANDSIZE big, it makes normal fert,
 *     but it uses the MIN, OIL, GOLD, and URAN arrays to find out
 *     what resources to put there. The intent is to find all start
 *     islands, and give them all a pre-determined spread of stuff.
 *     To change the stuff, just change the MIN, OIL, GOLD, and URAN arrays.
 *
 * For example, if the GOLD array is 90,80,70,60, it'll try to put 1
 *     sector with 90 gold, 1 with 80 gold, 1 with 70, and 1 with 60 gold
 *     on each island of at least ISLANDSIZE. It will not overrun start
 *     sectors, and should be run AFTER you have placed capitals.
 */

* /
#if defined(aix) || defined(linux) || defined(solaris)
#include <unistd.h>
#endif /* aix or linux */
#include <stdio.h>
#include <fcntl.h>
#include "misc.h"
#include "var.h"
#include "sect.h"
#include "gamesdef.h"
#include "file.h"
#include "path.h"
#include "xy.h"
#include "prototypes.h"
#define XPLATES		WORLD_X/2
#define YPLATES		WORLD_Y
#define YbyX		(YPLATES*XPLATES/8)
#define ISLANDSIZE	25
#define ISLANDMAX	1000
int MIN[] = { 90, 85, 85, 85, 70, 70, 70, 70, 70 };
int OIL[] = { 90, 80, 70, 60, 50 };
int GOLD[] = { 90, 80, 70, 60, 50 };
int URAN[] = { 90, 80, 70, 60, 50 };

#define MIN_TYPE	0
#define OIL_TYPE	1
#define GOLD_TYPE	2
#define URAN_TYPE	3

struct sctstr sects[YPLATES * XPLATES];
u_char s[WORLD_Y][WORLD_X];
short c[WORLD_Y][WORLD_X];
short e[WORLD_Y][WORLD_X];
int size[YbyX];
int goldchance[YbyX];
int ironchance[YbyX];
int oilchance[YbyX];
int fertchance[YbyX];
int urchance[YbyX];
int ix[ISLANDMAX], iy[ISLANDMAX], ic[ISLANDMAX], in = 0;
int cont = 0;

void makec();
void makeore();
int place();
int nearelev();

int
main(argc, argp)
int argc;
s_char **argp;
{
    register struct sctstr *sp;
    register int y;
    register int x;
    int sectf;
    time_t now;
    s_char *sectfil = NULL;
    int xx, x1, y1;
    extern char *optarg;
    int opt;
    char *config_file = NULL;


    while ((opt = getopt(argc, argp, "e:s:")) != EOF) {
	switch (opt) {
	case 'e':
	    config_file = optarg;
	    break;
	case 's':
	    sectfil = optarg;
	    break;
	}
    }

    emp_config(config_file);

    if (sectfil == NULL)
	sectfil = empfile[EF_SECTOR].file;
    time(&now);
    srandom((unsigned int)now);
    sectf = open(sectfil, O_RDWR, 0);
    if (sectf < 0) {
	perror(sectfil);
	exit(1);
    }
    read(sectf, sects, sizeof(sects));
    sp = sects;
    for (y = 0; y < WORLD_Y; y++) {
	for (x = 0; x < WORLD_X; x++) {
	    if (((x ^ y) & 1))
		continue;
	    c[sp->sct_y][sp->sct_x] = 0;
	    s[sp->sct_y][sp->sct_x] = sp->sct_type;
	    e[sp->sct_y][sp->sct_x] = sp->sct_elev;
	    sp++;
	}
    }
    for (y = 0; y < WORLD_Y; y++) {
	for (x = 0; x < WORLD_X; x++) {
	    if (((x ^ y) & 1))
		continue;
	    if (c[y][x] != 0)
		continue;
	    if (s[y][x] == SCT_WATER)
		continue;

	    makec(x, y, 0);

	    for (y1 = 0; y1 < WORLD_Y; y1++)
		for (x1 = 0; x1 < WORLD_X; x1++)
		    if (c[y1][x1] < 0) {
			size[cont]++;
			c[y1][x1] = cont;
		    }
	    if (size[cont] == 0)
		continue;
	    if (size[cont] >= ISLANDSIZE) {
		xx = ISLANDSIZE / 2;
		for (y1 = 0; y1 < WORLD_Y; y1++)
		    for (x1 = 0; x1 < WORLD_X; x1++)
			if (c[y1][x1] == cont) {
			    if (--xx == 0) {
				ix[in] = x1;
				iy[in] = y1;
				ic[in] = cont;
				in++;
			    }
			}
		printf("Found island %d of size %d at %d,%d\n",
		       in, size[cont], ix[in - 1], iy[in - 1]);
	    } else {
		printf("Found islet %d of size %d\n", cont, size[cont]);
		for (y1 = 0; y1 < WORLD_Y; y1++)
		    for (x1 = 0; x1 < WORLD_X; x1++)
			if (c[y1][x1] == cont)
			    c[y1][x1] = ISLANDMAX + 1;
	    }
	}
    }

    sp = sects;
    for (y = 0; y < WORLD_Y; y++) {
	for (x = 0; x < WORLD_X; x++) {
	    if (((x ^ y) & 1))
		continue;
	    makeore(sp++);
	}
    }
    for (x = 0; x < in; x++) {
	printf("Making ore for island %d\n", x);
	place(x, MIN, sizeof(MIN) / sizeof(int), OFFSET(sctstr, sct_min),
	      "MIN");
	place(x, URAN, sizeof(URAN) / sizeof(int),
	      OFFSET(sctstr, sct_uran), "URAN");
	place(x, OIL, sizeof(OIL) / sizeof(int), OFFSET(sctstr, sct_oil),
	      "OIL");
	place(x, GOLD, sizeof(GOLD) / sizeof(int),
	      OFFSET(sctstr, sct_gmin), "GOLD");
    }

    lseek(sectf, 0L, 0);
    write(sectf, sects, sizeof(sects));
    close(sectf);
    exit(0);
}

#define max(a,b) (a < b ? b : a)

void
makec(x, y, n)
int x;
int y;
int n;
{
    register int dy;
    register int dx;
    register int dir;

    if (s[y][x] == SCT_WATER)
	return;

    if (c[y][x] != 0)
	return;

    if (n == 0)
	n = ++cont;

    c[y][x] = -1;

    for (dir = 1; dir <= 6; dir++) {
	dx = (diroff[dir][0] + x + WORLD_X) % WORLD_X;
	dy = (diroff[dir][1] + y + WORLD_Y) % WORLD_Y;
	makec(dx, dy, n);
    }
}

#define MINIMUM(x,y) (x > y ? y : x)
#define MAXIMUM(x,y) (x > y ? x : y)

void
makeore(sp)
register struct sctstr *sp;
{
    int elev;
    int oil, gmin, iron, fertil, ur;
    int lev;
    int i;
    int x, y;

    if (sp->sct_type == SCT_SANCT)
	return;

    y = sp->sct_y;
    x = sp->sct_x;

    elev = e[y][x];
    lev = (elev * 12 + nearelev(x, y)) / 18;
    i = c[y][x];
    oil = 0;
    iron = 0;
    gmin = 0;
    fertil = 0;
    ur = 0;

    if (sp->sct_type == SCT_WATER) {
	oil = MAXIMUM(((roll(6) + roll(6) - 4) * 10 + roll(10)), 0);
	sp->sct_oil = MINIMUM(oil, 127);
	fertil = MAXIMUM(((roll(6) + roll(6) - 2) * 12 + roll(10)), 0);
	sp->sct_fertil = MINIMUM(fertil, 127);
	return;
    }
    iron = roll(100) + 100 - roll(100);
    oil = roll(100) + 100 - roll(100);
    ur = roll(100) + 100 - roll(100);
    gmin = roll(100) + 100 - roll(100);
    fertil = 85 + 100 - roll(100);

    iron = MAXIMUM(iron, 0);
    oil = MAXIMUM(oil, 0);
    ur = MAXIMUM(ur, 0);
    gmin = MAXIMUM(gmin, 0);
    fertil = MAXIMUM(fertil, 0);
    iron = MINIMUM(iron, 127);
    oil = MINIMUM(oil, 127);
    ur = MINIMUM(ur, 127);
    gmin = MINIMUM(gmin, 127);
    fertil = MINIMUM(fertil, 127);

    sp->sct_fertil = fertil;
    if (i > ISLANDMAX) {
	sp->sct_oil = oil;
	sp->sct_min = iron;
	sp->sct_gmin = gmin;
	sp->sct_uran = ur;
    } else {
	fertil -= 20;
	fertil = MAXIMUM(fertil, 0);
	fertil = MINIMUM(fertil, 127);
	sp->sct_fertil = fertil;
    }
}

int
nearelev(x, y)
int x;
int y;
{
    int dir;
    int dx;
    int dy;
    int level;

    level = 0;
    for (dir = 1; dir <= 6; dir++) {
	dx = (diroff[dir][0] + x + WORLD_X) % WORLD_X;
	dy = (diroff[dir][1] + y + WORLD_Y) % WORLD_Y;
	level += e[dy][dx];
    }
    return level;
}

/*		Already in libgen.a
roll(n)
	int	n;
{
	return random() % n;
}
*/

int
min(a, b)
int a;
int b;
{
    if (a < b)
	return a;
    return b;
}

int
place(island, res, num, offset, restype)
int island, res[], num, offset;
s_char *restype;
{
    int x1, y1, x2, y2, x3, y3, j, k;
    int tried[WORLD_Y][WORLD_X];
    int numtried, there;
    struct sctstr *foo;

    x1 = ix[island];
    y1 = iy[island];

    for (k = 0; k < num; k++) {

	x2 = x1;
	y2 = y1;
	memset(tried, 0, sizeof(tried));
	numtried = 0;
	while ((sects[y2 * XPLATES + x2 / 2].sct_type == SCT_WATER) ||
	       (*
		((u_char *)(((s_char *)&sects[y2 * XPLATES + x2 / 2]) +
			    offset)) > 0)
	       || (sects[y2 * XPLATES + x2 / 2].sct_type == SCT_MOUNT)) {

	    there =
		*((u_char *)(((s_char *)&sects[y2 * XPLATES + x2 / 2]) +
			     offset));

	    foo = &sects[y2 * XPLATES + x2 / 2];

	    if (sects[y2 * XPLATES + x2 / 2].sct_type == SCT_RURAL) {
		if (!tried[y2][x2]) {
		    tried[y2][x2] = 1;
		    numtried++;
		    if (numtried == ISLANDSIZE) {
			int a, b;
			printf("*** %d) couldn't place %s %d (%d)\n",
			       island, restype, k, res[k]);
			for (a = 0; a < WORLD_Y; a++)
			    for (b = 0; b < WORLD_X; b++)
				if (tried[a][b])
				    printf("\tTried %d,%d\n", b, a);
			return (0);
		    }
		}
	    }
	    do {
		j = roll(6);
		x3 = (diroff[j][0] + x2 + WORLD_X) % WORLD_X;
		y3 = (diroff[j][1] + y2 + WORLD_Y) % WORLD_Y;
	    } while (sects[y3 * XPLATES + x3 / 2].sct_type == SCT_WATER);
	    x2 = x3;
	    y2 = y3;
	}

/*		sects[y2*XPLATES+x2/2].sct_min = MIN[k]; */
	*((u_char *)(((s_char *)&sects[y2 * XPLATES + x2 / 2]) + offset)) =
	    res[k];
    }
    return (1);
}
