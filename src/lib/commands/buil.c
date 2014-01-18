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
 *  buil.c: Build ships, nukes, bridges, planes, land units, bridge towers
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2014
 */

#include <config.h>

#include <limits.h>
#include <math.h>
#include "chance.h"
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
#include "unit.h"

static int build_ship(struct sctstr *sp, int type, int tlev);
static int build_land(struct sctstr *sp, int type, int tlev);
static int build_nuke(struct sctstr *sp, int type, int tlev);
static int build_plane(struct sctstr *sp, int type, int tlev);
static int pick_unused_unit_uid(int);
static int build_bridge(char);
static int build_bspan(struct sctstr *sp);
static int build_btower(struct sctstr *sp);
static int sector_can_build(struct sctstr *, short[], int, int, char *);
static void build_charge(struct sctstr *, short[], int, double, int);
static int build_can_afford(double, int, char *);

/*
 * build <WHAT> <SECTS> <TYPE|DIR|MEG> [NUMBER]
 */
int
buil(void)
{
    struct natstr *natp = getnatp(player->cnum);
    int tlev = (int)natp->nat_level[NAT_TLEV];
    struct sctstr sect;
    struct nstr_sect nstr;
    int rqtech, type, number, val, gotsect;
    char *p, *what, *prompt;
    int (*build_it)(struct sctstr *, int, int);
    char buf[1024];

    p = getstarg(player->argp[1],
		 "Build (ship, nuke, bridge, plane, land unit, tower)? ",
		 buf);
    if (!p)
	return RET_SYN;
    switch (*p) {
    case 'b':
    case 't':
	return build_bridge(*p);
    case 's':
	what = "ship";
	prompt = "Ship type? ";
	build_it = build_ship;
	break;
    case 'p':
	what = "plane";
	prompt = "Plane type? ";
	build_it = build_plane;
	break;
    case 'l':
	what = "land";
	prompt = "Land unit type? ";
	build_it = build_land;
	break;
    case 'n':
	if (!ef_nelem(EF_NUKE_CHR)) {
	    pr("There are no nukes in this game.\n");
	    return RET_FAIL;
	}
	if (drnuke_const > MIN_DRNUKE_CONST)
	    tlev = MIN(tlev,
		       (int)(natp->nat_level[NAT_RLEV] / drnuke_const));
	what = "nuke";
	prompt = "Nuke type? ";
	build_it = build_nuke;
	break;
    default:
	pr("You can't build that!\n");
	return RET_SYN;
    }

    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;

    p = getstarg(player->argp[3], prompt, buf);
    if (!p || !*p)
	return RET_SYN;

    rqtech = 0;
    switch (*what) {
    case 'p':
	type = ef_elt_byname(EF_PLANE_CHR, p);
	if (type >= 0)
	    rqtech = plchr[type].pl_tech;
	break;
    case 's':
	type = ef_elt_byname(EF_SHIP_CHR, p);
	if (type >= 0)
	    rqtech = mchr[type].m_tech;
	break;
    case 'l':
	type = ef_elt_byname(EF_LAND_CHR, p);
	if (type >= 0)
	    rqtech = lchr[type].l_tech;
	break;
    case 'n':
	type = ef_elt_byname(EF_NUKE_CHR, p);
	if (type >= 0)
	    rqtech = nchr[type].n_tech;
	break;
    default:
	CANT_REACH();
	return RET_FAIL;
    }

    if (tlev < rqtech && player->god)
	tlev = rqtech;
    if (type < 0 || tlev < rqtech) {
	pr("You can't build that!\n");
	pr("Use `show %s build %d' to show types you can build.\n",
	   what, tlev);
	return RET_FAIL;
    }

    number = 1;
    if (player->argp[4]) {
	number = atoi(player->argp[4]);
	if (number > 20) {
	    char bstr[80];
	    sprintf(bstr,
		    "Are you sure that you want to build %d of them? ",
		    number);
	    p = getstarg(player->argp[6], bstr, buf);
	    if (!p || *p != 'y')
		return RET_SYN;
	}
    }

    if (player->argp[5]) {
	val = atoi(player->argp[5]);
	if (val > tlev && !player->god) {
	    pr("Your%s tech level is only %d.\n",
	       *what == 'n' && drnuke_const > MIN_DRNUKE_CONST
	       ? " effective" : "", tlev);
	    return RET_FAIL;
	}
	if (rqtech > val) {
	    pr("Required tech is %d.\n", rqtech);
	    return RET_FAIL;
	}
	tlev = val;
	pr("Building with tech level %d.\n", tlev);
    }

    gotsect = 0;
    while (number-- > 0) {
	while (nxtsct(&nstr, &sect)) {
	    if (!player->owner)
		continue;
	    gotsect = 1;
	    if (build_it(&sect, type, tlev))
		putsect(&sect);
	}
	snxtsct_rewind(&nstr);
    }
    if (!gotsect)
	pr("No sectors.\n");
    return RET_OK;
}

