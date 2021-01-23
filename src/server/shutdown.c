/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  shutdown.c: Shuts down server.  Runs at low priority.
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Markus Armbruster, 2007-2013
 */

#include <config.h>

#include <time.h>
#include "empthread.h"
#include "prototypes.h"
#include "server.h"

int shutdown_pending;
static empth_t *shutdown_thread;

static void shutdown_sequence(void *unused);

/*
 * Initiate shutdown in @mins_from_now minutes.
 * If @mins_from_now is negative, cancel any pending shutdown instead.
 * Return -1 on error, zero when no shutdown was pending, positive
 * number when a pending shutdown was modified.
 */
int
shutdown_initiate(int mins_from_now)
{
    int old_pending = shutdown_pending;

    if (mins_from_now < 0) {
	if (shutdown_pending) {
	    shutdown_pending = 0;
	    pr_wall("The server shutdown has been cancelled!\n");
	}
	return old_pending;
    }

    shutdown_pending = mins_from_now + 1;

    if (shutdown_thread) {
	if (old_pending)
	    pr_wall("The shutdown time has been changed to %d minutes!\n",
		    mins_from_now);
	empth_wakeup(shutdown_thread);
    } else {
	shutdown_thread = empth_create(shutdown_sequence, 65536, 0,
				       "shutdownSeq", NULL);
	if (!shutdown_thread) {
	    shutdown_pending = 0;
	    return -1;
	}
    }

    return old_pending;
}

static void
shutdown_sequence(void *unused)
{
    time_t now;

    pr_wall("The server will shut down in %d minutes!\n",
	    shutdown_pending - 1);

    while (shutdown_pending > 0) {
	--shutdown_pending;
	time(&now);
	if (shutdown_pending <= 1440) {	/* one day */
	    if (shutdown_pending == 0) {
		shutdwn(0);
	    } else if (shutdown_pending == 1) {
		pr_wall("Server shutting down in 1 minute!\n");
	    } else if (shutdown_pending <= 5) {
		pr_wall("Server shutting down in %d minutes!\n",
			shutdown_pending);
	    } else if (shutdown_pending <= 60
		       && shutdown_pending % 10 == 0) {
		pr_wall("The server will be shutting down in %d minutes!\n",
			shutdown_pending);
	    } else if (shutdown_pending % 60 == 0) {
		pr_wall("The server will be shutting down %d hours from now.\n",
			shutdown_pending / 60);
	    }
	}
	/* FIXME error due to late wakeup accumulates */
	empth_sleep(now + 60);
    }

    shutdown_thread = NULL;
}
