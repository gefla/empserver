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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
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
#include "misc.h"
#include "player.h"
#include "empio.h"
#include "empthread.h"
#include "prototypes.h"
#include "optlist.h"
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
	/*if (update_pending) */
	/*continue; */
	for (p = player_next(0); p != 0; p = player_next(p)) {
	    if (p->state == PS_SHUTDOWN) {
		/* no more mr. nice guy */
		p->state = PS_KILL;
		p->aborted++;
		empth_terminate(p->proc);
		p = player_delete(p);
		continue;
	    }
	    if (p->curup + max_idle * 60 < now) {
		p->state = PS_SHUTDOWN;
		/* giving control to another thread while
		 * in the middle of walking player list is
		 * not a good idea at all. Sasha */
		p->aborted++;
		pr_flash(p, "idle connection terminated\n");
		empth_wakeup(p->proc);
		/* go to sleep because player thread
		   could vandalize a player list */

		break;
	    }
	}
    }
    /*NOTREACHED*/
}
