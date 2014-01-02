/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  getstring.c: get string, printing a prompt if there is one
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "prototypes.h"

/*
 * Print sub-prompt PROMPT, receive a line of input into BUF[1024].
 * Return BUF on success, else NULL.
 */
char *
getstring(char *prompt, char *buf)
{
    *buf = '\0';
    if (prmptrd(prompt, buf, 1024) < 0)
	return NULL;
    return buf;
}

/*
 * Print sub-prompt PROMPT, receive a line of UTF-8 input into BUF[1024].
 * Return BUF on success, else NULL.
 */
char *
ugetstring(char *prompt, char *buf)
{
    *buf = '\0';
    if (uprmptrd(prompt, buf, 1024) < 0)
	return NULL;
    return buf;
}
