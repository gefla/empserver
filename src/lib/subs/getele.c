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
 *  getele.c: Read a telegram from a file or stdin and send it to a country
 * 
 *  Known contributors to this file:
 *    
 */

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "tel.h"
#include "prototypes.h"

static int tilde_escape(s_char *s, s_char c);

int
getele(s_char *nation, s_char *buf)
{
	register s_char *bp;
	register int len;
	register int c;
	s_char	buffer[MAXTELSIZE + 2];
	s_char	left[MAXTELSIZE + 2];

	pr("Enter telegram for %s\n", nation);
	pr("undo last line with ~u, print with ~p, abort with ~q, end with ^D or .\n");
	bp = buf;
	while (!player->aborted) {
		sprintf(left, "%4d left: ", (int)(buf + MAXTELSIZE - bp));
		buffer[0] = 0;
		if (prmptrd(left, buffer, MAXTELSIZE-2) <= 0)
			break;
		if (tilde_escape(buffer, 'q'))
			return -1;
		if (tilde_escape(buffer, 'u')) {
			if (bp == buf) {
				pr("No more lines to undo\n");
				continue;
			}
			pr("Last line deleted.\n");
			for (bp -= 2; bp > buf && *bp != '\n'; --bp);
			if (bp > buf)
				*(++bp) = 0;
			else
				bp = buf;
			continue;
		}
		if (tilde_escape(buffer, 'p')) {
			pr("This is what you have written so far:\n");
			pr("%s", buf);
			continue;
		}
		if (buffer[0] == '.' && ((buffer[1] == 0) ||
			(buffer[1] == '\n') || (buffer[1] == '\r')))
			break;
		len = strlen(buffer);
		buffer[len++] = '\n';
		buffer[len] = 0;
		if (len + (bp - buf) > MAXTELSIZE)
			pr("Too long.  Try that last line again...\n");
		else {
			if (buffer[0] == '>')		/* forgery attempt? */
				buffer[0] = '?';	/* foil it */
			(void) strcpy(bp, buffer);
			bp += len;
		}
	}
	if (player->aborted)
		return -1;
	len = bp - buf;
	buf[len] = 0;
	/* Get rid of non-ASCII and control characters.  */
	for (bp = buf; 0 != (c = *bp); bp++) {
		if (isascii(c) && (isprint(c) || isspace(c)))
			continue;
		*bp = '?';
	}
	return len;
}

static int
tilde_escape(s_char *s, s_char c)
{
	if (s[0] == '~' && s[1] == c && 
	    ((s[2] == 0) || (s[2] == '\n') || (s[2] == '\r')))
		return 1;
	return 0;
}

