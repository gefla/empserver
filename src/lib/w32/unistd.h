/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  unistd.h: POSIX emulation for Windows
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 *     Markus Armbruster, 2007-2013
 */

/*
 * We can provide POSIX headers that don't exist in Windows, but we
 * can't augment existing headers.  Some stuff that should be in such
 * headers ends up here, and some in w32misc.h.  Can't be helped.
 */

#ifndef UNISTD_H
#define UNISTD_H

/*
 * We override system functions by defining them as macros.  This
 * breaks if the system's declaration is included later.  Include them
 * here.  Major name space pollution, can't be helped.
 */
#include <direct.h>
#include <getopt.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include "w32types.h"

/*
 * w32file.c
 */
/* Should be in sys/stat.h */
#define mkdir(dir, perm)    w32_mkdir((dir), (perm))
extern int w32_mkdir(const char *dirname, mode_t perm);

/* Should be in sys/stat.h */
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

/* Should be in fcntl.h */
#define O_NONBLOCK  1

#define F_GETFL	    1
#define F_SETFL	    2

extern int fcntl(int fd, int cmd, ...);

/* Stuff that actually belongs here */
#define close(fd) w32_close_function((fd))
extern int (*w32_close_function)(int);
#define ftruncate(fd, length) _chsize((fd), (length))
#define read(fd, buf, sz) w32_read_function((fd), (buf), (sz))
extern int (*w32_read_function)(int, void *, unsigned);
#define write(fd, buf, sz) w32_write_function((fd), (buf), (sz))
extern int (*w32_write_function)(int, const void *, unsigned);

#endif /* UNISTD_H */
