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
 *  bit.h: Definitions for variable sized bitfields
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef _BIT_H_
#define _BIT_H_

typedef unsigned int bit_mask;
typedef bit_mask *bit_fdmask;

#ifndef bit
#define	bit(x) 		(1 << (x))
#endif

/*
 * File descriptor bit manipulation macros for use with select(2)
 */
#define	BIT_NBBY	8
#define	BIT_BITSPERMASK	(sizeof(bit_mask) * BIT_NBBY)

#define BIT_SETB(a,b)	\
	((b)[(a)/BIT_BITSPERMASK] |= 1 << ((a) % BIT_BITSPERMASK))
#define BIT_CLRB(a,b)	\
	((b)[(a)/BIT_BITSPERMASK] &= ~(1<< ((a) % BIT_BITSPERMASK)))
#define BIT_ISSETB(a,b)	\
	((b)[(a)/BIT_BITSPERMASK] & (1<< ((a) % BIT_BITSPERMASK)))
#define BIT_ISCLRB(a,b)	\
	(((b)[(a)/BIT_BITSPERMASK] & (1<<((a) % BIT_BITSPERMASK))) == 0)

extern bit_fdmask bit_newfdmask();

extern bit_fdmask bit_newfdmask(void);
extern void bit_zero(bit_fdmask);
extern void bit_not(bit_fdmask);
extern void bit_copy(bit_fdmask, bit_fdmask);
extern void bit_or(bit_fdmask, bit_fdmask);
extern void bit_or3(bit_fdmask, bit_fdmask, bit_fdmask);
extern void bit_and(bit_fdmask, bit_fdmask);
extern void bit_and3(bit_fdmask, bit_fdmask, bit_fdmask);
extern int bit_fd(bit_fdmask);

#endif /* _BIT_H_ */
