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
 *  strscan.c: scan a string for any occurence of another string
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#ifdef Rel4
#include <string.h>
#endif /* Rel4 */
#include <stdio.h>
#include "misc.h"

int
strscan(s_char *target, s_char *string)
{
	int	i,n,delta;

	if ((string == NULL) || (target == NULL))
		return(1);
	n = strlen(target);
	delta = strlen(string);
	if (delta < n)
		return(1);
	delta -= n;
	for (i = 0; i <= delta; i++) {
		if (!strncmp(target,&(string[i]),n))
			return(0);
	}
	return(1);
}
