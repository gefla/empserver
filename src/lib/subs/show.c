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
 *  show.c: General show routines
 * 
 *  Known contributors to this file:
 *     Julian Onions, 1988
 *     Jeff Bailey, 1990
 *     Steve McClure, 1996
 */

/*
 * general routines that are callable to give info on things.
 * currently, planes, ships, nukes and bridges. Tanks & regiments one day?
 *
 * Added nuke_flags to be consistent. Jeff Bailey 12/15/90
 *                                    (bailey@mcs.kent.edu)
 */

#include "misc.h"
#include "player.h"
#include "nuke.h"
#include "var.h"
#include "ship.h"
#include "land.h"
#include "item.h"
#include "plane.h"
#include "sect.h"
#include "optlist.h"
#include "file.h"
#include "nat.h"
#include "prototypes.h"

double sqrt(double);
double logx(double, double);

/*
 * This cruft really belongs in the empglb.c file.
 * Yuck.
 */

struct lookup {
    int key;
    s_char *value;
};

struct lookup ship_flags[] = {
    {M_FOOD, "fish"},
    {M_TORP, "torp"},
    {M_DCH, "dchrg"},
    {M_FLY, "plane"},
    {M_MSL, "miss"},
    {M_OIL, "oil"},
    {M_SONAR, "sonar"},
    {M_MINE, "mine"},
    {M_SWEEP, "sweep"},
    {M_SUB, "sub"},
    {M_SPY, "spy"},
    {M_LAND, "land"},
    {M_SUBT, "sub-torp"},
    {M_TRADE, "trade"},
    {M_SEMILAND, "semi-land"},
    {M_OILER, "oiler"},
    {M_SUPPLY, "supply"},
    {M_ANTIMISSILE, "anti-missile"},
    {0, 0}
};

struct lookup land_flags[] = {
    {L_XLIGHT, "xlight"},
    {L_ENGINEER, "engineer"},
    {L_SUPPLY, "supply"},
    {L_SECURITY, "security"},
    {L_LIGHT, "light"},
    {L_MARINE, "marine"},
    {L_RECON, "recon"},
    {L_RADAR, "radar"},
    {L_ASSAULT, "assault"},
    {L_FLAK, "flak"},
    {L_SPY, "spy"},
    {L_TRAIN, "train"},
    {L_HEAVY, "heavy"},
    {0, 0}
};


struct lookup plane_flags[] = {
    {P_T, "tactical"},
    {P_B, "bomber"},
    {P_F, "intercept"},
    {P_C, "cargo"},
    {P_V, "VTOL"},
    {P_M, "missile"},
    {P_L, "light"},
    {P_S, "spy"},
    {P_I, "image"},
    {P_O, "satellite"},
    {P_X, "stealth"},
    {P_N, "SDI"},
    {P_H, "half-stealth"},
    {P_E, "x-light"},
    {P_K, "helo"},
    {P_A, "ASW"},
    {P_P, "para"},
    {P_ESC, "escort"},
    {P_MINE, "mine"},
    {P_SWEEP, "sweep"},
    {P_MAR, "marine"},
    {0, 0}
};

struct lookup nuke_flags[] = {
    {N_NEUT, "neutron"},
    {0, 0}
};

struct look_list {
    union {
	struct lchrstr *lp;
	struct plchrstr *pp;
	struct mchrstr *mp;
	int value;
    } l_u;
    int tech;
} lookup_list[200];		/* Change this if there are ever more than 200 planes, ships
				   or land units. */
static int lookup_list_cnt = 0;

void
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

static
    void
