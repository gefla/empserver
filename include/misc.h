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
 *  misc.h: Misc. definitions which don't really belong anywhere,
 *          but have to go somewhere!
 * 
 *  Known contributors to this file:
 *       Doug Hay, 1998
 */

#ifndef _MISC_H_
#define _MISC_H_

#include <ctype.h>
#include <sys/types.h>
#include <ctype.h>

#if defined(hpux) || defined(SUN4) || defined(solaris) || defined(_WIN32)
#include <string.h>
#else
#include <strings.h>
#endif

#if defined(_WIN32)
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef char *caddr_t;
typedef unsigned int u_int;

/* integral mismatch, due to misuse of sector short */
#pragma warning (disable : 4761 )

#include <minmax.h>
#include <io.h>

#define random rand
#define srandom srand

#define index strchr
#define rindex strrchr
/*#define _POSIX_ */
#endif

#include "prototype.h"
#include "options.h"

/* This is the structure we use to keep track of the global mobility
   things, such as when the server is supposed to start/stop updating
   mobility and the timestamp of when the game was last up.  These
   times are in seconds. */
struct mob_acc_globals {
    time_t timestamp;		/* Last timestamp of when the game was up */
    time_t starttime;		/* When we should start updating mobility again. */
};

#define ANNO_KEEP_DAYS 7	/* Keep announcements around for this many days */
#define RESOLVE_IPADDRESS	/* resolve ip addresses into hostnames */

typedef u_char natid;		/* also change NSC_NATID in nsc.h */

#if defined(__linux__ ) || defined(__osf__)
#define POSIXSIGNALS 1
#endif

#if !defined (_EMPTH_LWP) && !defined (_EMPTH_POSIX)
#define _EMPTH_LWP     1
#endif

#if defined(_EMPTH_POSIX) && !defined(POSIXSIGNALS)
#define POSIXSIGNALS
#endif

#if defined(POSIXSIGNALS) || defined(__STDC__)
typedef void emp_sig_t;
#else
typedef int emp_sig_t;
#endif


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
typedef int (*qsort_func_t) (const void *, const void *);

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
#if !defined(ALPHA) && !defined(__osf__) && !defined(__linux__) && !defined(_WIN32)
extern long random();
#endif

double dmax _PROTO((double n1, double n2));
double dmin _PROTO((double n1, double n2));

extern s_char *numstr _PROTO((s_char buf[], int n));
extern s_char *esplur _PROTO((int n));
extern s_char *splur _PROTO((int n));
extern s_char *iesplur _PROTO((int n));
extern s_char *plur _PROTO((int n, s_char *no, s_char *yes));
extern s_char *getstarg
_PROTO((s_char *input, s_char *prompt, s_char buf[]));
extern s_char *getstring _PROTO((s_char *prompt, s_char buf[]));
extern s_char *prbuf(s_char *format, ...);

/*
 * frequently used libc functions
 */

#if !defined(_WIN32)
#if defined(linux)
#ifndef __STDC__
extern char *malloc();
extern char *calloc();
#endif
#else
#if !defined(ALPHA) && !defined(__osf__) && !defined(ultrix) && !defined(hpux) && !defined(FBSD)
extern char *malloc();
extern char *calloc();
#endif
#endif
#endif

#if (!defined (aix) && !defined (sgi))
extern char *ctime();
extern char *strncpy();
extern char *strcpy();
#ifndef NeXT
extern char *index();
extern char *rindex();
#endif /* NeXT */
#endif /* !aix && !sgi */

extern time_t time();
#if !defined(_WIN32)
#ifdef linux
#ifndef atof
extern double atof();
#endif
#else
extern double atof();
#endif
#endif

#if !defined(_WIN32)
#if !defined (__ppc__) && !defined (hpux) && !defined (aix) && !defined (linux) && !defined(ALPHA) && !defined(__osf__) && !defined(SUN4) && !defined (Rel4) && !(defined (ultrix) && (defined (__STDC__) || defined (__SYSTEMFIVE) || defined (__POSIX)))
extern char *sprintf();
#endif /* aix */
#endif

#define MAXCHRNV 12

#include "prototypes.h"		/* must come at end, after defines and typedefs */

#endif /* _MISC_H_ */
