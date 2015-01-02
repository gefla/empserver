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
 *  surv.c: Show sector survey
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Markus Armbruster, 2004-2014
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "map.h"
#include "optlist.h"

static char code_char(struct valstr, struct sctstr *sp);

/*
 * survey type <sarg> ?cond
 *
 */
int
surv(void)
{
    int nsect;
    struct nstr_sect nstr;
    int y;
    struct valstr val;
    struct natstr *np;
    struct sctstr sect;
    struct range range;
    char *ptr;
    struct nscstr cond[NS_NCOND];
    int ncond;
    int i;
    char buf[1024];
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static char *mapbuf = NULL;
    static char **map = NULL;

    nsect = 0;
    ptr = getstarg(player->argp[1], "commodity or variable? ", buf);
    if (!ptr || !*ptr)
	return RET_SYN;
    ptr = nstr_comp_val(ptr, &val, EF_SECTOR);
    if (!ptr)
	return RET_SYN;
    if (val.val_cat != NSC_OFF || nstr_promote(val.val_type) != NSC_LONG) {
	pr("Can't survey this\n");
	return RET_SYN;
    }
    for (; isspace(*ptr); ++ptr) ;
    if (*ptr)
	return RET_SYN;
    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    if (!mapbuf)
	mapbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!map) {
	map = malloc(WORLD_Y * sizeof(char *));
	if (map && mapbuf) {
	    for (i = 0; i < WORLD_Y; i++)
		map[i] = &mapbuf[MAPWIDTH(1) * i];
	} else if (map) {
	    free(map);
	    map = NULL;
	}
    }
    if (!mapbuf || !map) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in sect\n");
	return RET_FAIL;
    }
    ncond = nstr.ncond;
    memcpy(cond, nstr.cond, sizeof(struct nscstr) * ncond);
    nstr.ncond = 0;
    np = getnatp(player->cnum);
    xyrelrange(np, &nstr.range, &range);
    border(&range, "     ", "");
    blankfill(mapbuf, &nstr.range, 1);
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	ptr = &map[nstr.dy][nstr.dx];
	if (nstr_exec(cond, ncond, &sect)) {
	    ++nsect;
	    *ptr = 0x80 | code_char(val, &sect);
	} else {
	    *ptr = dchr[sect.sct_type].d_mnem;
	}
    }
    for (y = nstr.range.ly, i = 0; i < nstr.range.height; y++, i++) {
	int yval;

	yval = yrel(np, y);
	pr("%4d %s %4d\n", yval, map[i], yval);
	if (y >= WORLD_Y)
	    y -= WORLD_Y;
    }
    border(&range, "     ", "");
    if (nsect > 0)
	pr("\n%d sector%s.\n", nsect, splur(nsect));
    return RET_OK;
}

static char
code_char(struct valstr val, struct sctstr *sp)
{
    int amt;
    int n;
    int large = val.val_type != NSC_CHAR && val.val_type != NSC_UCHAR;

    nstr_eval(&val, player->cnum, sp, NSC_LONG);
    if (CANT_HAPPEN(val.val_type != NSC_LONG))
	return ' ';
    amt = val.val_as.lng;
    if (amt <= 0)
	return ' ';
    n = amt / (large ? 100 : 10);
    if (n >= 10)
	return '$';
    return "0123456789"[n];
}
