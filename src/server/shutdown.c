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
 *  shutdown.c: Shuts down server.  Runs at low priority.
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "empio.h"
#include "empthread.h"
#include "nat.h"
#include "file.h"
#include <time.h>
#include "server.h"
#include "prototypes.h"

int shutdown_pending;

void
shutdown_init(void)
{
    shutdown_pending = 0;
}

void
shutdown_sequence(void *unused)
{
    struct natstr *god;
    struct tm *tm;
    time_t now;
    char header[100];

    if (shutdown_pending <= 0) {
	shutdown_pending = 0;
	logerror("shutdown called with 0 shutdown_pending");
	empth_exit();
	return;
    }
    god = getnatp(0);
    while (shutdown_pending > 0) {
	--shutdown_pending;
	time(&now);
	if (shutdown_pending <= 1440) {	/* one day */
	    tm = localtime(&now);
	    sprintf(header, "BROADCAST from %s @ %02d:%02d: ",
		    god->nat_cnam, tm->tm_hour, tm->tm_min);
	    if (!shutdown_pending) {
		pr_wall("%sServer shutting down NOW!\n", header);
		shutdwn(0);
	    } else if (shutdown_pending == 1) {
		pr_wall("%sServer shutting down in 1 minute!\n", header);
	    } else if (shutdown_pending <= 5) {
		pr_wall("%sServer shutting down in %d minutes!\n",
			header, shutdown_pending);
	    } else if (shutdown_pending <= 60
		       && shutdown_pending % 10 == 0) {
		pr_wall("%sThe server will be shutting down in %d minutes!\n",
			header, shutdown_pending);
	    } else if (shutdown_pending % 60 == 0) {
		pr_wall("%sThe server will be shutting down %d hours from now.\n",
			header, shutdown_pending / 60);
	    }
	}
	empth_sleep(now + 60);
    }
    empth_exit();
}
