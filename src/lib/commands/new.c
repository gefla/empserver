/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  new.c: Create a new capital for a player
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "nat.h"
#include "sect.h"
#include "path.h"
#include "file.h"
#include "xy.h"
#include "tel.h"
#include "land.h"
#include "nsc.h"
#include "options.h"
#include "optlist.h"
#include "commands.h"

#include <fcntl.h>

#ifdef START_UNITS
static int deity_build_land(int, coord, coord, natid, int);
#endif
static int isok(int x, int y);
static void ok(s_char *map, int x, int y);

static struct range defrealm = { -8, -5, 10, 5, 0, 0 };

#define	MAXAVAIL	300

int
new(void)
{
    struct sctstr sect;
    struct natstr *natp;
    struct boundstr newrealms;
    struct range absrealm;
    natid num;
    coord x, y;
    int i;
    s_char *p;
    int n;
    s_char buf[1024];

    natp = getnatp(player->cnum);
    if (natp->nat_xorg != 0 || natp->nat_yorg != 0) {
	pr("Must be at 0,0 to add a new country\n");
	return 0;
    }
    if ((n = natarg(player->argp[1], "Country? ")) < 0) {
	pr("Bad country number\n");
	return 0;
    }
    num = n;
    natp = getnatp(num);
    if ((natp->nat_stat & STAT_NEW) == 0) {
	pr("Country #%d (%s) isn't a new country!\n", num, cname(num));
	return RET_SYN;
    }
    if (player->argp[2] != 0) {
	if ((p = getstarg(player->argp[2], "sanctuary pair : ", buf)) == 0)
	    return RET_SYN;
	if (!sarg_xy(p, &x, &y) || !getsect(x, y, &sect))
	    return RET_SYN;
	if (sect.sct_type != SCT_RURAL) {
	    pr("%s is a %s; try again...\n",
	       xyas(x, y, player->cnum), dchr[sect.sct_type].d_name);
	    return RET_SYN;
	}
	getsect(x + 2, y, &sect);
	if (sect.sct_type != SCT_RURAL) {
	    pr("%s is a %s; try again...\n",
	       xyas(x + 2, y, player->cnum), dchr[sect.sct_type].d_name);
	    return RET_SYN;
	}
    } else {
	srandom(time(NULL));
	for (i = 0; i < 300 && !player->aborted; i++) {
	    /* Both x and y should be either odd or even */
	    x = (random() % WORLD_X) - (WORLD_X / 2);
	    y = (((random() % WORLD_Y) - (WORLD_Y / 2)) & ~1) | (x & 1);
	    /*
	     * If either of the two potential
	     * sanctuary sectors are already
	     * owned by someone else, pick
	     * another place on the map.
	     */
	    getsect(x, y, &sect);
	    if (sect.sct_type == SCT_WATER || sect.sct_own != 0)
		continue;
	    getsect(x + 2, y, &sect);
	    if (sect.sct_type == SCT_WATER || sect.sct_own != 0)
		continue;
	    if (isok(x, y))
		break;
	}
	if (i == 300) {
	    pr("couldn't find an empty slot!\n");
	    return RET_FAIL;
	}
    }

    if (player->aborted)
	return RET_FAIL;
    pr("added country %d at %s\n", num, xyas(x, y, player->cnum));
    getsect(x, y, &sect);
    sect.sct_own = num;
    sect.sct_type = SCT_SANCT;
    sect.sct_newtype = SCT_SANCT;
    sect.sct_effic = 100;
    sect.sct_road = 0;
    sect.sct_rail = 0;
    sect.sct_defense = 0;
    if (!opt_DEFENSE_INFRA)
	sect.sct_defense = sect.sct_effic;
    sect.sct_mobil = startmob;
    sect.sct_work = 100;
    sect.sct_oldown = num;
    if (at_least_one_100) {
	sect.sct_oil = 100;
	sect.sct_fertil = 100;
	sect.sct_uran = 100;
	sect.sct_min = 100;
	sect.sct_gmin = 100;
    }
    sect.sct_item[I_CIVIL] = opt_RES_POP ? 550 : 999;
    sect.sct_item[I_MILIT] = 55;
    sect.sct_item[I_FOOD] = 1000;
    sect.sct_item[I_UW] = 75;
    putsect(&sect);
    getsect(x + 2, y, &sect);
    sect.sct_own = num;
    sect.sct_type = SCT_SANCT;
    sect.sct_newtype = SCT_SANCT;
    sect.sct_effic = 100;
    sect.sct_road = 0;
    sect.sct_rail = 0;
    sect.sct_defense = 0;
    if (!opt_DEFENSE_INFRA)
	sect.sct_defense = sect.sct_effic;
    sect.sct_work = 100;
    sect.sct_oldown = num;
    sect.sct_mobil = startmob;
    if (at_least_one_100) {
	sect.sct_oil = 100;
	sect.sct_fertil = 100;
	sect.sct_uran = 100;
	sect.sct_min = 100;
	sect.sct_gmin = 100;
    }
    sect.sct_item[I_CIVIL] = opt_RES_POP ? 550 : 999;
    sect.sct_item[I_MILIT] = 55;
    sect.sct_item[I_FOOD] = 100;
    sect.sct_item[I_UW] = 75;
    putsect(&sect);
    natp->nat_btu = max_btus;
    natp->nat_stat &= ~STAT_NEW;
    natp->nat_stat |= STAT_SANCT;
    natp->nat_xstart = x;
    natp->nat_ystart = y;
    natp->nat_xcap = x;
    natp->nat_ycap = y;
    if (players_at_00) {
	natp->nat_xorg = 0;
	natp->nat_yorg = 0;
    } else {
	natp->nat_xorg = x;
	natp->nat_yorg = y;
    }
    xyabsrange(natp, &defrealm, &absrealm);
    newrealms.b_xl = absrealm.lx;
    newrealms.b_xh = absrealm.hx;
    newrealms.b_yl = absrealm.ly;
    newrealms.b_yh = absrealm.hy;
    natp->nat_money = START_CASH;
    natp->nat_level[NAT_HLEV] = start_happiness;
    natp->nat_level[NAT_RLEV] = start_research;
    natp->nat_level[NAT_TLEV] = start_technology;
    natp->nat_level[NAT_ELEV] = start_education;
    for (i = 0; i < MAXNOR; i++)
	natp->nat_b[i] = newrealms;
    natp->nat_tgms = 0;
    (void)close(open(mailbox(buf, num), O_RDWR | O_TRUNC | O_CREAT, 0660));
#ifdef START_UNITS
    for (n = 0; n < START_UNITS; n++)
	deity_build_land(start_unit_type[n], x, y, num, 0);
#endif /* START_UNITS */
    putnat(natp);
    return 0;
}

