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
 *  misc.h: Misc client definitions
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#ifndef _MISC_H_
#define _MISC_H_

#include <ctype.h>
#include <sys/types.h>
#include <ctype.h>

#ifdef hpux
#include <string.h>
#else
#ifdef _WIN32
#include <string.h>

typedef unsigned char u_char;
typedef unsigned short u_short;
#else
#include <strings.h>
#endif
#endif

#define MAXNOC		100

#define WORLD_X		32
#define WORLD_Y		16

#define	ETUS		60	/* Defined here.. may be overridden later,
				   but things still use this define */
typedef u_char natid;		/* also change NSC_NATID in nsc.h */
#if !defined(aix) && !defined(sgi)
#ifndef ultrix			/* already defined in ultrix */
typedef char s_char;		/* change to signed char for aix */
#endif /* ultrix */
#else
typedef signed char s_char;
#endif /* !aix && !sgi */
typedef short coord;		/* also change NSC_COORD in nsc.h */
/*
 * watch it; your compiler might not like this.
 * If you think this is bogus, look at /usr/include/struct.h
 */
#define	OFFSET(stype, oset) ((int)(&(((struct stype *)0)->oset)))
#define	SETOFF(sinst, oset) ((s_char *) (sinst) + (oset))

#ifndef bit
#define	bit(x)		(1<<(x))
#endif

#define minutes(x)	(60*(x))
#define hours(x)	(60*60*(x))
#define days(x)		(60*60*24*(x))

typedef void (*voidfunc) ();

	/* return codes from command routines */
#define	RET_OK		0	/* command completed sucessfully */
#define	RET_FAIL	1	/* command completed unsucessfully [?] */
#define	RET_SYN		2	/* syntax error in command */
#define	RET_SYS		3	/* system error (missing file, etc) */

#define MAX_DISTPATH_LEN	10	/* Has to go somewhere */

/*
 * references to library functions which aren't related to any
 * particular object, and are of general interest
 */
extern long random();

extern double dmax();
extern double dmin();

extern s_char *fmt();
extern s_char *copy();
extern s_char *numstr();
extern s_char *esplur();
extern s_char *splur();
extern s_char *iesplur();
extern s_char *plur();
extern s_char *getstarg();
extern s_char *getstring();

/*
 * frequently used libc functions
 */

#ifndef _WIN32
extern s_char *malloc();
extern s_char *calloc();
#endif

#if (!defined (aix) && !defined (sgi))
extern s_char *ctime();
extern s_char *strncpy();
extern s_char *strcpy();
#ifndef NeXT
extern s_char *index();
extern s_char *rindex();
#endif /* NeXT */
#endif /* !aix && !sgi */

extern time_t time();
extern double atof();

#endif
