/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  upgr.c: Upgrade tech of ships/planes/units
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996-2000
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "commands.h"
#include "land.h"
#include "plane.h"
#include "ship.h"

enum {
    UPGR_COST = 15,		/* how much avail and money to charge */
    UPGR_EFF = 35		/* efficiency reduction */
};

static int lupgr(void);
static int pupgr(void);
static int supgr(void);

int
upgr(void)
{
    char *p;
    char buf[1024];

    if (!(p = getstarg(player->argp[1], "Ship, land, or plane? ", buf)))
	return RET_SYN;
    switch (*p) {
    case 's':
    case 'S':
	return supgr();
    case 'p':
    case 'P':
	return pupgr();
    case 'l':
    case 'L':
	return lupgr();
    default:
	break;
    }
    pr("Ships, land units or planes only!\n");
    return RET_SYN;
}

static int
lupgr(void)
{
    struct sctstr sect;
    struct natstr *natp;
    struct nstr_item ni;
    struct lndstr land;
    struct lchrstr *lp;
    int n;
    int tlev;
    int avail, cost;
    int cash;

    if (!snxtitem(&ni, EF_LAND, player->argp[2], NULL))
	return RET_SYN;
    natp = getnatp(player->cnum);
    cash = natp->nat_money;
    tlev = (int)natp->nat_level[NAT_TLEV];
    n = 0;
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == 0)
	    continue;
	getsect(land.lnd_x, land.lnd_y, &sect);
	if (sect.sct_own != player->cnum)
	    continue;
	if (sect.sct_type != SCT_HEADQ || sect.sct_effic < 60)
	    continue;
	if (relations_with(land.lnd_own, sect.sct_own) < FRIENDLY) {
	    pr("You are not on friendly terms with the owner of unit %d!\n",
	       land.lnd_uid);
	    continue;
	}
	n++;
	lp = &lchr[(int)land.lnd_type];
	avail = (lp->l_bwork * UPGR_COST + 99) / 100;
	if (sect.sct_avail < avail) {
	    pr("Not enough available work in %s to upgrade a %s\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), lp->l_name);
	    pr(" (%d available work required)\n", avail);
	    continue;
	}
	if (land.lnd_effic < 60) {
	    pr("%s is too damaged to upgrade!\n", prland(&land));
	    continue;
	}
	if (land.lnd_tech >= tlev) {
	    pr("%s tech: %d, yours is only %d\n",
	       prland(&land), land.lnd_tech, tlev);
	    continue;
	}
	cost = lp->l_cost * UPGR_COST / 100;
	if (cost + player->dolcost > cash) {
	    pr("You don't have enough money to upgrade %s!\n",
	       prland(&land));
	    continue;
	}

	sect.sct_avail -= avail;
	land.lnd_effic -= UPGR_EFF;
	lnd_set_tech(&land, tlev);
	land.lnd_harden = 0;
	land.lnd_mission = 0;

	putland(land.lnd_uid, &land);
	putsect(&sect);
	player->dolcost += cost;
	pr("%s upgraded to tech %d, at a cost of %d\n",
	   prland(&land), land.lnd_tech, cost);
	if (land.lnd_own != player->cnum)
	    wu(0, land.lnd_own,
	       "%s upgraded by %s to tech %d, at a cost of %d\n",
	       prland(&land), cname(player->cnum), land.lnd_tech, cost);
    }
    if (n == 0) {
	pr("No land units\n");
	return RET_SYN;
    }
    return RET_OK;
}

