/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2004-2006
 */

#include <config.h>

#include "file.h"
#include "item.h"
#include "land.h"
#include "match.h"
#include "nuke.h"
#include "plane.h"
#include "product.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"

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
 * Search table TYPE for an element matching NAME, return its index.
 * Accepts EF_BAD, but of course never finds anything then.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 */
int
ef_elt_byname(int type, char *name)
{
    switch (type) {
    case EF_BAD:
	return M_NOTFOUND;
    case EF_NATION:
	return cnumb(name);
    case EF_SECTOR_CHR:
	return sct_typematch(name);
    case EF_SHIP_CHR:
	return stmtch(name, mchr,
		      offsetof(struct mchrstr, m_name),
		      sizeof(mchr[0]));
    case EF_LAND_CHR:
	return stmtch(name, lchr,
		      offsetof(struct lchrstr, l_name),
		      sizeof(lchr[0]));
    case EF_PLANE_CHR:
	return stmtch(name, plchr,
		      offsetof(struct plchrstr, pl_name),
		      sizeof(plchr[0]));
    case EF_NUKE_CHR:
	return stmtch(name, nchr,
		      offsetof(struct nchrstr, n_name),
		      sizeof(nchr[0]));
    case EF_ITEM:
	return stmtch(name, ichr,
		      offsetof(struct ichrstr, i_name),
		      sizeof(ichr[0]));
    case EF_PRODUCT:
	return stmtch(name, pchr,
		      offsetof(struct pchrstr, p_sname),
		      sizeof(pchr[0]));
    case EF_TABLE:
	return stmtch(name, empfile,
		      offsetof(struct empfile, name),
		      sizeof(empfile[0]));
    default:
	if (ef_cadef(type) == symbol_ca)
	    return stmtch(name, ef_ptr(type, 0),
			  offsetof(struct symbol, name),
			  sizeof(struct symbol));
    }
    return M_NOTFOUND;
}
