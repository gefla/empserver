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
 *  perfect.c: Create a perfect world
 * 
 *  Known contributors to this file:
 *     James Anderson, 1986
 *     Dave Pare, 1986
 */

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

#define XPLATES		(WORLD_X / 2)
#define YPLATES		WORLD_Y
#define YbyX		(YPLATES*XPLATES/8)

struct	sctstr sects[YPLATES*XPLATES];
u_char	s[YPLATES][XPLATES];
short	c[YPLATES][XPLATES];
short	e[YPLATES][XPLATES];
int	size[YbyX];
int	goldchance[YbyX];
int	ironchance[YbyX];
int	oilchance[YbyX];
int	fertchance[YbyX];
int	urchance[YbyX];

void	makec();
void    makeore();
int     nearelev();

int
main(argc, argp)
	int     argc;
	s_char  **argp;
{
	register struct sctstr *sp;
	register int y;
	register int x;
	int     sectf;
	int	i;
	time_t	now;
	s_char	*sectfil = NULL;
	extern char *optarg;
	int opt;
	char *config_file = NULL;
	

	while ((opt = getopt (argc, argp, "e:s:")) != EOF) {
		switch (opt) {
		    case 'e':
			config_file = optarg;
			break;
		    case 's':
			sectfil = optarg;
			break;
		}
	}

	emp_config (config_file);

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
			c[y][x/2] = 0;
			s[y][x/2] = sp->sct_type;
			e[y][x/2] = sp->sct_elev;
			sp++;
		}
	}
	for (y = 0; y < WORLD_Y; y++) {
		for (x = 0; x < WORLD_X; x++) {
			if (((x ^ y) & 1))
				continue;
			if (c[y][x/2] != 0)
				continue;
			makec(x, y, 0);
		}
	}
	printf("size  oil  gold fert iron\n");
	for (i=1; i<YbyX && size[i]; i++) {
		oilchance[i] =  5 + roll(75) - min(size[i] / 15, 10);
/*		goldchance[i] = 3 + roll(45) -  */
		goldchance[i] = 3 + roll(55) -
			(oilchance[i] + min(size[i] / 15, 10));
/*		urchance[i] = roll(40) - goldchance[i]; */
		urchance[i] = roll(30) - goldchance[i];
		fertchance[i] = roll(40) - (goldchance[i] + oilchance[i]);
		ironchance[i] = roll(45) - (goldchance[i] + oilchance[i]/2);
		if (size[i] > 10) {
			printf("%3d   %3d  %3d  %3d  %3d\n",
				size[i], oilchance[i], goldchance[i],
				fertchance[i], ironchance[i]);
		}
	}
	if (i == YbyX)
		printf("Oops.  YbyX not big enough\n");
	sp = sects;
	for (y = 0; y < WORLD_Y; y++) {
		for (x = 0; x < WORLD_X; x++) {
			if (((x ^ y) & 1))
				continue;
			makeore(sp++);
		}
	}
	lseek(sectf, 0L, 0);
	write(sectf, sects, sizeof(sects));
	close(sectf);
	exit(0);
}

int	cont = 0;

#define max(a,b) (a < b ? b : a)

void
makec(x, y, n)
	int	x;
	int	y;
	int	n;
{
	register int dy;
	register int dx;
	register int dir;
	register short *cp;

	if (s[y][x/2] == SCT_WATER)
		return;
	cp = &c[y][x/2];
	if (*cp != 0)
		return;
	if (n == 0)
		n = ++cont;
	*cp = n;
	size[n]++;
	for (dir = 1; dir <= 6; dir++) {
		dx = (diroff[dir][0] + x + WORLD_X) % WORLD_X;
		dy = (diroff[dir][1] + y + WORLD_Y) % WORLD_Y;
		makec(dx, dy, n);
	}
}

void
makeore(sp)
	register struct sctstr *sp;
{
	int	elev;
	int	oil, gmin, iron, fertil, ur;
	int	lev;
	int	i;
	int	x,y;

	y = sp->sct_y;
	x = sp->sct_x;
	elev = e[y][x/2];
	lev = (elev*12 + nearelev(x, y))/18;
	i = c[y][x/2];
	oil = 0;
	iron = 0;
	gmin = 0;
	fertil = 0;
	ur = 0;
	if (lev < 20 && lev > -15 && roll(100) < 30+oilchance[i]+lev) {
		oil = (45 - lev) + roll(50);
		if (oil > 100)
			oil = 100;
		if (oil < 10)
			oil = 0;
	}
	if (lev > 15 && roll(20+lev) < lev+ironchance[i]) {
		iron = lev + roll(55);
		if (iron > 100)
			iron = 100;
		if (iron < 10)
			iron = 0;
	}
	if (lev > 16 && roll(35+lev) < lev+goldchance[i]) {
		gmin = (lev - 5) + roll(45);
		if (gmin > 100)
			gmin = 100;
		if (gmin < 10)
			gmin = 0;
	}
	if (lev > 14 && roll(35+lev) < lev+urchance[i]) {
		ur = (lev - 5) + roll(45);
		if (ur > 100)
			ur = 100;
		if (ur < 10)
			ur = 0;
	}
	fertil = (85 + fertchance[i] - lev) - (((iron + gmin)/2) - oil/3);
	if (fertil > 120)
		fertil = 120;
	if (fertil < 0)
		fertil = 0;
	sp->sct_oil = 100;
	sp->sct_min = 100;
	sp->sct_gmin = 100;
	sp->sct_fertil = 100;
	sp->sct_uran = 100;
}

int
nearelev(x, y)
	int	x;
	int	y;
{
	int	dir;
	int	dx;
	int	dy;
	int	level;

	level = 0;
	for (dir = 1; dir <= 6; dir++) {
		dx = (diroff[dir][0] + x + WORLD_X) % WORLD_X;
		dy = (diroff[dir][1] + y + WORLD_Y) % WORLD_Y;
		level += e[dy][dx/2];
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
	int	a;
	int	b;
{
	if (a < b)
		return a;
	return b;
}
