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
 *  item.h: Definitions for item characteristics stuff
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef _ITEM_H_
#define _ITEM_H_

#define	NUMPKG	4		/* number of different kinds of packaging */

struct ichrstr {
    int i_mnem;			/* usually the initial letter */
    int i_vtype;		/* var type */
    int i_value;		/* mortgage value */
    int i_sell;			/* can this be sold? */
    int i_lbs;			/* how hard to move */
    int i_pkg[NUMPKG];		/* units for reg, ware, urb, bank */
    s_char *i_name;		/* full name of item */
};

/* variables using this structure */

extern struct ichrstr ichr[];
extern int maxitem;

/* procedures using/returning this struct */

extern struct ichrstr *whatitem(s_char *ptr, s_char *prompt);

extern int itm_maxno;

#endif /* _ITEM_H_ */
