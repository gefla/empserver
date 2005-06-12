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
 *  laun.c: Launch missiles from land or sea
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "plane.h"
#include "news.h"
#include "mission.h"
#include "ship.h"
#include "nsc.h"
#include "nat.h"
#include "path.h"
#include "file.h"
#include "optlist.h"
#include "damage.h"
#include "commands.h"

static int launch_as(struct plnstr *pp);
static int launch_missile(struct plnstr *pp, int sublaunch);
static int launch_sat(struct plnstr *pp, int sublaunch);

/*
 * laun <PLANES>
 */
int
laun(void)
{
    struct nstr_item nstr;
    struct plnstr plane;
    struct shpstr ship;
    struct sctstr sect;
    int sublaunch;
    struct plchrstr *pcp;
    int rel;
    struct natstr *natp;

    if (!snxtitem(&nstr, EF_PLANE, player->argp[1]))
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
	if ((plane.pln_flags & PLN_LAUNCHED) && (pcp->pl_flags & P_O)) {
	    pr("%s already in orbit!\n", prplane(&plane));
	    continue;
	}
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, (int *)&plane)) {
		pr("plane #%d inelligible - it's for sale.\n",
		   plane.pln_uid);
		continue;
	    }
	}

	sublaunch = 0;
	if (plane.pln_ship >= 0) {
	    getship(plane.pln_ship, &ship);
	    if (!ship.shp_own) {
		pr("%s: ship #%d was sunk!\n",
		   prplane(&plane), ship.shp_uid);
		makelost(EF_PLANE, plane.pln_own, plane.pln_uid,
			 plane.pln_x, plane.pln_y);
		plane.pln_own = 0;
		putplane(plane.pln_uid, &plane);
		continue;
	    }
	    natp = getnatp(ship.shp_own);
	    rel = getrel(natp, player->cnum);
	    if (ship.shp_own != player->cnum && rel != ALLIED) {
		pr("%s: you or an ally do not own ship #%d\n",
		   prplane(&plane), ship.shp_uid);
		makelost(EF_PLANE, plane.pln_own, plane.pln_uid,
			 plane.pln_x, plane.pln_y);
		plane.pln_own = 0;
		putplane(plane.pln_uid, &plane);
		continue;
	    }
	    if (mchr[(int)ship.shp_type].m_flags & M_SUB)
		sublaunch = 1;
	} else {
	    sublaunch = 0;
	    getsect(plane.pln_x, plane.pln_y, &sect);
	    natp = getnatp(sect.sct_own);
	    rel = getrel(natp, player->cnum);
	    if (sect.sct_own && sect.sct_own != player->cnum
		&& rel != ALLIED) {
		pr("%s: you or an ally do not own sector %s!\n",
		   prplane(&plane), xyas(plane.pln_x, plane.pln_y,
					 player->cnum));
		continue;
	    }
	}
	if (plane.pln_effic < 60) {
	    pr("%s is damaged (%d%%)\n", prplane(&plane), plane.pln_effic);
	    continue;
	}
	pr("%s at %s; range %d, eff %d%%\n", prplane(&plane),
	   xyas(plane.pln_x, plane.pln_y, player->cnum),
	   plane.pln_range, plane.pln_effic);
	if (!(pcp->pl_flags & P_O)) {
	    if (launch_missile(&plane, sublaunch) < 0)
		continue;
	} else if ((pcp->pl_flags & (P_M | P_O)) == (P_M | P_O)) {
	    if (launch_as(&plane) < 0)	/* anti-sat */
		continue;
	} else {		/* satellites */
	    if (launch_sat(&plane, sublaunch) < 0)
		continue;
	}
	makelost(EF_PLANE, plane.pln_own, plane.pln_uid, plane.pln_x,
		 plane.pln_y);
	plane.pln_own = 0;
	putplane(plane.pln_uid, &plane);
    }
    return RET_OK;
}

/*
 * Launch an anti-sat weapon.
 * Return -1 on failure, 0 on success (even if missile explodes).
 */

