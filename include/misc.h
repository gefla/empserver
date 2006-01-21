/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  misc.h: Misc. definitions which don't really belong anywhere,
 *          but have to go somewhere!
 * 
 *  Known contributors to this file:
 *       Doug Hay, 1998
 */

#ifndef MISC_H
#define MISC_H

#include <ctype.h>
#include <string.h>
#include <sys/types.h>

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

#if defined(_WIN32)
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef long ssize_t;

/* integral mismatch, due to misuse of sector short */
#ifndef __GNUC__
#pragma warning (disable : 4761 )
#endif

#include <io.h>

#define random rand
#define srandom srand

#define strncasecmp(s1, s2, s3) _strnicmp(s1, s2, s3)
#define mkdir(dir,perm) _mkdir((dir))
#ifndef F_OK
#define F_OK 0  /* FILE existence */
#endif
#ifndef W_OK
#define W_OK 02 /* Write permission */
#endif
#ifndef R_OK
#define R_OK 04 /* Read permission */
#endif

typedef int pid_t;
#define vsnprintf _vsnprintf

#endif /* _WIN32 */

/* This is the structure we use to keep track of the global mobility
   things, such as when the server is supposed to start/stop updating
   mobility and the timestamp of when the game was last up.  These
   times are in seconds. */
struct mob_acc_globals {
    time_t timestamp;		/* Last timestamp of when the game was up */
    time_t starttime;		/* When we should start updating mobility again. */
};

#ifdef __GNUC__
#define ATTRIBUTE(attrs) __attribute__ (attrs)
#else
#define ATTRIBUTE(attrs)
#endif

/* This uses a lot of thread stack with some versions of GNU libc,
   which can lead to nasty heap smashes (observed with 2.2.93).
   Disabled for now, until we readjust thread stack sizes.  */
#if 0
#define RESOLVE_IPADDRESS	/* resolve ip addresses into hostnames */
#endif

typedef u_char natid;		/* NSC_NATID must match this */

/*
 * TODO s_char has to go.  Use plain char for characters and strings,
 * signed char for small integers.
 */
#ifdef __CHAR_UNSIGNED__
typedef signed char s_char;
#else
typedef char s_char;
#endif

typedef short coord;

#ifndef bit
#define	bit(x)		(1<<(x))
#endif

#define minutes(x)	(60*(x))
#define hours(x)	(60*60*(x))
#define days(x)		(60*60*24*(x))

extern int debug;
extern int daemonize;

/*
 * If EXPR is true, an internal error occured.
 * Return EXPR != 0.
 * Usage: if (CANT_HAPPEN(...)) recovery_code();
 */
#define CANT_HAPPEN(expr) ((expr) ? oops(#expr, __FILE__, __LINE__) : 0)
extern int oops(char *, char *, int);

	/* return codes from command routines */
#define	RET_OK		0	/* command completed sucessfully */
#define	RET_FAIL	1	/* command completed unsucessfully [?] */
#define	RET_SYN		2	/* syntax error in command */
#define	RET_SYS		3	/* system error (missing file, etc) */

extern char *getstarg(char *input, char *prompt, char buf[]);
extern char *getstring(char *prompt, char buf[]);
extern char *ugetstring(char *prompt, char buf[]);
             
extern char *prbuf(char *format, ...)
    ATTRIBUTE((format (printf, 1, 2)));

#define AGREE_FREE	0
#define AGREE_PROPOSED	1
#define AGREE_SIGNED	2

#endif
