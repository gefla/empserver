/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 */

#include <config.h>

#include "commands.h"
#include "empthread.h"
#include "server.h"

int
shut(void)
{
    int shutdown_minutes;
    int shutdown_was_pending;
    char buf[1024];
    char msgbuf[100];
    struct natstr *us;
    char *p;

    if (update_pending) {
	pr("Update is pending\n");
	return RET_FAIL;
    }
    shutdown_minutes =
	onearg(player->argp[1],
	       "Time until shutdown in minutes (0 to abort shutdown sequence)? ");
    if (shutdown_minutes < 0)
	return RET_SYN;
    if (!updates_disabled())
	if (!(p = getstarg(player->argp[2], "Disable update [y]? ", buf))
	    || *p != 'n')
	    disa();

    shutdown_was_pending = shutdown_pending;
    shutdown_pending = shutdown_minutes + !!shutdown_minutes;
    msgbuf[0] = '\0';
    if (shutdown_was_pending) {
	if (shutdown_minutes) {
	    sprintf(msgbuf,
		    ": The shutdown time has been changed to %d minutes!",
		    shutdown_minutes);
	} else {
	    sprintf(msgbuf, ": The server shutdown has been cancelled!");
	}
    } else if (shutdown_minutes) {
	pr("Shutdown sequence begun.\n");
	logerror("Shutdown sequence begun");
	empth_create(PP_SHUTDOWN, shutdown_sequence, (50 * 1024),
		     0, "shutdownSeq", "Counts down server shutdown", 0);
    }
    us = getnatp(player->cnum);
    if (msgbuf[0]) {
	sendmessage(us, 0, msgbuf, 1);
	pr("%s\n", msgbuf + 2);
	logerror("%s", msgbuf + 2);
    }
    if (shutdown_minutes) {
	sprintf(msgbuf, ": The server will shut down in %d minutes!",
		shutdown_minutes);
	sendmessage(us, 0, msgbuf, 1);
	pr("%s\n", msgbuf + 2);
	logerror("%s", msgbuf + 2);
    }
    return RET_OK;
}
