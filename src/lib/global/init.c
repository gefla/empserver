/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
#include "product.h"
#include "prototypes.h"
#include "sect.h"

static void init_pchr(void);

void
global_init(void)
{
    if (opt_RAILWAYS)
	intrchr[INT_RAIL].in_enable = 0;
    init_pchr();
}

static void
init_pchr(void)
{
    struct pchrstr *p;

    for (p = pchr; p->p_sname; p++) {
	if (!p->p_sname[0])
	    continue;
	if (opt_GO_RENEW) {
	    if (p->p_type == I_DUST || p->p_type == I_OIL)
		p->p_nrdep = 0;
	}
    }
}
