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
 *  lock.c: Lock a file
 * 
 *  Known contributors to this file:
 *      Doug Hay, 1998
 */

#ifdef aix
#define L_SET 0
#endif /* aix */

#include "misc.h"
#include "gen.h"
#include "common.h"
#include <fcntl.h>

#ifdef sys5
#include <unistd.h>
#endif
#if !defined(L_SET) && !defined(_WIN32)
#include <sys/file.h>
#endif

#ifdef aix
#define L_SET 0
#endif /* aix */

#if defined(_WIN32)
#include <sys/locking.h>

int
file_lock(int fd)
{
    if (_locking(fd, _LK_LOCK, 0) < 0) {
	logerror("file lock (fd %d) failed", fd);
	return 0;
    }
    return 1;
}

int
file_unlock(int fd)
{
    if (_locking(fd, _LK_UNLCK, 0) < 0) {
	logerror("file lock (fd %d) failed", fd);
	return 0;
    }
    return 1;
}

#else

#ifndef NOFLOCK

int flock();

int
file_lock(int fd)
{
    if (flock(fd, LOCK_EX) < 0) {
	logerror("file lock (fd %d) failed", fd);
	return 0;
    }
    return 1;
}

int
file_unlock(int fd)
{
    if (flock(fd, LOCK_UN) < 0) {
	logerror("file unlock (fd %d) failed", fd);
	return 0;
    }
    return 1;
}

#else

int
file_lock(int fd)
{
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = L_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = 0;
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
	logerror("file lock (fd %d) failed", fd);
	return 0;
    }
    return 1;
}

int
file_unlock(int fd)
{
    struct flock lock;

    lock.l_type = F_UNLCK;
    lock.l_whence = L_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = 0;
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
	logerror("file unlock (fd %d) failed", fd);
	return 0;
    }
    return 1;
}
#endif

#endif /* _WIN32 */
