/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  type.c: typename to array offset translation
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 2000
 */

#include <config.h>

#include <stddef.h>
#include <string.h>
#include "misc.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "sect.h"
#include "nuke.h"
#include "file.h"
#include "common.h"
#include "match.h"

/*
 * Return index of sector called NAME in dchr[], or M_NOTFOUND.
 */
int
sct_typematch(char *name)
{
    int i;

    if (name[0] && !name[1])
	for (i = 0; dchr[i].d_name; ++i)
	    if (dchr[i].d_mnem == *name)
		return i;
    return M_NOTFOUND;
}

/*
 * Search for NAME in the characteristics table for TYPE, return index.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 * If TYPE is EF_SECTOR, search dchr[]
 * If TYPE is EF_SHIP, search mchr[].
 * If TYPE is EF_PLANE, search plchr[].
 * If TYPE is EF_LAND, search lchr[].
 * If TYPE is EF_NUKE, search nchr[].
 */
int
typematch(char *name, int type)
{
    switch (type) {
    case EF_SECTOR:
	return sct_typematch(name);
    case EF_SHIP:
	return stmtch(name, mchr,
		      offsetof(struct mchrstr, m_name),
		      sizeof(mchr[0]));
    case EF_LAND:
	return stmtch(name, lchr,
		      offsetof(struct lchrstr, l_name),
		      sizeof(lchr[0]));
    case EF_PLANE:
	return stmtch(name, plchr,
		      offsetof(struct plchrstr, pl_name),
		      sizeof(plchr[0]));
    case EF_NUKE:
	return stmtch(name, nchr,
		      offsetof(struct nchrstr, n_name),
		      sizeof(nchr[0]));
    default:
	break;
    }
    return M_NOTFOUND;
}
