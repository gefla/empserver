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
 *  land.c: Misc. land unit routines
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "var.h"
#include "file.h"
#include "path.h"
#include "xy.h"
#include "land.h"
#include "nsc.h"
#include "common.h"
#include "subs.h"

int
adj_units(coord x, coord y, natid own)
{
    register int i;
    struct sctstr sect;

    for (i = DIR_FIRST; i <= DIR_LAST; i++) {
	getsect(x + diroff[i][0], y + diroff[i][1], &sect);
	if (has_units(sect.sct_x, sect.sct_y, own, 0))
	    return 1;
    }
    return 0;
}

int
has_units(coord x, coord y, natid cn, struct lndstr *lp)
{
    register int n;
    struct lndstr land;

    for (n = 0; ef_read(EF_LAND, n, (s_char *)&land); n++) {
	if (land.lnd_x != x || land.lnd_y != y)
	    continue;
	if (lp) {
	    /* Check this unit.  If it is this one, we don't want
	       it included in the count. */
	    if (lp->lnd_uid == land.lnd_uid)
		continue;
	}
	if (land.lnd_own == cn)
	    return 1;
    }

    return 0;
}

int
has_units_with_mob(coord x, coord y, natid cn)
{
    struct nstr_item ni;
    struct lndstr land;

    snxtitem_xy(&ni, EF_LAND, x, y);
    while (nxtitem(&ni, (s_char *)&land)) {
	if (land.lnd_own != cn)
	    continue;
	if (land.lnd_mobil > 0)
	    return 1;
    }

    return 0;
}
