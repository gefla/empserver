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
 *  mobu.c: Adjust mobility updating
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1996
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "deity.h"
#include "path.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"
#include "prototypes.h"

#include <stdio.h>

int
mobupdate(void)
{
	FILE    *fp;
	long    minites;
	extern  int updating_mob;
	struct  mob_acc_globals timestamps;
	long    now;
	extern  s_char *timestampfil;
	
	if (!opt_MOB_ACCESS) {
		pr("Command invalid - MOB_ACCESS is not enabled.\n");
		return RET_FAIL;
	}
	if (!player->argp[1])
		return RET_SYN;
	if (*player->argp[1] == 'c')
		minites = -1;
	else
		minites = atol(player->argp[1]) * 60;
	time(&now);
#if !defined(_WIN32)
	if ((fp = fopen(timestampfil, "r+")) == NULL) {
#else
	if ((fp = fopen(timestampfil, "r+b")) == NULL) {
#endif
		logerror("Unable to edit timestamp file.");
	} else {
		rewind(fp);
		fread(&timestamps, sizeof(timestamps), 1, fp);
		if (minites < 0) {
			fclose(fp);
			if (updating_mob)
				pr("Mobility updating is enabled.");
			else {
				pr("Mobility updating will come back on around %s",
				   ctime(&timestamps.starttime));
				pr("within 3 minutes, depending on when the server checks.");
			}
			return 0;
		}
		timestamps.timestamp = now;
		timestamps.starttime = now + minites;
		rewind(fp);
		fwrite(&timestamps, sizeof(timestamps), 1, fp);
		fclose(fp);
		if (now >= timestamps.starttime) {
			pr("Turning on mobility updating.");
			update_all_mob();
			updating_mob = 1;
		} else if (updating_mob == 1) {
			pr("Turning off mobility updating.\n\r");
			pr("Mobility updating will come back on around %s",
			   ctime(&timestamps.starttime));
			pr("within 3 minutes, depending on when the server checks.");
			update_all_mob();
			updating_mob = 0;
		} else if (updating_mob == 0) {
			pr("Mobility updating is already off.\n\r");
			pr("Mobility updating will come back on around %s",
			   ctime(&timestamps.starttime));
			pr("within 3 minutes, depending on when the server checks.");
		}
	}
	
	return 0;
}
