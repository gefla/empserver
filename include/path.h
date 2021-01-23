/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  path.h: Definitions for directions, paths, etc.
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2005-2014
 */

#ifndef PATH_H
#define PATH_H

#include <stddef.h>
#include "types.h"

	/* direction indices */
#define DIR_STOP	0
#define DIR_UR		1
#define DIR_R		2
#define DIR_DR		3
#define DIR_DL		4
#define DIR_L		5
#define DIR_UL		6
#define DIR_VIEW	7
#define DIR_MAP		8
#define DIR_FIRST	1
#define DIR_LAST	6

#define DIR_BACK(dir) ((dir) >= DIR_FIRST + 3 ? (dir) - 3 : (dir) + 3)

#define MOB_MOVE	0
#define MOB_MARCH	1
#define MOB_RAIL	2
#define MOB_SAIL	3
#define MOB_FLY		4

/* src/lib/global/dir.c */
extern signed char dirindex['z'-'a'+1];
extern int diroff[DIR_MAP+1][2];
extern char dirch[DIR_MAP+2];
extern char *routech[DIR_LAST+1];

/* src/lib/common/findpath.c */
extern void path_find_from(coord, coord, natid, int);
extern double path_find_to(coord, coord);
extern double path_find(coord, coord, coord, coord, natid, int);
extern size_t path_find_route(char *, size_t, coord, coord, coord, coord);
#ifdef PATH_FIND_DEBUG
extern void path_find_visualize(coord, coord, coord, coord);
#endif
#ifdef PATH_FIND_STATS
extern void path_find_print_stats(void);
#else
#define path_find_print_stats() ((void)0)
#endif

/* src/lib/subs/paths.c */
extern char *getpath(char *, char *, coord, coord, int, int, int);
extern double fcost(struct sctstr *, natid);
extern double ncost(struct sctstr *, natid);
extern double pathtoxy(char *, coord *, coord *,
		       double (*)(struct sctstr *, natid));
extern int chkdir(char, int, int);
extern int diridx(char);
extern void direrr(char *, char *, char *);
extern void pathrange(coord, coord, char *, int, struct range *);

extern double sector_mcost(struct sctstr *, int);
extern double speed_factor(double, int);

#define MAX_PATH_LEN 1024

#endif
