/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  turn.c: Turn the game on, off, or set the login message.
 * 
 *  Known contributors to this file:
 *    
 */

#include "misc.h"
#include "player.h"
#include "tel.h"
#include "commands.h"
#include "optlist.h"

#include <errno.h>
#include <fcntl.h>

/*
 * Enable / disable logins and set the message of the day.
 */
int
turn(void)
{
    int fd;
    struct telstr tgm;
    char *p;
    char buf[MAXTELSIZE];
    char *msgfilepath;

    p = getstarg(player->argp[1], "on, off or motd? ", buf);
    if (!p)
	return RET_SYN;
    if (strcmp(p, "off") == 0) {
	msgfilepath = downfil;
    } else if (strcmp(p, "on") == 0) {
	pr("Removing no-login message and re-enabling logins.\n");
	if ((unlink(downfil) == -1) && (errno != ENOENT))
	{
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
    tgm.tel_length = getele("The World", buf);

    if (tgm.tel_length < 0)
    {
	pr("Ignored\n");
	if (msgfilepath == downfil)
	    pr("NOT disabling logins.\n");
	return RET_SYN;
    } else if (tgm.tel_length == 0)
    {
	if (msgfilepath == motdfil)
	{
	    pr("Removing exsting motd.\n");
	    if ((unlink(msgfilepath) == -1) && (errno != ENOENT))
	    {
		pr("Could not remove motd.\n");
		logerror("Could not remove motd file (%s).\n", msgfilepath);
		return RET_SYS;
	    }
	    return RET_OK;
	} else
	    pr("Writing empty no-login message.\n");
    }

#if !defined(_WIN32)
    fd = open(msgfilepath, O_RDWR | O_CREAT | O_TRUNC, 0660);
#else
    fd = open(msgfilepath, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0660);
#endif
    if (fd == -1)
    {
	pr("Something went wrong opening the message file.\n");
	logerror("Could not open message file (%s).\n", msgfilepath);
	return RET_SYS;
    }

    if (msgfilepath == downfil)
	pr("Logins disabled.\n");

    if ((write(fd, &tgm, sizeof(tgm)) < sizeof(tgm)) ||
	(write(fd, buf, tgm.tel_length) < tgm.tel_length) ||
	(close(fd) == -1))
    {
	pr("Something went wrong writing the message file.\n");
	logerror("Could not properly write message file (%s).\n",
	    msgfilepath);
	return RET_SYS;
    }
    pr("\n");

    return RET_OK;
}
