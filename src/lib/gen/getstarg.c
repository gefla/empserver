/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  getstarg.c: Get a string argument (ask if not there)
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "gen.h"

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
    if (input == 0 || *input == 0) {
	if (getstring(prompt, buf) == 0)
	    return 0;
    } else {
	strcpy(buf, input);
    }
    return buf;
}
