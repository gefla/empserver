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
 *  res_pop.c: Get maximum pop of a sector
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include <math.h>
#include "misc.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "item.h"
#include "news.h"
#include "file.h"
#include "xy.h"
#include "optlist.h"
#include "common.h"

int
max_pop(float research, struct sctstr *sp)
{
    int maxpop = 999;

    if (opt_RES_POP) {
	maxpop = 400 + 600 * ((50.0 + 4.0 * research)
			      / (200.0 + 3.0 * research));
	if (maxpop > 999)
	    maxpop = 999;
    }

    if (opt_BIG_CITY) {
	if (sp && dchr[sp->sct_type].d_pkg == UPKG)
	    maxpop = (int)(maxpop * ((9.0 * sp->sct_effic) / 100 + 1));
    }

    if (sp) {
	if (sp->sct_type == SCT_MOUNT)
	    maxpop /= 10;
	else if (sp->sct_type == SCT_PLAINS)
	    maxpop /= 20;
    }

    return maxpop;
}