make_new_list(int tlev, int type)
{
    struct plchrstr *pp;
    struct lchrstr *lp;
    struct mchrstr *mp;
    int count;

    lookup_list_cnt = 0;
    if (type == EF_PLANE) {
	for (pp = plchr, count = 0; count < pln_maxno; count++, pp++) {
	    if (pp->pl_tech > tlev)
		continue;
	    if (pp->pl_name == 0 || pp->pl_name[0] == '\0')
		continue;
	    lookup_list[lookup_list_cnt].l_u.pp = pp;
	    lookup_list[lookup_list_cnt].tech = pp->pl_tech;
	    lookup_list_cnt++;
	}
    } else if (type == EF_SHIP) {
	for (mp = mchr, count = 0; count < shp_maxno; count++, mp++) {
	    if (mp->m_tech > tlev)
		continue;
	    if (mp->m_name == 0 || mp->m_name[0] == '\0')
		continue;
	    lookup_list[lookup_list_cnt].l_u.mp = mp;
	    lookup_list[lookup_list_cnt].tech = mp->m_tech;
	    lookup_list_cnt++;
	}
    } else if (type == EF_LAND) {
	for (lp = lchr, count = 0; count < lnd_maxno; count++, lp++) {
	    if (lp->l_tech > tlev)
		continue;
	    if (lp->l_name == 0 || lp->l_name[0] == '\0')
		continue;
	    lookup_list[lookup_list_cnt].l_u.lp = lp;
	    lookup_list[lookup_list_cnt].tech = lp->l_tech;
	    lookup_list_cnt++;
	}
    } else
	return;

    sort_lookup_list();
}

static
s_char *
lookup(int key, struct lookup *table)
{
    int match;

    if ((match = intmatch(key, &table->key, sizeof(*table))) < 0)
	return 0;
    return table[match].value;
}

void
show_bridge(int tlev)
{
    extern double buil_bt, buil_bc;
    extern int buil_bh;

    if (tlev < buil_bt)
	return;
    pr("Bridges require %g tech,", buil_bt);
    if (!opt_NO_HCMS)
	pr(" %d hcm,", buil_bh);
    else if (!opt_NO_LCMS)
	pr(" %d lcm,", buil_bh);
    pr(" %d workers,\n", buil_bh * 2);
    pr("%d available workforce, and cost $%g\n",
       1 + (buil_bh * 40 / 100), buil_bc);
}

void
show_tower(int tlev)
{
    extern double buil_tower_bt, buil_tower_bc;
    extern int buil_tower_bh;

    if (tlev < buil_tower_bt)
	return;
    pr("Bridge Towers require %g tech,", buil_tower_bt);
    if (!opt_NO_HCMS)
	pr(" %d hcm,", buil_tower_bh);
    else if (!opt_NO_LCMS)
	pr(" %d lcm,", buil_tower_bh);
    pr(" %d workers,\n", buil_tower_bh * 2);
    pr("%d available workforce, and cost $%g\n",
       1 + (buil_tower_bh * 40 / 100), buil_tower_bc);
}

void
show_item(int tlev)
{
    register struct ichrstr *ip;
    register int n;

    pr("item   value sell lbs   packing   item\n");
    pr("mnemo                 rg wh ur bk name\n");

    for (n = 1; n <= itm_maxno; n++) {
	ip = &ichr[n];
	pr("     %c %5d %4s %3d %2d %2d %2d %2d %s\n",
	   ip->i_mnem, ip->i_value, (ip->i_sell == 1) ? "yes" : "no",
	   ip->i_lbs, ip->i_pkg[NPKG], ip->i_pkg[WPKG], ip->i_pkg[UPKG],
	   ip->i_pkg[BPKG], ip->i_name);
    }
}
void
show_nuke_stats(int tlev)
{
    show_nuke_capab(tlev);
}

void
show_nuke_build(int tlev)
{
    register struct nchrstr *np;
    register int n;
    register int avail;
    extern float drnuke_const;

    if (opt_DRNUKE)
	pr("%13s lcm hcm  oil  rad avail tech res $\n", "");
    else
	pr("%13s lcm hcm  oil  rad avail tech $\n", "");

    if (opt_NONUKES)
	return;
    for (np = nchr, n = 0; n < N_MAXNUKE; np++, n++) {
	avail =
	    (4 + np->n_rad + np->n_oil + np->n_lcm + np->n_hcm * 2) / 5;
	if (np->n_tech > tlev)
	    continue;
	if (np->n_name == 0 || np->n_name[0] == '\0')
	    continue;
	if (opt_NEUTRON == 0 && (np->n_flags & N_NEUT))
	    continue;
	if (opt_DRNUKE)
	    pr("%-13.13s %3d %3d %4d %4d %5d %4d %3d $%6d\n",
	       np->n_name, np->n_lcm, np->n_hcm, np->n_oil,
	       np->n_rad, avail, np->n_tech,
	       (int)(np->n_tech * drnuke_const) + 1, np->n_cost);
	else			/* not DRNUKE */
	    pr("%-13.13s %3d %3d %4d %4d %5d %4d $%6d\n",
	       np->n_name, np->n_lcm, np->n_hcm, np->n_oil,
	       np->n_rad, avail, np->n_tech, np->n_cost);
    }
}

