/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  shut.c: Shut down the server with a warning.
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#include <sys/types.h>
#include "misc.h"
#include "player.h"
#include "empthread.h"
#include "nat.h"
#include "file.h"
#include "commands.h"
#include "server.h"
#include "prototypes.h"

int
shut(void)
{
    int shutdown_minutes;
    int shutdown_was_pending;
    s_char buf[100];
    s_char newbuf[100];
    struct natstr *us;
    s_char *p;

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
	if (!(p = getstarg(player->argp[3], "Disable update [y]? ", buf))
	    || *p != 'n')
	    disa();

    shutdown_was_pending = shutdown_pending;
    shutdown_pending = shutdown_minutes + !!shutdown_minutes;
    buf[0] = '\0';
    if (shutdown_was_pending) {
	if (shutdown_minutes) {
	    sprintf(buf,
		    "The shutdown time has been changed to %d minutes",
		    shutdown_minutes);
	} else {
	    sprintf(buf, "The server shutdown has been cancelled");
	}
    } else if (shutdown_minutes) {
	pr("Shutdown sequence begun.\n");
	logerror("Shutdown sequence begun");
	empth_create(PP_SHUTDOWN, shutdown_sequence, (50 * 1024),
		     0, "shutdownSeq", "Counts down server shutdown", 0);
    }
    us = getnatp(player->cnum);
    if (buf[0]) {
	sprintf(newbuf, ": %s!", buf);
	sendmessage(us, 0, newbuf, 1);
	pr("%s.\n", buf);
	logerror(buf);
    }
    if (shutdown_minutes) {
	sprintf(buf, "The server will shut down in %d minutes",
		shutdown_minutes);
	sprintf(newbuf, ": %s!", buf);
	sendmessage(us, 0, newbuf, 1);
	pr("%s.\n", buf);
	logerror(buf);
    }
    return RET_OK;
}
