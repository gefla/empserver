/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  show.c: General show routines
 * 
 *  Known contributors to this file:
 *     Julian Onions, 1988
 *     Jeff Bailey, 1990
 *     Steve McClure, 1996
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2006
 */

#include <config.h>

#include <math.h>
#include "file.h"
#include "item.h"
#include "land.h"
#include "nat.h"
#include "news.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "product.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"

struct look_list {
    union {
	struct lchrstr *lp;
	struct plchrstr *pp;
	struct mchrstr *mp;
    } l_u;
    int tech;
};

/*
 * Change this if there are ever more than 200 ships, plane or land
 * unit types.
 */
static struct look_list lookup_list[200];
static int lookup_list_cnt = 0;

static void
sort_lookup_list(void)
{
    struct natstr *np = getnatp(player->cnum);
    struct look_list tmp;
    int i;
    int j;

    if (!(np->nat_flags & NF_TECHLISTS))
	return;
    for (i = 0; i < lookup_list_cnt; i++) {
	for (j = i; j < lookup_list_cnt; j++) {
	    if (lookup_list[j].tech < lookup_list[i].tech) {
		tmp = lookup_list[j];
		lookup_list[j] = lookup_list[i];
		lookup_list[i] = tmp;
	    }
	}
    }
}

static void
make_new_list(int tlev, int type)
{
    struct plchrstr *pp;
    struct lchrstr *lp;
    struct mchrstr *mp;

    lookup_list_cnt = 0;
    if (type == EF_PLANE) {
	for (pp = plchr; pp->pl_name; pp++) {
	    if (pp->pl_tech > tlev)
		continue;
	    lookup_list[lookup_list_cnt].l_u.pp = pp;
	    lookup_list[lookup_list_cnt].tech = pp->pl_tech;
	    lookup_list_cnt++;
	}
    } else if (type == EF_SHIP) {
	for (mp = mchr; mp->m_name; mp++) {
	    if (mp->m_tech > tlev)
		continue;
	    lookup_list[lookup_list_cnt].l_u.mp = mp;
	    lookup_list[lookup_list_cnt].tech = mp->m_tech;
	    lookup_list_cnt++;
	}
    } else if (type == EF_LAND) {
	for (lp = lchr; lp->l_name; lp++) {
	    if (lp->l_tech > tlev)
		continue;
	    lookup_list[lookup_list_cnt].l_u.lp = lp;
	    lookup_list[lookup_list_cnt].tech = lp->l_tech;
	    lookup_list_cnt++;
	}
    } else
	return;

    sort_lookup_list();
}

void
show_bridge(int tlev)
{
    if (tlev < buil_bt)
	return;
    pr("Bridges require %g tech,", buil_bt);
    pr(" %d hcm,", buil_bh);
    pr(" %d workers,\n", 0);
    pr("%d available workforce, and cost $%g\n",
       (SCT_BLD_WORK(0, buil_bh) * SCT_MINEFF + 99) / 100,
       buil_bc);
}

void
show_tower(int tlev)
{
    if (tlev < buil_tower_bt)
	return;
    pr("Bridge Towers require %g tech,", buil_tower_bt);
    pr(" %d hcm,", buil_tower_bh);
    pr(" %d workers,\n", 0);
    pr("%d available workforce, and cost $%g\n",
       (SCT_BLD_WORK(0, buil_tower_bh) * SCT_MINEFF + 99) / 100,
       buil_tower_bc);
}

void
show_nuke_stats(int tlev)
{
    show_nuke_capab(tlev);
}

void
show_nuke_build(int tlev)
{
    struct nchrstr *np;
    int avail;

    pr("%13s lcm hcm  oil  rad avail tech res $\n", "");

    for (np = nchr; np->n_name; np++) {
	avail = NUK_BLD_WORK(np->n_lcm, np->n_hcm, np->n_oil, np->n_rad);
	if (np->n_tech > tlev)
	    continue;
	pr("%-13.13s %3d %3d %4d %4d %5d %4d %3.0f $%6d\n",
	   np->n_name, np->n_lcm, np->n_hcm, np->n_oil,
	   np->n_rad, avail, np->n_tech,
	   drnuke_const > MIN_DRNUKE_CONST ?
	   	(np->n_tech * drnuke_const) + 1.0 : 0.0,
	   np->n_cost);
    }
}

