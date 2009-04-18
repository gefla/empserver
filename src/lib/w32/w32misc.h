/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  w32misc.h: POSIX emulation for WIN32, miscellaneous stuff
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 */

/*
 * Stuff that cannot be put in the "correct" include files.  More of
 * that in unistd.h.
 */

#ifndef W32MISC_H
#define W32MISC_H

#include <stddef.h>

#ifdef _MSC_VER
/* integral mismatch, due to misuse of sector short */
#pragma warning (disable : 4761 )

/* strings.h */
#define strncasecmp(s1, s2, s3) _strnicmp((s1), (s2), (s3))
#endif /* _MSC_VER */

/* errno.h */
#define EWOULDBLOCK EAGAIN

/* fcntl.h */
#ifdef _MSC_VER
#define O_ACCMODE (_O_RDONLY|_O_WRONLY|_O_RDWR)
#endif

/* stdio.h */
#ifdef _MSC_VER
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

/* stdlib.h */
extern char *initstate(unsigned seed, char *state, size_t size);
extern long random(void);
extern char *setstate(char *state);
extern void srandom(unsigned seed);

/* sys/types.h */
#ifdef _MSC_VER
typedef unsigned short mode_t;
typedef long off_t;
typedef int pid_t;
typedef int __w64 ssize_t;
#endif

/* time.h */
struct tm;
extern char *strptime(const char *buf, const char *fmt, struct tm *tm);

#endif /* W32MISC_H */
