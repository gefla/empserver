/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nuke.c: Nuke post-read and pre-write data massage
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 *     Markus Armbruster, 2006-2011
 */

#include <config.h>

#include "file.h"
#include "lost.h"
#include "misc.h"
#include "nuke.h"
#include "player.h"
#include "prototypes.h"

void
nuk_postread(int n, void *ptr)
{
    struct nukstr *np = ptr;

    if (np->nuk_uid != n) {
	logerror("nuk_postread: Error - %d != %d, zeroing.\n",
		 np->nuk_uid, n);
	memset(np, 0, sizeof(struct nukstr));
    }
    player->owner = (player->god || np->nuk_own == player->cnum);
}

void
nuk_prewrite(int n, void *old, void *new)
{
    struct nukstr *oldnp = old;
    struct nukstr *np = new;
    natid own = np->nuk_effic == 0 ? 0 : np->nuk_own;
    int plane = np->nuk_plane;

    /* Be careful with writing to *np, in case oldnp == np */

    if (!own) {
	np->nuk_effic = 0;
	plane = -1;
    }

    if (oldnp->nuk_plane != plane)
	nuk_carrier_change(np, EF_PLANE, oldnp->nuk_plane, plane);

    if (oldnp->nuk_own != own)
	lost_and_found(EF_NUKE, oldnp->nuk_own, own,
		       np->nuk_uid, np->nuk_x, np->nuk_y);

    np->nuk_own = own;
    np->nuk_plane = plane;
}

char *
prnuke(struct nukstr *np)
{
    return prbuf("%s warhead #%d",
		 nchr[(int)np->nuk_type].n_name, np->nuk_uid);
}