void
show_nuke_capab(int tlev)
{
    struct nchrstr *np;
    int i, j;
    char *p;

    pr("%13s blst dam lbs tech res $%7s abilities\n", "", "");

    for (np = nchr; np->n_name; np++) {
	if (np->n_tech > tlev)
	    continue;
	pr("%-13.13s %4d %3d %3d %4d %3.0f $%7d ",
	   np->n_name, np->n_blast, np->n_dam,
	   np->n_weight, np->n_tech,
	   drnuke_const > MIN_DRNUKE_CONST ?
	   	(np->n_tech * drnuke_const) + 1.0 : 0.0,
	   np->n_cost);
	for (i = j = 0; i < 32; i++) {
	    if (!(np->n_flags & bit(i)))
		continue;
	    if (NULL != (p = symbol_by_value(bit(i), nuke_chr_flags))) {
		if (j++ > 0)
		    pr(" ");
		pr("%s", p);
	    }
	}
	pr("\n");
    }
}

void
show_ship_build(int tlev)
{
    struct mchrstr *mp;
    int n;

    pr("%25s lcm hcm avail tech $\n", "");
    make_new_list(tlev, EF_SHIP);
    for (n = 0; n < lookup_list_cnt; n++) {
	mp = (struct mchrstr *)lookup_list[n].l_u.mp;
	/* Can't show trade ships unless it's turned on */
	if ((mp->m_flags & M_TRADE) && !opt_TRADESHIPS)
	    continue;

	pr("%-25.25s %3d %3d %5d %4d $%d\n",
	   mp->m_name, mp->m_lcm, mp->m_hcm,
	   SHP_BLD_WORK(mp->m_lcm, mp->m_hcm), mp->m_tech, mp->m_cost);
    }
}

void
show_ship_stats(int tlev)
{
    struct mchrstr *mp;
    int scount;
    int techdiff;

    pr("%25s      s  v  s  r  f  l  p", "");
    pr("  h");
    pr("  x");
    if (opt_FUEL)
	pr("  fuel");
    pr("\n");

    pr("%25s      p  i  p  n  i  n  l", "");
    pr("  e");
    pr("  p");
    if (opt_FUEL)
	pr("   c/u");
    pr("\n");

    pr("%25s def  d  s  y  g  r  d  n", "");
    pr("  l");
    pr("  l");
    if (opt_FUEL)
	pr("      ");
    pr("\n");


    make_new_list(tlev, EF_SHIP);
    for (scount = 0; scount < lookup_list_cnt; scount++) {
	mp = (struct mchrstr *)lookup_list[scount].l_u.mp;
	/* Can't show trade ships unless it's turned on */
	if ((mp->m_flags & M_TRADE) && !opt_TRADESHIPS)
	    continue;

	techdiff = (int)(tlev - mp->m_tech);
	pr("%-25.25s %3d %2d %2d %2d %2d %2d ",
	   mp->m_name,
	   (short)SHP_DEF(mp->m_armor, techdiff),
	   (short)SHP_SPD(mp->m_speed, techdiff),
	   (short)SHP_VIS(mp->m_visib, techdiff),
	   mp->m_vrnge,
	   (short)SHP_RNG(mp->m_frnge, techdiff),
	   (short)SHP_FIR(mp->m_glim, techdiff));

	pr("%2d ", mp->m_nland);
	pr("%2d ", mp->m_nplanes);
	pr("%2d ", mp->m_nchoppers);
	pr("%2d ", mp->m_nxlight);
	if (opt_FUEL)
	    pr("%3d/%1d ", mp->m_fuelc, mp->m_fuelu);
	pr("\n");
    }
}

void
show_ship_capab(int tlev)
{
    struct mchrstr *mp;
    i_type i;
    int j;
    int scount;
    int n;
    char *p;

    pr("%25s cargos & capabilities\n", "");

    make_new_list(tlev, EF_SHIP);
    for (scount = 0; scount < lookup_list_cnt; scount++) {
	mp = (struct mchrstr *)lookup_list[scount].l_u.mp;
	/* Can't show trade ships unless it's turned on */
	if ((mp->m_flags & M_TRADE) && !opt_TRADESHIPS)
	    continue;

	pr("%-25.25s ", mp->m_name);

	for (i = I_NONE + 1; i <= I_MAX; ++i)
	    if (mp->m_item[i])
		pr(" %d%c", mp->m_item[i], ichr[i].i_mnem);
	pr(" ");
	for (j = n = 0; j < 32; j++) {
	    if (!(mp->m_flags & bit(j)))
		continue;
	    if (NULL != (p = symbol_by_value(bit(j), ship_chr_flags))) {
		if (n++ > 0)
		    pr(" ");
		pr("%s", p);
	    }
	}
	pr("\n");
    }
}