void
show_nuke_capab(int tlev)
{
    register struct nchrstr *np;
    register int i, j, n;
    s_char *p;
    extern float drnuke_const;

    if (opt_DRNUKE)
	pr("%13s blst dam lbs tech res $%7s abilities\n", "", "");
    else
	pr("%13s blst dam lbs tech $%7s abilities\n", "", "");

    if (opt_NONUKES)
	return;
    for (np = nchr, n = 0; n < N_MAXNUKE; np++, n++) {
	if (np->n_tech > tlev)
	    continue;
	if (opt_NEUTRON == 0 && (np->n_flags & N_NEUT))
	    continue;
	if (np->n_name == 0 || np->n_name[0] == '\0')
	    continue;
	if (opt_DRNUKE)
	    pr("%-13.13s %4d %3d %3d %4d %3d $%7d ",
	       np->n_name, np->n_blast, np->n_dam,
	       np->n_weight, np->n_tech,
	       (int)(np->n_tech * drnuke_const) + 1, np->n_cost);
	else			/* not DRNUKE */
	    pr("%-13.13s %4d %3d %3d %4d $%7d ",
	       np->n_name, np->n_blast, np->n_dam,
	       np->n_weight, np->n_tech, np->n_cost);

	for (i = j = 0; i < 32; i++) {
	    if (!(np->n_flags & bit(i)))
		continue;
	    if (NULL != (p = lookup(bit(i), nuke_flags))) {
		if (j++ > 0)
		    pr(" ");
		pr(p);
	    }
	}
	pr("\n");
    }
}

void
show_ship_build(int tlev)
{
    register struct mchrstr *mp;
    register int n;

    pr("%25s lcm hcm avail tech $\n", "");
    make_new_list(tlev, EF_SHIP);
    for (n = 0; n < lookup_list_cnt; n++) {
	mp = (struct mchrstr *)lookup_list[n].l_u.mp;
	/* Can't show trade ships unless it's turned on */
	if ((mp->m_flags & M_TRADE) && !opt_TRADESHIPS)
	    continue;

	pr("%-25.25s %3d %3d %5d %4d $%d\n",
	   mp->m_name, mp->m_lcm, mp->m_hcm,
	   20 + mp->m_lcm + mp->m_hcm * 2, mp->m_tech, mp->m_cost);
    }
}

void
show_ship_stats(int tlev)
{
    register struct mchrstr *mp;
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
    register struct mchrstr *mp;
    register u_short *ap;
    register u_char *type;
    register int i;
    register int it;
    int scount;
    int n;
    s_char c;
    s_char *p;

    pr("%25s cargos & capabilities\n", "");

    make_new_list(tlev, EF_SHIP);
    for (scount = 0; scount < lookup_list_cnt; scount++) {
	mp = (struct mchrstr *)lookup_list[scount].l_u.mp;
	/* Can't show trade ships unless it's turned on */
	if ((mp->m_flags & M_TRADE) && !opt_TRADESHIPS)
	    continue;

	pr("%-25.25s ", mp->m_name);

	/*
	 * should use vector stuff
	 */
	for (ap = mp->m_vamt, type = mp->m_vtype, i = 0;
	     i < mp->m_nv; i++, ap++, type++) {
	    it = unitem((int)*type);
	    if (it > 0 && it <= I_MAX && ichr[it].i_name != 0)
		c = ichr[it].i_name[0];
	    else
		c = '?';
	    pr(" %d%c", *ap, c);
	}
	pr(" ");
	for (i = n = 0; i < 32; i++) {
	    if (!(mp->m_flags & bit(i)))
		continue;
	    if (NULL != (p = lookup(bit(i), ship_flags))) {
		if (n++ > 0)
		    pr(" ");
		pr(p);
	    }
	}
	pr("\n");
    }
}

