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
 *  spy.c: Spy on your neighbors
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998-2000
 */

#include "misc.h"
#include "player.h"
#include "sect.h"
#include "item.h"
#include "nat.h"
#include "xy.h"
#include "nsc.h"
#include "news.h"
#include "path.h"
#include "file.h"
#include "land.h"
#include "commands.h"
#include "optlist.h"

/*
 * format:  spy <SECTS>
 *
 */

static int check(coord *table, int *len, coord x, coord y);
static void insert(coord *table, int *len, coord x, coord y);
static int num_units(int, int);
static void prplanes(int, int);
static void prunits(int, int);
static void spyline(struct sctstr *sp);

int
spy(void)
{
    int caught;
    natid own;
    int relat;
    coord x, y;
    coord nx, ny;
    int military;
    int savemil;
    int btucost;
    int i;
    coord *table;		/* sectors already seen */
    int t_len = 0;
    int nrecon;
    int nunits;
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
    /*
     * set up all the goodies we need later
     * 6 = neighbors, 2 = x,y
     */
    table = (coord *)malloc((nsects + 1) * 6 * 2 * sizeof(coord));
    memset(table, 0, (nsects + 1) * 6 * 2 * sizeof(coord));
    pr("SPY report\n");
    prdate();
    pr("                 old sct rd  rl  def\n");
    pr("   sect   de own own eff eff eff eff  civ  mil  shl gun  pet food bars lnd pln\n");
    while (nxtsct(&nstr, &from)) {
	if (!player->owner && !player->god)
	    continue;
	nrecon = 0;
	nunits = 0;
	snxtitem_xy(&ni, EF_LAND, from.sct_x, from.sct_y);
	while (nxtitem(&ni, (s_char *)&land)) {
	    nunits++;
	    if (lchr[(int)land.lnd_type].l_flags & L_RECON)
		nrecon++;
	}
	if ((military = from.sct_item[I_MILIT]) == 0 && (nunits == 0))
	    continue;
	x = from.sct_x;
	y = from.sct_y;
	/* Print out the units/planes in this sector */
	prunits(x, y);
	prplanes(x, y);
	savemil = military;
	/*
	 * check the neighboring sectors.
	 */
	for (i = 1; i <= 6; i++) {
	    if ((military == 0) && (nunits == 0))
		break;
	    nx = x + diroff[i][0];
	    ny = y + diroff[i][1];
	    /*
	     * if we've already seen the
	     * sector, don't bother checking it
	     * out.
	     */
	    if (check(table, &t_len, nx, ny)) {
		continue;
	    }
	    getsect(nx, ny, &dsect);
	    if (player->owner
		|| (dsect.sct_type == SCT_WATER)
		|| (!dsect.sct_item[I_MILIT] && !dsect.sct_item[I_CIVIL]
		    && num_units(nx, ny) == 0)) {
		/* mark sector as seen */
		insert(table, &t_len, nx, ny);
		continue;
	    }
	    /* catch spy N/200 chance, N = # military */
	    caught = chance((double)dsect.sct_item[I_MILIT] / 200.0);
	    own = dsect.sct_own;
	    /* determine spyee relations with spyer */
	    relat = getrel(getnatp(own), player->cnum);
	    if (relat == NEUTRAL && caught) {
		/* neutral spy-ee */
		pr("Spy deported from %s\n", xyas(nx, ny, player->cnum));
		if (own != 0)
		    wu(0, own, "%s (#%d) spy deported from %s\n",
		       cname(player->cnum), player->cnum,
		       xyas(nx, ny, own));
	    } else if (relat < NEUTRAL && caught) {
		/* at-war with spy-ee */
		pr("BANG!! A spy was shot in %s\n",
		   xyas(nx, ny, player->cnum));
		military--;
		if (own != 0)
		    wu(0, own, "%s (#%d) spy caught in %s\n",
		       cname(player->cnum), player->cnum,
		       xyas(nx, ny, own));
		nreport(player->cnum, N_SPY_SHOT, own, 1);
	    } else {
		insert(table, &t_len, nx, ny);
		spyline(&dsect);
		changed += map_set(player->cnum, dsect.sct_x,
				   dsect.sct_y,
				   dchr[dsect.sct_type].d_mnem, 0);
		prunits(dsect.sct_x, dsect.sct_y);
		prplanes(dsect.sct_x, dsect.sct_y);
		if (opt_HIDDEN) {
		    setcont(player->cnum, own, FOUND_SPY);
		}
	    }
	    /*
	     * If you have a recon unit, it'll
	     * see the sector anyway...
	     */
	    if (nrecon && caught) {
		insert(table, &t_len, nx, ny);
		spyline(&dsect);
		changed += map_set(player->cnum, dsect.sct_x,
				   dsect.sct_y,
				   dchr[dsect.sct_type].d_mnem, 0);
		prunits(dsect.sct_x, dsect.sct_y);
		prplanes(dsect.sct_x, dsect.sct_y);
	    }
	}
	/* subtract any military if necessary */
	if ((savemil != military) && (savemil > 0)) {
	    if ((military < 0) || (military > savemil))
		military = 0;
	    from.sct_item[I_MILIT] = military;
	    putsect(&from);
	}
    }
    if (changed)
	writemap(player->cnum);
    player->btused += btucost;
    free((s_char *)table);
    return RET_OK;
}


