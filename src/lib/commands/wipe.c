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
 *  wipe.c: Clear all existing distribution thresholds
 * 
 *  Known contributors to this file:
 *     Jim Griffith, 1989
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "item.h"
#include "file.h"
#include "commands.h"

int
wipe(void)
{
	struct	sctstr sect;
	struct	nstr_sect nstr;
	int	vec[I_MAX+1];

	if (!snxtsct(&nstr, player->argp[1]))
		return RET_SYN;
	bzero((s_char *)vec, sizeof(vec));
	while (nxtsct(&nstr, &sect)) {
		if (!player->owner)
			continue;
		putvec(VT_DIST, vec, (s_char *)&sect, EF_SECTOR);
		pr("Distribution thresholds wiped from %s\n",
			xyas(nstr.x, nstr.y, player->cnum));
		putsect(&sect);
	}
	return RET_OK;
}
