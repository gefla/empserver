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
 *  sys/time.h: POSIX emulation for WIN32
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2009
 */

#ifndef SYS_TIME_H
#define SYS_TIME_H

#ifdef _MSC_VER

/* include winsock2.h thru sys/socket.h to get struct timeval */
#include "sys/socket.h"

extern int gettimeofday(struct timeval *tv, void *tz);

#else  /* !_MSC_VER */
#include_next <sys/time.h>
#endif

#endif /* SYS_TIME_H */
