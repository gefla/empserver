/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  bridgefall.c: Knock a bridge down
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2014
 */

#include <config.h>

#include "file.h"
#include "land.h"
#include "misc.h"
#include "nsc.h"
#include "nuke.h"
#include "optlist.h"
#include "path.h"
#include "plague.h"
#include "plane.h"
#include "prototypes.h"
#include "sect.h"
#include "xy.h"

static void knockdown(struct sctstr *);

/*
 * Is there support for a bridge at SP?
 * Ignore support coming from direction IGNORE_DIR.
 */
int
bridge_support_at(struct sctstr *sp, int ignore_dir)
{
    int i, nx, ny;
    struct sctstr sect;

    for (i = 1; i <= 6; i++) {
	if (i == ignore_dir)
	    continue;
	nx = sp->sct_x + diroff[i][0];
	ny = sp->sct_y + diroff[i][1];
	getsect(nx, ny, &sect);
	if (opt_EASY_BRIDGES
	    && sect.sct_type != SCT_WATER && sect.sct_type != SCT_BSPAN)
	    return 1;
	if (sect.sct_effic < SCT_MINEFF)
	    continue;
	if (sect.sct_type == SCT_BHEAD && sect.sct_newtype == SCT_BHEAD)
	    return 1;
	if (sect.sct_type == SCT_BTOWER)
	    return 1;
    }
    return 0;
}

/*
 * Check bridges at and around SP after damage to SP.
 * If SP is an inefficent bridge, splash it.
 * If SP can't support a bridge, splash unsupported adjacent bridges.
 * Write back splashed bridges, except for SP; writing that one is
 * left to the caller.
 */
void
bridge_damaged(struct sctstr *sp)
{
    int des;

    if (sp->sct_effic >= SCT_MINEFF)
	return;

    des = sp->sct_type;
    if (des == SCT_BSPAN || des == SCT_BTOWER)
	knockdown(sp);
    if ((des == SCT_BHEAD && !opt_EASY_BRIDGES) || des == SCT_BTOWER)
	bridgefall(sp);
}

void
bridgefall(struct sctstr *sp)
{
    int i;
    struct sctstr sect;
    int nx;
    int ny;

    for (i = 1; i <= 6; i++) {
	nx = sp->sct_x + diroff[i][0];
	ny = sp->sct_y + diroff[i][1];
	getsect(nx, ny, &sect);
	if (sect.sct_type == SCT_BSPAN
	    && !bridge_support_at(&sect, DIR_BACK(i))) {
	    knockdown(&sect);
	    putsect(&sect);
	}
    }
}

/* Knock down a bridge span.  Note that this does NOT write the
 * sector out to the database, it's up to the caller to do that. */
static void
knockdown(struct sctstr *sp)
{
    struct lndstr land;
    struct plnstr plane;
    struct nukstr nuke;
    struct nstr_item ni;

    mpr(sp->sct_own,
	"Crumble... SCREEEECH!  Splash! Bridge%s falls at %s!\n",
	sp->sct_type == SCT_BTOWER ? " tower" : "",
	xyas(sp->sct_x, sp->sct_y, sp->sct_own));
    if (!SCT_MINES_ARE_SEAMINES(sp))
	sp->sct_mines = 0;
    sp->sct_type = SCT_WATER;
    sp->sct_newtype = SCT_WATER;
    sp->sct_own = 0;
    sp->sct_oldown = 0;
    sp->sct_mobil = 0;
    sp->sct_effic = 0;

    /* Sink all the units */
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == 0)
	    continue;
	if (land.lnd_ship >= 0)
	    continue;
	mpr(land.lnd_own, "     AARGH! %s tumbles to its doom!\n",
	    prland(&land));
	land.lnd_effic = 0;
	putland(land.lnd_uid, &land);
    }
    /* Sink all the planes */
    snxtitem_xy(&ni, EF_PLANE, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own == 0)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	if (plane.pln_ship >= 0)
	    continue;
	mpr(plane.pln_own, "     AARGH! %s tumbles to its doom!\n",
	    prplane(&plane));
	plane.pln_effic = 0;
	putplane(plane.pln_uid, &plane);
    }
    /* Sink all the nukes */
    snxtitem_xy(&ni, EF_NUKE, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, &nuke)) {
	if (nuke.nuk_own == 0)
	    continue;
	if (nuke.nuk_plane >= 0)
	    continue;
	mpr(nuke.nuk_own, "     %s sinks to the bottom of the sea!\n",
	    prnuke(&nuke));
	nuke.nuk_effic = 0;
	putnuke(nuke.nuk_uid, &nuke);
    }
    memset(sp->sct_item, 0, sizeof(sp->sct_item));
    memset(sp->sct_del, 0, sizeof(sp->sct_del));
    memset(sp->sct_dist, 0, sizeof(sp->sct_dist));
    sp->sct_pstage = PLG_HEALTHY;
    sp->sct_ptime = 0;
    sp->sct_che = 0;
    sp->sct_che_target = 0;
}
