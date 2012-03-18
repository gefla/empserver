/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  retreat.c: Retreat subroutines
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#include <config.h>

#include "damage.h"
#include "file.h"
#include "land.h"
#include "map.h"
#include "misc.h"
#include "nat.h"
#include "news.h"
#include "nsc.h"
#include "optlist.h"
#include "path.h"
#include "prototypes.h"
#include "retreat.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

static int findcondition(char);
static int retreat_land1(struct lndstr *, char, int);
static int retreat_ship1(struct shpstr *, char, int);

struct ccode {
    char code;
    char *desc[2];
};

static struct ccode conditions[] = {
    { 'i', { "retreated with a damaged friend",
	     "was damaged" } },
    { 't', { "retreated with a torpedoed ship",
	     "was hit by a torpedo" } },
    { 's', { "retreated with a ship scared by sonar",
	     "detected a sonar ping" } },
    { 'h', { "retreated with a helpless ship",
	     "was fired upon with no one able to defend it" } },
    { 'b', { "retreated with a bombed friend",
	     "was bombed" } },
    { 'd', { "retreated with a depth-charged ship",
	     "was depth-charged" } },
    { 'u', { "retreated with a boarded ship", "was boarded" } },
    { 0,   { "panicked", "panicked"} }
};

int
check_retreat_and_do_shipdamage(struct shpstr *sp, int dam)
{
    if (dam <= 0)
	return 0;

    shipdamage(sp, dam);
    if (sp->shp_rflags & RET_INJURED)
	retreat_ship(sp, 'i');

    return 1;
}

void
retreat_ship(struct shpstr *sp, char code)
{
    struct nstr_item ni;
    struct shpstr ship;

    if (sp->shp_rflags & RET_GROUP) {
	snxtitem_group(&ni, EF_SHIP, sp->shp_fleet);
	while (nxtitem(&ni, &ship))
	    if (ship.shp_own == sp->shp_own) {
		if (ship.shp_uid == sp->shp_uid) {
		    retreat_ship1(sp, code, 1);
		    if (sp->shp_rpath[0] == 0)
			sp->shp_rflags = 0;
		} else {
		    retreat_ship1(&ship, code, 0);
		    getship(ship.shp_uid, &ship);
		    if (ship.shp_rpath[0] == 0) {
			ship.shp_rflags = 0;
			putship(ship.shp_uid, &ship);
		    }
		}
	    }
    } else {
	retreat_ship1(sp, code, 1);
	if (sp->shp_rpath[0] == 0)
	    sp->shp_rflags = 0;
    }
}

