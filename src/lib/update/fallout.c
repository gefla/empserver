/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fallout.c: Nuclear fallout
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1997
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "chance.h"
#include "optlist.h"
#include "land.h"
#include "ship.h"
#include "path.h"
#include "prototypes.h"
#include "update.h"

static void do_fallout(struct sctstr *, int);
static void meltitems(int, int, int, short *, int, int, int, int);
static void spread_fallout(struct sctstr *, int);
static void decay_fallout(struct sctstr *, int);

void
fallout(int etu)
{
    int n;
    struct sctstr *sp;

    if (!opt_FALLOUT)
	return;

    /* First, we determine which sectors to process fallout in */
    for (n = 0; NULL != (sp = getsectid(n)); n++)
	sp->sct_updated = sp->sct_fallout != 0;
    /* Next, we process the fallout there */
    for (n = 0; NULL != (sp = getsectid(n)); n++)
	if (sp->sct_updated && sp->sct_type != SCT_SANCT)
	    do_fallout(sp, etu);
    /* Next, we spread the fallout */
    for (n = 0; NULL != (sp = getsectid(n)); n++)
	if (sp->sct_updated)
	    spread_fallout(sp, etu);
    /* Next, we decay the fallout */
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if (sp->sct_fallout)
	    decay_fallout(sp, etu);
	sp->sct_updated = 0;
    }
}

/*
 * Do fallout meltdown for sector @sp.
 * @etus above 24 are treated as 24 to avoid *huge* kill offs in
 * large ETU games.
 */
static void
do_fallout(struct sctstr *sp, int etus)
{
    struct shpstr *spp;
    struct lndstr *lp;
    int i;

/* This check shouldn't be needed, but just in case. :) */
    if (!sp->sct_fallout || !sp->sct_updated)
	return;
    if (etus > 24)
	etus = 24;
    meltitems(etus, sp->sct_fallout, sp->sct_own, sp->sct_item,
	      EF_SECTOR, sp->sct_x, sp->sct_y, 0);
    for (i = 0; NULL != (lp = getlandp(i)); i++) {
	if (!lp->lnd_own)
	    continue;
	if (lp->lnd_x != sp->sct_x || lp->lnd_y != sp->sct_y)
	    continue;
	meltitems(etus, sp->sct_fallout, lp->lnd_own, lp->lnd_item,
		  EF_LAND, lp->lnd_x, lp->lnd_y, lp->lnd_uid);
    }
    for (i = 0; NULL != (spp = getshipp(i)); i++) {
	if (!spp->shp_own)
	    continue;
	if (spp->shp_x != sp->sct_x || spp->shp_y != sp->sct_y)
	    continue;
	if (mchr[(int)spp->shp_type].m_flags & M_SUB)
	    continue;
	meltitems(etus, sp->sct_fallout, spp->shp_own, spp->shp_item,
		  EF_SHIP, spp->shp_x, spp->shp_y, spp->shp_uid);
    }
}

static void
meltitems(int etus, int fallout, int own, short *vec,
	  int type, int x, int y, int uid)
{
    i_type n;
    int melt;

    for (n = I_NONE + 1; n <= I_MAX; n++) {
	melt = roundavg(vec[n] * etus * (double)fallout
			/ (1000.0 * ichr[n].i_melt_denom));
	if (melt > vec[n])
	    melt = vec[n];
	if (melt > 5 && own) {
	    if (type == EF_SECTOR)
		wu(0, own, "Lost %d %s to radiation in %s.\n",
		   melt, ichr[n].i_name,
		   xyas(x, y, own));
	    else if (type == EF_LAND)
		wu(0, own, "Unit #%d lost %d %s to radiation in %s.\n",
		   uid, melt, ichr[n].i_name,
		   xyas(x, y, own));
	    else if (type == EF_SHIP)
		wu(0, own, "Ship #%d lost %d %s to radiation in %s.\n",
		   uid, melt, ichr[n].i_name,
		   xyas(x, y, own));
	}
	vec[n] -= melt;
    }
}

static void
spread_fallout(struct sctstr *sp, int etus)
{
    struct sctstr *ap;
    int n;
    int inc;

    if (etus > 24)
	etus = 24;
    for (n = DIR_FIRST; n <= DIR_LAST; n++) {
	ap = getsectp(sp->sct_x + diroff[n][0], sp->sct_y + diroff[n][1]);
	if (ap->sct_type == SCT_SANCT)
	    continue;
	inc = roundavg(etus * fallout_spread * (sp->sct_fallout)) - 1;
	if (inc < 0)
	    inc = 0;
	ap->sct_fallout = MIN(ap->sct_fallout + inc, FALLOUT_MAX);
    }
}

static void
decay_fallout(struct sctstr *sp, int etus)
{
    int decay;

    if (etus > 24)
	etus = 24;
    decay = roundavg((decay_per_etu + 6.0) * fallout_spread *
		     (double)etus * (double)sp->sct_fallout);

    sp->sct_fallout = decay < sp->sct_fallout ? sp->sct_fallout - decay : 0;
}
