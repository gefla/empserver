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
 *  disassoc.c: Boilerplate daemonization code
 *
 *  Known contributors to this file:
 *     Doug Hay, 1998
 *     Markus Armbruster, 2005
 */

/*
 * See W. Richard Stevens: UNIX Network Programming, Vol. 1
 */

#include <config.h>

#ifndef _WIN32
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "prototypes.h"

int
disassoc(void)
{
    pid_t pid;
    int i;

    if ((pid = fork()) < 0)
	return -1;
    else if (pid)
	_exit(0);		/* parent */

    /* Become session leader of new session, lose controlling tty */
    if (setsid() < 0)
	return -1;

    /* Lose session leader status, so we can't acquire a controlling tty */
    if ((pid = fork()) < 0)
	return -1;
    else if (pid)
	_exit(0);		/* parent */
    /* Note: no controlling tty, therefore no SIGHUP sent */

    /* datadir is working directory, that's fine */

    /* We opened a bunch of files already, so just close 0..2 */
    for (i = 0; i < 3; i++)
	close(i);

    /* Library code may use 0..2; make sure that's safe */
    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);

    return 0;
}
#endif
