/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  check.c: Check a sector, plane, land unit, ship or nuke
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2009
 */

#include <config.h>

#include "commodity.h"
#include "empobj.h"
#include "file.h"
#include "player.h"
#include "prototypes.h"

/* Note that timestamps make things tricky.  And, we don't
 * really care about the timestamp, we just care about the rest
 * of the structure.  So, we make a copy, and zero the timestamps
 * in both copies, and then compare. */

static int
obj_changed(struct empobj *obj, size_t sz)
{
    union empobj_storage old, tobj;

    get_empobj(obj->ef_type, obj->uid, &old);
    memcpy(&tobj, obj, sz);
    old.gen.timestamp = tobj.gen.timestamp = 0;
    return memcmp(&tobj, &old, sz);
}

int
check_sect_ok(struct sctstr *sectp)
{
    if (obj_changed((struct empobj *)sectp, sizeof(*sectp))) {
	pr("Sector %s has changed!\n",
	   xyas(sectp->sct_x, sectp->sct_y, player->cnum));
	return 0;
    }
    return 1;
}

int
check_ship_ok(struct shpstr *shipp)
{
    if (obj_changed((struct empobj *)shipp, sizeof(*shipp))) {
	pr("Ship #%d has changed!\n", shipp->shp_uid);
	return 0;
    }
    return 1;
}

int
check_plane_ok(struct plnstr *planep)
{
    if (obj_changed((struct empobj *)planep, sizeof(*planep))) {
	pr("Plane #%d has changed!\n", planep->pln_uid);
	return 0;
    }
    return 1;
}

int
check_land_ok(struct lndstr *landp)
{
    if (obj_changed((struct empobj *)landp, sizeof(*landp))) {
	pr("Land unit #%d has changed!\n", landp->lnd_uid);
	return 0;
    }
    return 1;
}

int
check_nuke_ok(struct nukstr *nukep)
{
    if (obj_changed((struct empobj *)nukep, sizeof(*nukep))) {
	pr("Nuke %d has changed!\n", nukep->nuk_uid);
	return 0;
    }
    return 1;
}

int
check_loan_ok(struct lonstr *loanp)
{
    if (obj_changed((struct empobj *)loanp, sizeof(*loanp))) {
	pr("Loan %d has changed!\n", loanp->l_uid);
	return 0;
    }
    return 1;
}

int
check_comm_ok(struct comstr *commp)
{
    if (obj_changed((struct empobj *)commp, sizeof(*commp))) {
	pr("Commodity %d has changed!\n", commp->com_uid);
	return 0;
    }
    return 1;
}

int
check_trade_ok(struct trdstr *tp)
{
    if (obj_changed((struct empobj *)tp, sizeof(*tp))) {
	pr("Trade lot #%d has changed!\n", tp->trd_uid);
	return 0;
    }
    return 1;
}
