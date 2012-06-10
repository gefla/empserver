/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sys/resource.h: POSIX resource emulation for WIN32
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2011
 */

#ifndef SYS_RESOURCE_H
#define SYS_RESOURCE_H

#include "sys/time.h"

#define RUSAGE_SELF 0
#define RUSAGE_CHILDREN 1

struct rusage {
    struct timeval ru_utime;
    struct timeval ru_stime;
};

extern int getrusage(int, struct rusage *);

#endif /* SYS_RESOURCE_H */
