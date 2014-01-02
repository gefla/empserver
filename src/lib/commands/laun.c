/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  laun.c: Launch missiles from land or sea
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2005-2012
 */

#include <config.h>

#include "chance.h"
#include "commands.h"
#include "damage.h"
#include "mission.h"
#include "news.h"
#include "nuke.h"
#include "optlist.h"
#include "path.h"
#include "plague.h"
#include "plane.h"
#include "ship.h"

static int launch_as(struct plnstr *pp);
static int launch_missile(struct plnstr *pp);
static int launch_sat(struct plnstr *pp);
static int msl_equip(struct plnstr *, char);

/*
 * laun <PLANES>
 */
int
laun(void)
{
    struct nstr_item nstr;
    struct plnstr plane;
    struct plchrstr *pcp;
    int retval, gone;

    if (!snxtitem(&nstr, EF_PLANE, player->argp[1], NULL))
	return RET_SYN;
    while (nxtitem(&nstr, &plane)) {
	if (plane.pln_own != player->cnum)
	    continue;
	pcp = &plchr[(int)plane.pln_type];
	if ((pcp->pl_flags & (P_M | P_O)) == 0) {
	    pr("%s isn't a missile!\n", prplane(&plane));
	    continue;
	}
	if (pcp->pl_flags & P_F) {
	    pr("%s is a surface-to-air missile!\n", prplane(&plane));
	    continue;
	}
	if (pcp->pl_flags & P_N) {
	    pr("%s is an anti-ballistic-missile missile!\n",
	       prplane(&plane));
	    continue;
	}
	if (pln_is_in_orbit(&plane)) {
	    pr("%s already in orbit!\n", prplane(&plane));
	    continue;
	}
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, &plane)) {
		pr("plane #%d inelligible - it's for sale.\n",
		   plane.pln_uid);
		continue;
	    }
	}

	if (plane.pln_effic < 40) {
	    pr("%s is damaged (%d%%)\n", prplane(&plane), plane.pln_effic);
	    continue;
	}
	if (!pln_airbase_ok(&plane, 1, 1))
	    continue;
	pr("%s at %s; range %d, eff %d%%\n", prplane(&plane),
	   xyas(plane.pln_x, plane.pln_y, player->cnum),
	   plane.pln_range, plane.pln_effic);
	if (!(pcp->pl_flags & P_O)) {
	    retval = launch_missile(&plane);
	    gone = 1;
	} else if ((pcp->pl_flags & (P_M | P_O)) == (P_M | P_O)) {
	    retval = launch_as(&plane);
	    gone = 1;
	} else {		/* satellites */
	    retval = launch_sat(&plane);
	    gone = !(plane.pln_flags & PLN_LAUNCHED);
	}
	if (retval != RET_OK)
	    return retval;
	if (gone) {
	    plane.pln_effic = 0;
	    putplane(plane.pln_uid, &plane);
	}
    }
    return RET_OK;
}

/*
 * Launch anti-sat weapon PP.
 * Return RET_OK if launched (even when missile explodes),
 * else RET_SYN or RET_FAIL.
 */
static int
launch_as(struct plnstr *pp)
{
    char *cp, buf[1024];
    struct plnstr plane;

    cp = getstarg(player->argp[2], "Target satellite? ", buf);
    if (!check_plane_ok(pp))
	return RET_FAIL;
    if (!cp || !*cp)
	return RET_SYN;
    if (!getplane(atoi(cp), &plane) || !plane.pln_own
	|| !pln_is_in_orbit(&plane)) {
	pr("No such satellite exists!\n");
	return RET_FAIL;
	/* Can be abused to find satellite ids.  Tolerable.  */
    }

    if (mapdist(pp->pln_x, pp->pln_y, plane.pln_x, plane.pln_y)
	> pp->pln_range) {
	pr("Range too great!\n");
	return RET_FAIL;
    }
    if (msl_equip(pp, 'i') < 0)
	return RET_FAIL;
    if (msl_launch(pp, EF_PLANE, prplane(&plane),
		   plane.pln_x, plane.pln_y, plane.pln_own, NULL) < 0)
	return RET_OK;
    if (msl_hit(pp, pln_def(&plane), EF_PLANE, 0, 0, 0, plane.pln_own)) {
	pr("Satellite shot down\n");
	mpr(plane.pln_own, "%s anti-sat destroyed %s over %s\n",
	    cname(player->cnum), prplane(&plane),
	    xyas(plane.pln_x, plane.pln_y, plane.pln_own));
	nreport(pp->pln_own, N_SAT_KILL, plane.pln_own, 1);
	plane.pln_effic = 0;
	putplane(plane.pln_uid, &plane);
    }
    return RET_OK;
}

