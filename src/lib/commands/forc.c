/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  force.c: Force an update to occur (deity)
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 */

#include <sys/types.h>
#include "misc.h"
#include "player.h"
#include "empthread.h"
#include "commands.h"
#include "server.h"

int
force(void)
{
    int seconds;
    time_t now;

    if (update_pending) {
	pr("Update is pending\n");
	return RET_FAIL;
    }
    if (shutdown_pending) {
	pr("Shutdown is pending\n");
	return RET_FAIL;
    }
    if (updates_disabled()) {
	pr("Updates are disabled\n");
	return RET_FAIL;
    }
    seconds = onearg(player->argp[1], "Time until update [in seconds]? ");
    if (seconds < 0)
	return RET_FAIL;
    if (seconds) {
	time(&now);
	pr("Waiting %d seconds...\n", seconds);
	empth_sleep(now + seconds);
    }
    pr("Scheduling update now\n");
    empth_sem_signal(update_sem);
    return RET_OK;
}
