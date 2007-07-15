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
 *  misc.h: Misc. definitions which don't really belong anywhere,
 *          but have to go somewhere!
 * 
 *  Known contributors to this file:
 *       Doug Hay, 1998
 */

#ifndef MISC_H
#define MISC_H

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#ifdef _WIN32
#ifndef __GNUC__

typedef long ssize_t;

/* integral mismatch, due to misuse of sector short */
#pragma warning (disable : 4761 )

#define strncasecmp(s1, s2, s3) _strnicmp((s1), (s2), (s3))

typedef int pid_t;
#define vsnprintf _vsnprintf
#define snprintf _snprintf

#endif /* !__GNUC__ */

#define mkdir(dir,perm) _mkdir((dir))
#define random rand
#define srandom srand
#ifndef S_IRUSR
#define S_IRUSR	    _S_IREAD
#define S_IWUSR	    _S_IWRITE
#define S_IXUSR	    _S_IEXEC
#define S_IRWXU	    S_IRUSR | S_IWUSR | S_IXUSR
#endif
#ifndef S_IRGRP
#define S_IRGRP	    0
#define S_IWGRP	    0
#define S_IXGRP	    0
#define S_IRWXG	    S_IRGRP | S_IWGRP | S_IXGRP
#endif
#ifndef S_IROTH
#define S_IROTH	    0
#define S_IWOTH	    0
#define S_IXOTH	    0
#define S_IRWXO	    S_IROTH | S_IWOTH | S_IXOTH
#endif
#endif /* _WIN32 */

#ifndef S_IRWUG
#define S_IRWUG	    S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR
#endif

#ifdef __GNUC__
#define ATTRIBUTE(attrs) __attribute__ (attrs)
#else
#define ATTRIBUTE(attrs)
#endif

/*
 * This uses a lot of thread stack with some versions of GNU libc,
 * which can lead to nasty heap smashes (observed with 2.2.93).
 * Disabled for now, until we readjust thread stack sizes.
 */
#if 0
#define RESOLVE_IPADDRESS	/* resolve ip addresses into hostnames */
#endif

#ifndef bit
#define	bit(x)		(1<<(x))
#endif

#define minutes(x)	(60*(x))
#define hours(x)	(60*60*(x))
#define days(x)		(60*60*24*(x))

extern int debug;

/*
 * If EXPR is true, an internal error occured.
 * Return EXPR != 0.
 * Usage: if (CANT_HAPPEN(...)) <recovery code>;
 */
#define CANT_HAPPEN(expr) ((expr) ? oops(#expr, __FILE__, __LINE__) : 0)

/*
 * Report internal error.
 * Usage: CANT_REACH(); <recovery code>;
 */
#define CANT_REACH() (void)oops(NULL, __FILE__, __LINE__)

extern int oops(char *, char *, int);

void exit_nomem(void) ATTRIBUTE((noreturn));

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