void
show_plane_stats(int tlev)
{
    struct plchrstr *pp;
    int pcount;

    pr("%25s acc load att def ran fuel stlth\n", "");
    make_new_list(tlev, EF_PLANE);
    for (pcount = 0; pcount < lookup_list_cnt; pcount++) {
	pp = (struct plchrstr *)lookup_list[pcount].l_u.pp;
	pr("%-25.25s %3d %4d %3d %3d %3d %4d ",
	   pp->pl_name,
	   (int)PLN_ACC(pp->pl_acc, (int)(tlev - pp->pl_tech)),
	   (int)PLN_LOAD(pp->pl_load, (int)(tlev - pp->pl_tech)),
	   (int)PLN_ATTDEF(pp->pl_att, (int)(tlev - pp->pl_tech)),
	   (int)PLN_ATTDEF(pp->pl_def, (int)(tlev - pp->pl_tech)),
	   (int)PLN_RAN(pp->pl_range, (int)(tlev - pp->pl_tech)),
	   pp->pl_fuel);
	pr("%4d%% ", pp->pl_stealth);
	pr("\n");
    }
}

void
show_plane_capab(int tlev)
{
    struct plchrstr *pp;
    int i;
    int pcount;
    int n;
    char *p;

    pr("%25s capabilities\n", "");
    make_new_list(tlev, EF_PLANE);
    for (pcount = 0; pcount < lookup_list_cnt; pcount++) {
	pp = (struct plchrstr *)lookup_list[pcount].l_u.pp;
	pr("%-25.25s  ", pp->pl_name);

	for (i = n = 0; i < 32; i++) {
	    if (!(pp->pl_flags & bit(i)))
		continue;
	    if (NULL != (p = symbol_by_value(bit(i), plane_chr_flags))) {
		if (n++ > 0)
		    pr(" ");
		pr("%s", p);
	    }
	}
	pr("\n");
    }
}

void
show_plane_build(int tlev)
{
    struct plchrstr *pp;
    int pcount;

    pr("%25s lcm hcm crew avail tech $\n", "");
    make_new_list(tlev, EF_PLANE);
    for (pcount = 0; pcount < lookup_list_cnt; pcount++) {
	pp = (struct plchrstr *)lookup_list[pcount].l_u.pp;
	pr("%-25.25s %3d %3d %4d %5d %4d $%d\n",
	   pp->pl_name, pp->pl_lcm,
	   pp->pl_hcm, pp->pl_crew,
	   PLN_BLD_WORK(pp->pl_lcm, pp->pl_hcm), pp->pl_tech, pp->pl_cost);
    }
}

void
show_land_build(int tlev)
{
    struct lchrstr *lp;
    int n;

    pr("%25s lcm hcm guns avail tech $\n", "");
    make_new_list(tlev, EF_LAND);
    for (n = 0; n < lookup_list_cnt; n++) {
	lp = (struct lchrstr *)lookup_list[n].l_u.lp;
	if ((lp->l_flags & L_SPY) && !opt_LANDSPIES)
	    continue;
	pr("%-25.25s %3d %3d %4d %5d %4d $%d\n",
	   lp->l_name, lp->l_lcm,
	   lp->l_hcm,
	   lp->l_gun,
	   LND_BLD_WORK(lp->l_lcm, lp->l_hcm), lp->l_tech, lp->l_cost);
    }
}

void
show_land_capab(int tlev)
{
    struct lchrstr *lcp;
    int lcount;
    i_type i;
    int j, n;
    char *p;

    pr("%25s capabilities\n", "");

    make_new_list(tlev, EF_LAND);
    for (lcount = 0; lcount < lookup_list_cnt; lcount++) {
	lcp = (struct lchrstr *)lookup_list[lcount].l_u.lp;
	if ((lcp->l_flags & L_SPY) && !opt_LANDSPIES)
	    continue;

	pr("%-25s ", lcp->l_name);

	for (i = I_NONE + 1; i <= I_MAX; ++i)
	    if (lcp->l_item[i])
		pr(" %d%c", lcp->l_item[i], ichr[i].i_mnem);
	pr(" ");
	for (j = n = 0; j < 32; j++) {
	    if (!(lcp->l_flags & bit(j)))
		continue;
	    if (NULL != (p = symbol_by_value(bit(j), land_chr_flags))) {
		if (n++ > 0)
		    pr(" ");
		pr("%s", p);
	    }
	}
	pr("\n");
    }
}

