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
 *  init_nats.c: Initialize country and nation file stuff
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 2000
 */

#include "prototypes.h"
#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "file.h"
#include "optlist.h"
#include "common.h"
#include "gen.h"

static int nat_cap(int);

int
init_nats(void)
{
    struct natstr *np;

    if ((np = getnatp(player->cnum)) == 0)
	return -1;
    player->nstat = np->nat_stat;
    player->god = np->nat_stat & STAT_GOD;
    player->map = ef_ptr(EF_MAP, player->cnum);
    player->bmap = ef_ptr(EF_BMAP, player->cnum);
    if (opt_HIDDEN) {
	putcontact(np, player->cnum, FOUND_SPY);
    }
    if (np->nat_money <= 0)
	player->broke = 1;
    else {
	player->nstat |= MONEY;
	player->broke = 0;
    }
    if (nat_cap(np->nat_btu) < 0)
	return -1;
    return 0;
}

static int
nat_cap(int btu)
{
    struct sctstr sect;
    struct natstr *np;
    double d;
    double civ;
    int delta;

    np = getnatp(player->cnum);
    if (!getsect(np->nat_xcap, np->nat_ycap, &sect)) {
	logerror("can't read %s's cap @ %d,%d",
		 np->nat_cnam, np->nat_xcap, np->nat_ycap);
	return -1;
    }
    if ((player->nstat & NORM) == NORM) {
	if (player->owner && (sect.sct_type == SCT_CAPIT ||
			      sect.sct_type == SCT_MOUNT ||
			      sect.sct_type == SCT_SANCT))
	    player->nstat |= CAP;
	else
	    player->nstat &= ~CAP;
	/* Ok, has the country owner reset his capital yet after it was sacked? */
	if (np->nat_flags & NF_SACKED)
	    player->nstat &= ~CAP;	/* No capital yet */
    }
    delta = 0;
    if ((player->nstat & CAP) || player->god) {
	d = (double)(player->curup - np->nat_last_login) / s_p_etu;
	if (d > 336.0)
	    d = 336.0;
	civ = sect.sct_item[I_CIVIL];
	if (civ > 999)
	    civ = 999;
	if ((sect.sct_effic) && (sect.sct_type != SCT_MOUNT))
	    delta = roundavg(d * civ * sect.sct_effic * btu_build_rate);
	else			/* Assume 1/2% efficiency minimum */
	    delta = roundavg(d * civ * btu_build_rate / 2.0);
	if (player->god)
	    delta = max_btus - btu;
	if (delta + btu > max_btus)
	    delta = max_btus - btu;
	if (btu > max_btus)
	    delta = max_btus - btu;
	if (opt_BLITZ)
	    delta = max_btus - btu;

	if (delta > 0) {
	    /* set date if BTUs made */
	    np->nat_btu += delta;
	}
	if (btu > max_btus)
	    np->nat_btu = max_btus;
    }
    if (np->nat_stat == VIS)
	np->nat_btu = max_btus;
    putnat(np);
    return 0;
}
