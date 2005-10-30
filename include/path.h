/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  path.h: Definitions for directions, paths, etc.
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef _PATH_H_
#define _PATH_H_

#include "misc.h"
#include "sect.h"
#include "xy.h"

	/* direction indices */
#define	DIR_STOP	0
#define	DIR_UR		1
#define	DIR_R		2
#define	DIR_DR		3
#define	DIR_DL		4
#define	DIR_L		5
#define	DIR_UL		6
#define	DIR_VIEW	7
#define	DIR_MAP		8
#define	DIR_FIRST	1
#define	DIR_LAST	6

extern signed char dirindex['z'-'a'+1];
extern int diroff[DIR_MAP+1][2];
extern char dirch[DIR_MAP+2];
extern char *routech[DIR_LAST+1][2];

/* src/lib/subs/paths.c */
extern char *getpath(char *, char *, coord, coord, int, int, int);
extern double fcost(struct sctstr *, natid);
extern double ncost(struct sctstr *, natid);
extern double pathtoxy(s_char *, coord *, coord *,
		       double (*)(struct sctstr * sp, natid own));
extern int chkdir(s_char, int, int);
extern int diridx(char);
extern void direrr(s_char *, s_char *, s_char *);
extern void pathrange(register coord, register coord, register s_char *,
		      int, struct range *);

extern double sector_mcost(struct sctstr *sp, int do_bonus);

#define	P_NONE		0	/* NO destinations allowed */
#define	P_WALKING	1	/* use BestLandPath, only owned */
#define	P_FLYING	2	/* use bestpath, any */
#define	P_SAILING	3	/* use bestpath, any */

#define MAX_PATH_LEN 1024

#endif /* _PATH_H_ */