static int
build_ship(struct sctstr *sp, int type, int tlev)
{
    struct mchrstr *mp = &mchr[type];
    short mat[I_MAX+1];
    int work;
    struct shpstr ship;

    memset(mat, 0, sizeof(mat));
    mat[I_LCM] = mp->m_lcm;
    mat[I_HCM] = mp->m_hcm;
    work = SHP_BLD_WORK(mp->m_lcm, mp->m_hcm);

    if (sp->sct_type != SCT_HARBR && !player->god) {
	pr("Ships must be built in harbours.\n");
	return 0;
    }
    if (!sector_can_build(sp, mat, work, SHIP_MINEFF, mp->m_name))
	return 0;
    if (!build_can_afford(mp->m_cost, SHIP_MINEFF, mp->m_name))
	return 0;
    build_charge(sp, mat, work, mp->m_cost, SHIP_MINEFF);

    ef_blank(EF_SHIP, pick_unused_unit_uid(EF_SHIP), &ship);
    ship.shp_x = sp->sct_x;
    ship.shp_y = sp->sct_y;
    ship.shp_own = sp->sct_own;
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
    ship.shp_orig_own = sp->sct_own;
    ship.shp_orig_x = sp->sct_x;
    ship.shp_orig_y = sp->sct_y;
    shp_set_tech(&ship, tlev);
    unit_wipe_orders((struct empobj *)&ship);

    if (sp->sct_pstage == PLG_INFECT)
	ship.shp_pstage = PLG_EXPOSED;
    putship(ship.shp_uid, &ship);
    pr("%s", prship(&ship));
    pr(" built in sector %s\n", xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_land(struct sctstr *sp, int type, int tlev)
{
    struct lchrstr *lp = &lchr[type];
    short mat[I_MAX+1];
    int work;
    struct lndstr land;

    memset(mat, 0, sizeof(mat));
    mat[I_LCM] = lp->l_lcm;
    mat[I_HCM] = lp->l_hcm;
    work = LND_BLD_WORK(lp->l_lcm, lp->l_hcm);

    if (sp->sct_type != SCT_HEADQ && !player->god) {
	pr("Land units must be built in headquarters.\n");
	return 0;
    }
    if (!sector_can_build(sp, mat, work, LAND_MINEFF, lp->l_name))
	return 0;
    if (!build_can_afford(lp->l_cost, LAND_MINEFF, lp->l_name))
	return 0;
    build_charge(sp, mat, work, lp->l_cost, LAND_MINEFF);

    ef_blank(EF_LAND, pick_unused_unit_uid(EF_LAND), &land);
    land.lnd_x = sp->sct_x;
    land.lnd_y = sp->sct_y;
    land.lnd_own = sp->sct_own;
    land.lnd_type = lp - lchr;
    land.lnd_effic = LAND_MINEFF;
    if (opt_MOB_ACCESS) {
	game_tick_to_now(&land.lnd_access);
	land.lnd_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	land.lnd_mobil = 0;
    }
    land.lnd_ship = -1;
    land.lnd_land = -1;
    land.lnd_harden = 0;
    memset(land.lnd_item, 0, sizeof(land.lnd_item));
    land.lnd_pstage = PLG_HEALTHY;
    land.lnd_ptime = 0;
    lnd_set_tech(&land, tlev);
    unit_wipe_orders((struct empobj *)&land);

    if (sp->sct_pstage == PLG_INFECT)
	land.lnd_pstage = PLG_EXPOSED;
    putland(land.lnd_uid, &land);
    pr("%s", prland(&land));
    pr(" built in sector %s\n", xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_nuke(struct sctstr *sp, int type, int tlev)
{
    struct nchrstr *np = &nchr[type];
    short mat[I_MAX+1];
    int work;
    struct nukstr nuke;

    if (sp->sct_type != SCT_NUKE && !player->god) {
	pr("Nuclear weapons must be built in nuclear plants.\n");
	return 0;
    }
    /*
     * XXX when nukes turn into units (or whatever), then
     * make them start at 20%.  Since they don't have efficiency
     * now, we charge all the work right away.
     */
    memset(mat, 0, sizeof(mat));
    mat[I_LCM] = np->n_lcm;
    mat[I_HCM] = np->n_hcm;
    mat[I_OIL] = np->n_oil;
    mat[I_RAD] = np->n_rad;
    work = NUK_BLD_WORK(np->n_lcm, np->n_hcm, np->n_oil, np->n_rad);

    if (!sector_can_build(sp, mat, work, 100, np->n_name))
	return 0;
    if (!build_can_afford(np->n_cost, 100, np->n_name))
	return 0;
    build_charge(sp, mat, work, np->n_cost, 100);

    ef_blank(EF_NUKE, pick_unused_unit_uid(EF_NUKE), &nuke);
    nuke.nuk_x = sp->sct_x;
    nuke.nuk_y = sp->sct_y;
    nuke.nuk_own = sp->sct_own;
    nuke.nuk_type = np - nchr;
    nuke.nuk_effic = 100;
    nuke.nuk_plane = -1;
    nuke.nuk_tech = tlev;
    unit_wipe_orders((struct empobj *)&nuke);

    putnuke(nuke.nuk_uid, &nuke);
    pr("%s created in %s\n", prnuke(&nuke),
       xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_plane(struct sctstr *sp, int type, int tlev)
{
    struct plchrstr *pp = &plchr[type];
    short mat[I_MAX+1];
    int work;
    struct plnstr plane;

    memset(mat, 0, sizeof(mat));
    mat[I_MILIT] = pp->pl_crew;
    mat[I_LCM] = pp->pl_lcm;
    mat[I_HCM] = pp->pl_hcm;
    work = PLN_BLD_WORK(pp->pl_lcm, pp->pl_hcm);

    if (sp->sct_type != SCT_AIRPT && !player->god) {
	pr("Planes must be built in airports.\n");
	return 0;
    }
    if (!sector_can_build(sp, mat, work, PLANE_MINEFF, pp->pl_name))
	return 0;
    if (!build_can_afford(pp->pl_cost, PLANE_MINEFF, pp->pl_name))
	return 0;
    build_charge(sp, mat, work, pp->pl_cost, PLANE_MINEFF);

    ef_blank(EF_PLANE, pick_unused_unit_uid(EF_PLANE), &plane);
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

    putplane(plane.pln_uid, &plane);
    pr("%s built in sector %s\n", prplane(&plane),
       xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
pick_unused_unit_uid(int type)
{
    struct nstr_item nstr;
    union empobj_storage unit;

    snxtitem_all(&nstr, type);
    while (nxtitem(&nstr, &unit)) {
	if (!unit.gen.own)
	    return nstr.cur;
    }
    ef_extend(type, 50);
    return nstr.cur;
}

static int
build_bridge(char what)
{
    struct natstr *natp = getnatp(player->cnum);
    struct nstr_sect nstr;
    int (*build_it)(struct sctstr *);
    int gotsect;
    struct sctstr sect;

    switch (what) {
    case 'b':
	if (natp->nat_level[NAT_TLEV] < buil_bt && !player->god) {
	    pr("Building a span requires a tech of %.0f\n", buil_bt);
	    return RET_FAIL;
	}
	build_it = build_bspan;
	break;
    case 't':
	if (!opt_BRIDGETOWERS) {
	    pr("Bridge tower building is disabled.\n");
	    return RET_FAIL;
	}
	if (natp->nat_level[NAT_TLEV] < buil_tower_bt && !player->god) {
	    pr("Building a tower requires a tech of %.0f\n",
	       buil_tower_bt);
	    return RET_FAIL;
	}
	build_it = build_btower;
	break;
    default:
	CANT_REACH();
	return RET_FAIL;
    }

    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    gotsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	gotsect = 1;
	if (build_it(&sect))
	    putsect(&sect);
    }
    if (!gotsect)
	pr("No sectors.\n");
    return RET_OK;
}

static int
build_bspan(struct sctstr *sp)
{
    struct sctstr sect;
    short mat[I_MAX+1];
    int work;
    int val;
    int newx, newy;
    char *p;
    char buf[1024];

    if (!opt_EASY_BRIDGES && !player->god) {
	/* must have a bridge head or tower */
	if (sp->sct_type != SCT_BTOWER) {
	    if (sp->sct_type != SCT_BHEAD)
		return 0;
	    if (sp->sct_newtype != SCT_BHEAD)
		return 0;
	}
    }

    memset(mat, 0, sizeof(mat));
    mat[I_HCM] = buil_bh;
    work = (SCT_BLD_WORK(0, buil_bh) * SCT_MINEFF + 99) / 100;

    if (!sector_can_build(sp, mat, work, 100, dchr[SCT_BSPAN].d_name))
	return 0;
    if (!build_can_afford(buil_bc, 100, dchr[SCT_BSPAN].d_name))
	return 0;
    if (!player->argp[3]) {
	pr("Bridge head at %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	nav_map(sp->sct_x, sp->sct_y, 1);
    }
    p = getstarg(player->argp[3], "build span in what direction? ", buf);
    if (!p || !*p) {
	return 0;
    }
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
    if (!bridge_support_at(&sect, DIR_STOP)) {
	if (opt_EASY_BRIDGES) {
	    pr("%s is not next to land or a bridge tower",
	       xyas(newx, newy, player->cnum));
	} else {
	    /*
	     * Note: because players need a 60% bridge head or tower,
	     * we can get here only for a deity.
	     */
	    pr("%s is not next to a supporting bridge head or tower\n",
	       xyas(newx, newy, player->cnum));
	}
	return 0;
    }
    build_charge(sp, mat, work, buil_bc, 100);

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
    return 1;
}

static int
build_btower(struct sctstr *sp)
{
    struct sctstr sect;
    short mat[I_MAX+1];
    int work;
    int val;
    int newx, newy;
    char *p;
    char buf[1024];
    int i;
    int nx;
    int ny;

    if (sp->sct_type != SCT_BSPAN && !player->god) {
	pr("Bridge towers can only be built from bridge spans.\n");
	return 0;
    }

    memset(mat, 0, sizeof(mat));
    mat[I_HCM] = buil_tower_bh;
    work = (SCT_BLD_WORK(0, buil_tower_bh) * SCT_MINEFF + 99) / 100;

    if (!sector_can_build(sp, mat, work, 100, dchr[SCT_BTOWER].d_name))
	return 0;
    if (!build_can_afford(buil_tower_bc, 100, dchr[SCT_BTOWER].d_name))
	return 0;
    if (!player->argp[3]) {
	pr("Building from %s\n", xyas(sp->sct_x, sp->sct_y, player->cnum));
	nav_map(sp->sct_x, sp->sct_y, 1);
    }
    p = getstarg(player->argp[3], "build tower in what direction? ", buf);
    if (!p || !*p) {
	return 0;
    }
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
    for (i = 1; i <= 6; i++) {
	struct sctstr s2;
	nx = sect.sct_x + diroff[i][0];
	ny = sect.sct_y + diroff[i][1];
	getsect(nx, ny, &s2);
	if ((s2.sct_type != SCT_WATER) &&
	    (s2.sct_type != SCT_BTOWER) && (s2.sct_type != SCT_BSPAN)) {
	    pr("%s is next to land, can't build bridge tower there",
	       xyas(newx, newy, player->cnum));
	    return 0;
	}
    }

    build_charge(sp, mat, work, buil_tower_bc, 100);

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
    return 1;
}

static int
sector_can_build(struct sctstr *sp, short mat[], int work,
		 int effic, char *what)
{
    int i, avail, ret;
    double needed;

    if (player->god)
	return 1;		/* Deity builds ex nihilo */

    if (sp->sct_effic < 60 && !player->god) {
	pr("Sector %s is not 60%% efficient.\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }

    avail = (work * effic + 99) / 100;
    if (sp->sct_avail < avail) {
	pr("Not enough available work in %s to build a %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum), what);
	pr(" (%d available work required)\n", avail);
	return 0;
    }

    ret = 1;
    for (i = I_NONE + 1; i <= I_MAX; i++) {
	needed = mat[i] * (effic / 100.0);
	if (sp->sct_item[i] < needed) {
	    pr("Not enough %s in %s (need %g more)\n",
	       ichr[i].i_name, xyas(sp->sct_x, sp->sct_y, player->cnum),
	       ceil(needed - sp->sct_item[i]));
	    ret = 0;
	}
	mat[i] = roundavg(needed);
    }

    return ret;
}

static void
build_charge(struct sctstr *sp,
	     short mat[], int work, double cost, int effic)
{
    int i;

    if (player->god)
	return;			/* Deity builds ex nihilo */

    for (i = I_NONE + 1; i <= I_MAX; i++)
	sp->sct_item[i] -= mat[i];
    sp->sct_avail -= (work * effic + 99) / 100;
    player->dolcost += cost * effic / 100.0;
}

static int
build_can_afford(double cost, int effic, char *what)
{
    struct natstr *natp = getnatp(player->cnum);

    if (natp->nat_money < player->dolcost + cost * effic / 100.0) {
	pr("Not enough money left to build a %s\n", what);
	return 0;
    }
    return 1;
}
