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
 *  turn.c: Turn the game on, off, or set the login message.
 *
 *  Known contributors to this file:
 *     Marc Olzheim, 2004
 *     Ron Koenderink, 2005-2007
 *     Markus Armbruster, 2005-2009
 */

#include <config.h>

#include <errno.h>
#include <unistd.h>
#include "game.h"
#include "tel.h"
#include "commands.h"
#include "optlist.h"

int
turn(void)
{
    FILE *fptr;
    struct telstr tgm;
    char *p;
    char buf[1024];
    char msgbuf[MAXTELSIZE + 1]; /* UTF-8 */
    char *msgfilepath;
    int len, down;

    p = getstarg(player->argp[1], "on, off or motd? ", buf);
    if (!p)
	return RET_SYN;
    if (strcmp(p, "off") == 0) {
	msgfilepath = downfil;
	pr("Enter a message explaining the down time.\n");
	len = getele("The World", msgbuf);
	down = 1;
    } else if (strcmp(p, "on") == 0) {
	msgfilepath = downfil;
	len = 0;
	down = 0;
    } else {
	msgfilepath = motdfil;
	pr("Enter a new message of the day.\n");
	len = getele("The World", msgbuf);
	down = -1;
    }

    if (len < 0)
	return RET_FAIL;
    if (len == 0) {
	if (unlink(msgfilepath) < 0 && (errno != ENOENT)) {
	    pr("Could not remove %s file.\n", msgfilepath);
	    logerror("Could not remove %s file (%s)",
		     msgfilepath, strerror(errno));
	    return RET_FAIL;
	}
    } else {
	fptr = fopen(msgfilepath, "wb");
	if (fptr == NULL) {
	    pr("Something went wrong opening the message file.\n");
	    logerror("Could not open message file (%s).\n", msgfilepath);
	    return RET_FAIL;
	}
	memset(&tgm, 0, sizeof(tgm));
	time(&tgm.tel_date);
	tgm.tel_length = len;
	if (fwrite(&tgm, sizeof(tgm), 1, fptr) != 1 ||
	    fwrite(msgbuf, 1, tgm.tel_length, fptr) != tgm.tel_length) {
	    fclose(fptr);
	    pr("Something went wrong writing the message file.\n");
	    logerror("Could not properly write message file (%s).\n",
		     msgfilepath);
	    return RET_FAIL;
	}
	if (fclose(fptr)) {
	    pr("Something went wrong closing the message.\n");
	    logerror("Could not properly close message file (%s).\n",
		     msgfilepath);
	    return RET_FAIL;
	}
    }

    if (down >= 0)
	game_ctrl_play(!down);

    /* "The game is down" will be printed automatically */
    return RET_OK;
}
