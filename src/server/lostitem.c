/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  lostitem.c:  This deletes the old lost items
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1997
 */

#include "options.h"
#include "misc.h"
#include "player.h"
#include "keyword.h"
#include "empthread.h"
#include "file.h"
#include "lost.h"
#include <stdio.h>
#include "prototypes.h"

/*ARGSUSED*/
void
delete_lostitems(void *argv)
{
    extern int lost_items_timeout;
    time_t now;
    struct loststr lost;
    int n;
    int ncnt;

    while (1) {
	time(&now);
/*	logerror("Deleting lost items at %s", ctime(&now));*/
	ncnt = 0;
	for (n = 0; getlost(n, &lost); n++) {
	    if (!lost.lost_owner)
		continue;
	    if (lost.lost_timestamp > (now - lost_items_timeout))
		continue;
	    lost.lost_owner = 0;
	    lost.lost_timestamp = 0;
	    putlost(n, &lost);
	    ncnt++;
	}
/*	logerror("Deleted %d lost items", ncnt, ctime(&now));*/
	now = now + 900;	/* Every 15 minutes */
	empth_sleep(now);
    }
    /*NOTREACHED*/
}
