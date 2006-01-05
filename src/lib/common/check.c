/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  check.c: Check a sector, plane, land unit, ship or nuke
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "file.h"
#include "sect.h"
#include "ship.h"
#include "plane.h"
#include "nuke.h"
#include "land.h"
#include "nsc.h"
#include "nat.h"
#include "commodity.h"
#include "loan.h"
#include "commands.h"
#include "trade.h"

/* Note that timestamps make things tricky.  And, we don't
 * really care about the timestamp, we just care about the rest
 * of the structure.  So, we make a copy, and zero the timestamps
 * in both copies, and then compare. */

int
check_sect_ok(struct sctstr *sectp)
{
    struct sctstr chksect;
    struct sctstr tsect;

    getsect(sectp->sct_x, sectp->sct_y, &chksect);
    memcpy(&tsect, sectp, sizeof(struct sctstr));
    tsect.sct_timestamp = chksect.sct_timestamp = 0;
    if (memcmp(&tsect, &chksect, sizeof(struct sctstr))) {
	pr("Sector %s has changed!\n",
	   xyas(sectp->sct_x, sectp->sct_y, player->cnum));
	return 0;
    }
    return 1;
}

int
check_ship_ok(struct shpstr *shipp)
{
    struct shpstr chkship;
    struct shpstr tship;

    getship(shipp->shp_uid, &chkship);
    memcpy(&tship, shipp, sizeof(struct shpstr));
    tship.shp_timestamp = chkship.shp_timestamp = 0;
    if (memcmp(&tship, &chkship, sizeof(struct shpstr))) {
	pr("Ship #%d has changed!\n", shipp->shp_uid);
	return 0;
    }
    return 1;
}

int
check_plane_ok(struct plnstr *planep)
{
    struct plnstr chkplane;
    struct plnstr tplane;

    getplane(planep->pln_uid, &chkplane);
    memcpy(&tplane, planep, sizeof(struct plnstr));
    tplane.pln_timestamp = chkplane.pln_timestamp = 0;
    if (memcmp(&tplane, &chkplane, sizeof(struct plnstr))) {
	pr("Plane #%d has changed!\n", planep->pln_uid);
	return 0;
    }
    return 1;
}

int
check_land_ok(struct lndstr *landp)
{
    struct lndstr chkland;
    struct lndstr tland;

    getland(landp->lnd_uid, &chkland);
    memcpy(&tland, landp, sizeof(struct lndstr));
    tland.lnd_timestamp = chkland.lnd_timestamp = 0;
    if (memcmp(&tland, &chkland, sizeof(struct lndstr))) {
	pr("Land unit #%d has changed!\n", landp->lnd_uid);
	return 0;
    }
    return 1;
}

int
check_nuke_ok(struct nukstr *nukep)
{
    struct nukstr chknuke;
    struct nukstr tnuke;

    getnuke(nukep->nuk_uid, &chknuke);
    memcpy(&tnuke, nukep, sizeof(struct nukstr));
    tnuke.nuk_timestamp = chknuke.nuk_timestamp = 0;
    if (memcmp(&tnuke, &chknuke, sizeof(struct nukstr))) {
	pr("Nuclear stockpile %d has changed!\n", nukep->nuk_uid);
	return 0;
    }
    return 1;
}

int
check_loan_ok(struct lonstr *loanp)
{
    struct lonstr chkloan;

    getloan(loanp->l_uid, &chkloan);
    if (memcmp(loanp, &chkloan, sizeof(struct lonstr))) {
	pr("Loan %d has changed!\n", loanp->l_uid);
	return 0;
    }
    return 1;
}

int
check_comm_ok(struct comstr *commp)
{
    struct comstr chkcomm;

    getcomm(commp->com_uid, &chkcomm);
    if (memcmp(commp, &chkcomm, sizeof(struct comstr))) {
	pr("Commodity %d has changed!\n", commp->com_uid);
	return 0;
    }
    return 1;
}

int
check_trade_ok(struct trdstr *tp)
{
    struct trdstr chktrade;

    gettrade(tp->trd_uid, &chktrade);
    if (memcmp(tp, &chktrade, sizeof(struct trdstr))) {
	pr("Trade lot #%d has changed!\n", tp->trd_uid);
	return 0;
    }
    return 1;
}
