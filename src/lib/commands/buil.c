/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#include <math.h>
#include <string.h>
#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "retreat.h"
#include "ship.h"
#include "land.h"
#include "nuke.h"
#include "plane.h"
#include "xy.h"
#include "nsc.h"
#include "treaty.h"
#include "deity.h"
#include "file.h"
#include "path.h"
#include "optlist.h"
#include "commands.h"

static int build_nuke(register struct sctstr *sp,
		      register struct nchrstr *np, register int *vec);
static int build_ship(register struct sctstr *sp,
		      register struct mchrstr *mp, register int *vec,
		      int tlev);
static int build_land(register struct sctstr *sp,
		      register struct lchrstr *lp, register int *vec,
		      int tlev);
static int build_bridge(register struct sctstr *sp, register int *vec);
static int build_tower(register struct sctstr *sp, register int *vec);
static int build_plane(register struct sctstr *sp,
		       register struct plchrstr *pp, register int *vec,
		       int tlev);

static int cash;		/* static ok */

extern int morale_base;
extern int sect_mob_neg_factor;
extern int etu_per_update;

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
    int n;
    int type;
    int what;
    struct lchrstr *lp;
    struct mchrstr *mp;
    struct plchrstr *pp;
    struct nchrstr *np;
    s_char *p;
    int vec[I_MAX + 1];
    int gotsect = 0;
    int built;
    int hold, found, number = 1, x;
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
      ask_again:
	tlev = (int)natp->nat_level[NAT_TLEV];
	rlev = (int)natp->nat_level[NAT_RLEV];

	switch (what) {
	case 'p':
	    p = getstarg(player->argp[3], "Plane type? ", buf);
	    if (p == 0 || *p == 0)
		return RET_SYN;
	    n = strlen(p);
	    while (n && iscntrl(p[n - 1]))
		n--;
	    if (!n)
		return RET_SYN;
	    for (found = 0, type = 0, pp = plchr; type <= pln_maxno;
		 type++, pp++) {
		if (pp->pl_tech > tlev)
		    continue;
		if (pp->pl_name && strncmp(p, pp->pl_name, n) == 0) {
		    found++;
		    hold = type;
		    break;
		}
	    }
	    if (found != 1) {
		pr("Illegal plane type: \"%s\"\n", p);
		if (confirm("List plane types? "))
		    show_plane_build(tlev);
		player->argp[3] = 0;
		goto ask_again;
	    }
	    type = hold;
	    pp = &(plchr[type]);
	    rqtech = pp->pl_tech;
	    break;
	case 's':
	    p = getstarg(player->argp[3], "Ship type? ", buf);
	    if (p == 0 || *p == 0)
		return RET_SYN;
	    n = strlen(p);
	    while (n && iscntrl(p[n - 1]))
		n--;
	    if (!n)
		return RET_SYN;
	    for (found = 0, mp = mchr, type = 0; type <= shp_maxno;
		 type++, mp++) {
		if (mp->m_tech > tlev)
		    continue;
		/* Can't build trade ships unless it's turned on */
		if ((mp->m_flags & M_TRADE) && !opt_TRADESHIPS)
		    continue;
		if (mp->m_name && strncmp(p, mp->m_name, n) == 0) {
		    found++;
		    hold = type;
		    break;
		}
	    }
	    if (found != 1) {
		pr("Illegal ship type: \"%s\"\n", p);
		if (confirm("List ship types? "))
		    show_ship_build(tlev);
		player->argp[3] = 0;
		goto ask_again;
	    }
	    type = hold;
	    mp = &(mchr[type]);
	    rqtech = mp->m_tech;
	    break;
	case 'l':
	    p = getstarg(player->argp[3], "Land unit type? ", buf);
	    if (p == 0 || *p == 0)
		return RET_SYN;
	    n = strlen(p);
	    while (n && iscntrl(p[n - 1]))
		n--;
	    if (!n)
		return RET_SYN;
	    for (found = 0, lp = lchr, type = 0; type <= lnd_maxno;
		 type++, lp++) {
		if (lp->l_tech > tlev)
		    continue;
		if ((lp->l_flags & L_SPY) && !opt_LANDSPIES)
		    continue;
		if (lp->l_name && strncmp(p, lp->l_name, n) == 0) {
		    found++;
		    hold = type;
		    break;
		}
	    }
	    if (found != 1) {
		pr("Illegal land unit type: \"%s\"\n", p);
		if (confirm("List unit types? "))
		    show_land_build(tlev);
		player->argp[3] = 0;
		goto ask_again;
	    }
	    type = hold;
	    lp = &(lchr[type]);
	    rqtech = lp->l_tech;
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
	    if (opt_NONUKES) {
		pr("There are no nukes in this game.\n");
		return RET_FAIL;
	    }
	    p = getstarg(player->argp[3], "Nuke type? ", buf);
	    if (p == 0 || *p == 0)
		return RET_SYN;
	    n = strlen(p);
	    while (n && iscntrl(p[n - 1]))
		n--;
	    if (!n)
		return RET_SYN;
	    for (found = 0, np = nchr, type = 0; type < nuk_maxno;
		 type++, np++) {
		if ((np->n_tech > tlev)
		    || (opt_DRNUKE
			&& ((np->n_tech * drnuke_const) > rlev)))
		    continue;
		if (opt_NEUTRON == 0 && (np->n_flags & N_NEUT))
		    continue;

		if (np->n_name && strncmp(p, np->n_name, n) == 0) {
		    found++;
		    hold = type;
		    break;
		}
	    }
	    if (found != 1) {
		int tt = tlev;
		pr("Possible nuke types are:\n");
		if (opt_DRNUKE)
		    tt = (tlev < (rlev / drnuke_const) ? (int)tlev :
			  (int)(rlev / drnuke_const));

		show_nuke_build(tt);
		player->argp[3] = 0;
		goto ask_again;
	    }
	    type = hold;
	    np = &(nchr[type]);
	    rqtech = np->n_tech;
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
	    getvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
	    switch (what) {
	    case 'l':
		built = build_land(&sect, lp, vec, tlev);
		break;
	    case 's':
		built = build_ship(&sect, mp, vec, tlev);
		break;
	    case 'b':
		built = build_bridge(&sect, vec);
		break;
	    case 't':
		built = build_tower(&sect, vec);
		break;
	    case 'n':
		built = build_nuke(&sect, np, vec);
		break;
	    case 'p':
		built = build_plane(&sect, pp, vec, tlev);
		break;
	    default:
		pr("internal error in build (%d)\n", what);
		return RET_FAIL;
	    }
	    if (built) {
		putvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
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
build_ship(register struct sctstr *sp, register struct mchrstr *mp,
	   register int *vec, int tlev)
{
    struct shpstr ship;
    struct nstr_item nstr;
    int cost, i;
    int w_p_eff, x;
    float eff = ((float)SHIP_MINEFF / 100.0);
    int points, lcm, hcm;
    int freeship = 0;
    int techdiff;

    hcm = roundavg(((double)mp->m_hcm * (double)eff));
    lcm = roundavg(((double)mp->m_lcm * (double)eff));

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
    w_p_eff = (20 + mp->m_lcm + (mp->m_hcm * 2));
    points = sp->sct_avail * 100 / w_p_eff;
    if (points < SHIP_MINEFF) {
	pr("Not enough available work in %s to build a %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum), mp->m_name);
	pr(" (%d available work required)\n",
	   1 + (w_p_eff * SHIP_MINEFF) / 100);
	return 0;
    }
    cost = mp->m_cost * eff;
    if (cash < cost) {
	pr("Not enough money left to build a %s\n", mp->m_name);
	return 0;
    }
    if (!trechk(player->cnum, 0, NEWSHP))
	return 0;
    if (!check_sect_ok(sp))
	return 0;
    sp->sct_avail = (sp->sct_avail * 100 - w_p_eff * SHIP_MINEFF) / 100;
    player->dolcost += cost;
    cash -= cost;
    snxtitem_all(&nstr, EF_SHIP);
    while (nxtitem(&nstr, (s_char *)&ship)) {
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
	ship.shp_tstart[i] = ' ';
	ship.shp_tend[i] = ' ';
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
    ship.shp_nv = 0;
    ship.shp_sell = 0;
    ship.shp_tech = tlev;

    techdiff = (int)(tlev - mp->m_tech);
    ship.shp_armor = (short)SHP_DEF(mp->m_armor, techdiff);
    ship.shp_speed = (short)SHP_SPD(mp->m_speed, techdiff);
    ship.shp_visib = (short)SHP_VIS(mp->m_visib, techdiff);
    ship.shp_frnge = (short)SHP_RNG(mp->m_frnge, techdiff);
    ship.shp_glim = (short)SHP_FIR(mp->m_glim, techdiff);

    ship.shp_mobquota = 0;
    *ship.shp_path = 0;
    ship.shp_follow = nstr.cur;
    ship.shp_name[0] = 0;
    ship.shp_orig_own = player->cnum;
    ship.shp_orig_x = sp->sct_x;
    ship.shp_orig_y = sp->sct_y;
    ship.shp_fuel = mchr[(int)ship.shp_type].m_fuelc;
    ship.shp_rflags = 0;
    for (x = 0; x < 10; x++)
	ship.shp_rpath[x] = 0;

    vec[I_LCM] -= lcm;
    vec[I_HCM] -= hcm;

    if (getvar(V_PSTAGE, (s_char *)sp, EF_SECTOR) == PLG_INFECT)
	putvar(V_PSTAGE, PLG_EXPOSED, (s_char *)&ship, EF_SHIP);
    makenotlost(EF_SHIP, ship.shp_own, ship.shp_uid, ship.shp_x,
		ship.shp_y);
    putship(ship.shp_uid, &ship);
    pr("%s", prship(&ship));
    pr(" built in sector %s\n", xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_land(register struct sctstr *sp, register struct lchrstr *lp,
	   register int *vec, int tlev)
{
    struct lndstr land;
    struct nstr_item nstr;
    int cost;
    int w_p_eff;
    int points;
    struct natstr *natp;
    float eff = ((float)LAND_MINEFF / 100.0);
    int mil, lcm, hcm, gun, shell;
    int freeland = 0;

    /*
       mil = roundavg(((double)lp->l_mil * (double)eff));
       shell = roundavg(((double)lp->l_shell * (double)eff));
       gun = roundavg(((double)lp->l_gun * (double)eff));
     */
    mil = shell = gun = 0;
    hcm = roundavg(((double)lp->l_hcm * (double)eff));
    lcm = roundavg(((double)lp->l_lcm * (double)eff));

    natp = getnatp(player->cnum);

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
    w_p_eff = (20 + lp->l_lcm + (lp->l_hcm * 2));
    points = sp->sct_avail * 100 / w_p_eff;
    if (points < LAND_MINEFF) {
	pr("Not enough available work in %s to build a %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum), lp->l_name);
	pr(" (%d available work required)\n",
	   1 + (w_p_eff * LAND_MINEFF) / 100);
	return 0;
    }
    cost = ((float)lp->l_cost * eff);
    /*  cost = (int)LND_COST(cost, tlev - lp->l_tech); */
    if (cash < cost) {
	pr("Not enough money left to build a %s\n", lp->l_name);
	return 0;
    }
    sp->sct_avail = (sp->sct_avail * 100 - w_p_eff * LAND_MINEFF) / 100;
    player->dolcost += cost;
    cash -= cost;
    snxtitem_all(&nstr, EF_LAND);
    while (nxtitem(&nstr, (s_char *)&land)) {
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
    land.lnd_sell = 0;
    land.lnd_tech = tlev;
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
    land.lnd_nv = 0;
    land.lnd_att = (float)LND_ATTDEF(lp->l_att, tlev - lp->l_tech);
    land.lnd_def = (float)LND_ATTDEF(lp->l_def, tlev - lp->l_tech);
    land.lnd_vul = (int)LND_VUL(lp->l_vul, tlev - lp->l_tech);
    land.lnd_spd = (int)LND_SPD(lp->l_spd, tlev - lp->l_tech);
    land.lnd_vis = (int)LND_VIS(lp->l_vis, tlev - lp->l_tech);
    land.lnd_spy = (int)LND_SPY(lp->l_spy, tlev - lp->l_tech);
    land.lnd_rad = (int)LND_RAD(lp->l_rad, tlev - lp->l_tech);
    land.lnd_frg = (int)LND_FRG(lp->l_frg, tlev - lp->l_tech);
    land.lnd_acc = (int)LND_ACC(lp->l_acc, tlev - lp->l_tech);
    land.lnd_dam = (int)LND_DAM(lp->l_dam, tlev - lp->l_tech);
    land.lnd_ammo = (int)LND_AMM(lp->l_ammo, lp->l_dam, tlev - lp->l_tech);
    land.lnd_aaf = (int)LND_AAF(lp->l_aaf, tlev - lp->l_tech);
    land.lnd_fuelc = (int)LND_FC(lp->l_fuelc, tlev - lp->l_tech);
    land.lnd_fuelu = (int)LND_FU(lp->l_fuelu, tlev - lp->l_tech);
    land.lnd_maxlight = (int)LND_XPL(lp->l_nxlight, tlev - lp->l_tech);
    land.lnd_maxland = (int)LND_MXL(lp->l_mxland, tlev - lp->l_tech);

    vec[I_LCM] -= lcm;
    vec[I_HCM] -= hcm;
    vec[I_MILIT] -= mil;
    vec[I_GUN] -= gun;
    vec[I_SHELL] -= shell;

/* Disabled autoloading of food onto units
    max_amt = vl_find(V_FOOD, lp->l_vtype, lp->l_vamt, (int) lp->l_nv);
    food_needed = (etu_per_update * eatrate) *
	(vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW])+1;
    if ((vec[I_FOOD]-max_amt) < food_needed)
	max_amt = (vec[I_FOOD]-food_needed);
    
    if (max_amt < 0)
	max_amt = 0;
    
    vec[I_FOOD] -= max_amt;
    
    memset(lvec, 0, sizeof(lvec));
    getvec(VT_ITEM, lvec, (s_char *)&land, EF_LAND);
    lvec[I_FOOD] += max_amt;
    putvec(VT_ITEM, lvec, (s_char *)&land, EF_LAND);
*/

    if (getvar(V_PSTAGE, (s_char *)sp, EF_SECTOR) == PLG_INFECT)
	putvar(V_PSTAGE, PLG_EXPOSED, (s_char *)&land, EF_LAND);
    putland(nstr.cur, &land);
    makenotlost(EF_LAND, land.lnd_own, land.lnd_uid, land.lnd_x,
		land.lnd_y);
    pr("%s", prland(&land));
    pr(" built in sector %s\n", xyas(sp->sct_x, sp->sct_y, player->cnum));
    return 1;
}

static int
build_bridge(register struct sctstr *sp, register int *vec)
{
    struct sctstr sect;
    int val;
    int newx, newy;
    int w_p_eff;
    int points;
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
#if 0
    else {

	for (i = 1; i <= 6; i++) {
	    struct sctstr s2;
	    nx = sp->sct_x + diroff[i][0];
	    ny = sp->sct_y + diroff[i][1];
	    getsect(nx, ny, &sect);
	    for (j = 1; j <= 6; j++) {
		nx2 = sect.sct_x + diroff[j][0];
		ny2 = sect.sct_y + diroff[j][1];
		getsect(nx2, ny2, &s2);
		if ((s2.sct_type != SCT_WATER) &&
		    (s2.sct_type != SCT_BSPAN))
		    good = 1;
	    }

	}

	if (!good) {
	    pr("Bridges must be built adjacent to land or bridge towers.\n");
	    pr("No eligible sectors adjacent to this sector.\n");
	    return 0;
	}
    }				/* end EASY_BRIDGES */
#endif

    if (sp->sct_effic < 60 && !player->god) {
	pr("Sector %s is not 60%% efficient.\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	return 0;
    }

    if (!opt_NO_HCMS) {
	if (vec[I_HCM] < buil_bh) {
	    pr("%s only has %d unit%s of hcm,\n",
	       xyas(sp->sct_x, sp->sct_y, player->cnum),
	       vec[I_HCM], vec[I_HCM] > 1 ? "s" : "");
	    pr("(a bridge span requires %d)\n", buil_bh);
	    return 0;
	}
    } else if (!opt_NO_LCMS) {
	if (vec[I_LCM] < buil_bh) {
	    pr("%s only has %d unit%s of lcm,\n",
	       xyas(sp->sct_x, sp->sct_y, player->cnum),
	       vec[I_LCM], vec[I_LCM] > 1 ? "s" : "");
	    pr("(a bridge span requires %d)\n", buil_bh);
	    return 0;
	}
    }
    if (cash < buil_bc) {
	pr("A span costs $%.2f to build; ", buil_bc);
	pr("you only have %d.\n", cash);
	return 0;
    }
    w_p_eff = buil_bh * 2;
    points = sp->sct_avail * 100 / w_p_eff;
    if (points < 20) {
	pr("Not enough available work in %s to build a bridge\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	pr(" (%d available work required)\n", 1 + (w_p_eff * 20) / 100);
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
    sp->sct_avail = (sp->sct_avail * 100 - w_p_eff * 20) / 100;
    player->dolcost += buil_bc;
    cash -= buil_bc;
    sect.sct_type = SCT_BSPAN;
    sect.sct_newtype = SCT_BSPAN;
    sect.sct_effic = 20;
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
    putvar(V_MINE, 0, (s_char *)&sect, EF_SECTOR);
    putsect(&sect);
    pr("Bridge span built over %s\n",
       xyas(sect.sct_x, sect.sct_y, player->cnum));
    if (!opt_NO_HCMS)
	vec[I_HCM] -= buil_bh;
    else if (!opt_NO_LCMS)
	vec[I_LCM] -= buil_bh;
    return 1;
}

static int
build_nuke(register struct sctstr *sp, register struct nchrstr *np,
	   register int *vec)
{
    int w_p_eff;
    int points;

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
    w_p_eff = np->n_rad + np->n_oil + np->n_lcm + np->n_hcm * 2;
    points = sp->sct_avail * 100 / w_p_eff;
    /*
     * XXX when nukes turn into units (or whatever), then
     * make them start at 20%.  Since they don't have efficiency
     * now, we choose 20% as a "big" number.
     */
    if (points < 20) {
	pr("Not enough available work in %s to build a %s;\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum), np->n_name);
	pr(" (%d available work required)\n", 1 + w_p_eff * 20 / 100);
	return 0;
    }
    if (!trechk(player->cnum, 0, NEWNUK))
	return 0;
    if (!check_sect_ok(sp))
	return 0;
    sp->sct_avail = (sp->sct_avail * 100 - w_p_eff * 20) / 100;
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
build_plane(register struct sctstr *sp, register struct plchrstr *pp,
	    register int *vec, int tlev)
{
    struct plnstr plane;
    int cost;
    struct nstr_item nstr;
    float eff = ((float)PLANE_MINEFF / 100.0);
    int points;
    int w_p_eff;
    int hcm, lcm, mil;
    int freeplane = 0;

    mil = roundavg(((double)pp->pl_crew * (double)eff));
    /* Always use at least 1 mil to build a plane */
    if (mil == 0 && pp->pl_crew > 0)
	mil = 1;
    hcm = roundavg(((double)pp->pl_hcm * (double)eff));
    lcm = roundavg(((double)pp->pl_lcm * (double)eff));
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
    w_p_eff = (20 + pp->pl_lcm + (pp->pl_hcm * 2));
    points = sp->sct_avail * 100 / w_p_eff;
    if (points < PLANE_MINEFF) {
	pr("Not enough available work in %s to build a %s\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum), pp->pl_name);
	pr(" (%d available work required)\n",
	   1 + PLANE_MINEFF * w_p_eff / 100);
	return 0;
    }
    cost = pp->pl_cost * eff;
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
    sp->sct_avail = (sp->sct_avail * 100 - w_p_eff * PLANE_MINEFF) / 100;
    player->dolcost += cost;
    cash -= cost;
    snxtitem_all(&nstr, EF_PLANE);
    freeplane = 0;
    while (nxtitem(&nstr, (s_char *)&plane)) {
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

    /* Note that this next block of variables can be changed so that individual
     * planes may have their own stats (like based on tech maybe? :) )  Thus,
     * the code now checks the pln_acc, pln_load and pln_fuel instead of using
     * the static definitions of them. */
/*
  n = (int) (pp->pl_range * (0.75 + techfact(tlev - pp->pl_tech, 2.0)));
  if (n > 127)
  n = 127;
  plane.pln_range = n;
  plane.pln_range_max = n;
  n = (int) (pp->pl_att * (0.75 + techfact(tlev - pp->pl_tech, 2.0)));
  if (n > 127)
  n = 127;
  plane.pln_att = n;
  n = (int) (pp->pl_def * (0.75 + techfact(tlev - pp->pl_tech, 2.0)));
  if (n > 127)
  n = 127;
  if (n < pp->pl_def)
  n = pp->pl_def;
  plane.pln_def = n;
  plane.pln_acc = pp->pl_acc;
  plane.pln_load = pp->pl_load;
  plane.pln_fuel = pp->pl_fuel;
*/
    plane.pln_att = PLN_ATTDEF(pp->pl_att, (int)(tlev - pp->pl_tech));
    plane.pln_def = PLN_ATTDEF(pp->pl_def, (int)(tlev - pp->pl_tech));
    plane.pln_acc = PLN_ACC(pp->pl_acc, (int)(tlev - pp->pl_tech));
    plane.pln_range = PLN_RAN(pp->pl_range, (int)(tlev - pp->pl_tech));
    plane.pln_range_max = plane.pln_range;
    plane.pln_load = PLN_LOAD(pp->pl_load, (int)(tlev - pp->pl_tech));
    plane.pln_fuel = pp->pl_fuel;

    plane.pln_wing = ' ';
    plane.pln_tech = tlev;
    plane.pln_ship = -1;
    plane.pln_land = -1;
    plane.pln_uid = nstr.cur;
    plane.pln_nuketype = -1;
    plane.pln_harden = 0;
    plane.pln_sell = 0;
    plane.pln_flags = 0;
    makenotlost(EF_PLANE, plane.pln_own, plane.pln_uid, plane.pln_x,
		plane.pln_y);
    putplane(plane.pln_uid, &plane);
    pr("%s built in sector %s\n", prplane(&plane),
       xyas(sp->sct_x, sp->sct_y, player->cnum));
    vec[I_LCM] -= lcm;
    vec[I_HCM] -= hcm;
    vec[I_MILIT] -= mil;
    return 1;
}

static int
build_tower(register struct sctstr *sp, register int *vec)
{
    struct sctstr sect;
    int val;
    int newx, newy;
    int w_p_eff;
    int points;
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

    if (!opt_NO_HCMS) {
	if (vec[I_HCM] < buil_tower_bh) {
	    pr("%s only has %d unit%s of hcm,\n",
	       xyas(sp->sct_x, sp->sct_y, player->cnum),
	       vec[I_HCM], vec[I_HCM] > 1 ? "s" : "");
	    pr("(a bridge tower requires %d)\n", buil_tower_bh);
	    return 0;
	}
    } else if (!opt_NO_LCMS) {
	if (vec[I_LCM] < buil_tower_bh) {
	    pr("%s only has %d unit%s of lcm,\n",
	       xyas(sp->sct_x, sp->sct_y, player->cnum),
	       vec[I_LCM], vec[I_LCM] > 1 ? "s" : "");
	    pr("(a bridge tower requires %d)\n", buil_tower_bh);
	    return 0;
	}
    }
    if (cash < buil_tower_bc) {
	pr("A bridge tower costs $%.2f to build; ", buil_tower_bc);
	pr("you only have %d.\n", cash);
	return 0;
    }
    w_p_eff = buil_tower_bh * 2;
    points = sp->sct_avail * 100 / w_p_eff;
    if (points < 20) {
	pr("Not enough available work in %s to build a bridge tower\n",
	   xyas(sp->sct_x, sp->sct_y, player->cnum));
	pr(" (%d available work required)\n", 1 + (w_p_eff * 20) / 100);
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

    sp->sct_avail = (sp->sct_avail * 100 - w_p_eff * 20) / 100;
    player->dolcost += buil_tower_bc;
    cash -= buil_tower_bc;
    sect.sct_type = SCT_BTOWER;
    sect.sct_newtype = SCT_BTOWER;
    sect.sct_effic = 20;
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
    putvar(V_MINE, 0, (s_char *)&sect, EF_SECTOR);
    putsect(&sect);
    pr("Bridge tower built in %s\n",
       xyas(sect.sct_x, sect.sct_y, player->cnum));
    if (!opt_NO_HCMS)
	vec[I_HCM] -= buil_tower_bh;
    else if (!opt_NO_LCMS)
	vec[I_LCM] -= buil_tower_bh;
    return 1;
}
