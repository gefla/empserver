/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  retreat.c: Retreat subroutines
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     
 */

#include <string.h>
#include "misc.h"
#include "player.h"
#include "nat.h"
#include "retreat.h"
#include "ship.h"
#include "land.h"
#include "sect.h"
#include "news.h"
#include "xy.h"
#include "nsc.h"
#include "path.h"
#include "file.h"
#include "damage.h"
#include "prototypes.h"
#include "optlist.h"

static int findcondition(s_char);
static int retreat_land1(struct lndstr *, s_char, int);
static int retreat_ship1(struct shpstr *, s_char, int);

struct ccode {
    s_char code;
    s_char *desc[2];
} conditions[] = {
    {
	'i', {
    "retreated with a damaged friend", "was damaged",},}, {
	't', {
    "retreated with a torpedoed ship", "was hit by a torpedo",},}, {
	's', {
    "retreated with a ship scared by sonar",
		"detected a sonar ping",},}, {
	'h', {
    "retreated with a helpless ship",
		"was fired upon with no one able to defend it",},}, {
	'b', {
    "retreated with a bombed friend", "was bombed",},}, {
	'd', {
    "retreated with a depth-charged ship", "was depth-charged",},}, {
	'u', {
    "retreated with a boarded ship", "was boarded",},}, {
	0, {
"", ""}},};

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
retreat_ship(struct shpstr *sp, s_char code)
{
    struct nstr_item ni;
    struct shpstr ship;
    s_char buf[2];

    if (sp->shp_rflags & RET_GROUP) {
	buf[0] = sp->shp_fleet;
	buf[1] = 0;
	snxtitem(&ni, EF_SHIP, buf);
	while (nxtitem(&ni, (s_char *)&ship))
	    if ((ship.shp_fleet == buf[0]) &&
		(ship.shp_own == sp->shp_own)) {
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
retreat_ship1(struct shpstr *sp, s_char code, int orig)


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
    int time_to_stop;

    sp->shp_mission = 0;
    if (sp->shp_own == 0)
	return 0;

    if (isupper(code))
	code = tolower(code);

    n = 0;
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
    switch (check_nav(&sect)) {
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

    n = (-1 * MAX_RETREAT);
    stopping = 0;
    time_to_stop = 0;
    while ((!stopping) && n) {
	dx = dy = 0;
	if (sp->shp_rpath[0] == 0 || sp->shp_rpath[0] == 0) {
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
	dir = chkdir(sp->shp_rpath[0], DIR_STOP, DIR_VIEW);
	memmove(sp->shp_rpath, sp->shp_rpath+1, sizeof(sp->shp_rpath) - 1);
	if (dir == -1)
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
	mobcost = sp->shp_effic * 0.01 * sp->shp_speed;
	mobcost = 480.0 / (mobcost + techfact(sp->shp_tech, mobcost));

	getsect(newx, newy, &sect);
	if (check_nav(&sect) != CN_NAVIGABLE ||
	    (sect.sct_own && !player->owner &&
	     getrel(getnatp(sect.sct_own), sp->shp_own) < FRIENDLY)) {
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
	if ((mcp->m_flags & M_SWEEP) && mines > 0 && !player->owner) {
	    max = mcp->m_item[I_SHELL];
	    shells = sp->shp_item[I_SHELL];
	    for (m = 0; mines > 0 && m < 5; m++) {
		if (chance(0.66)) {
		    mines--;
		    shells = min(max, shells + 1);
		}
	    }
	    sect.sct_mines = mines;
	    sect.sct_item[I_SHELL] = shells;
	    putsect(&sect);
	}
	if (mines > 0 && !player->owner && chance(DMINE_HITCHANCE(mines))) {
	    wu(0, sp->shp_own,
	       "%s %s,\nand hit a mine in %s while retreating!\n",
	       prship(sp), conditions[findcondition(code)].desc[orig],
	       xyas(newx, newy, sp->shp_own));
	    nreport(sp->shp_own, N_HIT_MINE, 0, 1);
	    m = MINE_DAMAGE();
	    shipdamage(sp, m);
	    mines--;
	    sect.sct_mines = mines;
	    putsect(&sect);
	    if (sp->shp_effic < SHIP_MINEFF)
		time_to_stop = 1;
	    if (!orig)
		putship(sp->shp_uid, sp);
	    return 0;
	}
	if (time_to_stop)
	    stopping = 1;
    }

    if (orig) {
	wu(0, sp->shp_own, "%s %s, and retreated to %s\n", prship(sp),
	   conditions[findcondition(code)].desc[orig],
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

#if 0
static int
check_nav(sect)
struct sctstr *sect;
{
    switch (dchr[sect->sct_type].d_flg & 03) {
    case NAVOK:
	break;

    case NAV_02:
	if (sect->sct_effic < 2)
	    return CN_CONSTRUCTION;
	break;
    case NAV_60:
	if (sect->sct_effic < 60)
	    return CN_CONSTRUCTION;
	break;
    default:
	return CN_LANDLOCKED;
    }
    return CN_NAVIGABLE;
}
#endif

static int
findcondition(s_char code)
{
    int x;

    x = 0;
    while (conditions[x].code) {
	if (conditions[x].code == code)
	    return (x);
	x++;
    }

    return (x);
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
retreat_land(struct lndstr *lp, s_char code)
{
    struct nstr_item ni;
    struct lndstr land;
    s_char buf[2];

    if (lp->lnd_rflags & RET_GROUP) {
	buf[0] = lp->lnd_army;
	buf[1] = 0;
	snxtitem(&ni, EF_SHIP, buf);
	while (nxtitem(&ni, (s_char *)&land))
	    if ((land.lnd_army == buf[0]) && (land.lnd_own == lp->lnd_own)) {
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
retreat_land1(struct lndstr *lp, s_char code, int orig)


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
    int time_to_stop;

    lp->lnd_mission = 0;
    if (lp->lnd_own == 0)
	return 0;

    if (isupper(code))
	code = tolower(code);

    n = 0;
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

    n = (-1 * MAX_RETREAT);
    stopping = 0;
    time_to_stop = 0;
    while ((!stopping) && n) {
	dx = dy = 0;
	if (lp->lnd_rpath[0] == 0 || lp->lnd_rpath[0] == 0) {
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
	dir = chkdir(lp->lnd_rpath[0], DIR_STOP, DIR_VIEW);
	memmove(lp->lnd_rpath, lp->lnd_rpath+1, sizeof(lp->lnd_rpath) - 1);
	if (dir == -1)
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
	if ((sect.sct_type == SCT_WATER) ||
	    (sect.sct_type == SCT_MOUNT) ||
	    (sect.sct_type == SCT_SANCT) ||
	    (sect.sct_type == SCT_WASTE) ||
	    (sect.sct_own != lp->lnd_own)) {
	    wu(0, lp->lnd_own, "%s %s,\nbut could not retreat to %s!\n",
	       prland(lp),
	       conditions[findcondition(code)].desc[orig],
	       xyas(newx, newy, lp->lnd_own));
	    if (!orig)
		putland(lp->lnd_uid, lp);
	    return 0;
	}
	mobcost = lnd_mobcost(lp, &sect, MOB_ROAD);
	lp->lnd_x = newx;
	lp->lnd_y = newy;
	lp->lnd_mobil -= mobcost;
	if (stopping)
	    continue;

	mines = sect.sct_mines;
	if ((lcp->l_flags & L_ENGINEER) && mines > 0 &&
	    (sect.sct_oldown != lp->lnd_own)) {
	    max = lcp->l_item[I_SHELL];
	    shells = lp->lnd_item[I_SHELL];
	    for (m = 0; mines > 0 && m < 5; m++) {
		if (chance(0.66)) {
		    mines--;
		    shells = min(max, shells + 1);
		}
	    }
	    sect.sct_mines = mines;
	    lp->lnd_item[I_SHELL] = shells;
	    putsect(&sect);
	}
	if (mines > 0 && (sect.sct_oldown != lp->lnd_own) &&
	    chance(DMINE_HITCHANCE(mines))) {
	    wu(0, lp->lnd_own,
	       "%s %s,\nand hit a mine in %s while retreating!\n",
	       prland(lp),
	       conditions[findcondition(code)].desc[orig],
	       xyas(newx, newy, lp->lnd_own));
	    nreport(lp->lnd_own, N_LHIT_MINE, 0, 1);
	    m = MINE_LDAMAGE();
	    landdamage(lp, m);
	    mines--;
	    sect.sct_mines = mines;
	    putsect(&sect);
	    if (lp->lnd_effic < LAND_MINEFF)
		time_to_stop = 1;
	    if (!orig)
		putland(lp->lnd_uid, lp);
	    return 0;
	}
	if (time_to_stop)
	    stopping = 1;
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