void
show_plane_stats(int tlev)
{
    register struct plchrstr *pp;
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
    register struct plchrstr *pp;
    register int i;
    int pcount;
    int n;
    s_char *p;

    pr("%25s capabilities\n", "");
    make_new_list(tlev, EF_PLANE);
    for (pcount = 0; pcount < lookup_list_cnt; pcount++) {
	pp = (struct plchrstr *)lookup_list[pcount].l_u.pp;
	pr("%-25.25s  ", pp->pl_name);

	for (i = n = 0; i < 32; i++) {
	    if (!(pp->pl_flags & bit(i)))
		continue;
	    if (NULL != (p = lookup(bit(i), plane_flags))) {
		if (n++ > 0)
		    pr(" ");
		pr(p);
	    }
	}
	pr("\n");
    }
}

void
show_plane_build(int tlev)
{
    register struct plchrstr *pp;
    register int pcount;

    pr("%25s lcm hcm crew avail tech  $\n", "");
    make_new_list(tlev, EF_PLANE);
    for (pcount = 0; pcount < lookup_list_cnt; pcount++) {
	pp = (struct plchrstr *)lookup_list[pcount].l_u.pp;
	pr("%-25.25s %3d %3d %3d %5d %4d $%d\n",
	   pp->pl_name, pp->pl_lcm,
	   pp->pl_hcm, pp->pl_crew,
	   20 + 2 * pp->pl_hcm + pp->pl_lcm, pp->pl_tech, pp->pl_cost);
    }
}

void
show_land_build(int tlev)
{
    register struct lchrstr *lp;
    register int n;

    pr("%25s lcm hcm guns avail tech  $\n", "");
    make_new_list(tlev, EF_LAND);
    for (n = 0; n < lookup_list_cnt; n++) {
	lp = (struct lchrstr *)lookup_list[n].l_u.lp;
	if ((lp->l_flags & L_SPY) && !opt_LANDSPIES)
	    continue;
	pr("%-25.25s %3d %3d %4d %5d %4d  $%d\n",
	   lp->l_name, lp->l_lcm,
	   lp->l_hcm,
	   lp->l_gun,
	   20 + lp->l_lcm + (lp->l_hcm * 2), lp->l_tech, lp->l_cost);
    }
}