static int
supgr(void)
{
    struct sctstr sect;
    struct natstr *natp;
    struct nstr_item ni;
    struct shpstr ship;
    struct mchrstr *mp;
    int n;
    int tlev;
    int avail, cost;
    int cash;

    if (!snxtitem(&ni, EF_SHIP, player->argp[2], NULL))
	return RET_SYN;
    natp = getnatp(player->cnum);
    cash = natp->nat_money;
    tlev = (int)natp->nat_level[NAT_TLEV];
    n = 0;
    while (nxtitem(&ni, &ship)) {
	if (ship.shp_own == 0)
	    continue;
	getsect(ship.shp_x, ship.shp_y, &sect);
	if (sect.sct_own != player->cnum)
	    continue;
	if (sect.sct_type != SCT_HARBR || sect.sct_effic < 60)
	    continue;
	if (relations_with(ship.shp_own, sect.sct_own) < FRIENDLY) {
	    pr("You are not on friendly terms with the owner of ship %d!\n",
	       ship.shp_uid);
	    continue;
	}
	n++;
	mp = &mchr[(int)ship.shp_type];
	avail = (mp->m_bwork * UPGR_COST + 99) / 100;
	if (sect.sct_avail < avail) {
	    pr("Not enough available work in %s to upgrade a %s\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), mp->m_name);
	    pr(" (%d available work required)\n", avail);
	    continue;
	}
	if (ship.shp_effic < 60) {
	    pr("%s is too damaged to upgrade!\n", prship(&ship));
	    continue;
	}
	if (ship.shp_tech >= tlev) {
	    pr("%s tech: %d, yours is only %d\n",
	       prship(&ship), ship.shp_tech, tlev);
	    continue;
	}
	cost = mp->m_cost * UPGR_COST / 100;
	if (cost + player->dolcost > cash) {
	    pr("You don't have enough money to upgrade %s!\n",
	       prship(&ship));
	    continue;
	}

	sect.sct_avail -= avail;
	ship.shp_effic -= UPGR_EFF;
	shp_set_tech(&ship, tlev);
	ship.shp_mission = 0;

	putship(ship.shp_uid, &ship);
	putsect(&sect);
	player->dolcost += cost;
	pr("%s upgraded to tech %d, at a cost of %d\n",
	   prship(&ship), ship.shp_tech, cost);
	if (ship.shp_own != player->cnum)
	    wu(0, ship.shp_own,
	       "%s upgraded by %s to tech %d, at a cost of %d\n",
	       prship(&ship), cname(player->cnum), ship.shp_tech, cost);
    }
    if (n == 0) {
	pr("No ships\n");
	return RET_SYN;
    }
    return RET_OK;
}

static int
pupgr(void)
{
    struct sctstr sect;
    struct natstr *natp;
    struct nstr_item ni;
    struct plnstr plane;
    struct plchrstr *pp;
    int n;
    int tlev;
    int avail, cost;
    int cash;

    if (!snxtitem(&ni, EF_PLANE, player->argp[2], NULL))
	return RET_SYN;
    natp = getnatp(player->cnum);
    cash = natp->nat_money;
    tlev = (int)natp->nat_level[NAT_TLEV];
    n = 0;
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own == 0)
	    continue;
	getsect(plane.pln_x, plane.pln_y, &sect);
	if (sect.sct_own != player->cnum)
	    continue;
	if (sect.sct_type != SCT_AIRPT || sect.sct_effic < 60)
	    continue;
	if (relations_with(plane.pln_own, sect.sct_own) < FRIENDLY) {
	    pr("You are not on friendly terms with the owner of plane %d!\n",
	       plane.pln_uid);
	    continue;
	}
	if (pln_is_in_orbit(&plane)) {
	    pr("Plane %s is in orbit!\n", prplane(&plane));
	    continue;
	}
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	n++;
	pp = &plchr[(int)plane.pln_type];
	avail = (pp->pl_bwork * UPGR_COST + 99) / 100;
	if (sect.sct_avail < avail) {
	    pr("Not enough available work in %s to upgrade a %s\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), pp->pl_name);
	    pr(" (%d available work required)\n", avail);
	    continue;
	}
	if (plane.pln_effic < 60) {
	    pr("%s is too damaged to upgrade!\n", prplane(&plane));
	    continue;
	}
	if (plane.pln_tech >= tlev) {
	    pr("%s tech: %d, yours is only %d\n",
	       prplane(&plane), plane.pln_tech, tlev);
	    continue;
	}
	cost = pp->pl_cost * UPGR_COST / 100;
	if (cost + player->dolcost > cash) {
	    pr("You don't have enough money to upgrade %s!\n",
	       prplane(&plane));
	    continue;
	}

	sect.sct_avail -= avail;
	plane.pln_effic -= UPGR_EFF;
	pln_set_tech(&plane, tlev);
	plane.pln_harden = 0;
	plane.pln_mission = 0;

	putplane(plane.pln_uid, &plane);
	putsect(&sect);
	player->dolcost += cost;
	pr("%s upgraded to tech %d, at a cost of %d\n",
	   prplane(&plane), plane.pln_tech, cost);
	if (plane.pln_own != player->cnum)
	    wu(0, plane.pln_own,
	       "%s upgraded by %s to tech %d, at a cost of %d\n",
	       prplane(&plane), cname(player->cnum), plane.pln_tech, cost);
    }
    if (n == 0) {
	pr("No planes.\n");
	return RET_SYN;
    }
    return RET_OK;
}
