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
 *  bit.c: Allocate and search select bitfields
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#include "misc.h"
#include "bit.h"
#include "gen.h"		/* getfdtablesize etc. */

static int bit_nbytes;

bit_fdmask
bit_newfdmask(void)
{
    bit_fdmask mask;
    int nfile;

    if (bit_nbytes == 0) {
	nfile = getfdtablesize();
	bit_nbytes = (nfile + (BIT_BITSPERMASK - 1)) / BIT_NBBY;
    }
    mask = (bit_fdmask)malloc(bit_nbytes);
    (void)bit_zero(mask);
    return mask;
}

/*
 * zero the bitfield
 */
void
bit_zero(bit_fdmask bitp)
{
    bit_mask *mask;
    register int i;
    register int nwords;

    mask = bitp;
    nwords = bit_nbytes / sizeof(*mask);
    for (i = 0; i < nwords; i++)
	*mask++ = 0;
}

/*
 * zero the bitfield
 */
void
bit_not(bit_fdmask bitp)
{
    register bit_mask *mask;
    register int i;
    register int nwords;

    mask = bitp;
    nwords = bit_nbytes / sizeof(*mask);
    for (i = 0; i < nwords; i++, mask++)
	*mask = ~(*mask);
}

/*
 * zero the bitfield
 */
void
bit_copy(bit_fdmask bitsrc, bit_fdmask bitdst)
{
    register bit_mask *src;
    register bit_mask *dst;
    register int i;
    register int nwords;

    dst = bitdst;
    src = bitsrc;
    nwords = bit_nbytes / sizeof(*dst);
    for (i = 0; i < nwords; i++)
	*dst++ = *src++;
}

/*
 * zero the bitfield
 */
void
bit_or(bit_fdmask bitsrc, bit_fdmask bitdst)
{
    register bit_mask *src;
    register bit_mask *dst;
    register int i;
    register int nwords;

    nwords = bit_nbytes / sizeof(*dst);
    src = bitsrc;
    dst = bitdst;
    for (i = 0; i < nwords; i++)
	*dst++ |= *src++;
}

/*
 * zero the bitfield
 */
void
bit_or3(bit_fdmask bitsrc1, bit_fdmask bitsrc2, bit_fdmask bitdst)
{
    register bit_mask *src1;
    register bit_mask *src2;
    register bit_mask *dst;
    register int i;
    register int nwords;

    src1 = bitsrc1;
    src2 = bitsrc2;
    dst = bitdst;
    nwords = bit_nbytes / sizeof(*dst);
    for (i = 0; i < nwords; i++)
	*dst++ = *src1++ | *src2++;
}

/*
 * zero the bitfield
 */
void
bit_and(bit_fdmask bitsrc, bit_fdmask bitdst)
{
    register bit_mask *src;
    register bit_mask *dst;
    register int i;
    register int nwords;

    nwords = bit_nbytes / sizeof(*src);
    src = bitsrc;
    dst = bitdst;
    for (i = 0; i < nwords; i++)
	*dst++ &= *src++;
}

/*
 * zero the bitfield
 */
void
bit_and3(bit_fdmask bitsrc1, bit_fdmask bitsrc2, bit_fdmask bitdst)
{
    register bit_mask *src1;
    register bit_mask *src2;
    register bit_mask *dst;
    register int i;
    register int nwords;

    src1 = bitsrc1;
    src2 = bitsrc2;
    dst = bitdst;
    nwords = bit_nbytes / sizeof(*dst);
    for (i = 0; i < nwords; i++)
	*dst++ = *src1++ & *src2++;
}

/*
 * Return first bit set in fd mask.
 * speedy version, not using BIT_ISSETB()
 */
int
bit_fd(bit_fdmask bitp)
{
    register bit_mask *mask;
    register unsigned int j;
    register bit_mask m;
    register int i;
    int nwords;

    mask = bitp;
    nwords = bit_nbytes / sizeof(m);
    for (i = 0; i < nwords; i++, mask++) {
	if ((m = *mask) == 0)
	    continue;
	for (j = 0; j < BIT_BITSPERMASK; j++) {
	    if (m & bit(j))
		return i * BIT_BITSPERMASK + j;
	}
	/*NOTREACHED*/
    }
    return -1;
}
