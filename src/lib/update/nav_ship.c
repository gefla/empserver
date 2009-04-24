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
 *  nav_ship.c: Navigate ships and such
 *
 *  Known contributors to this file:
 *     Chad Zabel, 1994
 *     Ken Stevens, 1995
 */

#include <config.h>

#include "nsc.h"
#include "path.h"
#include "update.h"
#include "empobj.h"
#include "unit.h"

static void swap(struct shpstr *);

static void
scuttle_it(struct shpstr *sp)
{
    struct sctstr *sectp;

    sp->shp_autonav &= ~AN_SCUTTLE;
    if (!(sectp = getsectp(sp->shp_x, sp->shp_y))) {
	wu(0, 0, "bad sector (%d,%d) ship %d\n",
	   sp->shp_x, sp->shp_y, sp->shp_uid);
	return;
    }
    if (opt_TRADESHIPS) {
	if (!(mchr[(int)sp->shp_type].m_flags & M_TRADE)) {
	    wu(0, sp->shp_own, "You can only autoscuttle trade ships!\n");
	    return;
	}
    }
    if (!scuttle_tradeship(sp, 0)) {
	wu(0, sp->shp_own,
	   "%s doesn't pay here!  Not scuttled.\n", prship(sp));
	return;
    }
    wu(0, sp->shp_own, "Scuttling %s in sector %s\n",
       prship(sp), xyas(sp->shp_x, sp->shp_y, sp->shp_own));
    if (sectp->sct_own == sp->shp_own)
	unit_drop_cargo((struct empobj *)sp, sectp->sct_own);
    sp->shp_effic = 0;
    putship(sp->shp_uid, sp);
}

static void
nav_check_atdest(struct shpstr *sp)
{
    if ((sp->shp_x == sp->shp_destx[0]) && (sp->shp_y == sp->shp_desty[0])) {
	if ((sp->shp_destx[0] == sp->shp_destx[1]) &&
	    (sp->shp_desty[0] == sp->shp_desty[1])) {

	    /* End of road */

	    sp->shp_autonav &= ~AN_AUTONAV;
	    wu(0, sp->shp_own, "%s arrived at %s, finished\n",
	       prship(sp), xyas(sp->shp_x, sp->shp_y, sp->shp_own));
	    if (sp->shp_autonav & AN_SCUTTLE) {
		scuttle_it(sp);
	    }
	} else {
	    /* unload all cargo */
	    unload_it(sp);
	    wu(0, sp->shp_own, "%s arrived at %s\n",
	       prship(sp), xyas(sp->shp_x, sp->shp_y, sp->shp_own));
	    /* Swap */
	    swap(sp);
	}
    } else
	sp->shp_autonav &= ~AN_LOADING;
}

/* flip the 2 fields that deal with autonav movement. */
/* CZ 6/1/94					      */

static void
swap(struct shpstr *sp)
{
    coord tcord;
    i_type tcomm[TMAX];
    short lev[TMAX];
    int i;

    tcord = sp->shp_destx[0];
    sp->shp_destx[0] = sp->shp_destx[1];
    sp->shp_destx[1] = tcord;
    tcord = sp->shp_desty[0];
    sp->shp_desty[0] = sp->shp_desty[1];
    sp->shp_desty[1] = tcord;

    for (i = 0; i < TMAX; ++i) {
	lev[i] = sp->shp_lstart[i];
	tcomm[i] = sp->shp_tstart[i];
    }

    for (i = 0; i < TMAX; ++i) {
	sp->shp_lstart[i] = sp->shp_lend[i];
	sp->shp_tstart[i] = sp->shp_tend[i];
    }

    for (i = 0; i < TMAX; ++i) {
	sp->shp_lend[i] = lev[i];
	sp->shp_tend[i] = tcomm[i];
    }

    /* set load bit */
    sp->shp_autonav |= AN_LOADING;
}

/*  New Autonav code.
 *  Chad Zabel
 *  6-1-94
 */

static int
nav_loadship(struct shpstr *sp, natid cnum)
{
    struct sctstr *sectp;
    int i, landown, shipown, didsomething[TMAX], rel;

    for (i = 0; i < TMAX; i++)
	didsomething[i] = 0;

    /* Turn off the loading flag.
     * if any of the loads fail on the ship
     * it will be turned back on.
     */

    sp->shp_autonav &= ~AN_LOADING;

    if (!(sectp = getsectp(sp->shp_x, sp->shp_y)))
	return 0;		/* safety */

    landown = sectp->sct_own;
    shipown = sp->shp_own;
    rel = getrel(getnatp(sectp->sct_own), cnum);

    /* loop through each field for that ship */
    for (i = 0; i < TMAX; ++i) {
	/* check and see if the data fields have been set. */

	if (sp->shp_tend[i] == I_NONE || sp->shp_lend[i] == 0) {
	    /* nothing to do move on. */
	    didsomething[i] = 1;
	    continue;
	}
	if (landown == 0) {
	    /* either sea or deity harbor */
	    didsomething[i] = 1;
	    continue;
	}
	if (!sect_has_dock(sectp)) {
	    /* we can only load in harbors */
	    didsomething[i] = 1;
	    continue;
	}
	if (landown == shipown || rel >= FRIENDLY)
	    didsomething[i] = load_it(sp, sectp, i);
    }

    /* check for any unsucessful loads */
    /* if we have any return 0 to stop */
    /* the nav_ship loop.              */

    for (i = 0; i < TMAX; i++) {
	if (didsomething[i] == 0)
	    return 0;
    }
    /* All loads were succesful */
    return 1;
}

