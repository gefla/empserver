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
 *  buil.c: Build ships, nukes, bridges, planes, land units, bridge towers
 * 
 *  Known contributors to this file:
 *      Steve McClure, 1998-2000
 */

#include <config.h>

#include <limits.h>
#include <string.h>
#include "misc.h"
#include "player.h"
#include "plague.h"
#include "sect.h"
#include "nat.h"
#include "ship.h"
#include "land.h"
#include "nuke.h"
#include "plane.h"
#include "xy.h"
#include "nsc.h"
#include "treaty.h"
#include "file.h"
#include "path.h"
#include "optlist.h"
#include "commands.h"

static int build_nuke(struct sctstr *sp,
		      struct nchrstr *np, short *vec);
static int build_ship(struct sctstr *sp,
		      struct mchrstr *mp, short *vec,
		      int tlev);
static int build_land(struct sctstr *sp,
		      struct lchrstr *lp, short *vec,
		      int tlev);
static int build_bridge(struct sctstr *sp, short *vec);
static int build_tower(struct sctstr *sp, short *vec);
static int build_plane(struct sctstr *sp,
		       struct plchrstr *pp, short *vec,
		       int tlev);

static int cash;		/* static ok */

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
    int what;
    struct lchrstr *lp;
    struct mchrstr *mp;
    struct plchrstr *pp;
    struct nchrstr *np;
    s_char *p;
    int gotsect = 0;
    int built;
    int number = 1, x;
    int asked = 0;
    s_char buf[1024];

    natp = getnatp(player->cnum);
    if ((p =
	 getstarg(player->argp[1],
		  "Build (ship, nuke, bridge, plane, land unit, tower)? ",
		  buf)) == 0)
	return RET_SYN;
    what = *p;

    for (x = 0; x < number; x++) {
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
	    type = typematch(p, EF_PLANE);
	    if (type >= 0) {
		pp = &plchr[type];
		rqtech = pp->pl_tech;
		if (rqtech > tlev)
		    type = -1;
	    }
	    if (type < 0) {
		pr("You can't build that!\n");
		pr("Use `show plane build %d' to show types you can build.\n", tlev);
		return RET_FAIL;
	    }
	    break;
	case 's':
	    p = getstarg(player->argp[3], "Ship type? ", buf);
	    if (p == 0 || *p == 0)
		return RET_SYN;
	    type = typematch(p, EF_SHIP);
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
		pr("Use `show ship build %d' to show types you can build.\n", tlev);
		return RET_FAIL;
	    }
	    break;
	case 'l':
	    p = getstarg(player->argp[3], "Land unit type? ", buf);
	    if (p == 0 || *p == 0)
		return RET_SYN;
	    type = typematch(p, EF_LAND);
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
		pr("Use `show land build %d' to show types you can build.\n", tlev);
		return RET_FAIL;
	    }
	    break;
	case 'b':
	    if (natp->nat_level[NAT_TLEV] + 0.005 < buil_bt) {
		pr("Building a span requires a tech of %.0f\n", buil_bt);
		return 2;
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
		return 2;
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
	    type = typematch(p, EF_NUKE);
	    if (type >= 0) {
		np = &nchr[type];
		rqtech = np->n_tech;
		if (rqtech > tlev
		    || (opt_DRNUKE && np->n_tech * drnuke_const > rlev))
		    type = -1;
	    }
	    if (type < 0) {
		int tt = tlev;
		if (opt_DRNUKE)
		    tt = (tlev < (rlev / drnuke_const) ? (int)tlev :
			  (int)(rlev / drnuke_const));
		pr("You can't build that!\n");
		pr("Use `show nuke build %d' to show types you can build.\n", tlev);
		return RET_FAIL;
	    }
	    break;
	default:
	    pr("You can't build that!\n");
	    return RET_FAIL;
	}
	if (what != 'b' && what != 't') {
	    if (player->argp[4]) {
		if (atoi(player->argp[4]) > 20 && !asked) {
		    s_char bstr[80];
		    asked = 1;
		    (void)sprintf(bstr,
				  "Are you sure that you want to build %s of them? ",
				  player->argp[4]);
		    p = getstarg(player->argp[6], bstr, buf);
		    if (p == 0 || *p != 'y')
			return RET_SYN;
		}
		number = atoi(player->argp[4]);
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
	    } else
		tlev = (int)natp->nat_level[NAT_TLEV];
	}
	cash = natp->nat_money;
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
		built = build_nuke(&sect, np, sect.sct_item);
		break;
	    case 'p':
		built = build_plane(&sect, pp, sect.sct_item, tlev);
		break;
	    default:
		CANT_HAPPEN("Bad WHAT");
		return RET_FAIL;
	    }
	    if (built) {
		putsect(&sect);
	    }
	}
    }
    if (!gotsect) {
	pr("Bad sector specification.\n");
    }
    return RET_OK;
}

