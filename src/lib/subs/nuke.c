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
 *  nuke.c: Nuke post-read and pre-write data massage
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 *     Markus Armbruster, 2006-2008
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
    natid own = np->nuk_own;

    if (np->nuk_effic == 0) {
	own = 0;
	np->nuk_plane = -1;
    }

    if (oldnp->nuk_plane != np->nuk_plane)
	nuk_carrier_change(np, EF_PLANE, oldnp->nuk_plane, np->nuk_plane);

    /* We've avoided assigning to np->nuk_own, in case oldsp == sp */
    if (oldnp->nuk_own != own)
	lost_and_found(EF_NUKE, oldnp->nuk_own, own,
		       np->nuk_uid, np->nuk_x, np->nuk_y);

    np->nuk_own = own;
}

void
nuk_oninit(void *ptr)
{
    struct nukstr *np = ptr;

    np->nuk_plane = -1;
}

char *
prnuke(struct nukstr *np)
{
    return prbuf("%s warhead #%d",
		 nchr[(int)np->nuk_type].n_name, np->nuk_uid);
}
