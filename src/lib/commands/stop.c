/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  stop.c: Stop a sector or unit from producing
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Steve McClure, 1998
 *     Markus Armbruster, 2006-2013
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "empobj.h"

static int start_stop(int);
static int start_stop_sector(char *, int);
static void start_stop_hdr(int);
static void proff(int);
static int start_stop_unit(int, char *, int);
static void start_stop_unit_hdr(int);

int
start(void)
{
    return start_stop(0);
}

int
stop(void)
{
    return start_stop(1);
}

static int
start_stop(int off)
{
    static int sct_or_unit[] = {
	EF_SECTOR, EF_SHIP, EF_PLANE, EF_LAND, EF_NUKE, EF_BAD
    };
    int type;
    char *arg, *p;
    char buf[1024];

    if (player->argp[1] && !isalpha(*player->argp[1])) {
	/* accept legacy syntax */
	type = EF_SECTOR;
	arg = player->argp[1];
    } else {
	p = getstarg(player->argp[1],
		     "Sector, ship, plane, land unit or nuke? ", buf);
	if (!p)
	    return RET_SYN;
	type = ef_byname_from(p, sct_or_unit);
	if (type < 0) {
	    pr("Sectors, ships, planes, land units or nukes only!\n");
	    return RET_SYN;
	}
	arg = player->argp[2];
    }

    if (type == EF_SECTOR)
	return start_stop_sector(arg, off);
    return start_stop_unit(type, arg, off);
}

static int
start_stop_sector(char *arg, int off)
{
    struct sctstr sect;
    int nsect;
    struct nstr_sect nstr;

    if (!snxtsct(&nstr, arg))
	return RET_SYN;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (!sect.sct_off == !off)
	    continue;
	if (nsect++ == 0)
	    start_stop_hdr(off);
	if (player->god)
	    pr("%3d ", sect.sct_own);
	prxy("%4d,%-4d", nstr.x, nstr.y);
	pr(" %c", dchr[sect.sct_type].d_mnem);
	if (sect.sct_newtype != sect.sct_type)
	    pr("%c", dchr[sect.sct_newtype].d_mnem);
	else
	    pr(" ");
	pr("%4d%%", sect.sct_effic);
	proff(off);
	sect.sct_off = off;
	putsect(&sect);
    }
    if (nsect == 0) {
	pr("%s: No sector(s)\n", arg ? arg : "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return 0;
}

static void
start_stop_hdr(int off)
{
    if (player->god)
	pr("    ");
    pr("PRODUCTION %s\n", off ? "STOPPAGE" : "STARTING");
    if (player->god)
	pr("own ");
    pr("  sect        eff\n");
}

static void
proff(int off)
{
    if (off)
	pr("  will not produce or gain efficiency.\n");
    else
	pr("  will be updated normally.\n");
}

static int
start_stop_unit(int type, char *arg, int off)
{
    union empobj_storage unit;
    int nunit;
    struct nstr_item nstr;

    if (!snxtitem(&nstr, type, arg, NULL))
	return RET_SYN;
    prdate();
    nunit = 0;
    while (nxtitem(&nstr, &unit)) {
	if (!player->owner || !unit.gen.own)
	    continue;
	if (!unit.gen.off == !off)
	    continue;
	if (nunit++ == 0)
	    start_stop_unit_hdr(off);
	if (player->god)
	    pr("%3d ", unit.gen.own);
	pr("%4d %-4.4s ", nstr.cur, empobj_chr_name(&unit.gen));
	prxy("%4d,%-4d", unit.gen.x, unit.gen.y);
	pr("%4d%%", unit.gen.effic);
	proff(off);
	unit.gen.off = off;
	ef_write(type, nstr.cur, &unit);
    }
    if (nunit == 0) {
	pr("%s: No %s(s)\n", arg ? arg : "", ef_nameof(type));
	return RET_FAIL;
    } else
	pr("%d %s%s\n", nunit, ef_nameof(type), splur(nunit));
    return 0;
}

static void
start_stop_unit_hdr(int off)
{
    if (player->god)
	pr("    ");
    pr("PRODUCTION %s\n", off ? "STOPPAGE" : "STARTING");
    if (player->god)
	pr("own ");
    pr("   #         x,y     eff\n");
}