void
show_land_stats(int tlev)
{
    struct lchrstr *lcp;
    int lcount;
    int ourtlev;

    pr("%25s              s  v  s  r  r  a  f  a  a        x  l\n", "");
    pr("%25s              p  i  p  a  n  c  i  m  a  f  f  p  n\n", "");
    pr("%25s att def vul  d  s  y  d  g  c  r  m  f  c  u  l  d\n", "");

    make_new_list(tlev, EF_LAND);
    for (lcount = 0; lcount < lookup_list_cnt; lcount++) {
	lcp = (struct lchrstr *)lookup_list[lcount].l_u.lp;
	if ((lcp->l_flags & L_SPY) && !opt_LANDSPIES)
	    continue;

	ourtlev = (int)(tlev - lcp->l_tech);
	pr("%-25s %1.1f %1.1f %3d ",
	   lcp->l_name,
	   LND_ATTDEF(lcp->l_att, ourtlev),
	   LND_ATTDEF(lcp->l_def, ourtlev),
	   (int)LND_VUL(lcp->l_vul, ourtlev));
	pr("%2d %2d %2d %2d ",
	   (int)LND_SPD(lcp->l_spd, ourtlev),
	   (int)LND_VIS(lcp->l_vis, ourtlev),
	   (int)LND_SPY(lcp->l_spy, ourtlev),
	   (int)LND_RAD(lcp->l_rad, ourtlev));
	pr("%2d %2d %2d %2d %2d ",
	   (int)LND_FRG(lcp->l_frg, ourtlev),
	   (int)LND_ACC(lcp->l_acc, ourtlev),
	   (int)LND_DAM(lcp->l_dam, ourtlev),
	   (int)LND_AMM(lcp->l_ammo, ourtlev),
	   (int)LND_AAF(lcp->l_aaf, ourtlev));
	pr("%2d %2d %2d %2d ",
	   (int)LND_FC(lcp->l_fuelc, ourtlev),
	   (int)LND_FU(lcp->l_fuelu, ourtlev),
	   (int)LND_XPL(lcp->l_nxlight, ourtlev),
	   (int)LND_MXL(lcp->l_nland, ourtlev));

	pr("\n");
    }
}

void
show_sect_build(int foo)
{
    int x, first;

    first = 1;
    for (x = 0; dchr[x].d_name; x++) {
	if (dchr[x].d_mnem == 0)
	    continue;
	if (dchr[x].d_cost < 0)
	    continue;
	if ((dchr[x].d_cost > 0) || (dchr[x].d_build != 1) ||
	    (dchr[x].d_lcms > 0) || (dchr[x].d_hcms > 0)) {
	    if (first) {
		pr("sector type    cost to des    cost for 1%% eff   lcms for 1%%    hcms for 1%%\n");
		first = 0;
	    }
	    pr("%-14c %-14d %-17d %-14d %d\n",
	       dchr[x].d_mnem, dchr[x].d_cost, dchr[x].d_build,
	       dchr[x].d_lcms, dchr[x].d_hcms);
	}
    }

    first = 1;
    for (x = 0; intrchr[x].in_name; x++) {
	if (!intrchr[x].in_enable)
	    continue;
	if (first)
	    pr("\nInfrastructure building - adding 1 point of efficiency costs:\n"
	       "       type          lcms    hcms    mobility    $$$$\n");
	pr("%-20s %4d    %4d    %8d    %4d\n",
	   intrchr[x].in_name, intrchr[x].in_lcms, intrchr[x].in_hcms,
	   intrchr[x].in_mcost, intrchr[x].in_dcost);
	first = 0;
    }
}

