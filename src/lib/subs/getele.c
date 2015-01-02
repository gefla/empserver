/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  getele.c: Read a telegram from a file or stdin and send it to a country
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2005-2009
 *
 */

#include <config.h>

#include "prototypes.h"
#include "tel.h"

static int tilde_escape(char *s);

/*
 * Read a telegram for RECIPIENT into BUF, in UTF-8.
 * BUF must have space for MAXTELSIZE+1 characters.
 * Return telegram length, or -1 on error.
 */
int
getele(char *recipient, char *buf)
{
    char *bp;
    size_t len;
    char buffer[MAXTELSIZE + 2]; /* UTF-8 */
    char left[16];

    pr("Enter telegram for %s\n", recipient);
    pr("undo last line with ~u, print with ~p, abort with ~q, end with .\n");
    bp = buf;
    *bp = 0;
    for (;;) {
	sprintf(left, "%4d left: ", (int)(buf + MAXTELSIZE - bp));
	if (uprmptrd(left, buffer, sizeof(buffer) - 2) <= 0)
	    return -1;
	switch (tilde_escape(buffer)) {
	case 'q':
	    return -1;
	case 'u':
	    if (bp == buf) {
		pr("No more lines to undo\n");
		continue;
	    }
	    for (bp -= 2; bp >= buf && *bp != '\n'; --bp) ;
	    *++bp = 0;
	    pr("Last line deleted.\n");
	    continue;
	case 'p':
	    pr("This is what you have written so far:\n");
	    uprnf(buf);
	    continue;
	}
	if (buffer[0] == '.' && buffer[1] == 0)
	    break;
	if (buffer[0] == '>')	/* forgery attempt? */
	    buffer[0] = '?';	/* foil it */
	len = strlen(buffer);
	if (CANT_HAPPEN(len > sizeof(buffer) - 2))
	    len = sizeof(buffer) - 2;
	buffer[len++] = '\n';
	buffer[len] = 0;
	if (bp + len > buf + MAXTELSIZE)
	    pr("Too long.  Try that last line again...\n");
	else {
	    memcpy(bp, buffer, len + 1);
	    bp += len;
	}
    }
    return bp - buf;
}

/*
 * If S is a `tilde escape', return its code, else 0.
 * A tilde escape is '~' followed by the code character.
 */
static int
tilde_escape(char *s)
{
    return s[0] == '~' && s[2] == 0 ? s[1] : 0;
}
