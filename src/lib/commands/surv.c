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
 *  surv.c: Show sector survey
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "deity.h"
#include "file.h"
#include "nat.h"
#include "map.h"
#include "commands.h"
#include "optlist.h"

static	s_char code_char(long int coding, struct sctstr *sp);

/*
 * survey type <sarg> ?cond
 *
 */
int
surv(void)
{
	int     nsect;
	struct	nstr_sect nstr;
	int     y;
	long    coding;
	struct	natstr *np;
	struct	sctstr sect;
	struct	range range;
	s_char	*ptr;
	struct	nscstr cond[NS_NCOND];
	int	ncond;
	int	i;
	s_char	what[64];
	s_char	buf[1024];
	s_char	*str;
	/* Note this is not re-entrant anyway, so we keep the buffers
	   around */
	static s_char      *mapbuf = (s_char *)0;
	static s_char      **map = (s_char **)0;

	nsect = 0;
	if ((ptr = getstarg(player->argp[1], "commodity or variable? ", buf)) == 0)
		return RET_SYN;
	if (encode(ptr, &coding, EF_SECTOR) < 0)
		return RET_SYN;
	if(player->argp[2] == (s_char *) 0) {
		if ((str = getstring("(sects)? ", buf)) == 0)
			return RET_SYN;
	} else {
		str = player->argp[2];
	}
	if(*str == '*') {
		sprintf(what, "%d:%d,%d:%d",
			-WORLD_X/2, WORLD_X/2-1,
			-WORLD_Y/2, WORLD_Y/2-1);
		if (!snxtsct(&nstr, what))
			return RET_FAIL;
	}
	else
	if (!snxtsct(&nstr, str))
		return RET_SYN;
	if (!mapbuf)
	    mapbuf = (s_char *)malloc((WORLD_Y*MAPWIDTH(1))*sizeof(s_char));
	if (!map) {
	    map = (s_char **)malloc(WORLD_Y * sizeof(s_char *));
	    if (map && mapbuf) {
		for (i = 0; i < WORLD_Y; i++)
		    map[i] = &mapbuf[MAPWIDTH(1) * i];
	    } else if (map) {
		free((s_char *)map);
		map = (s_char **)0;
	    }
	}
	if (!mapbuf || !map) {
	    pr("Memory error, tell the deity.\n");
	    logerror("malloc failed in sect\n");
	    return RET_FAIL;
	}
	ncond = nstr.ncond;
	bcopy((s_char *)nstr.cond, (s_char *)cond, sizeof(struct nscstr) * ncond);
	nstr.ncond = 0;
	np = getnatp(player->cnum);
	xyrelrange(np, &nstr.range, &range);
	border(&range, "     ", "");
	blankfill((s_char *)mapbuf, &nstr.range, 1);
	while (nxtsct(&nstr, &sect)) {
		if (!player->owner)
			continue;
		ptr = &map[nstr.dy][nstr.dx];
		if (nstr_exec(cond, ncond, (s_char *)&sect, EF_SECTOR)) {
			++nsect;
			*ptr = 0x80 | code_char(coding, &sect);
		} else {
			*ptr = dchr[sect.sct_type].d_mnem;
		}
	}
	for (y=nstr.range.ly, i=0; i < nstr.range.height; y++, i++) {
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

static
s_char
code_char(long int coding, struct sctstr *sp)
{
	int     amt;
	int     n;

	if (!(coding & NSC_VAR))
		coding |= NSC_OFF;
	amt = decode(player->cnum, coding, (s_char *) sp, EF_SECTOR);
	n = 0;
	if ((coding & NSC_CMASK) == NSC_VAR) {
		if (amt != 0)
			n = (amt / 100) + 1;
	} else if (amt != 0)
		n = (amt / 10) + 1;
	if (n > 11)
		n = 11;
	if (n < 0)
		n = 0;
	return " 0123456789$"[n];
}
