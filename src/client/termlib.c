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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  termlib.c: Various termlib stuff
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 */

#include <config.h>

#ifndef _WIN32

#include <curses.h>
#include <stdio.h>
#include <term.h>
#include <unistd.h>
#include "misc.h"

static char *smso = 0;
static char *rmso = 0;

void
getsose(void)
{
    int err;

    if (!isatty(fileno(stdout)))
	return;

    if (setupterm(NULL, fileno(stdout), &err) != OK) {
	fprintf(stderr,
		"Can't setup terminal, check environment variable TERM\n");
	return;
    }

    smso = tigetstr("smso");
    rmso = tigetstr("rmso");
}

void
putso(void)
{
    if (smso)
	putp(smso);
}

void
putse(void)
{
    if (rmso)
	putp(rmso);
}

#endif /* !_WIN32 */
