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
 *  getstarg.c: Get a string argument (ask if not there)
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include <string.h>
#include "misc.h"
#include "prototypes.h"

/*
 * Get string argument.
 * If INPUT is not empty, use it, else prompt for more input using PROMPT.
 * Copy input to BUF[1024].
 * Return BUF on success, else NULL.
 */
char *
getstarg(char *input, char *prompt, char *buf)
{
    *buf = '\0';
    if (!input || !*input) {
	if (!getstring(prompt, buf))
	    return NULL;
    } else {
	strcpy(buf, input);
	make_stale_if_command_arg(input);
    }
    return buf;
}