static int
retreat_ship1(struct shpstr *sp, char code, int orig)


			/* Is this the originally scared ship, or a follower */
{
    struct sctstr sect;
    int n;
    int m;
    int max;
    int dir;
    coord newx;
    coord newy;
    coord dx;
    coord dy;
    int stopping;
    int mines;
    int shells;
    double mobcost;
    struct mchrstr *mcp;
    int changed;

    sp->shp_mission = 0;
    if (sp->shp_own == 0)
	return 0;

    if (sp->shp_effic < SHIP_MINEFF) {
	wu(0, sp->shp_own,
	   "%s %s,\nbut it died in the attack, and so couldn't retreat!\n",
	   prship(sp), conditions[findcondition(code)].desc[orig]);
	if (!orig)
	    putship(sp->shp_uid, sp);
	return 0;
    }

    if (opt_SAIL) {
	/* can't retreat a ship that's sailin, bad things happend */
	if (*sp->shp_path) {
	    wu(0, sp->shp_own,
	       "%s %s,\nbut had sailing orders, and couldn't retreat!\n",
	       prship(sp), conditions[findcondition(code)].desc[orig]);
	    if (!orig)
		putship(sp->shp_uid, sp);
	    return 0;
	}
    }
    /* check crew - uws don't count */
    if (sp->shp_item[I_MILIT] == 0 && sp->shp_item[I_CIVIL] == 0) {
	wu(0, sp->shp_own,
	   "%s %s,\nbut had no crew, and couldn't retreat!\n", prship(sp),
	   conditions[findcondition(code)].desc[orig]);
	if (!orig)
	    putship(sp->shp_uid, sp);
	return 0;
    }

    getsect(sp->shp_x, sp->shp_y, &sect);
    switch (shp_check_nav(&sect, sp)) {
    case CN_CONSTRUCTION:
	wu(0, sp->shp_own,
	   "%s %s,\nbut was caught in a construction zone, and couldn't retreat!\n",
	   prship(sp), conditions[findcondition(code)].desc[orig]);
	if (!orig)
	    putship(sp->shp_uid, sp);
	return 0;
    case CN_LANDLOCKED:
	wu(0, sp->shp_own,
	   "%s %s,\nbut was landlocked, and couldn't retreat!\n",
	   prship(sp), conditions[findcondition(code)].desc[orig]);
	if (!orig)
	    putship(sp->shp_uid, sp);
	return 0;
	/*NOTREACHED*/
    case CN_NAVIGABLE:
	break;
    case CN_ERROR:
    default:
	wu(0, sp->shp_own,
	   "%s %s,\nbut was subject to an empire error, and couldn't retreat!\n",
	   prship(sp), conditions[findcondition(code)].desc[orig]);
	if (!orig)
	    putship(sp->shp_uid, sp);
	return 0;
	/*NOTREACHED*/
    }

    if (sp->shp_mobil <= 0.0) {
	wu(0, sp->shp_own,
	   "%s %s,\nbut had no mobility, and couldn't retreat!\n",
	   prship(sp), conditions[findcondition(code)].desc[orig]);
	if (!orig)
	    putship(sp->shp_uid, sp);
	return 0;
    }

    n = -MAX_RETREAT;
    stopping = 0;
    while (!stopping && n) {
	dx = dy = 0;
	if (sp->shp_rpath[0] == 0) {
	    stopping = 1;
	    continue;
	}
	if (sp->shp_mobil <= 0.0) {
	    wu(0, sp->shp_own,
	       "%s %s,\nbut ran out of mobility, and couldn't retreat fully!\n",
	       prship(sp), conditions[findcondition(code)].desc[orig]);
	    if (!orig)
		putship(sp->shp_uid, sp);
	    return 0;
	}
	dir = chkdir(sp->shp_rpath[0], DIR_STOP, DIR_LAST);
	memmove(sp->shp_rpath, sp->shp_rpath+1, sizeof(sp->shp_rpath) - 1);
	if (dir < 0)
	    continue;
	if (dir == DIR_STOP)
	    stopping++;
	else {
	    dx = diroff[dir][0];
	    dy = diroff[dir][1];
	}
	n++;

	mcp = &mchr[(int)sp->shp_type];
	newx = xnorm(sp->shp_x + dx);
	newy = ynorm(sp->shp_y + dy);
	mobcost = shp_mobcost(sp);

	getsect(newx, newy, &sect);
	if (shp_check_nav(&sect, sp) != CN_NAVIGABLE ||
	    (sect.sct_own
	     && relations_with(sect.sct_own, sp->shp_own) < FRIENDLY)) {
	    wu(0, sp->shp_own, "%s %s,\nbut could not retreat to %s!\n",
	       prship(sp), conditions[findcondition(code)].desc[orig],
	       xyas(newx, newy, sp->shp_own));
	    if (!orig)
		putship(sp->shp_uid, sp);
	    return 0;
	}
	sp->shp_x = newx;
	sp->shp_y = newy;
	sp->shp_mobil -= mobcost;
	if (stopping)
	    continue;

	mines = sect.sct_mines;
	changed = 0;
	if (sect.sct_type != SCT_WATER || mines <= 0)
	    continue;
	if (mcp->m_flags & M_SWEEP) {
	    max = mcp->m_item[I_SHELL];
	    shells = sp->shp_item[I_SHELL];
	    for (m = 0; mines > 0 && m < 5; m++) {
		if (chance(0.66)) {
		    mines--;
		    shells = MIN(max, shells + 1);
		    changed |= map_set(sp->shp_own, sp->shp_x, sp->shp_y,
				       'X', 0);
		}
	    }
	    if (sect.sct_mines != mines) {
		wu(0, sp->shp_own,
		   "%s cleared %d mine%s in %s while retreating\n",
		   prship(sp), sect.sct_mines - mines,
		   splur(sect.sct_mines - mines),
		   xyas(newx, newy, sp->shp_own));
		sect.sct_mines = mines;
		sp->shp_item[I_SHELL] = shells;
		putsect(&sect);
	    }
	    if (changed)
		writemap(sp->shp_own);
	}
	if (chance(DMINE_HITCHANCE(mines))) {
	    wu(0, sp->shp_own,
	       "%s %s,\nand hit a mine in %s while retreating!\n",
	       prship(sp), conditions[findcondition(code)].desc[orig],
	       xyas(newx, newy, sp->shp_own));
	    nreport(sp->shp_own, N_HIT_MINE, 0, 1);
	    m = MINE_DAMAGE();
	    shipdamage(sp, m);
	    mines--;
	    if (map_set(sp->shp_own, sp->shp_x, sp->shp_y, 'X', 0))
		writemap(sp->shp_own);
	    sect.sct_mines = mines;
	    putsect(&sect);
	    if (!orig)
		putship(sp->shp_uid, sp);
	    return 0;
	}
    }

    if (orig) {
	wu(0, sp->shp_own, "%s %s, and retreated to %s\n",
	   prship(sp), conditions[findcondition(code)].desc[orig],
	   xyas(sp->shp_x, sp->shp_y, sp->shp_own));
    } else {
	wu(0, sp->shp_own, "%s %s, and ended up at %s\n",
	   prship(sp),
	   conditions[findcondition(code)].desc[orig],
	   xyas(sp->shp_x, sp->shp_y, sp->shp_own));
    }
    if (!orig)
	putship(sp->shp_uid, sp);
    return 1;
}

