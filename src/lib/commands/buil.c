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
 *  buil.c: Build ships, nukes, bridges, planes, land units, bridge towers
 *
 *  Known contributors to this file:
 *      Steve McClure, 1998-2000
 *      Markus Armbruster, 2004-2008
 */

#include <config.h>

#include <limits.h>
#include "commands.h"
#include "game.h"
#include "land.h"
#include "lost.h"
#include "map.h"
#include "nuke.h"
#include "optlist.h"
#include "path.h"
#include "plague.h"
#include "plane.h"
#include "ship.h"
#include "treaty.h"
#include "unit.h"

static int build_nuke(struct sctstr *sp,
		      struct nchrstr *np, short *vec, int tlev);
static int build_ship(struct sctstr *sp,
		      struct mchrstr *mp, short *vec, int tlev);
static int build_land(struct sctstr *sp,
		      struct lchrstr *lp, short *vec, int tlev);
static int build_bridge(struct sctstr *sp, short *vec);
static int build_tower(struct sctstr *sp, short *vec);
static int build_plane(struct sctstr *sp,
		       struct plchrstr *pp, short *vec, int tlev);
static int build_can_afford(double, char *);

/*
 * build <WHAT> <SECTS> <TYPE|DIR|MEG> [NUMBER]
 */
int
buil(void)
{
    struct sctstr sect;
    struct nstr_sect nstr;
    struct natstr *natp;
    int rqtech;
    int tlev;
    int rlev;
    int type;
    char what;
    struct lchrstr *lp;
    struct mchrstr *mp;
    struct plchrstr *pp;
    struct nchrstr *np;
    char *p;
    int gotsect = 0;
    int built;
    int number;
    char buf[1024];

    natp = getnatp(player->cnum);
    p = getstarg(player->argp[1],
		 "Build (ship, nuke, bridge, plane, land unit, tower)? ",
		 buf);
    if (p == 0)
	return RET_SYN;
    what = *p;

    if (!snxtsct(&nstr, player->argp[2])) {
	pr("Bad sector specification.\n");
	return RET_SYN;
    }
    tlev = (int)natp->nat_level[NAT_TLEV];
    rlev = (int)natp->nat_level[NAT_RLEV];

    switch (what) {
    case 'p':
	p = getstarg(player->argp[3], "Plane type? ", buf);
	if (p == 0 || *p == 0)
	    return RET_SYN;
	type = ef_elt_byname(EF_PLANE_CHR, p);
	if (type >= 0) {
	    pp = &plchr[type];
	    rqtech = pp->pl_tech;
	    if (rqtech > tlev)
		type = -1;
	}
	if (type < 0) {
	    pr("You can't build that!\n");
	    pr("Use `show plane build %d' to show types you can build.\n",
	       tlev);
	    return RET_FAIL;
	}
	break;
    case 's':
	p = getstarg(player->argp[3], "Ship type? ", buf);
	if (p == 0 || *p == 0)
	    return RET_SYN;
	type = ef_elt_byname(EF_SHIP_CHR, p);
	if (type >= 0) {
	    mp = &mchr[type];
	    rqtech = mp->m_tech;
	    if (rqtech > tlev)
		type = -1;
	    if ((mp->m_flags & M_TRADE) && !opt_TRADESHIPS)
		type = -1;
	}
	if (type < 0) {
	    pr("You can't build that!\n");
	    pr("Use `show ship build %d' to show types you can build.\n",
	       tlev);
	    return RET_FAIL;
	}
	break;
    case 'l':
	p = getstarg(player->argp[3], "Land unit type? ", buf);
	if (p == 0 || *p == 0)
	    return RET_SYN;
	type = ef_elt_byname(EF_LAND_CHR, p);
	if (type >= 0) {
	    lp = &lchr[type];
	    rqtech = lp->l_tech;
	    if (rqtech > tlev)
		type = -1;
	    if ((lp->l_flags & L_SPY) && !opt_LANDSPIES)
		type = -1;
	}
	if (type < 0) {
	    pr("You can't build that!\n");
	    pr("Use `show land build %d' to show types you can build.\n",
	       tlev);
	    return RET_FAIL;
	}
	break;
    case 'b':
	if (natp->nat_level[NAT_TLEV] + 0.005 < buil_bt) {
	    pr("Building a span requires a tech of %.0f\n", buil_bt);
	    return RET_FAIL;
	}
	break;
    case 't':
	if (!opt_BRIDGETOWERS) {
	    pr("Bridge tower building is disabled.\n");
	    return RET_FAIL;
	}
	if (natp->nat_level[NAT_TLEV] + 0.005 < buil_tower_bt) {
	    pr("Building a tower requires a tech of %.0f\n",
	       buil_tower_bt);
	    return RET_FAIL;
	}
	break;
    case 'n':
	if (!ef_nelem(EF_NUKE_CHR)) {
	    pr("There are no nukes in this game.\n");
	    return RET_FAIL;
	}
	p = getstarg(player->argp[3], "Nuke type? ", buf);
	if (p == 0 || *p == 0)
	    return RET_SYN;
	type = ef_elt_byname(EF_NUKE_CHR, p);
	if (type >= 0) {
	    np = &nchr[type];
	    rqtech = np->n_tech;
	    if (rqtech > tlev
		|| (drnuke_const > MIN_DRNUKE_CONST &&
		    np->n_tech * drnuke_const > rlev))
		type = -1;
	}
	if (type < 0) {
	    int tt = tlev;
	    if (drnuke_const > MIN_DRNUKE_CONST)
		tt = (tlev < (rlev / drnuke_const) ? (int)tlev :
		      (int)(rlev / drnuke_const));
	    pr("You can't build that!\n");
	    pr("Use `show nuke build %d' to show types you can build.\n",
	       tt);
	    return RET_FAIL;
	}
	break;
    default:
	pr("You can't build that!\n");
	return RET_SYN;
    }

    number = 1;
    if (what != 'b' && what != 't') {
	if (player->argp[4]) {
	    number = atoi(player->argp[4]);
	    if (number > 20) {
		char bstr[80];
		sprintf(bstr,
			"Are you sure that you want to build %s of them? ",
			player->argp[4]);
		p = getstarg(player->argp[6], bstr, buf);
		if (p == 0 || *p != 'y')
		    return RET_SYN;
	    }
	}
    }

    if (what != 'b' && what != 'n' && what != 't') {
	if (player->argp[5]) {
	    tlev = atoi(player->argp[5]);
	    if (tlev > natp->nat_level[NAT_TLEV] && !player->god) {
		pr("Your tech level is only %d.\n",
		   (int)natp->nat_level[NAT_TLEV]);
		return RET_FAIL;
	    }
	    if (rqtech > tlev) {
		pr("Required tech is %d.\n", rqtech);
		return RET_FAIL;
	    }
	    pr("building with tech level %d.\n", tlev);
	}
    }

    while (number-- > 0) {
	while (nxtsct(&nstr, &sect)) {
	    gotsect++;
	    if (!player->owner)
		continue;
	    switch (what) {
	    case 'l':
		built = build_land(&sect, lp, sect.sct_item, tlev);
		break;
	    case 's':
		built = build_ship(&sect, mp, sect.sct_item, tlev);
		break;
	    case 'b':
		built = build_bridge(&sect, sect.sct_item);
		break;
	    case 't':
		built = build_tower(&sect, sect.sct_item);
		break;
	    case 'n':
		built = build_nuke(&sect, np, sect.sct_item, tlev);
		break;
	    case 'p':
		built = build_plane(&sect, pp, sect.sct_item, tlev);
		break;
	    default:
		CANT_REACH();
		return RET_FAIL;
	    }
	    if (built) {
		putsect(&sect);
	    }
	}
	snxtsct_rewind(&nstr);
    }
    if (!gotsect) {
	pr("Bad sector specification.\n");
    }
    return RET_OK;
}