void
show_land_capab(int tlev)
{
    struct lchrstr *lcp;
    int lcount;
    register u_short *ap;
    register u_char *type;
    register int i, n;
    register int it;
    register s_char *p, c;

    pr("%25s capabilities\n", "");

    make_new_list(tlev, EF_LAND);
    for (lcount = 0; lcount < lookup_list_cnt; lcount++) {
	lcp = (struct lchrstr *)lookup_list[lcount].l_u.lp;
	if ((lcp->l_flags & L_SPY) && !opt_LANDSPIES)
	    continue;

	pr("%-25s ", lcp->l_name);

	/*
	 * should use vector stuff
	 */
	for (ap = lcp->l_vamt, type = lcp->l_vtype, i = 0;
	     i < lcp->l_nv; i++, ap++, type++) {
	    it = unitem((int)*type);
	    if (it > 0 && it <= I_MAX && ichr[it].i_name != 0)
		c = ichr[it].i_name[0];
	    else
		c = '?';
	    pr(" %d%c", *ap, c);
	}
	pr(" ");
	for (i = n = 0; i < 32; i++) {
	    if (!(lcp->l_flags & bit(i)))
		continue;
	    if (NULL != (p = lookup(bit(i), land_flags))) {
		if (n++ > 0)
		    pr(" ");
		pr(p);
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
	   lcp->l_name, (float)LND_ATTDEF(lcp->l_att, ourtlev),
	   (float)LND_ATTDEF(lcp->l_def, ourtlev),
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
	   (int)LND_AMM(lcp->l_ammo, lcp->l_dam, ourtlev),
	   (int)LND_AAF(lcp->l_aaf, ourtlev));
	pr("%2d %2d %2d %2d ",
	   (int)LND_FC(lcp->l_fuelc, ourtlev),
	   (int)LND_FU(lcp->l_fuelu, ourtlev),
	   (int)LND_XPL(lcp->l_nxlight, ourtlev),
	   (int)LND_MXL(lcp->l_mxland, ourtlev));

	pr("\n");
    }
}

void
show_sect_build(int foo)
{
    register int x, first = 1;

    for (x = 5; x < SCT_MAXDEF + 2; x++) {
	if (dchr[x].d_mnem == 0)
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
    pr("\n");
    pr("Infrastructure building - adding 1 point of efficiency costs:\n");
    pr("       type          lcms    hcms    mobility    $$$$\n");
    for (x = 0; intrchr[x].in_name; x++) {
	pr("%-20s %4d    %4d    %8d    %4d\n", intrchr[x].in_name,
	   intrchr[x].in_lcms, intrchr[x].in_hcms,
	   intrchr[x].in_mcost, intrchr[x].in_dcost);
    }
}

void
show_sect_stats(int foo)
{
    register int x, first = 1;
    struct sctstr sect;
    struct natstr *natp;

    natp = getnatp(player->cnum);
    /* We fake this */
    sect.sct_effic = 100;
    for (x = 0; x < SCT_MAXDEF + 2; x++) {
	if (dchr[x].d_mnem == 0)
	    continue;
	if (first) {
	    pr("                        base     max   max   --  packing bonus  --   max\n");
	    pr("  sector type           mcost    off   def   mil  uw civ bar other   pop\n");
	    first = 0;
	}
	sect.sct_type = x;
	pr("%c %-23s %3d  %5.2f %5.2f   %3d %3d %3d %3d %5d %5d\n",
	   dchr[x].d_mnem, dchr[x].d_name,
	   dchr[x].d_mcst, dchr[x].d_ostr,
	   dchr[x].d_dstr,
	   ichr[I_MILIT].i_pkg[dchr[x].d_pkg],
	   ichr[I_UW].i_pkg[dchr[x].d_pkg],
	   ichr[I_CIVIL].i_pkg[dchr[x].d_pkg],
	   ichr[I_BAR].i_pkg[dchr[x].d_pkg],
	   ichr[I_LCM].i_pkg[dchr[x].d_pkg],
	   max_pop(natp->nat_level[NAT_RLEV], &sect));
    }
}

void
show_sect_capab(int foo)
{
    register int x, first = 1, i, j;
    char *tmpstr;
    char c;
    char *outputs = " cmsgpidbfolhur";

    for (x = 0; x < SCT_MAXDEF + 2; x++) {
	if ((dchr[x].d_mnem == 0) || (dchr[x].d_prd == 0))
	    continue;
	if (first) {
	    pr("                                                 --- level ---          reso \n");
	    pr("  sector type             product use1 use2 use3 level min lag eff%% $$$ dep c\n");
	    first = 0;
	}

	j = dchr[x].d_prd;

	pr("%c %-23s %-7s ", dchr[x].d_mnem, dchr[x].d_name,
	   pchr[j].p_sname);
	/*for(i=0;i<MAXCHRNV;i++) */
	/* XXX currently no more than 3 items actually used */
	for (i = 0; i < 3; i++) {
	    if ((i < pchr[j].p_nv) && (pchr[j].p_vamt[i] > 0)) {
		pr("%2d %c ", pchr[j].p_vamt[i],
		   ichr[pchr[j].p_vtype[i] & (~VT_ITEM)].i_name[0]);
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
	if (pchr[j].p_type)
	    c = outputs[pchr[j].p_type - VT_ITEM];
	else
	    c = ' ';
	pr("%-5s %3d %3d %4d %3d %3d %c",
	   tmpstr,
	   pchr[j].p_nlmin,
	   pchr[j].p_nllag,
	   pchr[j].p_effic, pchr[j].p_cost, pchr[j].p_nrdep, c);

	pr("\n");
    }
}
