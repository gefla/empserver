/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  timestamp.c: Timestamp writer/maintainer thread
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1996
 *     Doug Hay, 1998
 */

#include <config.h>

#include <stdio.h>
#include <time.h>
#include "empthread.h"
#include "misc.h"
#include "optlist.h"
#include "prototypes.h"
#include "server.h"

/*ARGSUSED*/
void
mobility_check(void *unused)
{
    struct mob_acc_globals timestamps;
    time_t now;
    FILE *fp;

    while (1) {
	time(&now);
/*		logerror("Updating timestamp file at %s", ctime(&now));*/
	if ((fp = fopen(timestampfil, "rb+")) == NULL) {
	    logerror("Unable to edit timestamp file.");
	    continue;
	}
	rewind(fp);
	fread(&timestamps, sizeof(timestamps), 1, fp);
	timestamps.timestamp = now;
	rewind(fp);
	fwrite(&timestamps, sizeof(timestamps), 1, fp);
	fclose(fp);
	if (!gamehours(now)) {
	    if (updating_mob == 1) {
		update_all_mob();
		logerror("Turning off mobility updating (gamehours).");
		updating_mob = 0;
	    }
	} else if (updating_mob == 1 && now < timestamps.starttime) {
	    logerror("Turning off mobility updating at %s", ctime(&now));
	    update_all_mob();
	    updating_mob = 0;
	} else if (updating_mob == 0 && now >= timestamps.starttime) {
	    logerror("Turning on mobility updating at %s", ctime(&now));
	    update_all_mob();
	    updating_mob = 1;
	}
	now = now + 180;	/* Every 3 minutes */
	empth_sleep(now);

    }
    /*NOTREACHED*/
}

void
mobility_init(void)
{
    struct mob_acc_globals timestamps;
    time_t now;
    time_t lastsavedtime;
    FILE *fp;

    /* During downtime, we don't want mobility to accrue.  So, we look
       at the timestamp file, and determine how far forward to push
       mobility */

    time(&now);
    if ((fp = fopen(timestampfil, "rb+")) == NULL) {
	logerror("Unable to edit timestamp file.");
	/* FIXME safe to continue? */
    } else {
	rewind(fp);
	fread(&timestamps, sizeof(timestamps), 1, fp);
	lastsavedtime = timestamps.timestamp;
	timestamps.timestamp = now;
	rewind(fp);
	fwrite(&timestamps, sizeof(timestamps), 1, fp);
	fclose(fp);
    }
    time(&now);
    logerror("Adjusting timestamps at %s", ctime(&now));
    logerror("(was %s)", ctime(&lastsavedtime));
    /* Update the timestamps to this point in time */
    update_timestamps(lastsavedtime);
    time(&now);
    logerror("Done at %s", ctime(&now));

    if (now >= timestamps.starttime && gamehours(now)) {
	logerror("Turning on mobility updating.");
	updating_mob = 1;
    } else {
	logerror("Turning off mobility updating.");
	updating_mob = 0;
    }
}
