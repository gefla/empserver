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
 *  iom.c: implements the io mask routines
 * 
 *  Known contributors to this file:
 *     
 */

#include <stdlib.h>		/* malloc */
#include <errno.h>
#include "misc.h"
#include "bit.h"
#include "empio.h"
#include "io_mask.h"

struct io_mask *
iom_create(int what)
{
    struct io_mask *imp;

    imp = (struct io_mask *)malloc(sizeof(*imp));
    if (what & IO_READ) {
	imp->readmask = bit_newfdmask();
	imp->user_readmask = bit_newfdmask();
    } else {
	imp->readmask = 0;
	imp->user_readmask = 0;
    }
    if (what & IO_WRITE) {
	imp->writemask = bit_newfdmask();
	imp->user_writemask = bit_newfdmask();
    } else {
	imp->writemask = 0;
	imp->user_writemask = 0;
    }
    imp->what = what;
    imp->maxfd = 0;
    return imp;
}

void
iom_getmask(struct io_mask *mask, int *nfdp, bit_fdmask *readp,
	    bit_fdmask *writep)
{
    if (mask->what & IO_READ)
	bit_copy(mask->readmask, mask->user_readmask);
    if (mask->what & IO_WRITE)
	bit_copy(mask->writemask, mask->user_writemask);
    *readp = mask->user_readmask;
    *writep = mask->user_writemask;
    *nfdp = mask->maxfd;
}

void
iom_set(struct io_mask *mask, int what, int fd)
{
    if ((mask->what & what) == 0)
	return;
    if (what & IO_READ)
	BIT_SETB(fd, mask->readmask);
    if (what & IO_WRITE)
	BIT_SETB(fd, mask->writemask);
    if (fd > mask->maxfd)
	mask->maxfd = fd;
}

void
iom_clear(struct io_mask *mask, int what, int fd)
{
    if ((mask->what & what) == 0)
	return;
    if (what & IO_READ)
	BIT_CLRB(fd, mask->readmask);
    if (what & IO_WRITE)
	BIT_CLRB(fd, mask->writemask);
}

void
iom_zero(struct io_mask *mask, int what)
{
    if ((mask->what & what) == 0)
	return;
    if (what & IO_READ)
	bit_zero(mask->readmask);
    if (what & IO_WRITE)
	bit_zero(mask->writemask);
}
