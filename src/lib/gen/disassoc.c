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
 *  disassoc.c: Fork and close
 * 
 *  Known contributors to this file:
 *     Doug Hay, 1998
 */

/*
 * boilerplate daemon code; disassociate from
 * the current tty by forking, closing all file
 * descriptors, opening slash, and ioctl-ing
 * TIOCNOTTY
 */

#include <sys/types.h>
#if !defined(_WIN32)
#include <sys/ioctl.h>
#include <unistd.h>		/* fork close dup2 */
#endif
#include <fcntl.h>
#include "gen.h"

void
disassoc(void)
{
#if !defined(_WIN32)
    int i;

    if (fork() != 0)
	exit(0);
    for (i = 0; i < 10; i++)
	(void)close(i);
    (void)open("/", O_RDONLY, 0);
    (void)dup2(0, 1);
    (void)dup2(0, 2);
#if defined hpux || defined Rel4
    setsid();
#else
    i = open("/dev/tty", O_RDWR, 0);
    if (i > 0) {
	(void)ioctl(i, TIOCNOTTY, 0);
	(void)close(i);
    }
#endif
#endif
}