static int
build_ship(struct sctstr *sp, struct mchrstr *mp, short *vec, int tlev)
{
    struct shpstr ship;
    struct nstr_item nstr;
    int avail;
    double cost;
    double eff = SHIP_MINEFF / 100.0;
    int lcm, hcm;
    int freeship = 0;

    hcm = roundavg(mp->m_hcm * eff);
    lcm = roundavg(mp->m_lcm * eff);

    if (sp->sct_type != SCT_HARBR) {
	pr("Ships must be built in harbours.\n");
	return 0;
    }
    if (sp->sct_effic < 60 && !player->god) {
	pr("Sector %s is not 60%% efficient.\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
    if (vec[I_LCM] < lcm || vec[I_HCM] < hcm) {
	pr("Not enough materials in %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
    avail = (SHP_BLD_WORK(mp->m_lcm, mp->m_hcm) * SHIP_MINEFF + 99) / 100;
    if (sp->sct_avail < avail) {
	pr("Not enough available work in %s to build a %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum), mp->m_name);
	pr(" (%d available work required)\n", avail);
	return 0;
    }
    cost = mp->m_cost * SHIP_MINEFF / 100.0;
    if (!build_can_afford(cost, mp->m_name))
	return 0;
    if (!trechk(player->cnum, 0, NEWSHP))
	return 0;
    if (!check_sect_ok(sp))
	return 0;
    sp->sct_avail -= avail;
    player->dolcost += cost;
    snxtitem_all(&nstr, EF_SHIP);
    while (nxtitem(&nstr, &ship)) {
	if (ship.shp_own == 0) {
	    freeship++;
	    break;
	}
    }
    if (freeship == 0) {
	ef_extend(EF_SHIP, 50);
    }
    ef_blank(EF_SHIP, nstr.cur, &ship);
    ship.shp_x = sp->sct_x;
    ship.shp_y = sp->sct_y;
    ship.shp_own = player->cnum;
    ship.shp_type = mp - mchr;
    ship.shp_effic = SHIP_MINEFF;
    if (opt_MOB_ACCESS) {
	game_tick_to_now(&ship.shp_access);
	ship.shp_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	ship.shp_mobil = 0;
    }
    memset(ship.shp_item, 0, sizeof(ship.shp_item));
    ship.shp_pstage = PLG_HEALTHY;
    ship.shp_ptime = 0;
    ship.shp_name[0] = 0;
    ship.shp_orig_own = player->cnum;
    ship.shp_orig_x = sp->sct_x;
    ship.shp_orig_y = sp->sct_y;
    shp_set_tech(&ship, tlev);
    unit_wipe_orders((struct empobj *)&ship);

    vec[I_LCM] -= lcm;
    vec[I_HCM] -= hcm;

    if (sp->sct_pstage == PLG_INFECT)
	ship.shp_pstage = PLG_EXPOSED;
    putship(ship.shp_uid, &ship);
    pr("%s", prship(&ship));
    pr(" built in sector %s\n", xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_land(struct sctstr *sp, struct lchrstr *lp, short *vec, int tlev)
{
    struct lndstr land;
    struct nstr_item nstr;
    int avail;
    double cost;
    double eff = LAND_MINEFF / 100.0;
    int mil, lcm, hcm, gun, shell;
    int freeland = 0;

#if 0
    mil = roundavg(lp->l_mil * eff);
    shell = roundavg(lp->l_shell * eff);
    gun = roundavg(lp->l_gun * eff);
#else
    mil = shell = gun = 0;
#endif
    hcm = roundavg(lp->l_hcm * eff);
    lcm = roundavg(lp->l_lcm * eff);

    if (sp->sct_type != SCT_HEADQ) {
	pr("Land Units must be built in headquarters.\n");
	return 0;
    }
    if (sp->sct_effic < 60 && !player->god) {
	pr("Sector %s is not 60%% efficient.\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
    if (vec[I_LCM] < lcm || vec[I_HCM] < hcm) {
	pr("Not enough materials in %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
#if 0
    if (vec[I_GUN] < gun || vec[I_GUN] == 0) {
	pr("Not enough guns in %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
    if (vec[I_SHELL] < shell) {
	pr("Not enough shells in %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
    if (vec[I_MILIT] < mil) {
	pr("Not enough military in %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
#endif
    if (!trechk(player->cnum, 0, NEWLND))
	return 0;
    if (!check_sect_ok(sp))
	return 0;
    avail = (LND_BLD_WORK(lp->l_lcm, lp->l_hcm) * LAND_MINEFF + 99) / 100;
    if (sp->sct_avail < avail) {
	pr("Not enough available work in %s to build a %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum), lp->l_name);
	pr(" (%d available work required)\n", avail);
	return 0;
    }
    cost = lp->l_cost * LAND_MINEFF / 100.0;
    if (!build_can_afford(cost, lp->l_name))
	return 0;
    sp->sct_avail -= avail;
    player->dolcost += cost;
    snxtitem_all(&nstr, EF_LAND);
    while (nxtitem(&nstr, &land)) {
	if (land.lnd_own == 0) {
	    freeland++;
	    break;
	}
    }
    if (freeland == 0) {
	ef_extend(EF_LAND, 50);
    }
    ef_blank(EF_LAND, nstr.cur, &land);
    land.lnd_x = sp->sct_x;
    land.lnd_y = sp->sct_y;
    land.lnd_own = player->cnum;
    land.lnd_type = lp - lchr;
    land.lnd_effic = LAND_MINEFF;
    if (opt_MOB_ACCESS) {
	game_tick_to_now(&land.lnd_access);
	land.lnd_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	land.lnd_mobil = 0;
    }
    land.lnd_flags = 0;
    land.lnd_ship = -1;
    land.lnd_land = -1;
    land.lnd_harden = 0;
    memset(land.lnd_item, 0, sizeof(land.lnd_item));
    land.lnd_pstage = PLG_HEALTHY;
    land.lnd_ptime = 0;
    lnd_set_tech(&land, tlev);
    unit_wipe_orders((struct empobj *)&land);

    vec[I_LCM] -= lcm;
    vec[I_HCM] -= hcm;
    vec[I_MILIT] -= mil;
    vec[I_GUN] -= gun;
    vec[I_SHELL] -= shell;

    if (sp->sct_pstage == PLG_INFECT)
	land.lnd_pstage = PLG_EXPOSED;
    putland(nstr.cur, &land);
    pr("%s", prland(&land));
    pr(" built in sector %s\n", xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_bridge(struct sctstr *sp, short *vec)
{
    struct sctstr sect;
    int val;
    int newx, newy;
    int avail;
    int nx, ny, i, good = 0;
    char *p;
    char buf[1024];

    if (opt_EASY_BRIDGES == 0) {	/* must have a bridge head or tower */
	if (sp->sct_type != SCT_BTOWER) {
	    if (sp->sct_type != SCT_BHEAD)
		return 0;
	    if (sp->sct_newtype != SCT_BHEAD)
		return 0;
	}
    }

    if (sp->sct_effic < 60 && !player->god) {
	pr("Sector %s is not 60%% efficient.\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }

    if (vec[I_HCM] < buil_bh) {
	pr("%s only has %d unit%s of hcm,\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum),
	   vec[I_HCM], vec[I_HCM] > 1 ? "s" : "");
	pr("(a bridge span requires %d)\n", buil_bh);
	return 0;
    }

    if (!build_can_afford(buil_bc, dchr[SCT_BSPAN].d_name))
	return 0;
    avail = (SCT_BLD_WORK(0, buil_bh) * SCT_MINEFF + 99) / 100;
    if (sp->sct_avail < avail) {
	pr("Not enough available work in %s to build a bridge\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	pr(" (%d available work required)\n", avail);
	return 0;
    }
    if (!player->argp[3]) {
	pr("Bridge head at %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	nav_map(sp->sct_x, sp->sct_y, 1);
    }
    p = getstarg(player->argp[3], "build span in what direction? ", buf);
    if (!p || !*p) {
	return 0;
    }
    /* Sanity check time */
    if (!check_sect_ok(sp))
	return 0;

    if ((val = chkdir(*p, DIR_FIRST, DIR_LAST)) < 0) {
	pr("'%c' is not a valid direction...\n", *p);
	direrr(NULL, NULL, NULL);
	return 0;
    }
    newx = sp->sct_x + diroff[val][0];
    newy = sp->sct_y + diroff[val][1];
    if (getsect(newx, newy, &sect) == 0 || sect.sct_type != SCT_WATER) {
	pr("%s is not a water sector\n", xyas(newx, newy, player->cnum));
	return 0;
    }
    if (opt_EASY_BRIDGES) {
	good = 0;

	for (i = 1; i <= 6; i++) {
	    struct sctstr s2;
	    nx = sect.sct_x + diroff[i][0];
	    ny = sect.sct_y + diroff[i][1];
	    getsect(nx, ny, &s2);
	    if ((s2.sct_type != SCT_WATER) && (s2.sct_type != SCT_BSPAN))
		good = 1;
	}
	if (!good) {
	    pr("Bridges must be built adjacent to land or bridge towers.\n");
	    pr("That sector is not adjacent to land or a bridge tower.\n");
	    return 0;
	}
    }				/* end EASY_BRIDGES */
    sp->sct_avail -= avail;
    player->dolcost += buil_bc;
    sect.sct_type = SCT_BSPAN;
    sect.sct_newtype = SCT_BSPAN;
    sect.sct_effic = SCT_MINEFF;
    sect.sct_road = 0;
    sect.sct_rail = 0;
    sect.sct_defense = 0;
    if (opt_MOB_ACCESS) {
	game_tick_to_now(&sect.sct_access);
	sect.sct_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	sect.sct_mobil = 0;
    }
    sect.sct_mines = 0;
    map_set(player->cnum, sect.sct_x, sect.sct_y, dchr[SCT_BSPAN].d_mnem, 2);
    writemap(player->cnum);
    putsect(&sect);
    pr("Bridge span built over %s\n",
       xyas(sect.sct_x, sect.sct_y, player->cnum));
    vec[I_HCM] -= buil_bh;
    return 1;
}

static int
build_nuke(struct sctstr *sp, struct nchrstr *np, short *vec, int tlev)
{
    struct nukstr nuke;
    struct nstr_item nstr;
    int avail;
    int freenuke;

    if (sp->sct_type != SCT_NUKE && !player->god) {
	pr("Nuclear weapons must be built in nuclear plants.\n");
	return 0;
    }
    if (sp->sct_effic < 60 && !player->god) {
	pr("Sector %s is not 60%% efficient.\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
    if (vec[I_HCM] < np->n_hcm || vec[I_LCM] < np->n_lcm ||
	vec[I_OIL] < np->n_oil || vec[I_RAD] < np->n_rad) {
	pr("Not enough materials for a %s bomb in %s\n",
	   np->n_name, xyas(sp->sct_x, sp->sct_y, player->cnum));
	pr("(%d hcm, %d lcm, %d oil, & %d rads).\n",
	   np->n_hcm, np->n_lcm, np->n_oil, np->n_rad);
	return 0;
    }
    if (!build_can_afford(np->n_cost, np->n_name))
	return 0;
    avail = NUK_BLD_WORK(np->n_lcm, np->n_hcm, np->n_oil, np->n_rad);
    /*
     * XXX when nukes turn into units (or whatever), then
     * make them start at 20%.  Since they don't have efficiency
     * now, we charge all the work right away.
     */
    if (sp->sct_avail < avail) {
	pr("Not enough available work in %s to build a %s;\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum), np->n_name);
	pr(" (%d available work required)\n", avail);
	return 0;
    }
    if (!trechk(player->cnum, 0, NEWNUK))
	return 0;
    if (!check_sect_ok(sp))
	return 0;
    sp->sct_avail -= avail;
    player->dolcost += np->n_cost;
    snxtitem_all(&nstr, EF_NUKE);
    freenuke = 0;
    while (nxtitem(&nstr, &nuke)) {
	if (nuke.nuk_own == 0) {
	    freenuke++;
	    break;
	}
    }
    if (freenuke == 0) {
	ef_extend(EF_NUKE, 50);
    }
    ef_blank(EF_NUKE, nstr.cur, &nuke);
    nuke.nuk_x = sp->sct_x;
    nuke.nuk_y = sp->sct_y;
    nuke.nuk_own = sp->sct_own;
    nuke.nuk_type = np - nchr;
    nuke.nuk_effic = 100;
    nuke.nuk_ship = nuke.nuk_plane = nuke.nuk_land = -1;
    nuke.nuk_tech = tlev;
    unit_wipe_orders((struct empobj *)&nuke);

    vec[I_HCM] -= np->n_hcm;
    vec[I_LCM] -= np->n_lcm;
    vec[I_OIL] -= np->n_oil;
    vec[I_RAD] -= np->n_rad;

    putnuke(nuke.nuk_uid, &nuke);
    pr("%s created in %s\n", prnuke(&nuke),
       xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_plane(struct sctstr *sp, struct plchrstr *pp, short *vec, int tlev)
{
    struct plnstr plane;
    struct nstr_item nstr;
    int avail;
    double cost;
    double eff = PLANE_MINEFF / 100.0;
    int hcm, lcm, mil;
    int freeplane;

    mil = roundavg(pp->pl_crew * eff);
    /* Always use at least 1 mil to build a plane */
    if (mil == 0 && pp->pl_crew > 0)
	mil = 1;
    hcm = roundavg(pp->pl_hcm * eff);
    lcm = roundavg(pp->pl_lcm * eff);
    if (sp->sct_type != SCT_AIRPT && !player->god) {
	pr("Planes must be built in airports.\n");
	return 0;
    }
    if (sp->sct_effic < 60 && !player->god) {
	pr("Sector %s is not 60%% efficient.\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
    if (vec[I_LCM] < lcm || vec[I_HCM] < hcm) {
	pr("Not enough materials in %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
    avail = (PLN_BLD_WORK(pp->pl_lcm, pp->pl_hcm) * PLANE_MINEFF + 99) / 100;
    if (sp->sct_avail < avail) {
	pr("Not enough available work in %s to build a %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum), pp->pl_name);
	pr(" (%d available work required)\n", avail);
	return 0;
    }
    cost = pp->pl_cost * PLANE_MINEFF / 100.0;
    if (!build_can_afford(cost, pp->pl_name))
	return 0;
    if (vec[I_MILIT] < mil || (vec[I_MILIT] == 0 && pp->pl_crew > 0)) {
	pr("Not enough military for crew in %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }
    if (!trechk(player->cnum, 0, NEWPLN))
	return 0;
    if (!check_sect_ok(sp))
	return 0;
    sp->sct_avail -= avail;
    player->dolcost += cost;
    snxtitem_all(&nstr, EF_PLANE);
    freeplane = 0;
    while (nxtitem(&nstr, &plane)) {
	if (plane.pln_own == 0) {
	    freeplane++;
	    break;
	}
    }
    if (freeplane == 0) {
	ef_extend(EF_PLANE, 50);
    }
    ef_blank(EF_PLANE, nstr.cur, &plane);
    plane.pln_x = sp->sct_x;
    plane.pln_y = sp->sct_y;
    plane.pln_own = sp->sct_own;
    plane.pln_type = pp - plchr;
    plane.pln_effic = PLANE_MINEFF;
    if (opt_MOB_ACCESS) {
	game_tick_to_now(&plane.pln_access);
	plane.pln_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	plane.pln_mobil = 0;
    }
    plane.pln_range = UCHAR_MAX; /* will be adjusted by pln_set_tech() */
    plane.pln_ship = -1;
    plane.pln_land = -1;
    plane.pln_harden = 0;
    plane.pln_flags = 0;
    pln_set_tech(&plane, tlev);
    unit_wipe_orders((struct empobj *)&plane);

    vec[I_LCM] -= lcm;
    vec[I_HCM] -= hcm;
    vec[I_MILIT] -= mil;

    putplane(plane.pln_uid, &plane);
    pr("%s built in sector %s\n", prplane(&plane),
       xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_tower(struct sctstr *sp, short *vec)
{
    struct sctstr sect;
    int val;
    int newx, newy;
    int avail;
    char *p;
    char buf[1024];
    int good;
    int i;
    int nx;
    int ny;

    if (sp->sct_type != SCT_BSPAN) {
	pr("Bridge towers can only be built from bridge spans.\n");
	return 0;
    }

    if (sp->sct_effic < 60 && !player->god) {
	pr("Sector %s is not 60%% efficient.\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }

    if (vec[I_HCM] < buil_tower_bh) {
	pr("%s only has %d unit%s of hcm,\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum),
	   vec[I_HCM], vec[I_HCM] > 1 ? "s" : "");
	pr("(a bridge tower requires %d)\n", buil_tower_bh);
	return 0;
    }

    if (!build_can_afford(buil_tower_bc, dchr[SCT_BTOWER].d_name))
	return 0;
    avail = (SCT_BLD_WORK(0, buil_tower_bh) * SCT_MINEFF + 99) / 100;
    if (sp->sct_avail < avail) {
	pr("Not enough available work in %s to build a bridge tower\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	pr(" (%d available work required)\n", avail);
	return 0;
    }
    if (!player->argp[3]) {
	pr("Building from %s\n", xyas(sp->sct_x, sp->sct_y, player->cnum));
	nav_map(sp->sct_x, sp->sct_y, 1);
    }
    p = getstarg(player->argp[3], "build tower in what direction? ", buf);
    if (!p || !*p) {
	return 0;
    }
    /* Sanity check time */
    if (!check_sect_ok(sp))
	return 0;

    if ((val = chkdir(*p, DIR_FIRST, DIR_LAST)) < 0) {
	pr("'%c' is not a valid direction...\n", *p);
	direrr(NULL, NULL, NULL);
	return 0;
    }
    newx = sp->sct_x + diroff[val][0];
    newy = sp->sct_y + diroff[val][1];
    if (getsect(newx, newy, &sect) == 0 || sect.sct_type != SCT_WATER) {
	pr("%s is not a water sector\n", xyas(newx, newy, player->cnum));
	return 0;
    }

    /* Now, check.  You aren't allowed to build bridge towers
       next to land. */
    good = 0;
    for (i = 1; i <= 6; i++) {
	struct sctstr s2;
	nx = sect.sct_x + diroff[i][0];
	ny = sect.sct_y + diroff[i][1];
	getsect(nx, ny, &s2);
	if ((s2.sct_type != SCT_WATER) &&
	    (s2.sct_type != SCT_BTOWER) && (s2.sct_type != SCT_BSPAN)) {
	    good = 1;
	    break;
	}
    }
    if (good) {
	pr("Bridge towers cannot be built adjacent to land.\n");
	pr("That sector is adjacent to land.\n");
	return 0;
    }

    sp->sct_avail -= avail;
    player->dolcost += buil_tower_bc;
    sect.sct_type = SCT_BTOWER;
    sect.sct_newtype = SCT_BTOWER;
    sect.sct_effic = SCT_MINEFF;
    sect.sct_road = 0;
    sect.sct_rail = 0;
    sect.sct_defense = 0;
    if (opt_MOB_ACCESS) {
	game_tick_to_now(&sect.sct_access);
	sect.sct_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	sect.sct_mobil = 0;
    }
    sect.sct_mines = 0;
    map_set(player->cnum, sect.sct_x, sect.sct_y, dchr[SCT_BTOWER].d_mnem, 2);
    writemap(player->cnum);
    putsect(&sect);
    pr("Bridge tower built in %s\n",
       xyas(sect.sct_x, sect.sct_y, player->cnum));
    vec[I_HCM] -= buil_tower_bh;
    return 1;
}

static int
build_can_afford(double cost, char *what)
{
    struct natstr *natp = getnatp(player->cnum);
    if (natp->nat_money < player->dolcost + cost) {
	pr("Not enough money left to build a %s\n", what);
	return 0;
    }
    return 1;
}
