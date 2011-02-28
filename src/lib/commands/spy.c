/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  spy.c: Spy on your neighbors
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2005-2011
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "land.h"
#include "map.h"
#include "news.h"
#include "optlist.h"
#include "path.h"
#include "plane.h"

/*
 * format:  spy <SECTS>
 */

static void spy_report(struct sctstr *sp);
static void prplanes(int, int);
static void prunits(int, int);
static char *player_relstr(natid);

int
spy(void)
{
    natid own;
    int relat;
    coord x, y;
    coord nx, ny;
    int military;
    int btucost;
    int i;
    unsigned char *bitmap;
    int nrecon;
    struct nstr_sect nstr;
    struct nstr_item ni;
    struct natstr *natp;
    struct sctstr from;
    struct sctstr dsect;
    struct lndstr land;
    int changed = 0;
    int nsects;

    /*
     * first arg should be the range of sectors
     */
    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    nsects = (nstr.range.width + 1) * nstr.range.height / 2;
    btucost = (nsects / 40) + 1;
    natp = getnatp(player->cnum);
    if (natp->nat_btu < btucost) {
	pr("You don't have the BTU's for spying on that scale!\n");
	return RET_FAIL;
    }
    bitmap = calloc((WORLD_SZ() + 7) / 8, 1);
    if (!bitmap) {
	logerror("malloc failed in do_look\n");
	pr("Memory error.  Tell the deity.\n");
	return RET_FAIL;
    }

    pr("SPY report\n");
    prdate();
    pr("                 old sct rd  rl  def\n");
    pr("   sect   de own own eff eff eff eff  civ  mil  shl gun  pet food bars\n");
    while (nxtsct(&nstr, &from)) {
	if (!player->owner && !player->god)
	    continue;
	military = from.sct_item[I_MILIT];
	if (military == 0)
	    continue;
	x = from.sct_x;
	y = from.sct_y;
	nrecon = 0;
	snxtitem_xy(&ni, EF_LAND, x, y);
	while (nxtitem(&ni, &land)) {
	    if (lchr[(int)land.lnd_type].l_flags & L_RECON)
		nrecon++;
	}
	/* Print out the units/planes in this sector */
	prunits(x, y);
	prplanes(x, y);
	/*
	 * check the neighboring sectors.
	 */
	for (i = 1; i <= 6; i++) {
	    if (military == 0)
		break;
	    nx = x + diroff[i][0];
	    ny = y + diroff[i][1];
	    if (emp_getbit(nx, ny, bitmap))
		continue;	/* spied already */
	    getsect(nx, ny, &dsect);
	    if (player->owner || dsect.sct_type == SCT_WATER)
		continue;

	    own = dsect.sct_own;
	    relat = relations_with(own, player->cnum);
	    if (relat <= NEUTRAL
		&& chance(dsect.sct_item[I_MILIT] / 200.0)) {
		/* spy caught */
		if (relat == NEUTRAL) {
		    /* deport spy */
		    pr("Spy deported from %s\n",
		       xyas(nx, ny, player->cnum));
		    if (own != 0)
			wu(0, own, "%s (#%d) spy deported from %s\n",
			   cname(player->cnum), player->cnum,
			   xyas(nx, ny, own));
		} else {
		    /* execute spy */
		    pr("BANG!! A spy was shot in %s\n",
		       xyas(nx, ny, player->cnum));
		    military--;
		    if (own != 0)
			wu(0, own, "%s (#%d) spy caught in %s\n",
			   cname(player->cnum), player->cnum,
			   xyas(nx, ny, own));
		    nreport(player->cnum, N_SPY_SHOT, own, 1);
		}
		if (opt_HIDDEN)
		    setcont(own, player->cnum, FOUND_SPY);
		if (!nrecon)	/* unless you have a recon unit */
		    continue;	/* no report from caught spy */
	    }

	    /* spy report */
	    emp_setbit(nx, ny, bitmap);
	    spy_report(&dsect);
	    changed += map_set(player->cnum, dsect.sct_x, dsect.sct_y,
			       dchr[dsect.sct_type].d_mnem, 0);
	    if (opt_HIDDEN)
		setcont(player->cnum, own, FOUND_SPY);
	}
	/* subtract any military if necessary */
	if (from.sct_item[I_MILIT] != military) {
	    from.sct_item[I_MILIT] = military;
	    putsect(&from);
	}
    }
    if (changed)
	writemap(player->cnum);
    player->btused += btucost;
    free(bitmap);
    return RET_OK;
}

static void
spy_report(struct sctstr *sp)
{
    prxy("%4d,%-4d", sp->sct_x, sp->sct_y);
    pr(" %c%c %3d %3d %3d %3d %3d %3d %4d %4d %4d %3d %4d %4d %4d\n",
       dchr[sp->sct_type].d_mnem,
       sp->sct_newtype == sp->sct_type ? ' ' : dchr[sp->sct_newtype].d_mnem,
       sp->sct_own,
       sp->sct_oldown,
       roundintby((int)sp->sct_effic, 10),
       roundintby((int)sp->sct_road, 10),
       opt_RAILWAYS ? !!sct_rail_track(sp) : roundintby(sp->sct_rail, 10),
       roundintby((int)sp->sct_defense, 10),
       roundintby(sp->sct_item[I_CIVIL], 10),
       roundintby(sp->sct_item[I_MILIT], 10),
       roundintby(sp->sct_item[I_SHELL], 10),
       roundintby(sp->sct_item[I_GUN], 10),
       roundintby(sp->sct_item[I_PETROL], 10),
       roundintby(sp->sct_item[I_FOOD], 10),
       roundintby(sp->sct_item[I_BAR], 10));
    prunits(sp->sct_x, sp->sct_y);
    prplanes(sp->sct_x, sp->sct_y);
}

static void
prunits(int x, int y)
{
    struct lndstr land;
    struct nstr_item ni;
    char report[128];

    snxtitem_xy(&ni, EF_LAND, x, y);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == player->cnum || land.lnd_own == 0)
	    continue;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    continue;
	/* Don't always see spies */
	if (lchr[(int)land.lnd_type].l_flags & L_SPY) {
	    if (!(chance(LND_SPY_DETECT_CHANCE(land.lnd_effic))))
		continue;
	}
	sprintf(report, "%s (%s) unit in %s: ",
		player_relstr(land.lnd_own),
		cname(land.lnd_own),
		xyas(land.lnd_x, land.lnd_y, player->cnum));
	intelligence_report(player->cnum, &land, 3, report);
    }
}

static void
prplanes(int x, int y)
{
    struct plnstr plane;
    struct nstr_item ni;

    snxtitem_xy(&ni, EF_PLANE, x, y);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own == player->cnum || plane.pln_own == 0)
	    continue;
	if (plane.pln_ship >= 0 || plane.pln_land >= 0)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	pr("%s (%s) plane in %s: %s\n",
	   player_relstr(plane.pln_own),
	   cname(plane.pln_own),
	   xyas(plane.pln_x, plane.pln_y, player->cnum),
	   prplane(&plane));
    }
}

static char *
player_relstr(natid them)
{
    int rel = relations_with(player->cnum, them);

    if (rel == ALLIED)
	return "Allied";
    if (rel >= NEUTRAL)
	return "Neutral";
    return "Enemy";
}
