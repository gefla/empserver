/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  show.c: General show routines
 *
 *  Known contributors to this file:
 *     Julian Onions, 1988
 *     Jeff Bailey, 1990
 *     Steve McClure, 1996
 *     Ron Koenderink, 2005-2009
 *     Markus Armbruster, 2006-2011
 */

#include <config.h>

#include <math.h>
#include "file.h"
#include "game.h"
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
#include "server.h"
#include "ship.h"

static char *fmttime2822(time_t);
static void show_load(short[]);
static void show_capab(int, struct symbol *);

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
    pr("Bridges require %g tech, %d hcm, 0 workers,\n",
       buil_bt, buil_bh);
    pr("%d available workforce, and cost $%g\n",
       (SCT_BLD_WORK(0, buil_bh) * SCT_MINEFF + 99) / 100,
       buil_bc);
}

void
show_tower(int tlev)
{
    if (tlev < buil_tower_bt)
	return;
    pr("Bridge towers require %g tech, %d hcm, 0 workers,\n",
       buil_tower_bt, buil_tower_bh);
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
		ceil(np->n_tech * drnuke_const) : 0.0,
	   np->n_cost);
    }
}

void
show_nuke_capab(int tlev)
{
    struct nchrstr *np;

    pr("%13s blst dam lbs tech res $%7s abilities\n", "", "");

    for (np = nchr; np->n_name; np++) {
	if (np->n_tech > tlev)
	    continue;
	pr("%-13.13s %4d %3d %3d %4d %3.0f $%7d",
	   np->n_name, np->n_blast, np->n_dam,
	   np->n_weight, np->n_tech,
	   drnuke_const > MIN_DRNUKE_CONST ?
		ceil(np->n_tech * drnuke_const) : 0.0,
	   np->n_cost);
	show_capab(np->n_flags, nuke_chr_flags);
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

    pr("%25s      s  v  s  r  f  l  p  h  x", "");
    pr("\n");

    pr("%25s      p  i  p  n  i  n  l  e  p", "");
    pr("\n");

    pr("%25s def  d  s  y  g  r  d  n  l  l", "");
    pr("\n");


    make_new_list(tlev, EF_SHIP);
    for (scount = 0; scount < lookup_list_cnt; scount++) {
	mp = (struct mchrstr *)lookup_list[scount].l_u.mp;
	/* Can't show trade ships unless it's turned on */
	if ((mp->m_flags & M_TRADE) && !opt_TRADESHIPS)
	    continue;

	pr("%-25.25s %3d %2d %2d %2d %2d %2d ",
	   mp->m_name, m_armor(mp, tlev), m_speed(mp, tlev),
	   m_visib(mp, tlev), mp->m_vrnge,
	   m_frnge(mp, tlev), m_glim(mp, tlev));

	pr("%2d ", mp->m_nland);
	pr("%2d ", mp->m_nplanes);
	pr("%2d ", mp->m_nchoppers);
	pr("%2d ", mp->m_nxlight);
	pr("\n");
    }
}

void
show_ship_capab(int tlev)
{
    struct mchrstr *mp;
    int scount;

    pr("%25s cargos & capabilities\n", "");

    make_new_list(tlev, EF_SHIP);
    for (scount = 0; scount < lookup_list_cnt; scount++) {
	mp = (struct mchrstr *)lookup_list[scount].l_u.mp;
	/* Can't show trade ships unless it's turned on */
	if ((mp->m_flags & M_TRADE) && !opt_TRADESHIPS)
	    continue;

	pr("%-25.25s ", mp->m_name);
	show_load(mp->m_item);
	show_capab(mp->m_flags, ship_chr_flags);
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
	   pp->pl_name, pl_acc(pp, tlev), pl_load(pp, tlev),
	   pl_att(pp, tlev), pl_def(pp, tlev), pl_range(pp, tlev),
	   pp->pl_fuel);
	pr("%4d%% ", pp->pl_stealth);
	pr("\n");
    }
}

