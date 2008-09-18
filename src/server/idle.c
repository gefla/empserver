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
 *  idle.c: Stamps out idle players.  Runs at low priority
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 */

#include <config.h>

#include <time.h>
#include "empthread.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "server.h"

/*ARGSUSED*/
void
player_kill_idle(void *unused)
{
    struct player *p;
    time_t now;

    time(&now);
    while (1) {
	empth_sleep(now + 60);
	time(&now);
	for (p = player_next(0); p != 0; p = player_next(p)) {
	    if (p->state == PS_SHUTDOWN) {
		/*
		 * Player thread hung or just aborted by update or
		 * shutdown, we can't tell.
		 */
		continue;
	    }
	    if (p->curup + max_idle * 60 < now) {
		p->state = PS_SHUTDOWN;
		p->aborted++;
		pr_flash(p, "idle connection terminated\n");
		empth_wakeup(p->proc);
	    }
	}
    }
    /*NOTREACHED*/
}