void
show_sect_stats(int foo)
{
    int x, first = 1;
    struct natstr *natp;

    natp = getnatp(player->cnum);
    for (x = 0; dchr[x].d_name; x++) {
	if (dchr[x].d_mnem == 0)
	    continue;
	if (first) {
	    pr("                        mob cost   max   max   --  packing bonus  --   max\n");
	    pr("  sector type            0%% 100%%   off   def   mil  uw civ bar other   pop\n");
	    first = 0;
	}
	pr("%c %-21.21s", dchr[x].d_mnem, dchr[x].d_name);
	if (dchr[x].d_mob0 < 0)
	    pr("  no way ");
	else
	    pr(" %3.1f  %3.1f", dchr[x].d_mob0, dchr[x].d_mob1);
	pr("  %5.2f %5.2f   %3d %3d %3d %3d %5d %5d\n",
	   dchr[x].d_ostr, dchr[x].d_dstr,
	   ichr[I_MILIT].i_pkg[dchr[x].d_pkg],
	   ichr[I_UW].i_pkg[dchr[x].d_pkg],
	   ichr[I_CIVIL].i_pkg[dchr[x].d_pkg],
	   ichr[I_BAR].i_pkg[dchr[x].d_pkg],
	   ichr[I_LCM].i_pkg[dchr[x].d_pkg],
	   max_population(natp->nat_level[NAT_RLEV], x, 100));
    }
}

void
show_sect_capab(int foo)
{
    int x, first = 1, i, j;
    char *tmpstr;

    for (x = 0; dchr[x].d_name; x++) {
	if (dchr[x].d_mnem == 0 || dchr[x].d_prd < 0)
	    continue;
	if (first) {
	    pr("                                                 --- level ---          reso \n");
	    pr("  sector type             product use1 use2 use3 level min lag eff%% $$$ dep c\n");
	    first = 0;
	}

	j = dchr[x].d_prd;

	pr("%c %-23s %-7s ",
	   dchr[x].d_mnem, dchr[x].d_name, pchr[j].p_sname);
	(void)CANT_HAPPEN(MAXPRCON > 3); /* output has only three columns */
	for (i = 0; i < 3; i++) {
	    if (i < MAXPRCON
		&& pchr[j].p_camt[i]
		&& pchr[j].p_ctype[i] > I_NONE
		&& pchr[j].p_ctype[i] <= I_MAX) {
		pr("%2d %c ", pchr[j].p_camt[i],
		   ichr[pchr[j].p_ctype[i]].i_name[0]);
	    } else {
		pr("     ");
	    }
	}
	switch (pchr[j].p_nlndx) {
	case NAT_TLEV:
	    tmpstr = "tech";
	    break;
	case NAT_ELEV:
	    tmpstr = "edu";
	    break;
	case NAT_RLEV:
	    tmpstr = "res";
	    break;
	case NAT_HLEV:
	    tmpstr = "hap";
	    break;
	default:
	    tmpstr = " ";
	    break;
	}
	pr("%-5s %3d %3d %4d %3d %3d %c",
	   tmpstr,
	   pchr[j].p_nlmin,
	   pchr[j].p_nllag,
	   dchr[x].d_peffic, pchr[j].p_cost, pchr[j].p_nrdep,
	   pchr[j].p_type != I_NONE ? ichr[pchr[j].p_type].i_mnem : ' ');

	pr("\n");
    }
}

void
show_item(int tlev)
{
    struct ichrstr *ip;

    pr("item value sell lbs    packing     melt  item\n");
    pr("mnem                in rg wh ur bk deno  name\n");

    for (ip = ichr; ip->i_name; ip++) {
	pr("   %c %5d %4s %3d %2d %2d %2d %2d %2d %4d  %s\n",
	   ip->i_mnem, ip->i_value, ip->i_sell ? "yes" : "no", ip->i_lbs,
	   ip->i_pkg[IPKG], ip->i_pkg[NPKG], ip->i_pkg[WPKG],
	   ip->i_pkg[UPKG], ip->i_pkg[BPKG],
	   ip->i_melt_denom, ip->i_name);
    }
}

void
show_news(int tlev)
{
    int i, j;
    
    pr("id category           good will\n");
    pr("    messsages\n");

    for (i = 1; i < N_MAX_VERB + 1; i++) {
	pr("%-2d %-20.20s %4d\n", rpt[i].r_uid,
	    page_headings[rpt[i].r_newspage].name, rpt[i].r_good_will);

	for (j = 0; j < NUM_RPTS; j++)
	    pr("    %s\n", rpt[i].r_newstory[j]);
    }
}
