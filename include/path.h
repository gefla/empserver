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
 *  path.h: Definitions for directions, paths, etc.
 *
 *  Known contributors to this file:
 *
 */

#ifndef PATH_H
#define PATH_H

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

#define MOB_MOVE	0
#define MOB_MARCH	1
#define MOB_RAIL	2

enum p_mode {			/* How to find path to destination */
    P_NONE,			/* don't */
    P_FLYING,			/* use BestAirPath() */
    P_SAILING			/* use BestShipPath() */
};

/* src/lib/global/dir.c */
extern signed char dirindex['z'-'a'+1];
extern int diroff[DIR_MAP+1][2];
extern char dirch[DIR_MAP+2];
extern char *routech[DIR_LAST+1];

/* src/lib/common/bestpath.c */
extern char *bestownedpath(char *, char *, int, int, int, int, int);

/* src/lib/common/path.c */
extern void bp_enable_cachepath(void);
extern void bp_disable_cachepath(void);
extern void bp_clear_cachepath(void);
extern char *BestDistPath(char *, struct sctstr *, struct sctstr *,
			  double *);
extern char *BestLandPath(char *, struct sctstr *, struct sctstr *,
			  double *, int);
extern char *BestShipPath(char *, int, int, int, int, int);
extern char *BestAirPath(char *, int, int, int, int);
extern double pathcost(struct sctstr *, char *, int);

/* src/lib/subs/paths.c */
extern char *getpath(char *, char *, coord, coord, int, int, enum p_mode);
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
