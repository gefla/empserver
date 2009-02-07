/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  shutdown.c: Shuts down server.  Runs at low priority.
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Markus Armbruster, 2007-2008
 */

#include <config.h>

#include <time.h>
#include "empthread.h"
#include "file.h"
#include "nat.h"
#include "prototypes.h"
#include "server.h"

int shutdown_pending;
static empth_t *shutdown_thread;

static void shutdown_sequence(void *unused);

/*
 * Initiate shutdown in MINS_FROM_NOW minutes.
 * If MINS_FROM_NOW is negative, cancel any pending shutdown instead.
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
	shutdown_thread = empth_create(shutdown_sequence, 50 * 1024, 0,
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
