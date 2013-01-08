/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  gettimeofday.c: WIN32 equivalent for UNIX gettimeofday()
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2012
 *     Markus Armbruster, 2012
 */

#ifdef _MSC_VER

#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/time.h>

int
gettimeofday (struct timeval *tv, void *tz)
{
  struct _timeb timebuf;
  _ftime_s(&timebuf);
  tv->tv_sec = timebuf.time;
  tv->tv_usec = timebuf.millitm * 1000;

  return 0;
}

#endif /* _MSC_VER */