static int
launch_as(struct plnstr *pp)
{
    coord sx, sy;
    s_char *cp, buf[1024];
    struct plnstr plane;
    struct nstr_item ni;
    int goodtarget;
    int dam, nukedam;
    natid oldown;

    if (msl_equip(pp) < 0) {
	pr("%s not enough petrol or shells!\n", prplane(pp));
	return -1;
    }
    for (;;) {
	cp = getstarg(player->argp[2], "Target sector? ", buf);
	if (!check_plane_ok(pp))
	    return -1;
	player->argp[2] = 0;
	if (!cp || !*cp)
	    return -1;
	if (!sarg_xy(cp, &sx, &sy)) {
	    pr("Bad sector designation; try again!\n");
	    continue;
	}
	if (mapdist(pp->pln_x, pp->pln_y, sx, sy) > pp->pln_range) {
	    pr("Range too great; try again!\n");
	    continue;
	}
	break;
    }
    goodtarget = 0;
    snxtitem_dist(&ni, EF_PLANE, sx, sy, 0);
    while (!goodtarget && nxtitem(&ni, &plane)) {
	if (!plane.pln_own)
	    continue;
	if (!(plane.pln_flags & PLN_LAUNCHED))
	    continue;
	goodtarget = 1;

    }
    if (!goodtarget) {
	pr("No satellites there!\n");
	return -1;
    }
    if (msl_hit(pp, plane.pln_def, EF_PLANE, N_SAT_KILL, N_SAT_KILL,
		prplane(&plane), sx, sy, plane.pln_own)) {
	dam = pln_damage(pp, sx, sy, 'p', &nukedam, 1);
	oldown = plane.pln_own;
	planedamage(&plane, dam);
	pr("Hit satellite for %d%% damage!\n", dam);
	mpr(oldown,
	    "%s anti-sat did %d%% damage to %s over %s\n",
	    cname(player->cnum), dam, prplane(&plane),
	    xyas(plane.pln_x, plane.pln_y, plane.pln_own));
	putplane(plane.pln_uid, &plane);
	if (!plane.pln_own)
	    mpr(oldown, "Satellite shot down\n");
    }
    return 0;
}

/*
 * Launch a missile
 * Return -1 on failure, 0 on success (even if missile explodes).
 */
static int
launch_missile(struct plnstr *pp, int sublaunch)
{
    struct plchrstr *pcp = plchr + pp->pln_type;
    coord sx, sy;
    int n, dam;
    s_char *cp;
    struct mchrstr *mcp;
    struct shpstr target_ship;
    struct sctstr sect;
    int nukedam;
    int rel;
    struct natstr *natp;
    s_char buf[1024];

    for (;;) {
	if (pcp->pl_flags & P_MAR)
	    cp = getstarg(player->argp[2], "Target ship? ", buf);
	else
	    cp = getstarg(player->argp[2], "Target sector? ", buf);
	player->argp[2] = 0;
	if (!cp || !*cp)
	    return -1;
	if (!check_plane_ok(pp))
	    return -1;
	if (opt_PINPOINTMISSILE && sarg_type(cp) == NS_LIST) {
	    if (!(pcp->pl_flags & P_MAR)) {
		pr("Missile not designed to attack ships!\n");
		continue;
	    }
	    n = atoi(cp);
	    if ((n < 0) || !getship(n, &target_ship) ||
		!target_ship.shp_own) {
		pr("Bad ship number; try again!\n");
		continue;
	    }
	    sx = target_ship.shp_x;
	    sy = target_ship.shp_y;
	    mcp = &mchr[(int)target_ship.shp_type];
	    if (mcp->m_flags & M_SUB) {
		pr("Bad ship number; try again!\n");
		continue;
	    }
	} /* not PINPOINTMISSILE for ships */
	else if (!sarg_xy(cp, &sx, &sy)) {
	    pr("That's no good! try again!\n");
	    continue;
	} else if (opt_PINPOINTMISSILE) {
	    if (pcp->pl_flags & P_MAR) {
		pr("Missile designed to attack ships!\n");
		continue;
	    }
	}
	/* end PINPOINTMISSILE */
	if (mapdist(pp->pln_x, pp->pln_y, sx, sy) > pp->pln_range) {
	    pr("Range too great; try again!\n");
	    continue;
	}
	break;
    }
    if (msl_equip(pp) < 0) {
	pr("%s not enough shells!\n", prplane(pp));
	return -1;
    }
    if (opt_PINPOINTMISSILE == 0 || !(pcp->pl_flags & P_MAR)) {
	getsect(sx, sy, &sect);
	if (opt_SLOW_WAR) {
	    natp = getnatp(player->cnum);
	    rel = getrel(natp, sect.sct_own);
	    if ((rel != AT_WAR) && (sect.sct_own != player->cnum) &&
		(sect.sct_own) && (sect.sct_oldown != player->cnum)) {
		pr("You are not at war with the player->owner of the target sector!\n");
		pr_beep();
		pr("Kaboom!!!\n");
		pr("Missile monitoring officer destroys RV before detonation.\n");
		return 0;
	    }
	}
	if (!msl_hit(pp, SECT_HARDTARGET, EF_SECTOR, N_SCT_MISS,
		     N_SCT_SMISS, "sector", sx, sy, sect.sct_own)) {
	    /*
	       dam = pln_damage(pp, sect.sct_x, sect.sct_y, 's', &nukedam, 0);
	       collateral_damage(sect.sct_x, sect.sct_y, dam, 0);
	     */
	    return 0;
	}
	dam = pln_damage(pp, sect.sct_x, sect.sct_y, 's', &nukedam, 1);
	if (!nukedam) {
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
	    sectdamage(&sect, dam, 0);
	    putsect(&sect);
	}
    } /* end PINPOINTMISSILE conditional */
    else if (opt_PINPOINTMISSILE) {	/* else */
	if (!msl_hit(pp, shp_hardtarget(&target_ship), EF_SHIP,
		     N_SHP_MISS, N_SHP_SMISS, prship(&target_ship),
		     target_ship.shp_x, target_ship.shp_y,
		     target_ship.shp_own)) {
	    pr("splash\n");
	    /*
	       dam = pln_damage(pp,target_ship.shp_x,target_ship.shp_y,'p',&nukedam, 0);
	       collateral_damage(target_ship.shp_x, target_ship.shp_y, dam, 0);
	     */
	    return 0;
	}
	dam =
	    pln_damage(pp, target_ship.shp_x, target_ship.shp_y, 'p',
		       &nukedam, 1);
	if (!nukedam) {
	    check_retreat_and_do_shipdamage(&target_ship, dam);
	    if (target_ship.shp_effic < SHIP_MINEFF)
		pr("\t%s sunk!\n", prship(&target_ship));
	    putship(target_ship.shp_uid, &target_ship);
	}
	getship(target_ship.shp_uid, &target_ship);
	if (!target_ship.shp_own)
	    pr("%s sunk!\n", prship(&target_ship));
    }
    /* end PINPOINTMISSILE */
    return 0;
}

