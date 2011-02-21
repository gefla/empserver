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
 *  init.c: Initialize global unit arrays
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1997
 */

#include <config.h>

#include "optlist.h"
#include "plane.h"
#include "product.h"
#include "prototypes.h"
#include "ship.h"

static void init_mchr(void);
static void init_pchr(void);
static void init_plchr(void);

void
global_init(void)
{
    init_pchr();
    init_mchr();
    init_plchr();
}

static void
init_mchr(void)
{
    struct mchrstr *mp;

    for (mp = mchr; mp->m_name; mp++) {
	if (mp->m_nplanes && !(mp->m_flags & M_MSL || mp->m_flags & M_FLY))
	    mp->m_flags |= M_MSL;
    }
}

static void
init_plchr(void)
{
    struct plchrstr *pp;

    for (pp = plchr; pp->pl_name; pp++) {
	if (pp->pl_flags & P_M)
	    pp->pl_flags |= P_V;
    }
}

static void
init_pchr(void)
{
    struct pchrstr *p;

    for (p = pchr; p->p_name; p++) {
	if (opt_GO_RENEW) {
	    if (p->p_type == I_DUST || p->p_type == I_OIL)
		p->p_nrdep = 0;
	}
    }
}