static int
findcondition(char code)
{
    int i;

    for (i = 0; conditions[i].code && conditions[i].code != code; i++) ;
    CANT_HAPPEN(!conditions[i].code);
    return i;
}

int
check_retreat_and_do_landdamage(struct lndstr *lp, int dam)
{
    if (dam <= 0)
	return 0;

    landdamage(lp, dam);
    if (lp->lnd_rflags & RET_INJURED)
	retreat_land(lp, 'i');

    return 1;
}

void
retreat_land(struct lndstr *lp, char code)
{
    struct nstr_item ni;
    struct lndstr land;

    if (lp->lnd_rflags & RET_GROUP) {
	snxtitem_group(&ni, EF_SHIP, lp->lnd_army);
	while (nxtitem(&ni, &land))
	    if (land.lnd_own == lp->lnd_own) {
		if (land.lnd_uid == lp->lnd_uid) {
		    retreat_land1(lp, code, 1);
		    if (lp->lnd_rpath[0] == 0)
			lp->lnd_rflags = 0;
		} else {
		    retreat_land1(&land, code, 0);
		    getland(land.lnd_uid, &land);
		    if (land.lnd_rpath[0] == 0) {
			land.lnd_rflags = 0;
			putland(land.lnd_uid, &land);
		    }
		}
	    }
    } else {
	retreat_land1(lp, code, 1);
	if (lp->lnd_rpath[0] == 0)
	    lp->lnd_rflags = 0;
    }
}

