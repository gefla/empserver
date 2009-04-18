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
 *  sysdep_w32.c: system dependent functions for WIN32 environments
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 *     Markus Armbruster, 2009
 */

#ifdef _WIN32
#include <config.h>

#include <stdlib.h>
#include <windows.h>
#include "misc.h"
#include "sys/socket.h"

/*
 * Get user name in the WIN32 environment
 */
struct passwd *
w32_getpw(void)
{
    static char unamebuf[128];
    static struct passwd pwd;
    DWORD unamesize;

    unamesize = sizeof(unamebuf);
    if (GetUserName(unamebuf, &unamesize)) {
	pwd.pw_name = unamebuf;
	if (unamesize == 0 || strlen(unamebuf) == 0)
	    pwd.pw_name = "nobody";
    } else
	pwd.pw_name = "nobody";
    return &pwd;
}

void
w32_sysdep_init(void)
{
    int err;

    /*
     * stdout is unbuffered under Windows if connected to a character
     * device, and putchar() screws up when printing multibyte strings
     * bytewise to an unbuffered stream.  Switch stdout to line-
     * buffered mode.  Unfortunately, ISO C allows implementations to
     * screw that up, and of course Windows does.  Manual flushing
     * after each prompt is required.
     */
    setvbuf(stdout, NULL, _IOLBF, 4096);

    err = w32_socket_init();
    if (err != 0) {
	printf("WSAStartup Failed, error code %d\n", err);
	exit(1);
    }
}

#endif /* _WIN32 */
