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
 *  type.c: typename to array offset translation
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 2000
 */

#ifdef Rel4
#include <string.h>
#endif /* Rel4 */
#include "misc.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "sect.h"
#include "nuke.h"
#include "file.h"
#include "common.h"

int
typematch(s_char *buf, int type)
{
    register int n;
    int len;

    len = strlen(buf);
    switch (type) {
    case EF_SECTOR:{
	    register struct dchrstr *dcp;

	    if (!buf[0] || buf[1])
		return -1;
	    for (dcp = dchr, n = 0; dcp->d_name; n++, dcp++)
		if (dcp->d_mnem == *buf)
		    return n;
	}
	break;
    case EF_SHIP:{
	    register struct mchrstr *mcp;

	    for (mcp = mchr, n = 0; *mcp->m_name; n++, mcp++)
		if (strncmp(mcp->m_name, buf, len) == 0)
		    return n;
	}
	break;
    case EF_LAND:{
	    register struct lchrstr *lcp;

	    for (lcp = lchr, n = 0; *lcp->l_name; n++, lcp++)
		if (strncmp(lcp->l_name, buf, len) == 0)
		    return n;
	}
	break;
    case EF_PLANE:{
	    register struct plchrstr *pcp;

	    for (pcp = plchr, n = 0; *pcp->pl_name; n++, pcp++)
		if (strncmp(pcp->pl_name, buf, len) == 0)
		    return n;
	}
	break;
    case EF_NUKE:{
	    register struct nchrstr *ncp;

	    for (ncp = nchr, n = 0; *ncp->n_name; n++, ncp++)
		if (strncmp(ncp->n_name, buf, len) == 0)
		    return n;
	}
	break;
    default:
	break;
    }
    return -1;
}
