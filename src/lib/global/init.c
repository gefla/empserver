/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  init.c: Initialize global unit arrays
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1997
 */

#include "misc.h"
#include "var.h"
#include "ship.h"
#include "plane.h"
#include "product.h"
#include "nat.h"
#include "optlist.h"
#include "global.h"

static void init_mchr(void);
static void init_pchr(void);
static void init_plchr(void);

void
global_init(void)
{
    init_pchr();
    init_mchr();
    init_plchr();
    /* the same thing should be done for units... */
}

static void
init_mchr(void)
{
    register struct mchrstr *mp;
    register int n;

    for (mp = mchr, n = 0; n < shp_maxno; n++, mp++) {
	if (mp->m_nxlight)
	    mp->m_flags |= M_XLIGHT;
	if (mp->m_nchoppers)
	    mp->m_flags |= M_CHOPPER;
	if (mp->m_nplanes && !(mp->m_flags & M_MSL || mp->m_flags & M_FLY))
	    mp->m_flags |= M_MSL;
	if (mp->m_nland)
	    mp->m_flags |= M_UNIT;
    }
}

static void
init_plchr(void)
{
    register struct plchrstr *pp;
    int pcount;

    for (pp = plchr, pcount = 0; pcount < pln_maxno; pcount++, pp++) {
	if (pp->pl_flags & P_M)
	    pp->pl_flags |= P_V;
    }
}

static void
init_pchr(void)
{
    register struct pchrstr *p;
    int pcount;

    for (p = pchr, pcount = 0; pcount < prd_maxno; pcount++, p++) {
	if (opt_GO_RENEW) {
	    if (p->p_type == I_DUST || p->p_type == I_OIL)
		p->p_nrdep = 0;
	}
    }
}