/*
 * Launch missile PP.
 * Return RET_OK if launched (even when missile explodes),
 * else RET_SYN or RET_FAIL.
 */
static int
launch_missile(struct plnstr *pp)
{
    struct plchrstr *pcp = plchr + pp->pln_type;
    coord sx, sy;
    int n, dam, sublaunch;
    char *cp;
    struct mchrstr *mcp;
    struct shpstr target_ship;
    struct sctstr sect;
    struct nukstr nuke;
    char buf[1024];

    if (pcp->pl_flags & P_MAR)
	cp = getstarg(player->argp[2], "Target ship? ", buf);
    else
	cp = getstarg(player->argp[2], "Target sector? ", buf);
    if (!cp || !*cp)
	return RET_SYN;
    if (!check_plane_ok(pp))
	return RET_FAIL;
    if (sarg_type(cp) == NS_LIST) {
	if (!(pcp->pl_flags & P_MAR)) {
	    pr("Missile not designed to attack ships!\n");
	    return RET_FAIL;
	}
	n = atoi(cp);
	if ((n < 0) || !getship(n, &target_ship) || !target_ship.shp_own) {
	    pr("Bad ship number!\n");
	    return RET_FAIL;
	}
	sx = target_ship.shp_x;
	sy = target_ship.shp_y;
	mcp = &mchr[(int)target_ship.shp_type];
	if (mcp->m_flags & M_SUB) {
	    pr("Bad ship number!\n");
	    return RET_FAIL;
	}
    } else if (!sarg_xy(cp, &sx, &sy)) {
	pr("Not a sector!\n");
	return RET_FAIL;
    } else {
	if (pcp->pl_flags & P_MAR) {
	    pr("Missile designed to attack ships!\n");
	    return RET_FAIL;
	}
    }

    if (mapdist(pp->pln_x, pp->pln_y, sx, sy) > pp->pln_range) {
	pr("Range too great; try again!\n");
	return RET_FAIL;
    }
    if (!(pcp->pl_flags & P_MAR)) {
	if (msl_equip(pp, 's') < 0)
	    return RET_FAIL;
	getsect(sx, sy, &sect);
	if (msl_launch(pp, EF_SECTOR, "sector", sx, sy, sect.sct_own,
		       &sublaunch) < 0)
	    return RET_OK;
	getsect(sx, sy, &sect);
	if (!msl_hit(pp, SECT_HARDTARGET, EF_SECTOR,
		     N_SCT_MISS, N_SCT_SMISS, sublaunch, sect.sct_own))
	    CANT_REACH();
	if (getnuke(nuk_on_plane(pp), &nuke))
	    detonate(&nuke, sx, sy, pp->pln_flags & PLN_AIRBURST);
	else {
	    dam = pln_damage(pp, 's', 1);
	    pr("did %d damage in %s\n", PERCENT_DAMAGE(dam),
	       xyas(sx, sy, player->cnum));
	    if (sect.sct_own != 0) {
		if (sublaunch)
		    wu(0, sect.sct_own,
		       "Sub missile attack did %d damage in %s\n",
		       dam, xyas(sx, sy, sect.sct_own));
		else
		    wu(0, sect.sct_own,
		       "%s missile attack did %d damage in %s\n",
		       cname(player->cnum), dam,
		       xyas(sx, sy, sect.sct_own));
	    }
	    sectdamage(&sect, dam);
	    putsect(&sect);
	}
    } else {
	if (msl_equip(pp, 'p') < 0)
	    return RET_FAIL;
	if (msl_launch(pp, EF_SHIP, prship(&target_ship),
		       target_ship.shp_x, target_ship.shp_y,
		       target_ship.shp_own, &sublaunch) < 0)
	    return RET_OK;
	getship(n, &target_ship);
	if (!msl_hit(pp, shp_hardtarget(&target_ship), EF_SHIP,
		     N_SHP_MISS, N_SHP_SMISS, sublaunch,
		     target_ship.shp_own)) {
	    pr("splash\n");
	    dam = pln_damage(pp, 'p', 0);
	    collateral_damage(target_ship.shp_x, target_ship.shp_y, dam);
	    return RET_OK;
	}
	dam = pln_damage(pp, 'p', 1);
	check_retreat_and_do_shipdamage(&target_ship, dam);
	putship(target_ship.shp_uid, &target_ship);
	getship(target_ship.shp_uid, &target_ship);
	if (!target_ship.shp_own)
	    pr("%s sunk!\n", prship(&target_ship));
    }
    return RET_OK;
}

