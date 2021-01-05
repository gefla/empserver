/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  misc.h: Misc. definitions which don't really belong anywhere,
 *          but have to go somewhere!
 *
 *  Known contributors to this file:
 *     Doug Hay, 1998
 *     Markus Armbruster, 2004-2014
 */

#ifndef MISC_H
#define MISC_H

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define LIMIT_TO(val, min, max) \
    ((val) < (min) ? (min) : (val) > (max) ? (max) : (val))

#ifdef _WIN32
#include "w32misc.h"
#endif /* _WIN32 */

#ifdef __GNUC__
#define ATTRIBUTE(attrs) __attribute__ (attrs)
#else
#define ATTRIBUTE(attrs)
#endif

#ifndef bit
#define bit(x)		(1U<<(x))
#endif

#define minutes(x)	(60*(x))
#define hours(x)	(60*60*(x))
#define days(x)		(60*60*24*(x))

/*
 * If @expr is true, an internal error occured.
 * Return @expr != 0.
 * Usage: if (CANT_HAPPEN(...)) <recovery code>;
 */
#define CANT_HAPPEN(expr) ((expr) ? oops(#expr, __FILE__, __LINE__), 1 : 0)

/*
 * Report internal error.
 * Usage: CANT_REACH(); <recovery code>;
 */
#define CANT_REACH() oops(NULL, __FILE__, __LINE__)

extern void oops(char *, char *, int);
extern void (*oops_handler)(void);

/*
 * Assert constant expression @cond, return 1.
 * If @cond is zero, force a compilation error.
 */
#define BUILD_ASSERT_ONE(cond) \
    (sizeof(char[1 - 2 * !(cond)]))

/*
 * Assert constant expression @cond.
 * If @cond is zero, force a compilation error.
 */
#define BUILD_ASSERT(cond) \
    ((void)BUILD_ASSERT_ONE(cond))

void exit_nomem(void) ATTRIBUTE((noreturn));

	/* return codes from command routines */
#define RET_OK		0	/* command completed sucessfully */
#define RET_FAIL	1	/* command completed unsucessfully [?] */
#define RET_SYN		2	/* syntax error in command */

extern char *getstarg(char *input, char *prompt, char buf[]);
extern char *getstring(char *prompt, char buf[]);
extern char *ugetstring(char *prompt, char buf[]);

extern char *prbuf(char *format, ...)
    ATTRIBUTE((format (printf, 1, 2)));

#define AGREE_FREE	0
#define AGREE_PROPOSED	1
#define AGREE_SIGNED	2

#endif
