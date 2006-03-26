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
 *  surv.c: Show sector survey
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "map.h"
#include "commands.h"
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
    s_char *ptr;
    struct nscstr cond[NS_NCOND];
    int ncond;
    int i;
    s_char what[64];
    s_char buf[1024];
    s_char *str;
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static s_char *mapbuf = NULL;
    static s_char **map = NULL;

    nsect = 0;
    if ((ptr = getstarg(player->argp[1], "commodity or variable? ", buf)) == 0)
	return RET_SYN;
    ptr = nstr_comp_val(ptr, &val, EF_SECTOR);
    if (!ptr)
	return RET_SYN;
    if (val.val_cat != NSC_OFF || nstr_coerce_val(&val, NSC_LONG, NULL) < 0) {
	pr("Can't survey this\n");
	return RET_SYN;
    }
    for (; isspace(*ptr); ++ptr) ;
    if (*ptr)
	return RET_SYN;
    if (player->argp[2] == NULL) {
	if ((str = getstring("(sects)? ", buf)) == 0)
	    return RET_SYN;
    } else {
	str = player->argp[2];
    }
    if (*str == '*') {
	sprintf(what, "%d:%d,%d:%d",
		-WORLD_X / 2, WORLD_X / 2 - 1,
		-WORLD_Y / 2, WORLD_Y / 2 - 1);
	if (!snxtsct(&nstr, what))
	    return RET_FAIL;
    } else if (!snxtsct(&nstr, str))
	return RET_SYN;
    if (!mapbuf)
	mapbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!map) {
	map = malloc(WORLD_Y * sizeof(s_char *));
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
    blankfill((s_char *)mapbuf, &nstr.range, 1);
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

    nstr_exec_val(&val, player->cnum, sp, NSC_LONG);
    amt = val.val_as.lng;
    if (amt <= 0)
	return ' ';
    n = amt / (large ? 100 : 10);
    if (n >= 10)
	return '$';
    return "0123456789"[n];
}
