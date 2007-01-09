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
 *  turn.c: Turn the game on, off, or set the login message.
 * 
 *  Known contributors to this file:
 *     Marc Olzheim, 2004
 */

#include <config.h>

#include <errno.h>
#if !defined(_WIN32)
#include <unistd.h>
#elif defined(__GNUC__)
#include <io.h>
#endif
#include "tel.h"
#include "commands.h"
#include "optlist.h"

/*
 * Enable / disable logins and set the message of the day.
 */
int
turn(void)
{
    FILE *fptr;
    struct telstr tgm;
    char *p;
    char buf[1024];
    char msgbuf[MAXTELSIZE + 1]; /* UTF-8 */
    char *msgfilepath;

    p = getstarg(player->argp[1], "on, off or motd? ", buf);
    if (!p)
	return RET_SYN;
    if (strcmp(p, "off") == 0) {
	msgfilepath = downfil;
    } else if (strcmp(p, "on") == 0) {
	pr("Removing no-login message and re-enabling logins.\n");
	if ((unlink(downfil) == -1) && (errno != ENOENT)) {
	    pr("Could not remove no-login file, logins still disabled.\n");
	    logerror("Could not remove no-login file (%s).\n", downfil);
	    return RET_SYS;
	}
	return RET_OK;
    } else {
	msgfilepath = motdfil;
    }

    if (msgfilepath == downfil)
	pr("Enter a message shown to countries trying to log in.\n");
    else
	pr("Enter a new message of the day.\n");

    time(&tgm.tel_date);
    tgm.tel_length = getele("The World", msgbuf);

    if (tgm.tel_length < 0) {
	pr("Ignored\n");
	if (msgfilepath == downfil)
	    pr("NOT disabling logins.\n");
	return RET_SYN;
    } else if (tgm.tel_length == 0) {
	if (msgfilepath == motdfil) {
	    pr("Removing exsting motd.\n");
	    if ((unlink(msgfilepath) == -1) && (errno != ENOENT)) {
		pr("Could not remove motd.\n");
		logerror("Could not remove motd file (%s).\n",
			 msgfilepath);
		return RET_SYS;
	    }
	    return RET_OK;
	} else
	    pr("Writing empty no-login message.\n");
    }

    fptr = fopen(msgfilepath, "wb");
    if (fptr == NULL) {
	pr("Something went wrong opening the message file.\n");
	logerror("Could not open message file (%s).\n", msgfilepath);
	return RET_SYS;
    }

    if (msgfilepath == downfil)
	pr("Logins disabled.\n");

    if ((fwrite(&tgm, sizeof(tgm), 1, fptr) != 1) ||
	(fwrite(msgbuf, tgm.tel_length, 1, fptr) != 1)) {
	fclose(fptr);
	pr("Something went wrong writing the message file.\n");
	logerror("Could not properly write message file (%s).\n",
	    msgfilepath);
	return RET_SYS;
    }
    if (fclose(fptr)) {
    	pr("Something went wrong closing the message.\n");
	logerror("Could not properly close message file (%s).\n",
	    msgfilepath);
	return RET_SYS;
    }

    pr("\n");

    return RET_OK;
}