/*
 * Launch a satellite.
 * Return RET_OK if launched (even when satellite fails),
 * else RET_SYN or RET_FAIL.
 */
static int
launch_sat(struct plnstr *pp)
{
    coord sx, sy;
    int i;
    int dist;
    int dir;
    char *cp;
    char *p;
    char buf[1024];

    pr("\n");
    cp = getstarg(player->argp[2], "Target sector? ", buf);
    if (!check_plane_ok(pp))
	return RET_FAIL;
    if (!cp || !*cp)
	return RET_SYN;
    if (!sarg_xy(cp, &sx, &sy)) {
	pr("Bad sector designation!\n");
	return RET_SYN;
    }
    if ((dist = mapdist(pp->pln_x, pp->pln_y, sx, sy)) > pp->pln_range) {
	pr("Range too great; try again!\n");
	return RET_FAIL;
    }
    p = getstring("Geostationary orbit? ", buf);
    if (!p)
	return RET_SYN;
    if (!check_plane_ok(pp))
	return RET_FAIL;
    if (msl_equip(pp, 'r') < 0)
	return RET_FAIL;
    pp->pln_theta = 0;
    pp->pln_flags |= PLN_SYNCHRONOUS;
    if (*p == 0 || *p == 'n')
	pp->pln_flags &= ~(PLN_SYNCHRONOUS);
    pr("3... 2... 1... Blastoff!!!\n");
    if (chance(0.07 + (100 - pp->pln_effic) / 100.0)) {
	pr("KABOOOOM!  Range safety officer detonates booster!\n");
	pp->pln_effic = 0;
	return RET_OK;
    }
    i = pp->pln_tech + pp->pln_effic;
    if (chance(1.0 - (i / (i + 50.0)))) {
	dir = DIR_FIRST + roll0(6);
	sx = xnorm(sx + diroff[dir][0]);
	sy = ynorm(sy + diroff[dir][1]);
	pr("Your trajectory was a little off.\n");
    }
    nreport(player->cnum, N_LAUNCH, 0, 1);
    if (msl_asat_intercept(pp, sx, sy))
	return RET_OK;
    pp->pln_x = sx;
    pp->pln_y = sy;
    CANT_HAPPEN(pp->pln_flags & PLN_LAUNCHED);
    pp->pln_flags |= PLN_LAUNCHED;
    pp->pln_mobil = pp->pln_mobil > dist ? pp->pln_mobil - dist : 0;
    putplane(pp->pln_uid, pp);
    pr("%s positioned over %s, will be ready for use in %d time units\n",
       prplane(pp), xyas(sx, sy, player->cnum),
       plane_mob_max - pp->pln_mobil);
    return RET_OK;
}

static int
msl_equip(struct plnstr *pp, char mission)
{
    struct plist pl;

    pl.load = 0;
    pl.pstage = PLG_HEALTHY;
    pl.pcp = plchr + pp->pln_type;
    pl.plane = *pp;
    emp_initque(&pl.queue);
    return pln_equip(&pl, NULL, mission);
}
