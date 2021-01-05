/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  deliver.c: Deliver commodities to neighboring sector
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "item.h"
#include "optlist.h"
#include "path.h"
#include "plague.h"
#include "sect.h"
#include "prototypes.h"
#include "update.h"

#define DELIVER_BONUS 4.0

static int
deliver(struct sctstr *from, struct ichrstr *ip, int dir,
	int thresh, int amt_src, int plague, enum i_packing packing)
{
    struct sctstr *to;
    i_type vtype;		/* item vartype */
    int amt_moved;
    int amt_dst;
    int mobility;
    double mcost;
    int n;

    if (dir <= 0 || dir > DIR_UL)
	return 0;
    if (amt_src <= 0)
	return 0;
    if ((amt_moved = amt_src - thresh) <= 0)
	return 0;
    /*
     * make sure delivery looks OK.  Check where its going,
     * where its coming from, and see if there is more than
     * the threshold amount
     */
    if (!military_control(from))
	return 0;
    to = getsectp(from->sct_x + diroff[dir][0],
		  from->sct_y + diroff[dir][1]);
    if (to->sct_own != from->sct_own) {
	wu(0, from->sct_own, "%s delivery walkout at %s\n",
	   ip->i_name, ownxy(from));
	return 0;
    }
    vtype = ip->i_uid;
    mobility = from->sct_mobil / 2;
    if (vtype == I_CIVIL) {
	if (from->sct_own != from->sct_oldown) {
	    wu(0, from->sct_own,
	       "The conquered populace in %s refuses to relocate!\n",
	       ownxy(from));
	    return 0;
	}
	if (to->sct_own != to->sct_oldown) {
	    wu(0, from->sct_own,
	       "Citizens in %s refuse to relocate!\n", ownxy(from));
	    return 0;
	}
    }
    /* make sure not to abandon the sector */
    if (vtype == I_CIVIL || (vtype == I_MILIT && !to->sct_item[I_CIVIL]))
	amt_moved--;
    /*
     * disallow delivery into prohibited sectors.
     * calculate unit movement cost; decrease amount if
     * there isn't enough mobility.
     */
    mcost = sector_mcost(to, MOB_MOVE) * ip->i_lbs / ip->i_pkg[packing];
    mcost /= DELIVER_BONUS;

    if (mobility < mcost * amt_moved) {
	/* XXX can mcost be == 0? */
	amt_moved = (int)(mobility / mcost);
	if (amt_moved <= 0)
	    return 0;
    }
    amt_dst = to->sct_item[vtype];
    if (amt_moved > ITEM_MAX - amt_dst) {
	/* delivery backlog */
	amt_moved = ITEM_MAX - amt_dst;
    }
    to->sct_item[vtype] = amt_moved + amt_dst;
    /* deliver the plague too! */
    if (plague == PLG_INFECT && to->sct_pstage == PLG_HEALTHY)
	to->sct_pstage = PLG_EXPOSED;
    n = from->sct_mobil - (int)(mcost * amt_moved);
    if (n < 0)
	n = 0;
    from->sct_mobil = n;
    return amt_moved;
}

void
dodeliver(struct sctstr *sp)
{
    i_type i;
    int thresh;
    int dir;
    int plague;
    enum i_packing packing;
    int n;

    if (sp->sct_mobil <= 0)
	return;
    plague = sp->sct_pstage;
    packing = sp->sct_effic >= 60 ? dchr[sp->sct_type].d_pkg : IPKG;
    for (i = I_NONE + 1; i <= I_MAX; i++) {
	if (sp->sct_del[i] == 0)
	    continue;
	thresh = sp->sct_del[i] & ~0x7;
	dir = sp->sct_del[i] & 0x7;
	n = deliver(sp, &ichr[i], dir, thresh, sp->sct_item[i],
		    plague, packing);
	if (n > 0) {
	    sp->sct_item[i] -= n;
	    if (sp->sct_mobil <= 0)
		break;
	}
    }
}