/*
 * Launch a satellite.
 * Return -1 on error, 0 on success (even if the satellite fails).
 */
static int
launch_sat(struct plnstr *pp, int sublaunch)
{
    struct plchrstr *pcp = plchr + pp->pln_type;
    coord sx, sy;
    int i;
    int dist;
    int dir;
    s_char *cp;
    s_char *p;
    s_char buf[1024];

    pr("\n");
    while (1) {
	cp = getstarg(player->argp[2], "Target sector? ", buf);
	if (!check_plane_ok(pp))
	    return -1;
	player->argp[2] = 0;
	if (!cp || !*cp)
	    return -1;
	if (!sarg_xy(cp, &sx, &sy)) {
	    pr("Bad sector designation; try again!\n");
	    continue;
	}
	if ((dist = mapdist(pp->pln_x, pp->pln_y, sx, sy)) > pp->pln_range) {
	    pr("Range too great; try again!\n");
	    continue;
	}
	break;
    }
    p = getstring("Geostationary orbit? ", buf);
    if (p == 0)
	return -1;
    if (!check_plane_ok(pp))
	return -1;
    pp->pln_theta = 0;
    pp->pln_flags |= PLN_SYNCHRONOUS;
    if (*p == 0 || *p == 'n')
	pp->pln_flags &= ~(PLN_SYNCHRONOUS);
    pr("3... 2... 1... Blastoff!!!\n");
    if (chance(0.07 + (100 - pp->pln_effic) / 100.0)) {
	pr("KABOOOOM!  Range safety officer detonates booster!\n");
	makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x, pp->pln_y);
	pp->pln_own = 0;
	return 0;
    }
    i = pp->pln_tech + pp->pln_effic;
    if (chance(1.0 - (i / (i + 50.0)))) {
	dir = (random() % 6) + 1;
	sx += diroff[dir][0];
	sy += diroff[dir][1];
	pr("Your trajectory was a little off.\n");
    }
    nreport(player->cnum, N_LAUNCH, 0, 1);
    pr("%s positioned over %s", prplane(pp), xyas(sx, sy, player->cnum));
    if (msl_intercept(sx, sy, pp->pln_own, pcp->pl_def, sublaunch, P_O, 0)) {
	return 0;
    }
    pp->pln_x = sx;
    pp->pln_y = sy;
    pp->pln_flags |= PLN_LAUNCHED;
    pp->pln_mobil = (pp->pln_mobil > dist) ? (pp->pln_mobil - dist) : 0;
    putplane(pp->pln_uid, pp);
    pr(", will be ready for use in %d time units\n", 127 - pp->pln_mobil);
    return -1;
}
