/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  shut.c: Shut down the server with a warning.
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Markus Armbruster, 2007-2008
 */

#include <config.h>

#include "commands.h"
#include "empthread.h"
#include "game.h"
#include "server.h"

int
shut(void)
{
    int shutdown_minutes;
    int shutdown_was_pending;
    char buf[1024];
    char *p;

    shutdown_minutes =
	onearg(player->argp[1],
	       "Time until shutdown in minutes (-1 to abort shutdown sequence)? ");
    if (player->aborted)
	return RET_SYN;
    if (!updates_disabled()) {
	p = getstarg(player->argp[2], "Disable update [y]? ", buf);
	if (!p)
	    return RET_SYN;
	if (*p != 'n')
	    disa();
    }

    shutdown_was_pending = shutdown_initiate(shutdown_minutes);
    if (shutdown_was_pending < 0)
	return RET_FAIL;

    if (shutdown_minutes >= 0) {
	if (shutdown_was_pending)
	    pr("The shutdown time has been changed to %d minutes!\n",
	       shutdown_minutes);
	else
	    pr("Shutdown sequence begun.\n");
    } else {
	if (shutdown_was_pending)
	    pr("The server shutdown has been cancelled!\n");
	else {
	    pr("No shutdown to abort\n");
	    return RET_FAIL;
	}
    }
    return RET_OK;
}
