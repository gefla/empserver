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
 *  io_mask.h: Describes an i/o mask
 * 
 *  Known contributors to this file:
 *     
 */

#ifndef _IO_MASK_H_
#define _IO_MASK_H_

#include <sys/types.h>
#ifdef _WIN32
#include <winsock.h>
#endif

struct io_mask {
    int what;
    int maxfd;
    fd_set *readmask;
    fd_set *user_readmask;
    fd_set *writemask;
    fd_set *user_writemask;
};

extern struct io_mask *iom_create(int);
extern void iom_getmask(struct io_mask *, int *, fd_set **, fd_set **);
extern void iom_set(struct io_mask *, int, int);
extern void iom_clear(struct io_mask *, int, int);
extern void iom_zero(struct io_mask *, int);

#endif /* _IO_MASK_H_ */
