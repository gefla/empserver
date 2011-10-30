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
 *  new.c: Create a new capital for a player
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "commands.h"
#include "land.h"
#include "optlist.h"
#include "path.h"
#include "prototypes.h"

static void init_sanct(struct natstr *, coord, coord);

int
new(void)
{
    struct sctstr sect;
    struct natstr *natp;
    natid num;
    coord x, y;
    char *p;
    char buf[1024];

    natp = getnatp(player->cnum);
    if (natp->nat_xorg != 0 || natp->nat_yorg != 0) {
	pr("Origin must be reset to add a new country.\n");
	pr("Use `origin ~' to reset it.\n");
	return RET_FAIL;
    }
    if (!(natp = natargp(player->argp[1], "Country? ")))
	return RET_SYN;
    num = natp->nat_cnum;
    if (natp->nat_stat != STAT_NEW) {
	pr("Country #%d (%s) isn't a new country!\n", num, cname(num));
	return RET_SYN;
    }
    if (!(p = getstarg(player->argp[2], "sanctuary pair : ", buf)))
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

    pr("added country %d at %s\n", num, xyas(x, y, player->cnum));
    nat_reset(natp, STAT_SANCT, x, y);
    init_sanct(natp, x, y);
    init_sanct(natp, x + 2, y);
    putnat(natp);
    return RET_OK;
}

static void
init_sanct(struct natstr *natp, coord x, coord y)
{
    struct sctstr sect;

    getsect(x, y, &sect);
    sect.sct_own = natp->nat_cnum;
    sect.sct_type = SCT_SANCT;
    sect.sct_newtype = SCT_SANCT;
    sect.sct_effic = 100;
    sect.sct_road = 0;
    sect.sct_rail = 0;
    sect.sct_defense = 0;
    sect.sct_mobil = startmob;
    sect.sct_work = 100;
    sect.sct_oldown = natp->nat_cnum;
    if (at_least_one_100) {
	sect.sct_oil = 100;
	sect.sct_fertil = 100;
	sect.sct_uran = 100;
	sect.sct_min = 100;
	sect.sct_gmin = 100;
    }
    sect.sct_item[I_CIVIL] = max_pop(start_research, &sect);
    sect.sct_item[I_MILIT] = 55;
    sect.sct_item[I_FOOD] = opt_NOFOOD ? 0 : 550;
    sect.sct_item[I_UW] = 75;
    putsect(&sect);
}
