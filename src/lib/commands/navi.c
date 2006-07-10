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
#include "commands.h"
#include "map.h"
#include "optlist.h"
#include "path.h"
#include "ship.h"

static int set_flagship(struct emp_qelem *list, struct shpstr **flagshipp);
static void switch_flagship(struct emp_qelem *list, int ship_uid);

int
navi(void)
{
    struct nstr_item ni_ship;
    struct emp_qelem ship_list;
    double minmob, maxmob;
    int together;
    char *cp = NULL;
    struct shpstr *shp = NULL;	/* flagship */
    struct nstr_sect ns;
    char origin;
    int dir;
    int stopping = 0;
    int skip = 0;
    char buf[1024];
    char prompt[128];
    char scanspace[1024];
    char pathtaken[1024];	/* Doubtful we'll have a path longer than this */
    char *pt = pathtaken;
    char bmap_flag;
    int ac;

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
	char dp[80];

	if (cp == NULL || *cp == '\0' || stopping) {
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
	    /* Just in case any of our ships were shelled while we were
	     * at the prompt, we call shp_nav() again.
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
	    if (cp && !(cp = shp_path(together, shp, buf)))
		cp = buf;
	}
	radmapnopr(shp->shp_x, shp->shp_y, (int)shp->shp_effic,
		   (int)techfact(shp->shp_tech,
				 mchr[(int)shp->shp_type].m_vrnge),
		   (mchr[(int)shp->shp_type].m_flags & M_SONAR)
		   ? techfact(shp->shp_tech, 1.0) : 0.0);
	if (cp == NULL || *cp == '\0')
	    cp = &dirch[DIR_STOP];
	dir = chkdir(*cp, DIR_STOP, DIR_VIEW);
	if (dir >= 0) {
	    if (dir == DIR_VIEW)
		shp_view(&ship_list);
	    else {
		stopping |= shp_nav_one_sector(&ship_list, dir, player->cnum, together);
		if (stopping != 2) {
		    *pt++ = dirch[dir];
		    *pt = '\0';
		}
	    }
	    cp++;
	    continue;
	}
	ac = parse(cp, player->argp, NULL, scanspace, NULL);
	if (ac <= 1) {
	    sprintf(dp, "%d", shp->shp_uid);
	    player->argp[1] = dp;
	    cp++;
	} else
	    cp = NULL;
	bmap_flag = 0;
	switch (*player->argp[0]) {
	case 'B':
	    bmap_flag = 'b';
	    /*
	     * fall through
	     */
	case 'M':
	    unit_map(EF_SHIP, atoi(player->argp[1]), &ns, &origin);
	    draw_map(bmap_flag, origin, MAP_SHIP, &ns);
	    skip = 1;
	    break;
	case 'f':
	    if (ac <= 1) 
		switch_flagship(&ship_list, -1);
	    else
		switch_flagship(&ship_list, atoi(player->argp[1]));
	    set_flagship(&ship_list, &shp);
	    break;
	case 'i':
	    shp_list(&ship_list);
	    break;
	case 'm':
	    stopping |= shp_sweep(&ship_list, 1, 0, player->cnum);
	    break;
	case 'r':
	    rada();
	    skip = 1;
	    player->btused++;
	    break;
	case 'l':
	    look();
	    player->btused++;
	    break;
	case 's':
	    sona();
	    player->btused++;
	    skip = 1;
	    break;
	case 'd':
	    if (ac == 2) {
		player->argp[2] = player->argp[1];
		sprintf(dp, "%d", shp->shp_uid);
		player->argp[1] = dp;
	    }
	    mine();
	    skip = 1;
	    player->btused++;
	    break;
	default:
	    direrr("`%c' to stop", ", `%c' to view, ", 0);
	    pr("`i' to list ships, `f' to change flagship,\n");
	    pr("`r' to radar, `s' to sonar, `l' to look, `M' to map, `B' to bmap,\n");
	    pr("`d' to drop mines, and `m' to minesweep\n");
	    stopping = 1;
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

static void
switch_flagship(struct emp_qelem *list, int ship_uid)
{
    struct emp_qelem *qp, *save;
    struct mlist *mlp;

    if (QEMPTY(list))
	return;

    save = qp = list->q_back;
    do {
        emp_remque(qp);
        emp_insque(qp, list);
        qp = list->q_back;
        mlp = (struct mlist *)qp;
        if (mlp->ship.shp_uid == ship_uid || ship_uid == -1)
            break;
    } while (list->q_back != save);
}