static int nmin, ngold, noil, nur;
static int nfree, navail, nowned;

static int
isok(int x, int y)
{
    s_char *map;
    s_char *p;
    s_char buf[1024];

    nmin = ngold = noil = nur = 0;
    navail = nfree = nowned = 0;
    if ((map = (s_char *)malloc((WORLD_X * WORLD_Y) / 2)) == 0) {
	logerror("malloc failed in isok\n");
	pr("Memory error.  Tell the deity.\n");
	return 0;
    }
    memset(map, 0, (WORLD_X * WORLD_Y) / 2);
    ok(map, x, y);
    free((s_char *)map);
    if (nfree < 5)
	return 0;
    pr("Cap at %s; owned sectors: %d, free sectors: %d, avail: %d\n",
       xyas(x, y, player->cnum), nowned, nfree, navail);
    pr("min: %d, oil: %d, gold: %d, uranium: %d\n",
       nmin, noil, ngold, nur);
    p = getstring("This setup ok? ", buf);
    if (p == 0 || *p != 'y')
	return 0;
    return 1;
}

static void
ok(s_char *map, int x, int y)
{
    struct sctstr sect;
    int dir;
    int id;

    if (navail > MAXAVAIL)
	return;
    id = sctoff(x, y);
    if (map[id])
	return;
    if (!ef_read(EF_SECTOR, id, (s_char *)&sect))
	return;
    if (sect.sct_type == SCT_WATER || sect.sct_type == SCT_BSPAN)
	return;
    navail++;
    if (navail >= MAXAVAIL) {
	pr("At least %d...\n", MAXAVAIL);
	return;
    }
    if (sect.sct_type != SCT_MOUNT && sect.sct_type != SCT_PLAINS) {
	if (sect.sct_own == 0)
	    nfree++;
	else
	    nowned++;
	if (sect.sct_min > 9)
	    nmin++;
	if (sect.sct_gmin > 9)
	    ngold++;
	if (sect.sct_uran > 9)
	    nur++;
	if (sect.sct_oil > 9)
	    noil++;
    }
    map[id] = 1;
    for (dir = DIR_FIRST; dir <= DIR_LAST; dir++)
	ok(map, diroff[dir][0] + x, diroff[dir][1] + y);
}

#ifdef START_UNITS
static int
deity_build_land(int type, coord x, coord y, natid own, int tlev)
{
    struct lndstr land;
    struct lchrstr *lp;
    struct nstr_item nstr;
    struct natstr *natp;
    int extend = 1;

    natp = getnatp(own);

    snxtitem_all(&nstr, EF_LAND);
    while (nxtitem(&nstr, (s_char *)&land)) {
	if (land.lnd_own == 0) {
	    extend = 0;
	    break;
	}
    }
    if (extend)
	ef_extend(EF_LAND, 50);
    land.lnd_x = x;
    land.lnd_y = y;
    land.lnd_own = own;
    land.lnd_mission = 0;
    land.lnd_type = type;
    land.lnd_effic = 100;
    land.lnd_mobil = land_mob_max;
    land.lnd_tech = tlev;
    land.lnd_uid = nstr.cur;
    land.lnd_army = ' ';
    land.lnd_flags = 0;
    land.lnd_ship = -1;
    land.lnd_land = -1;
    land.lnd_nland = 0;
    land.lnd_harden = 0;
    time(&land.lnd_access);

    land.lnd_retreat = morale_base;

    lp = &lchr[type];
    land.lnd_fuel = lp->l_fuelc;
    land.lnd_nxlight = 0;
    land.lnd_rflags = 0;
    memset(land.lnd_rpath, 0, sizeof(land.lnd_rpath));
    land.lnd_rad_max = lp->l_rad;
    memset(land.lnd_item, 0, sizeof(land.lnd_item));

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
    land.lnd_item[I_FOOD] = lp->l_item[I_FOOD];

    putland(land.lnd_uid, &land);
    makenotlost(EF_LAND, land.lnd_own, land.lnd_uid, land.lnd_x,
		land.lnd_y);
    pr("%s", prland(&land));
    pr(" built in sector %s\n", xyas(x, y, player->cnum));
    return 1;
}
#endif /* START_UNITS */
