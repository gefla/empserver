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
 *  navi.c: Navigate ships and such
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995 (rewritten)
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "ship.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "path.h"
#include "file.h"
#include "map.h"
#include "commands.h"
#include "optlist.h"

static int set_flagship(struct emp_qelem *list, struct shpstr **flagshipp);

int
navi(void)
{
    struct nstr_item ni_ship;
    struct emp_qelem ship_list;
    double minmob, maxmob;
    int together;
    char *cp = 0;
    struct shpstr *shp = 0;	/* flagship */
    struct nstr_sect ns;
    char origin;
    int dir;
    int stopping = 0;
    int skip = 0;
    char buf[1024];
    char prompt[128];
    char pathtaken[1024];	/* Doubtful we'll have a path longer than this */
    char *pt = pathtaken;

    if (!snxtitem(&ni_ship, EF_SHIP, player->argp[1]))
	return RET_SYN;
    shp_sel(&ni_ship, &ship_list);
    shp_nav(&ship_list, &minmob, &maxmob, &together, player->cnum);
    if (QEMPTY(&ship_list)) {
	pr("No ships\n");
	return RET_FAIL;
    }
    set_flagship(&ship_list, &shp);
    if (player->argp[2]) {
	strcpy(buf, player->argp[2]);
	if (!(cp = shp_path(together, shp, buf)))
	    cp = player->argp[2];
    }

    *pt = '\0';
    while (!QEMPTY(&ship_list)) {
	char *bp, dp[80];

	if (cp == 0 || *cp == '\0' || stopping) {
	    stopping = 0;
	    shp_nav(&ship_list, &minmob, &maxmob, &together, player->cnum);
	    if (QEMPTY(&ship_list)) {
		pr("No ships left\n");
		if (strlen(pathtaken) > 0) {
		    pathtaken[strlen(pathtaken) - 1] = '\0';
		    if (strlen(pathtaken) > 0)
			pr("Path taken: %s\n", pathtaken);
		}
		return RET_OK;
	    }
	    if (set_flagship(&ship_list, &shp)) {
		stopping = 1;
		continue;
	    }
	    if (!skip)
		nav_map(shp->shp_x, shp->shp_y,
			!(mchr[(int)shp->shp_type].m_flags & M_SUB));
	    else
		skip = 0;
	    sprintf(prompt, "<%.1f:%.1f: %s> ", maxmob,
		    minmob, xyas(shp->shp_x, shp->shp_y, player->cnum));
	    cp = getstring(prompt, buf);
	    /* Just in case any of our ships were shelled while we were at the
	     * prompt, we call shp_nav() again.
	     */
	    shp_nav(&ship_list, &minmob, &maxmob, &together, player->cnum);
	    if (QEMPTY(&ship_list)) {
		pr("No ships left\n");
		if (strlen(pathtaken) > 0) {
		    pathtaken[strlen(pathtaken) - 1] = '\0';
		    if (strlen(pathtaken) > 0)
			pr("Path taken: %s\n", pathtaken);
		}
		return RET_OK;
	    }
	    if (set_flagship(&ship_list, &shp)) {
		stopping = 1;
		continue;
	    }
	}
	radmapnopr(shp->shp_x, shp->shp_y, (int)shp->shp_effic,
		   (int)techfact(shp->shp_tech,
				 (double)mchr[(int)shp->shp_type].m_vrnge),
		   (double)((mchr[(int)shp->shp_type].m_flags & M_SONAR)
			    ? techfact(shp->shp_tech, 1.0) : 0.0));
	if (cp == 0 || *cp == '\0')
	    cp = &dirch[DIR_STOP];
	if (*cp == 'M' ||
	    *cp == 'B' || *cp == 'f' || *cp == 'i' || *cp == 'm') {
	    ++cp;
	    if (cp[-1] == 'M') {
		unit_map(EF_SHIP, shp->shp_uid, &ns, &origin);
		draw_map(0, origin, MAP_SHIP, &ns);
		skip = 1;
	    } else if (cp[-1] == 'B') {
		unit_map(EF_SHIP, shp->shp_uid, &ns, &origin);
		draw_map('b', origin, MAP_SHIP, &ns);
		skip = 1;
	    } else if (cp[-1] == 'f') {
		struct emp_qelem *qp;
		qp = ship_list.q_back;
		emp_remque(ship_list.q_back);
		emp_insque(qp, &ship_list);
		set_flagship(&ship_list, &shp);
	    } else if (cp[-1] == 'i') {
		shp_list(&ship_list);
	    } else {
		stopping |= shp_sweep(&ship_list, 1, player->cnum);
	    }
	    continue;
	} else if (*cp == 'r' || *cp == 'l' || *cp == 's') {
	    for (bp = cp + 1; *bp && !isspace(*bp); bp++) ;
	    for (; *bp && isspace(*bp); bp++) ;
	    if (*bp)
		player->argp[1] = bp;
	    else {
		sprintf(dp, "%d", shp->shp_uid);
		player->argp[1] = dp;
	    }
	    if (*cp++ == 'r') {
		rada();
		skip = 1;
	    } else if (cp[-1] == 'l')
		look();
	    else {
		player->argp[2] = 0;
		sona();
		skip = 1;
	    }
	    *cp = 0;
	    player->btused++;
	    continue;
	} else {
	    dir = chkdir(*cp++, DIR_STOP, DIR_VIEW);
	    if (dir < 0) {
		if (NULL != (cp = shp_path(together, shp, buf)))
		    continue;
		direrr("`%c' to stop", ", `%c' to view, ", 0);
		pr("`i' to list ships, `f' to change flagship,\n");
		pr("`r' to radar, `s' to sonar, `l' to look, `M' to map, `B' to bmap,\n");
		pr("and `m' to minesweep\n");
		stopping = 1;
		continue;
	    } else if (dir == DIR_VIEW) {
		shp_view(&ship_list);
		continue;
	    }
	}
	stopping |=
	    shp_nav_one_sector(&ship_list, dir, player->cnum, together);
	if (stopping != 2) {
	    *pt++ = dirch[dir];
	    *pt = '\0';
	}
    }
    if (strlen(pathtaken) > 0) {
	pathtaken[strlen(pathtaken) - 1] = '\0';
	if (strlen(pathtaken) > 0)
	    pr("Path taken: %s\n", pathtaken);
    }
    return RET_OK;
}

