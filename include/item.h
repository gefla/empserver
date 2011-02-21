/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  item.h: Definitions for item characteristics stuff
 *
 *  Known contributors to this file:
 *
 */

#ifndef ITEM_H
#define ITEM_H

#include "misc.h"

enum i_packing {
    IPKG,			/* "inefficient" packaging (eff<60) */
    NPKG,			/* no special packaging */
    WPKG,			/* "warehouse" packaging */
    UPKG,			/* "urban" packaging */
    BPKG			/* "bank" packaging */
};

enum {
    NUMPKG = BPKG + 1
};

typedef enum {
    I_NONE = -1,
    I_CIVIL,
    I_MILIT,
    I_SHELL,
    I_GUN,
    I_PETROL,
    I_IRON,
    I_DUST,
    I_BAR,
    I_FOOD,
    I_OIL,
    I_LCM,
    I_HCM,
    I_UW,
    I_RAD,
    I_MAX = I_RAD
} ATTRIBUTE((packed)) i_type;

struct ichrstr {
    char i_mnem;		/* usually the initial letter */
    i_type i_uid;		/* index in ichr[] */
    int i_value;		/* mortgage value */
    int i_sell;			/* can this be sold? */
    int i_lbs;			/* how hard to move */
    int i_pkg[NUMPKG];		/* units for reg, ware, urb, bank */
    int i_melt_denom;		/* fallout meltdown denominator */
    char *i_name;		/* full name of item */
};

/* variables using this structure */

extern struct ichrstr ichr[I_MAX + 2];

/* procedures using/returning this struct */

extern struct ichrstr *whatitem(char *, char *);
extern struct ichrstr *item_by_name(char *);

#endif