static int
build_ship(struct sctstr *sp, struct mchrstr *mp,
	   short *vec, int tlev)
{
    struct shpstr ship;
    struct nstr_item nstr;
    int avail, cost, i;
    float eff = SHIP_MINEFF / 100.0;
    int lcm, hcm;
    int freeship = 0;

    hcm = roundavg((double)mp->m_hcm * eff);
    lcm = roundavg((double)mp->m_lcm * eff);

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
    cost = mp->m_cost * SHIP_MINEFF / 100;
    if (cash < cost) {
	pr("Not enough money left to build a %s\n", mp->m_name);
	return 0;
    }
    if (!trechk(player->cnum, 0, NEWSHP))
	return 0;
    if (!check_sect_ok(sp))
	return 0;
    sp->sct_avail -= avail;
    player->dolcost += cost;
    cash -= cost;
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
    memset(&ship, 0, sizeof(struct shpstr));
    ship.shp_x = sp->sct_x;
    ship.shp_y = sp->sct_y;
    ship.shp_destx[0] = sp->sct_x;
    ship.shp_desty[0] = sp->sct_y;
    ship.shp_destx[1] = sp->sct_x;
    ship.shp_desty[1] = sp->sct_y;
    ship.shp_autonav = 0;
    /* new code for autonav, Chad Zabel 1-15-94 */
    for (i = 0; i < TMAX; ++i) {
	ship.shp_tstart[i] = I_NONE;
	ship.shp_tend[i] = I_NONE;
	ship.shp_lstart[i] = 0;
	ship.shp_lend[i] = 0;
    }
    ship.shp_mission = 0;
    ship.shp_own = player->cnum;
    ship.shp_type = mp - mchr;
    ship.shp_effic = SHIP_MINEFF;
    if (opt_MOB_ACCESS) {
	time(&ship.shp_access);
	ship.shp_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	ship.shp_mobil = 0;
    }
    ship.shp_uid = nstr.cur;
    ship.shp_nplane = 0;
    ship.shp_nland = 0;
    ship.shp_nxlight = 0;
    ship.shp_nchoppers = 0;
    ship.shp_fleet = ' ';
    memset(ship.shp_item, 0, sizeof(ship.shp_item));
    ship.shp_pstage = PLG_HEALTHY;
    ship.shp_ptime = 0;
    ship.shp_mobquota = 0;
    *ship.shp_path = 0;
    ship.shp_follow = nstr.cur;
    ship.shp_name[0] = 0;
    ship.shp_orig_own = player->cnum;
    ship.shp_orig_x = sp->sct_x;
    ship.shp_orig_y = sp->sct_y;
    ship.shp_fuel = mchr[(int)ship.shp_type].m_fuelc;
    ship.shp_rflags = 0;
    memset(ship.shp_rpath, 0, sizeof(ship.shp_rpath));
    shp_set_tech(&ship, tlev);

    vec[I_LCM] -= lcm;
    vec[I_HCM] -= hcm;

    if (sp->sct_pstage == PLG_INFECT)
	ship.shp_pstage = PLG_EXPOSED;
    makenotlost(EF_SHIP, ship.shp_own, ship.shp_uid, ship.shp_x,
		ship.shp_y);
    putship(ship.shp_uid, &ship);
    pr("%s", prship(&ship));
    pr(" built in sector %s\n", xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_land(struct sctstr *sp, struct lchrstr *lp,
	   short *vec, int tlev)
{
    struct lndstr land;
    struct nstr_item nstr;
    int avail, cost;
    float eff = LAND_MINEFF / 100.0;
    int mil, lcm, hcm, gun, shell;
    int freeland = 0;

#if 0
    mil = roundavg(((double)lp->l_mil * eff));
    shell = roundavg(((double)lp->l_shell * eff));
    gun = roundavg(((double)lp->l_gun * eff));
#else
    mil = shell = gun = 0;
#endif
    hcm = roundavg(((double)lp->l_hcm * eff));
    lcm = roundavg(((double)lp->l_lcm * eff));

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
    cost = lp->l_cost * LAND_MINEFF / 100;
    if (cash < cost) {
	pr("Not enough money left to build a %s\n", lp->l_name);
	return 0;
    }
    sp->sct_avail -= avail;
    player->dolcost += cost;
    cash -= cost;
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
    memset(&land, 0, sizeof(struct lndstr));
    land.lnd_x = sp->sct_x;
    land.lnd_y = sp->sct_y;
    land.lnd_own = player->cnum;
    land.lnd_mission = 0;
    land.lnd_type = lp - lchr;
    land.lnd_effic = LAND_MINEFF;
    if (opt_MOB_ACCESS) {
	time(&land.lnd_access);
	land.lnd_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	land.lnd_mobil = 0;
    }
    land.lnd_uid = nstr.cur;
    land.lnd_army = ' ';
    land.lnd_flags = 0;
    land.lnd_ship = -1;
    land.lnd_land = -1;
    land.lnd_nland = 0;
    land.lnd_harden = 0;
    land.lnd_retreat = morale_base;
    land.lnd_fuel = lp->l_fuelc;
    land.lnd_nxlight = 0;
    land.lnd_rflags = 0;
    memset(land.lnd_rpath, 0, sizeof(land.lnd_rpath));
    land.lnd_rad_max = 0;
    memset(land.lnd_item, 0, sizeof(land.lnd_item));
    land.lnd_pstage = PLG_HEALTHY;
    land.lnd_ptime = 0;
    lnd_set_tech(&land, tlev);

    vec[I_LCM] -= lcm;
    vec[I_HCM] -= hcm;
    vec[I_MILIT] -= mil;
    vec[I_GUN] -= gun;
    vec[I_SHELL] -= shell;

    if (sp->sct_pstage == PLG_INFECT)
	land.lnd_pstage = PLG_EXPOSED;
    putland(nstr.cur, &land);
    makenotlost(EF_LAND, land.lnd_own, land.lnd_uid, land.lnd_x,
		land.lnd_y);
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
    s_char *p;
    s_char buf[1024];

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

    if (cash < buil_bc) {
	pr("A span costs $%.2f to build; ", buil_bc);
	pr("you only have %d.\n", cash);
	return 0;
    }
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
    if (!(p = getstarg(player->argp[3], "build span in what direction? ", buf))
	|| !*p) {
	return 0;
    }
    /* Sanity check time */
    if (!check_sect_ok(sp))
	return 0;

    if ((val = chkdir(*p, DIR_FIRST, DIR_LAST)) < 0) {
	pr("'%c' is not a valid direction...\n", *p);
	direrr(0, 0, 0);
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
    cash -= buil_bc;
    sect.sct_type = SCT_BSPAN;
    sect.sct_newtype = SCT_BSPAN;
    sect.sct_effic = SCT_MINEFF;
    sect.sct_road = 0;
    sect.sct_rail = 0;
    sect.sct_defense = 0;
    if (!opt_DEFENSE_INFRA)
	sect.sct_defense = sect.sct_effic;
    if (opt_MOB_ACCESS) {
	time(&sect.sct_access);
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
build_nuke(struct sctstr *sp, struct nchrstr *np,
	   short *vec)
{
    int avail;

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
    if (cash < np->n_cost) {
	pr("You need $%d, you only have %d.\n", np->n_cost, cash);
	return 0;
    }
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
    cash -= np->n_cost;
    nuk_add(sp->sct_x, sp->sct_y, np - nchr, 1);
    vec[I_HCM] -= np->n_hcm;
    vec[I_LCM] -= np->n_lcm;
    vec[I_OIL] -= np->n_oil;
    vec[I_RAD] -= np->n_rad;
    pr("%s warhead created in %s\n", np->n_name,
       xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_plane(struct sctstr *sp, struct plchrstr *pp,
	    short *vec, int tlev)
{
    struct plnstr plane;
    int avail, cost;
    struct nstr_item nstr;
    float eff = PLANE_MINEFF / 100.0;
    int hcm, lcm, mil;
    int freeplane = 0;

    mil = roundavg(((double)pp->pl_crew * eff));
    /* Always use at least 1 mil to build a plane */
    if (mil == 0 && pp->pl_crew > 0)
	mil = 1;
    hcm = roundavg(((double)pp->pl_hcm * eff));
    lcm = roundavg(((double)pp->pl_lcm * eff));
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
    cost = pp->pl_cost * PLANE_MINEFF / 100;
    if (cash < cost) {
	pr("Not enough money left to build a %s\n", pp->pl_name);
	return 0;
    }
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
    cash -= cost;
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
    memset(&plane, 0, sizeof(struct plnstr));
    plane.pln_x = sp->sct_x;
    plane.pln_y = sp->sct_y;
    plane.pln_own = sp->sct_own;
    plane.pln_type = pp - plchr;
    plane.pln_effic = PLANE_MINEFF;
    if (opt_MOB_ACCESS) {
	time(&plane.pln_access);
	plane.pln_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	plane.pln_mobil = 0;
    }
    plane.pln_mission = 0;
    plane.pln_opx = 0;
    plane.pln_opy = 0;
    plane.pln_radius = 0;
    plane.pln_range = UCHAR_MAX; /* will be adjusted by pln_set_tech() */
    plane.pln_range_max = plane.pln_range;
    plane.pln_wing = ' ';
    plane.pln_ship = -1;
    plane.pln_land = -1;
    plane.pln_uid = nstr.cur;
    plane.pln_nuketype = -1;
    plane.pln_harden = 0;
    plane.pln_flags = 0;
    pln_set_tech(&plane, tlev);

    vec[I_LCM] -= lcm;
    vec[I_HCM] -= hcm;
    vec[I_MILIT] -= mil;

    makenotlost(EF_PLANE, plane.pln_own, plane.pln_uid, plane.pln_x,
		plane.pln_y);
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
    s_char *p;
    s_char buf[1024];
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

    if (cash < buil_tower_bc) {
	pr("A bridge tower costs $%.2f to build; ", buil_tower_bc);
	pr("you only have %d.\n", cash);
	return 0;
    }
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
    if (!(p = getstarg(player->argp[3], "build tower in what direction? ", buf))
	|| !*p) {
	return 0;
    }
    /* Sanity check time */
    if (!check_sect_ok(sp))
	return 0;

    if ((val = chkdir(*p, DIR_FIRST, DIR_LAST)) < 0) {
	pr("'%c' is not a valid direction...\n", *p);
	direrr(0, 0, 0);
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
    cash -= buil_tower_bc;
    sect.sct_type = SCT_BTOWER;
    sect.sct_newtype = SCT_BTOWER;
    sect.sct_effic = SCT_MINEFF;
    sect.sct_road = 0;
    sect.sct_rail = 0;
    sect.sct_defense = 0;
    if (opt_MOB_ACCESS) {
	time(&sect.sct_access);
	sect.sct_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	sect.sct_mobil = 0;
    }
    if (!opt_DEFENSE_INFRA)
	sect.sct_defense = sect.sct_effic;
    sect.sct_mines = 0;
    map_set(player->cnum, sect.sct_x, sect.sct_y, dchr[SCT_BTOWER].d_mnem, 2);
    writemap(player->cnum);
    putsect(&sect);
    pr("Bridge tower built in %s\n",
       xyas(sect.sct_x, sect.sct_y, player->cnum));
    vec[I_HCM] -= buil_tower_bh;
    return 1;
}
