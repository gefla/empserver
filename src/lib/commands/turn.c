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
 *  turn.c: Turn the game on, off, or set the login message.
 * 
 *  Known contributors to this file:
 *    
 */

#include "misc.h"
#include "player.h"
#include "tel.h"
#include "commands.h"

#include <fcntl.h>

int
turn(void)
{
    extern s_char *upfil;
    extern s_char *downfil;
    int fd;
    struct telstr tgm;
    register s_char *p;
    s_char buf[MAXTELSIZE];

    p = getstarg(player->argp[1], "on, off or message? ", buf);
    if (!p)
	return RET_SYN;
    if (strcmp(p, "off") == 0) {
	(void)unlink(upfil);
#if !defined(_WIN32)
	fd = open(downfil, O_RDWR | O_CREAT | O_TRUNC, 0660);
#else
	fd = open(downfil, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0660);
#endif
	pr("off ");
    } else if (strcmp(p, "on") == 0) {
	(void)unlink(downfil);
#if !defined(_WIN32)
	fd = open(upfil, O_RDWR | O_CREAT | O_TRUNC, 0660);
#else
	fd = open(upfil, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0660);
#endif
	pr("on ");
    } else {
#if !defined(_WIN32)
	fd = open(upfil, O_RDWR | O_CREAT | O_TRUNC, 0660);
#else
	fd = open(upfil, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0660);
#endif
	pr("motd ");
    }
    (void)time(&tgm.tel_date);
    if ((tgm.tel_length = getele("The World", buf)) <= 0) {
	pr("Ignored\n");
	close(fd);
	return RET_SYN;
    }
    (void)write(fd, (s_char *)&tgm, sizeof(tgm));
    (void)write(fd, buf, tgm.tel_length);
    (void)close(fd);
    pr("\n");
    return RET_OK;
}
