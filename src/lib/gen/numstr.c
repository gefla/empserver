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
 *  numstr.c: Turn a number into a word
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#ifdef Rel4
#include <string.h>
#endif /* Rel4 */
#include "misc.h"
#include "gen.h"

s_char *
numstr(s_char *buf, int n)
{
	extern	s_char *numnames[];
	extern	s_char *tennames[];

	if (n > 100) {
		(void) strcpy(buf, "several");
	} else if (n < 0) {
		(void) strcpy(buf, "a negative number of");
	} else {
		if (n >= 20) {
			(void) strcpy(buf, tennames[n / 10]);
			if (n % 10) {
				(void) strcat(buf, "-");
				(void) strcat(buf, numnames[n % 10]);
			}
		} else {
			(void) strcpy(buf, numnames[n % 20]);
		}
	}
	return buf;
}

s_char *
effadv(int n)
{
	extern	s_char *effadv_list[];

	if (n < 0)
		n = 0;
	if (n >= 100)
		n = 99;
	return effadv_list[n/25];
}
