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
 *  getrusage.c: POSIX getrusage() emulation for Windows
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2011
 *     Ron Koenderink, 2011 (some of this code was taken from MinGW)
 */

#include <config.h>

#include <errno.h>
#include "sys/resource.h"

int
getrusage(int who, struct rusage *rus)
{
    if (who != RUSAGE_SELF && who != RUSAGE_CHILDREN) {
	errno = EINVAL;
	return -1;
    }

    if (who == RUSAGE_SELF) {
	HANDLE hProcess;
	FILETIME ftCreation, ftExit, ftUser, ftKernel;
	__int64 itmp;

	hProcess = GetCurrentProcess ();

	GetProcessTimes (hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser);

	itmp = (__int64)ftUser.dwLowDateTime +
	    ((__int64)ftUser.dwHighDateTime * 0x100000000LL);
	rus->ru_utime.tv_sec = (long)(itmp / 10000000U);
	rus->ru_utime.tv_usec = (long)((itmp % 10000000U) / 10.);

	itmp = (__int64)ftKernel.dwLowDateTime +
	    ((__int64)ftKernel.dwHighDateTime * 0x100000000LL);
	rus->ru_stime.tv_sec = (long)(itmp / 10000000U);
	rus->ru_stime.tv_usec = (long)((itmp % 10000000U) / 10.);
    } else {
	rus->ru_utime.tv_sec = rus->ru_utime.tv_usec = 0;
	rus->ru_stime.tv_sec = rus->ru_stime.tv_usec = 0;
    }
    return 0;
}
