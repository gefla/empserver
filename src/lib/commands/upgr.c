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
 *  upgr.c: Upgrade tech of ships/planes/units
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996-2000
 */

#include <math.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "var.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "sect.h"
#include "nat.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

static int lupgr(void);
static int pupgr(void);
static int supgr(void);

int
upgr(void)
{
    s_char *p;
    s_char buf[1024];

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
	pr("Ships, land units or planes only!\n");
	return RET_SYN;
    }
    return RET_OK;
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
    int w_p_eff;
    int points;
    int rel;
    int techdiff;
    long cash;

    if (!snxtitem(&ni, EF_LAND, player->argp[2]))
	return RET_SYN;
    ni.flags &= ~(EFF_OWNER);
    natp = getnatp(player->cnum);
    cash = natp->nat_money;
    tlev = (int)natp->nat_level[NAT_TLEV];
    n = 0;
    while (nxtitem(&ni, (s_char *)&land)) {
	if (land.lnd_own == 0)
	    continue;
	getsect(land.lnd_x, land.lnd_y, &sect);
	if (sect.sct_own != player->cnum)
	    continue;
	if (sect.sct_type != SCT_HEADQ || sect.sct_effic < 60)
	    continue;
	rel = getrel(getnatp(land.lnd_own), sect.sct_own);
	if ((rel < FRIENDLY) && (sect.sct_own != land.lnd_own)) {
	    pr("You are not on friendly terms with the owner of unit %d!\n", land.lnd_uid);
	    continue;
	}
	n++;
	lp = &lchr[(int)land.lnd_type];
	w_p_eff = ((lp->l_lcm / 2) + lp->l_hcm);
	points = sect.sct_avail * 100 / w_p_eff;
	if (points < 20) {
	    pr("Not enough available work in %s to upgrade a %s\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), lp->l_name);
	    pr(" (%d available work required)\n",
	       1 + (w_p_eff * 20) / 100);
	    continue;
	}
	if (land.lnd_effic < 60) {
	    pr("%s is too damaged to upgrade!\n", prland(&land));
	    continue;
	}
	if (land.lnd_tech >= tlev) {
	    pr("%s tech: %d, yours is only %d\n", prland(&land),
	       land.lnd_tech, tlev);
	    continue;
	}
	if (lp->l_cost * .15 + player->dolcost > cash) {
	    pr("You don't have enough money to upgrade %s!\n",
	       prland(&land));
	    continue;
	}

	sect.sct_avail = (sect.sct_avail * 100 - w_p_eff * 20) / 100;
	land.lnd_effic -= 35;

	land.lnd_tech = tlev;
	techdiff = (int)(tlev - lp->l_tech);

	land.lnd_harden = 0;
	land.lnd_mission = 0;

	land.lnd_att = (float)LND_ATTDEF(lp->l_att, techdiff);
	land.lnd_def = (float)LND_ATTDEF(lp->l_def, techdiff);
	land.lnd_vul = (int)LND_VUL(lp->l_vul, techdiff);
	land.lnd_spd = (int)LND_SPD(lp->l_spd, techdiff);
	land.lnd_vis = (int)LND_VIS(lp->l_vis, techdiff);
	land.lnd_spy = (int)LND_SPY(lp->l_spy, techdiff);
	land.lnd_rad = (int)LND_RAD(lp->l_rad, techdiff);
	land.lnd_frg = (int)LND_FRG(lp->l_frg, techdiff);
	land.lnd_acc = (int)LND_ACC(lp->l_acc, techdiff);
	land.lnd_dam = (int)LND_DAM(lp->l_dam, techdiff);
	land.lnd_ammo = (int)LND_AMM(lp->l_ammo, lp->l_dam, techdiff);
	land.lnd_aaf = (int)LND_AAF(lp->l_aaf, techdiff);
	land.lnd_fuelc = (int)LND_FC(lp->l_fuelc, techdiff);
	land.lnd_fuelu = (int)LND_FU(lp->l_fuelu, techdiff);
	land.lnd_maxlight = (int)LND_XPL(lp->l_nxlight, techdiff);
	land.lnd_maxland = (int)LND_MXL(lp->l_mxland, techdiff);

	time(&land.lnd_access);

	putland(land.lnd_uid, &land);
	putsect(&sect);
	player->dolcost += (double)lp->l_cost * .15;
	pr("%s upgraded to tech %d, at a cost of %d\n", prland(&land),
	   land.lnd_tech, (int)(lp->l_cost * .15));
	if (land.lnd_own != player->cnum)
	    wu(0, land.lnd_own,
	       "%s upgraded by %s to tech %d, at a cost of %d\n",
	       prland(&land), cname(player->cnum), land.lnd_tech,
	       (int)(lp->l_cost * .15));
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
    int w_p_eff;
    int points;
    int rel;
    int techdiff;
    long cash;

    if (!snxtitem(&ni, EF_SHIP, player->argp[2]))
	return RET_SYN;
    ni.flags &= ~(EFF_OWNER);
    natp = getnatp(player->cnum);
    cash = natp->nat_money;
    tlev = (int)natp->nat_level[NAT_TLEV];
    n = 0;
    while (nxtitem(&ni, (s_char *)&ship)) {
	if (ship.shp_own == 0)
	    continue;
	getsect(ship.shp_x, ship.shp_y, &sect);
	if (sect.sct_own != player->cnum)
	    continue;
	if (sect.sct_type != SCT_HARBR || sect.sct_effic < 60)
	    continue;
	rel = getrel(getnatp(ship.shp_own), sect.sct_own);
	if ((rel < FRIENDLY) && (sect.sct_own != ship.shp_own)) {
	    pr("You are not on friendly terms with the owner of ship %d!\n", ship.shp_uid);
	    continue;
	}
	n++;
	mp = &mchr[(int)ship.shp_type];
	w_p_eff = ((mp->m_lcm / 2) + mp->m_hcm);
	points = sect.sct_avail * 100 / w_p_eff;
	if (points < 20) {
	    pr("Not enough available work in %s to upgrade a %s\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), mp->m_name);
	    pr(" (%d available work required)\n",
	       1 + (w_p_eff * 20) / 100);
	    continue;
	}
	if (ship.shp_effic < 60) {
	    pr("%s is too damaged to upgrade!\n", prship(&ship));
	    continue;
	}
	if (ship.shp_tech >= tlev) {
	    pr("%s tech: %d, yours is only %d\n", prship(&ship),
	       ship.shp_tech, tlev);
	    continue;
	}
	if (mp->m_cost * .15 + player->dolcost > cash) {
	    pr("You don't have enough money to upgrade %s!\n",
	       prship(&ship));
	    continue;
	}

	sect.sct_avail = (sect.sct_avail * 100 - w_p_eff * 20) / 100;
	ship.shp_effic -= 35;
	ship.shp_tech = tlev;

	techdiff = (int)(tlev - mp->m_tech);
	ship.shp_armor = (short)SHP_DEF(mp->m_armor, techdiff);
	ship.shp_speed = (short)SHP_SPD(mp->m_speed, techdiff);
	ship.shp_visib = (short)SHP_VIS(mp->m_visib, techdiff);
	ship.shp_frnge = (short)SHP_RNG(mp->m_frnge, techdiff);
	ship.shp_glim = (short)SHP_FIR(mp->m_glim, techdiff);

	ship.shp_mission = 0;
	time(&ship.shp_access);

	putship(ship.shp_uid, &ship);
	putsect(&sect);
	player->dolcost += (double)mp->m_cost * .15;
	pr("%s upgraded to tech %d, at a cost of %d\n", prship(&ship),
	   ship.shp_tech, (int)(mp->m_cost * .15));
	if (ship.shp_own != player->cnum)
	    wu(0, ship.shp_own,
	       "%s upgraded by %s to tech %d, at a cost of %d\n",
	       prship(&ship), cname(player->cnum), ship.shp_tech,
	       (int)(mp->m_cost * .15));
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
    int w_p_eff;
    int points;
    int rel;
    int techdiff;
    long cash;

    if (!snxtitem(&ni, EF_PLANE, player->argp[2]))
	return RET_SYN;
    ni.flags &= ~(EFF_OWNER);
    natp = getnatp(player->cnum);
    cash = natp->nat_money;
    tlev = (int)natp->nat_level[NAT_TLEV];
    n = 0;
    while (nxtitem(&ni, (s_char *)&plane)) {
	if (plane.pln_own == 0)
	    continue;
	getsect(plane.pln_x, plane.pln_y, &sect);
	if (sect.sct_own != player->cnum)
	    continue;
	if (sect.sct_type != SCT_AIRPT || sect.sct_effic < 60)
	    continue;
	rel = getrel(getnatp(plane.pln_own), sect.sct_own);
	if ((rel < FRIENDLY) && (sect.sct_own != plane.pln_own)) {
	    pr("You are not on friendly terms with the owner of plane %d!\n", plane.pln_uid);
	    continue;
	}
	n++;
	pp = &plchr[(int)plane.pln_type];
	w_p_eff = ((pp->pl_lcm / 2) + pp->pl_hcm);
	points = sect.sct_avail * 100 / w_p_eff;
	if (points < 20) {
	    pr("Not enough available work in %s to upgrade a %s\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), pp->pl_name);
	    pr(" (%d available work required)\n",
	       1 + (w_p_eff * 20) / 100);
	    continue;
	}
	if (plane.pln_effic < 60) {
	    pr("%s is too damaged to upgrade!\n", prplane(&plane));
	    continue;
	}
	if (plane.pln_tech >= tlev) {
	    pr("%s tech: %d, yours is only %d\n", prplane(&plane),
	       plane.pln_tech, tlev);
	    continue;
	}
	if (pp->pl_cost * .15 + player->dolcost > cash) {
	    pr("You don't have enough money to upgrade %s!\n",
	       prplane(&plane));
	    continue;
	}
	if (plane.pln_flags & PLN_LAUNCHED) {
	    pr("Plane %s is in orbit!\n", prplane(&plane));
	    continue;
	}

	sect.sct_avail = (sect.sct_avail * 100 - w_p_eff * 20) / 100;
	plane.pln_effic -= 35;

	plane.pln_tech = tlev;
	techdiff = (int)(tlev - pp->pl_tech);

	plane.pln_mission = 0;
	plane.pln_opx = 0;
	plane.pln_opy = 0;
	plane.pln_radius = 0;
	plane.pln_harden = 0;
	plane.pln_mission = 0;

	plane.pln_att = PLN_ATTDEF(pp->pl_att, techdiff);
	plane.pln_def = PLN_ATTDEF(pp->pl_def, techdiff);
	plane.pln_acc = PLN_ACC(pp->pl_acc, techdiff);
	plane.pln_range = PLN_RAN(pp->pl_range, techdiff);
	plane.pln_range_max = plane.pln_range;
	plane.pln_load = PLN_LOAD(pp->pl_load, techdiff);

	time(&plane.pln_access);

	putplane(plane.pln_uid, &plane);
	putsect(&sect);
	player->dolcost += (double)pp->pl_cost * .15;
	pr("%s upgraded to tech %d, at a cost of %d\n", prplane(&plane),
	   plane.pln_tech, (int)(pp->pl_cost * .15));
	if (plane.pln_own != player->cnum)
	    wu(0, plane.pln_own,
	       "%s upgraded by %s to tech %d, at a cost of %d\n",
	       prplane(&plane), cname(player->cnum), plane.pln_tech,
	       (int)(pp->pl_cost * .15));
    }
    if (n == 0) {
	pr("No planes.\n");
	return RET_SYN;
    }
    return RET_OK;
}