static int
retreat_land1(struct lndstr *lp, char code, int orig)


			/* Is this the originally scared unit, or a follower */
{
    struct sctstr sect;
    int n;
    int m;
    int max;
    int dir;
    coord newx;
    coord newy;
    coord dx;
    coord dy;
    int stopping;
    int mines;
    int shells;
    double mobcost;
    struct lchrstr *lcp;

    lp->lnd_mission = 0;
    if (lp->lnd_own == 0)
	return 0;

    if (lp->lnd_effic < LAND_MINEFF) {
	wu(0, lp->lnd_own,
	   "%s %s,\nbut it died in the attack, and so couldn't retreat!\n",
	   prland(lp), conditions[findcondition(code)].desc[orig]);
	if (!orig)
	    putland(lp->lnd_uid, lp);
	return 0;
    }

    getsect(lp->lnd_x, lp->lnd_y, &sect);

    if (lp->lnd_mobil <= 0.0) {
	wu(0, lp->lnd_own,
	   "%s %s,\nbut had no mobility, and couldn't retreat!\n",
	   prland(lp), conditions[findcondition(code)].desc[orig]);
	if (!orig)
	    putland(lp->lnd_uid, lp);
	return 0;
    }

    n = -MAX_RETREAT;
    stopping = 0;
    while (!stopping && n) {
	dx = dy = 0;
	if (lp->lnd_rpath[0] == 0) {
	    stopping = 1;
	    continue;
	}
	if (lp->lnd_mobil <= 0.0) {
	    wu(0, lp->lnd_own,
	       "%s %s,\nbut ran out of mobility, and couldn't retreat fully!\n",
	       prland(lp), conditions[findcondition(code)].desc[orig]);
	    if (!orig)
		putland(lp->lnd_uid, lp);
	    return 0;
	}
	dir = chkdir(lp->lnd_rpath[0], DIR_STOP, DIR_LAST);
	memmove(lp->lnd_rpath, lp->lnd_rpath+1, sizeof(lp->lnd_rpath) - 1);
	if (dir < 0)
	    continue;
	if (dir == DIR_STOP)
	    stopping++;
	else {
	    dx = diroff[dir][0];
	    dy = diroff[dir][1];
	}
	n++;

	lcp = &lchr[(int)lp->lnd_type];
	newx = xnorm(lp->lnd_x + dx);
	newy = ynorm(lp->lnd_y + dy);

	getsect(newx, newy, &sect);
	mobcost = lnd_mobcost(lp, &sect);
	if (mobcost < 0
	    || sect.sct_type == SCT_MOUNT
	    || sect.sct_own != lp->lnd_own) {
	    wu(0, lp->lnd_own, "%s %s,\nbut could not retreat to %s!\n",
	       prland(lp),
	       conditions[findcondition(code)].desc[orig],
	       xyas(newx, newy, lp->lnd_own));
	    if (!orig)
		putland(lp->lnd_uid, lp);
	    return 0;
	}
	lp->lnd_x = newx;
	lp->lnd_y = newy;
	lp->lnd_mobil -= mobcost;
	if (stopping)
	    continue;

	mines = SCT_LANDMINES(&sect);
	if (mines <= 0 || sect.sct_oldown == lp->lnd_own)
	    continue;
	if (lcp->l_flags & L_ENGINEER) {
	    max = lcp->l_item[I_SHELL];
	    shells = lp->lnd_item[I_SHELL];
	    for (m = 0; mines > 0 && m < 5; m++) {
		if (chance(0.66)) {
		    mines--;
		    shells = MIN(max, shells + 1);
		}
	    }
	    sect.sct_mines = mines;
	    lp->lnd_item[I_SHELL] = shells;
	    putsect(&sect);
	}
	if (chance(DMINE_LHITCHANCE(mines))) {
	    wu(0, lp->lnd_own,
	       "%s %s,\nand hit a mine in %s while retreating!\n",
	       prland(lp),
	       conditions[findcondition(code)].desc[orig],
	       xyas(newx, newy, lp->lnd_own));
	    nreport(lp->lnd_own, N_LHIT_MINE, 0, 1);
	    m = MINE_LDAMAGE();
	    if (lcp->l_flags & L_ENGINEER)
		m /= 2;
	    landdamage(lp, m);
	    mines--;
	    sect.sct_mines = mines;
	    putsect(&sect);
	    if (!orig)
		putland(lp->lnd_uid, lp);
	    return 0;
	}
    }

    if (orig) {
	wu(0, lp->lnd_own, "%s %s, and retreated to %s\n",
	   prland(lp),
	   conditions[findcondition(code)].desc[orig],
	   xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));
    } else {
	wu(0, lp->lnd_own, "%s %s, and ended up at %s\n",
	   prland(lp),
	   conditions[findcondition(code)].desc[orig],
	   xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));
    }
    if (!orig)
	putland(lp->lnd_uid, lp);
    return 1;
}