void
show_plane_capab(int tlev)
{
    struct plchrstr *pp;
    int pcount;

    pr("%25s capabilities\n", "");
    make_new_list(tlev, EF_PLANE);
    for (pcount = 0; pcount < lookup_list_cnt; pcount++) {
	pp = (struct plchrstr *)lookup_list[pcount].l_u.pp;
	pr("%-25.25s ", pp->pl_name);

	show_capab(pp->pl_flags, plane_chr_flags);
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

    pr("%25s capabilities\n", "");

    make_new_list(tlev, EF_LAND);
    for (lcount = 0; lcount < lookup_list_cnt; lcount++) {
	lcp = (struct lchrstr *)lookup_list[lcount].l_u.lp;
	if ((lcp->l_flags & L_SPY) && !opt_LANDSPIES)
	    continue;

	pr("%-25s ", lcp->l_name);
	show_load(lcp->l_item);
	show_capab(lcp->l_flags, land_chr_flags);
	pr("\n");
    }
}

void
show_land_stats(int tlev)
{
    struct lchrstr *lcp;
    int lcount;

    pr("%25s              s  v  s  r  r  a  f  a  a  x  l\n", "");
    pr("%25s              p  i  p  a  n  c  i  m  a  p  n\n", "");
    pr("%25s att def vul  d  s  y  d  g  c  r  m  f  l  d\n", "");

    make_new_list(tlev, EF_LAND);
    for (lcount = 0; lcount < lookup_list_cnt; lcount++) {
	lcp = (struct lchrstr *)lookup_list[lcount].l_u.lp;
	if ((lcp->l_flags & L_SPY) && !opt_LANDSPIES)
	    continue;

	pr("%-25s %1.1f %1.1f %3d ",
	   lcp->l_name,
	   l_att(lcp, tlev), l_def(lcp, tlev), l_vul(lcp, tlev));
	pr("%2d %2d %2d %2d ",
	   l_spd(lcp, tlev), lcp->l_vis, lcp->l_spy, lcp->l_rad);
	pr("%2d %2d %2d %2d %2d ",
	   l_frg(lcp, tlev), l_acc(lcp, tlev), l_dam(lcp, tlev),
	   lcp->l_ammo, lcp->l_aaf);
	pr("\n");
    }
}

void
show_sect_build(int foo)
{
    int i, first;

    pr("                        desig   build 100%% eff  maint\n"
       "sector type                 $   lcm  hcm     $      $\n");
    for (i = 0; dchr[i].d_name; i++) {
	if (dchr[i].d_mnem == 0)
	    continue;
	if (dchr[i].d_mob0 < 0)
	    continue;
	if (dchr[i].d_cost <= 0 && dchr[i].d_build == 1
	    && dchr[i].d_lcms == 0 && dchr[i].d_hcms == 0
	    && dchr[i].d_maint == 0)
	    continue;		/* the usual, skip */
	pr("%c %-21.21s", dchr[i].d_mnem, dchr[i].d_name);
	if (dchr[i].d_cost < 0)
	    pr(" can't");
	else
	    pr(" %5d", dchr[i].d_cost);
	pr(" %5d%5d %5d  %5d\n",
	   100 * dchr[i].d_lcms,
	   100 * dchr[i].d_hcms,
	   100 * dchr[i].d_build,
	   dchr[i].d_maint * etu_per_update);
    }
    pr("any other                   0     0    0   100      0\n");

    first = 1;
    for (i = 0; intrchr[i].in_name; i++) {
	if (!intrchr[i].in_enable)
	    continue;
	if (first)
	    pr("\nInfrastructure building - adding 1 point of efficiency costs:\n"
	       "       type          lcms    hcms    mobility    $$$$\n");
	pr("%-20s %4d    %4d    %8d    %4d\n",
	   intrchr[i].in_name, intrchr[i].in_lcms, intrchr[i].in_hcms,
	   intrchr[i].in_mcost, intrchr[i].in_dcost);
	first = 0;
    }
}

void
show_sect_stats(int foo)
{
    int i;
    struct natstr *natp = getnatp(player->cnum);

    pr("                        mob cost   max   max   naviga    packing   max\n");
    pr("  sector type            0%% 100%%   off   def   bility      bonus   pop\n");

    for (i = 0; dchr[i].d_name; i++) {
	if (dchr[i].d_mnem == 0)
	    continue;
	pr("%c %-21.21s", dchr[i].d_mnem, dchr[i].d_name);
	if (dchr[i].d_mob0 < 0)
	    pr("  no way ");
	else
	    pr(" %3.1f  %3.1f", dchr[i].d_mob0, dchr[i].d_mob1);
	pr("  %5.2f %5.2f %7.7s %10.10s %5d\n",
	   dchr[i].d_ostr, dchr[i].d_dstr,
	   symbol_by_value(dchr[i].d_nav, sector_navigation),
	   symbol_by_value(dchr[i].d_pkg, packing),
	   max_population(natp->nat_level[NAT_RLEV], i, 100));
    }
}

void
show_sect_capab(int foo)
{
    int i;

    pr("  sector type             product  p.e.\n");

    for (i = 0; dchr[i].d_name; i++) {
	if (dchr[i].d_mnem == 0 || dchr[i].d_prd < 0)
	    continue;
	pr("%c %-23s %-7s %4d%%\n",
	   dchr[i].d_mnem, dchr[i].d_name, pchr[dchr[i].d_prd].p_sname,
	   dchr[i].d_peffic);
    }
}

void
show_item(int tlev)
{
    struct ichrstr *ip;

    pr("item value sell lbs    packing     melt  item\n");
    pr("mnem                in no wh ur bk deno  name\n");

    for (ip = ichr; ip->i_name; ip++) {
	pr("   %c %5d %4s %3d %2d %2d %2d %2d %2d %4d  %s\n",
	   ip->i_mnem, ip->i_value, ip->i_sell ? "yes" : "no", ip->i_lbs,
	   ip->i_pkg[IPKG], ip->i_pkg[NPKG], ip->i_pkg[WPKG],
	   ip->i_pkg[UPKG], ip->i_pkg[BPKG],
	   ip->i_melt_denom, ip->i_name);
    }
}

void
show_product(int tlev)
{
    struct pchrstr *pp;
    int i;
    char *lev;

    pr("product    cost  raw materials  reso dep  level p.e.\n");

    for (pp = pchr; pp->p_sname; pp++) {
	pr("%7.7s %c  $%-3d ",
	   pp->p_sname,
	   pp->p_type < 0 ? ' ' : ichr[pp->p_type].i_mnem,
	   pp->p_cost);
	(void)CANT_HAPPEN(MAXPRCON > 3); /* output has only three columns */
	for (i = 0; i < 3; i++) {
	    if (i < MAXPRCON && pp->p_camt[i]
		&& pp->p_ctype[i] > I_NONE && pp->p_ctype[i] <= I_MAX)
		pr(" %2d%c", pp->p_camt[i], ichr[pp->p_ctype[i]].i_mnem);
	    else
		pr("    ");
	}
	if (pp->p_nrndx)
	    pr("   %5.5s %3d  ",
	       symbol_by_value(pp->p_nrndx, resources), pp->p_nrdep);
	else
	    pr("              ");
	if (pp->p_nlndx < 0)
	    pr("1.0\n");
	else {
	    lev = symbol_by_value(pp->p_nlndx, level);
	    pr("(%.4s%+d)/(%.4s%+d)\n",
	       lev, -pp->p_nlmin, lev, pp->p_nllag - pp->p_nlmin);
	}
    }
}

void
show_news(int tlev)
{
    int i, j;

    pr("id category           good will\n");
    pr("    messsages\n");

    for (i = 1; i < N_MAX_VERB + 1; i++) {
	if (rpt[i].r_newspage == N_NOTUSED)
	    continue;
	pr("%-2d %-20.20s %4d\n", rpt[i].r_uid,
	   page_headings[rpt[i].r_newspage].name, rpt[i].r_good_will);
	for (j = 0; j < NUM_RPTS; j++)
	    pr("    %s\n", rpt[i].r_newstory[j]);
    }
}

/*
 * Show update policy and up to N scheduled updates.
 */
void
show_updates(int n)
{
    struct gamestr *game = game_tick_tick();
    int demand = 0;
    int i;

    pr("%s, Turn %d, ETU %d\n", fmttime2822(time(NULL)),
       game->game_turn, game->game_tick);

    if (update_time[0]) {
	if (update_demand == UPD_DEMAND_SCHED) {
	    pr("Demand updates occur according to schedule:\n");
	    demand = 1;
	} else
	    pr("Updates occur according to schedule:\n");
	for (i = 0; i < n && update_time[i]; i++)
	    pr("%3d.  %s\n", game->game_turn + i,
	       fmttime2822(update_time[i]));
	if (update_window) {
	    pr("Updates occur within %d seconds after the scheduled time\n",
	       update_window);
	}
    } else
	pr("There are no updates scheduled.\n");

    if (update_demand == UPD_DEMAND_ASYNC) {
	pr("Demand updates occur right after the demand is set.\n");
	if (*update_demandtimes != 0) {
	    pr("Demand updates are allowed during: %s\n",
	       update_demandtimes);
	}
	demand = 1;
    }

    if (demand) {
	pr("Demand updates require %d country(s) to want one.\n",
	   update_wantmin);
    }

    if (updates_disabled())
	pr("\nUPDATES ARE DISABLED!\n");
}

/*
 * Return T formatted according to RFC 2822.
 * The return value is statically allocated and overwritten on
 * subsequent calls.
 */
static char *
fmttime2822(time_t t)
{
    static char buf[32];
#if defined(_WIN32)
    size_t n;
    int nn;
    TIME_ZONE_INFORMATION tzi;
    long time_offset;
    struct tm *time;

    time = localtime(&t);

    n = strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S", time);
    if (CANT_HAPPEN(n == 0)) {
	buf[0] = 0;
	return buf;
    }
    GetTimeZoneInformation(&tzi);
    time_offset = -(tzi.Bias +
	(time->tm_isdst ? tzi.DaylightBias : tzi.StandardBias));

    nn = _snprintf(buf + n, sizeof(buf) - n, " %+03d%02d",
		   time_offset / 60, abs(time_offset) % 60);
    if (CANT_HAPPEN(nn <= 0 || nn + n >= sizeof(buf)))
	buf[0] = 0;
#else
    size_t n = strftime(buf, sizeof(buf), "%a, %d %b %Y %T %z",
			localtime(&t));
    if (CANT_HAPPEN(n == 0))
	buf[0] = 0;
#endif
    return buf;
}

static void
show_load(short item[])
{
    i_type i;

    for (i = I_NONE + 1; i <= I_MAX; ++i) {
	if (item[i])
	    pr(" %d%c", item[i], ichr[i].i_mnem);
    }
}

static void
show_capab(int flags, struct symbol *table)
{
    int i;
    char *p;

    for (i = 0; i < 32; i++) {
	if (!(flags & bit(i)))
	    continue;
	p = symbol_by_value(bit(i), table);
	if (p)
	    pr(" %s", p);
    }
}
