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
 *  hpux.c: hpux dependencies
 * 
 *  Known contributors to this file:
 *     
 */

#ifdef hpux
#include <memory.h>
#define NULL 0
#include "misc.h"

s_char *
rindex(sp, c)
	register s_char *sp;
	register int c;
{
	register s_char *r;

	r = NULL;
	do {
		if (*sp == c)
			r = sp;
	} while (*sp++);
	return r;
}

s_char *
index(sp, c)
	register s_char *sp;
	register int c;
{
	do {
		if (*sp == c)
			return (sp);
	} while (*sp++);
	return NULL;
}

int
ffs(marg)
	register unsigned marg;
{
	register unsigned bval;
	register int i;

	if (marg == 0)
		return 0;
	for (bval=1, i=1; i <= 32; i++, bval <<= 1)
		if (marg & bval)
			return i;
	return 0;
}

bzero(ptr, len)
	s_char	*ptr;
	int	len;
{
	memset(ptr, 0, len);
}

bcopy(src, dst, len)
	s_char	*src;
	s_char	*dst;
	int	len;
{
	memcpy(dst, src, len);
}
#endif