/*
 * just a big printf.
 */
static void
spyline(struct sctstr *sp)
{
    prxy("%4d,%-4d", sp->sct_x, sp->sct_y, player->cnum);
    pr(" %c%c %3d %3d %3d %3d %3d %3d %4d %4d %4d %3d %4d %4d %4d %3d %3d\n",
       dchr[sp->sct_type].d_mnem,
       (sp->sct_newtype == sp->sct_type) ? ' ' : dchr[sp->sct_newtype].d_mnem,
       sp->sct_own,
       sp->sct_oldown,
       roundintby((int)sp->sct_effic, 10),
       roundintby((int)sp->sct_road, 10),
       roundintby((int)sp->sct_rail, 10),
       roundintby((int)sp->sct_defense, 10),
       roundintby(sp->sct_item[I_CIVIL], 10),
       roundintby(sp->sct_item[I_MILIT], 10),
       roundintby(sp->sct_item[I_SHELL], 10),
       roundintby(sp->sct_item[I_GUN], 10),
       roundintby(sp->sct_item[I_PETROL], 10),
       roundintby(sp->sct_item[I_FOOD], 10),
       roundintby(sp->sct_item[I_BAR], 10),
       count_sect_units(sp),
       count_sect_planes(sp));
}


/*
 * insert a key into the table.
 */
static void
insert(coord *table, int *len, coord x, coord y)
{
    if (!check(table, len, x, y)) {
	table[(*len)++] = x;
	table[(*len)++] = y;
    }
}

/*
 * see if a key is in the bitmask table
 */
static int
check(coord *table, int *len, coord x, coord y)
{
    int i;

    for (i = 0; i < *len; i += 2)
	if (table[i] == x && table[i + 1] == y)
	    return 1;
    return 0;
}

static int
num_units(int x, int y)
{
    struct lndstr land;
    struct nstr_item ni;
    int n = 0;

    snxtitem_xy(&ni, EF_LAND, x, y);
    while (nxtitem(&ni, (s_char *)&land)) {
	if ((land.lnd_own == player->cnum) || (land.lnd_own == 0))
	    continue;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    continue;
	n++;
    }

    return n;
}

static void
prunits(int x, int y)
{
    struct lndstr land;
    struct nstr_item ni;
    s_char report[128];

    snxtitem_xy(&ni, EF_LAND, x, y);
    while (nxtitem(&ni, (s_char *)&land)) {
	if (land.lnd_own == player->cnum || land.lnd_own == 0)
	    continue;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    continue;
	/* Don't always see spies */
	if (lchr[(int)land.lnd_type].l_flags & L_SPY) {
	    if (!(chance(LND_SPY_DETECT_CHANCE(land.lnd_effic))))
		continue;
	}
	if ((land.lnd_own != player->cnum) && land.lnd_own) {
	    int rel;
	    s_char *format;

	    rel = getrel(getnatp(player->cnum), land.lnd_own);
	    if (rel == ALLIED)
		format = "Allied (%s) unit in %s: ";
	    else if (rel == FRIENDLY || rel == NEUTRAL)
		format = "Neutral (%s) unit in %s: ";
	    else
		format = "Enemy (%s) unit in %s: ";
	    sprintf(report, format, cname(land.lnd_own),
		    xyas(land.lnd_x, land.lnd_y, player->cnum));
	    intelligence_report(player->cnum, &land, 3, report);
	}
    }
}

static void
prplanes(int x, int y)
{
    struct plnstr plane;
    struct nstr_item ni;
    s_char report[128];

    snxtitem_xy(&ni, EF_PLANE, x, y);
    while (nxtitem(&ni, (s_char *)&plane)) {
	if (plane.pln_own == player->cnum || plane.pln_own == 0)
	    continue;
	if (plane.pln_ship >= 0 || plane.pln_land >= 0)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	if ((plane.pln_own != player->cnum) && plane.pln_own) {
	    int rel;
	    s_char *format;

	    rel = getrel(getnatp(player->cnum), plane.pln_own);
	    if (rel == ALLIED)
		format = "Allied (%s) plane in %s: %s\n";
	    else if (rel == FRIENDLY || rel == NEUTRAL)
		format = "Neutral (%s) plane in %s: %s\n";
	    else
		format = "Enemy (%s) plane in %s: %s\n";
	    sprintf(report, format, cname(plane.pln_own),
		    xyas(plane.pln_x, plane.pln_y, player->cnum),
		    prplane(&plane));
	    pr(report);
	}
    }
}