static int
nav_load_ship_at_sea(struct shpstr *sp)
{
    int i;
    int n_items;
    int max_amt, item_amt;
    struct mchrstr *mcp;
    struct sctstr *sectp;
    struct check_list_st {
	long cap;
	i_type item;
    } check_list[] = {{M_FOOD, I_FOOD}, {M_OIL, I_OIL}};

    n_items = sizeof(check_list) / sizeof(check_list[0]);

    mcp = &mchr[(int)sp->shp_type];
    sectp = getsectp(sp->shp_x, sp->shp_y);
    for (i = 0; i < n_items; i++) {
	if (mcp->m_flags & check_list[i].cap) {
	    item_amt = sp->shp_item[check_list[i].item];
	    max_amt = mcp->m_item[check_list[i].item];
	    if (item_amt < max_amt && sectp->sct_type == SCT_WATER)
		return 1;
	}
    }
    return 0;
}

/* new autonav code.
 *
 * 1. Try and move to the next sector/harbor given by the player.
 * 2. Once we reach a harbor try and load all cargo holds for that ship.
 * 3. If the ship reaches its max levels set by the player try to use
 *    up all mobility getting to the next harbor.
 * Continue to loop until the ship runs out of mobility, a load fails,
 * the ship gets sunk (forts,ect..), the ship hits a mine.
 *
 * Questions, bugs (fixes) , or new ideas should be directed at
 * Chad Zabel.
 * 6-1-94
 * Modified to use shp_nav by Ken Stevens 1995
 */
int
nav_ship(struct shpstr *sp)
{
    char *cp;
    int stopping;
    int quit;
    int didsomething = 0;
    char buf[1024];
    struct emp_qelem ship_list;
    struct emp_qelem *qp, *newqp;
    struct ulist *mlp;
    int dummyint;
    double dummydouble;
    int dir;
    natid cnum;

    /* just return if no autonaving to do for this ship */
    if (!(sp->shp_autonav & AN_AUTONAV) || (sp->shp_autonav & AN_STANDBY))
	return 0;

    cnum = sp->shp_own;

    /* Make a list of one ships so we can use the navi.c code */
    emp_initque(&ship_list);
    mlp = malloc(sizeof(struct ulist));
    mlp->chrp = (struct empobj_chr *)(mchr + sp->shp_type);
    mlp->unit.ship = *sp;
    ef_mark_fresh(EF_SHIP, &mlp->unit.ship);
    mlp->mobil = sp->shp_mobil;
    emp_insque(&mlp->queue, &ship_list);

    quit = 1;			/* setup loop, we want to check it 1 time. */

    do {
	if ((sp->shp_mobil > 0) && (!(sp->shp_autonav & AN_LOADING)) &&
	    (!(sp->shp_autonav & AN_STANDBY))) {
	    shp_nav(&ship_list, &dummydouble, &dummydouble, &dummyint,
		    sp->shp_own);
	    if (QEMPTY(&ship_list))
		return 0;

	    cp = BestShipPath(buf, sp->shp_x, sp->shp_y,
			      sp->shp_destx[0], sp->shp_desty[0],
			      sp->shp_own);
	    if (cp == 0) {
		wu(0, cnum,
		   "%s bad path, ship put on standby\n", prship(sp));
		sp->shp_autonav |= AN_STANDBY;
		putship(sp->shp_uid, sp);

		/* We need to free the ship list */
		qp = ship_list.q_forw;
		while (qp != &ship_list) {
		    newqp = qp->q_forw;
		    emp_remque(qp);
		    free(qp);
		    qp = newqp;
		}
		return -1;
	    }
	    stopping = 0;

	    while (*cp && !stopping && sp->shp_own && mlp->mobil > 0.0) {
		dir = diridx(*cp++);
		stopping |= shp_nav_one_sector(&ship_list, dir,
					       sp->shp_own, 0);
	    }

	    /* Ship not sunk */
	    if (sp->shp_own)
		nav_check_atdest(sp);
	}

	quit = 0;		/* stop loop */

	/* Try to load the ship */
	if (sp->shp_autonav & AN_LOADING) {
	    didsomething = nav_loadship(sp, cnum);
	    if (didsomething)
		quit = 1;
	}
	/* special case for fishing boats and oil derricks */
	if (nav_load_ship_at_sea(sp))
	    quit = 0;
	/* reset flag and check if we can move. */

    } while (quit);		/* end loop */

    putship(sp->shp_uid, sp);

    /* We need to free the ship list (just in case) */
    qp = ship_list.q_forw;
    while (qp != &ship_list) {
	newqp = qp->q_forw;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }
    return 0;
}
