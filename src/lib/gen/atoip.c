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
 *  atoip.c: Convert from decimal string to integer.
 * 
 *  Known contributors to this file:
 *     
 */

#include "gen.h"

int
atoip(s_char **ptrptr)
{
    register int num;
    register s_char *cp;
    register int neg;

    if (ptrptr == 0 || *ptrptr == 0)
	return 0;
    cp = *ptrptr;
    num = 0;
    neg = 0;
  loop:
    while (*cp == ' ' || *cp == '\t')
	cp++;
    if (*cp == '-') {
	neg++;
	cp++;
	goto loop;
    }
    while (*cp >= '0' && *cp <= '9')
	num = num * 10 + *cp++ - '0';
    *ptrptr = cp;
    return (neg ? -num : num);
}
