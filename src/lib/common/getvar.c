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
 *  getvar.c: Routines for manipulating variable lists.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include <limits.h>
#include "misc.h"
#include "var.h"
#include "file.h"
#include "common.h"

int
getvec(int class, int *vec, s_char *sp, int ptype)
{
    u_short *src = ef_items(ptype, sp);
    int i;

    if (!src || class != VT_ITEM) {
	logerror("getvec: ptype %d has no vars", ptype);
	return 0;
    }

    for (i = 0; i <= I_MAX; ++i)
	vec[i] = src[i];

    return I_MAX;
}

int
putvec(int class, int *vec, s_char *sp, int ptype)
{
    u_short *dst = ef_items(ptype, sp);
    int i;

    if (!dst || class != VT_ITEM) {
	logerror("putvec: ptype %d has no vars", ptype);
	return 0;
    }

    for (i = 0; i <= I_MAX; ++i) {
	if (vec[i] < 0) {
	    logerror("putvec: item %d underflow", i);
	    dst[i] = 0;
	} else if (vec[i] > SHRT_MAX) {
	    logerror("putvec: item %d overflow", i);
	    dst[i] = SHRT_MAX;
	} else
	    dst[i] = vec[i];
    }

    return I_MAX;
}