int
nav_map(int x, int y, int show_designations)
{
    char *ptr;
    struct nstr_sect ns;
    struct natstr *np;
    struct sctstr sect;
    struct range range;
    int i;
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static unsigned char *bitmap = NULL;
    static char *wmapbuf = NULL;
    static char **wmap = NULL;
    int changed = 0;

    if (!wmapbuf)
	wmapbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!wmap) {
	wmap = malloc(WORLD_Y * sizeof(*wmap));
	if (wmap && wmapbuf) {
	    for (i = 0; i < WORLD_Y; i++)
		wmap[i] = &wmapbuf[MAPWIDTH(1) * i];
	} else if (wmap) {
	    free(wmap);
	    wmap = NULL;
	}
    }
    if (!bitmap)
	bitmap = malloc((WORLD_X * WORLD_Y) / 8);
    if (!wmapbuf || !wmap || !bitmap) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in navi\n");
	return RET_FAIL;
    }
    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
    snxtsct_dist(&ns, x, y, 1);
    np = getnatp(player->cnum);
    xyrelrange(np, &ns.range, &range);
    blankfill(wmapbuf, &ns.range, 1);
    while (nxtsct(&ns, &sect)) {
	ptr = &wmap[ns.dy][ns.dx];
	*ptr = dchr[sect.sct_type].d_mnem;
	if (!show_designations &&
	    sect.sct_own != player->cnum &&
	    sect.sct_type != SCT_WATER &&
	    sect.sct_type != SCT_BSPAN && sect.sct_type != SCT_HARBR)
	    *ptr = '?';
	changed += map_set(player->cnum, sect.sct_x, sect.sct_y, *ptr, 0);
	/*
	 * We do it this way so that 'x' and 'X'
	 * bdesignations will show up. This can
	 * be used to mark mined sectors. So, the
	 * player will see the current des, UNLESS
	 * they've marked the sector 'x' or 'X',
	 * in which case they'll see that.
	 * --ts
	 */
	*ptr = player->bmap[sctoff(sect.sct_x, sect.sct_y)];
    }
    if (changed)
	writemap(player->cnum);
    for (i = 0; i < ns.range.height; i++)
	pr("%s\n", wmap[i]);
    return RET_OK;
}

static int
set_flagship(struct emp_qelem *list, struct shpstr **flagshipp)
{
    struct mlist *mlp = (struct mlist *)(list->q_back);

    if (!*flagshipp)
	pr("Flagship is ");
    else if ((*flagshipp)->shp_uid != mlp->ship.shp_uid)
	pr("Changing flagship to ");
    else
	return 0;
    *flagshipp = &mlp->ship;
    pr("%s\n", prship(&mlp->ship));
    return 1;
}
