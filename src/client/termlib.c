/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#ifdef HAVE_CURSES_TERMINFO

#include <curses.h>
#include <stdio.h>
#include <term.h>
#include <unistd.h>
#include "misc.h"

static char *smso;
static char *rmso;

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

#endif /* HAVE_CURSES_TERMINFO */
