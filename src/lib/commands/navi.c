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
 *  navi.c: Navigate ships and such
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995 (rewritten)
 *     Ron Koenderink, 2006-2007
 *     Markus Armbruster, 2006-2011
 */

#include <config.h>

#include "commands.h"
#include "map.h"
#include "optlist.h"
#include "path.h"
#include "unit.h"

static void pr_leader_change(struct empobj *leader);
static struct empobj *get_leader(struct emp_qelem *list);
static void switch_leader(struct emp_qelem *list, int uid);

int
navi(void)
{
    struct nstr_item ni_ship;
    struct emp_qelem ship_list;
    double minmob, maxmob;
    int together;

    if (!snxtitem(&ni_ship, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;
    shp_sel(&ni_ship, &ship_list);
    shp_nav(&ship_list, &minmob, &maxmob, &together, player->cnum);
    if (QEMPTY(&ship_list)) {
	pr("No ships\n");
	return RET_FAIL;
    }

    return do_unit_move(&ship_list, &together, &minmob, &maxmob);
}

int
do_unit_move(struct emp_qelem *ulist, int *together,
	     double *minmob, double *maxmob)
{
    char *cp = NULL;
    int leader_uid;
    struct empobj *leader;
    int dir;
    int stopping = 0;
    int skip = 0;
    char buf[1024];
    char prompt[128];
    char pathtaken[1024];  /* Doubtful we'll have a path longer than this */
    char *pt = pathtaken;
    char bmap_flag;
    int ac;
    int type;

    leader = get_leader(ulist);
    leader_uid = leader->uid;
    type = leader->ef_type;
    pr("%s is %s\n",
	type == EF_SHIP ? "Flagship" : "Leader",
	unit_nameof(leader));

    if (player->argp[2]) {
	strcpy(buf, player->argp[2]);
	cp = unit_path(*together, leader, buf);
    }

    *pt = '\0';
    while (!QEMPTY(ulist)) {
	char dp[80];

	if (cp == NULL || *cp == '\0' || stopping) {
	    stopping = 0;
	    if (type == EF_SHIP)
		shp_nav(ulist, minmob, maxmob, together, player->cnum);
	    else
		lnd_mar(ulist, minmob, maxmob, together, player->cnum);
	    if (QEMPTY(ulist)) {
		pr("No %s left\n", type == EF_SHIP ? "ships" : "lands");
		if (type == EF_SHIP && strlen(pathtaken) > 1) {
		    pathtaken[strlen(pathtaken) - 1] = '\0';
		    pr("Path taken: %s\n", pathtaken);
		}
		return RET_OK;
	    }
	    leader = get_leader(ulist);
	    if (leader->uid != leader_uid) {
		leader_uid = leader->uid;
		pr_leader_change(leader);
		stopping = 1;
		continue;
	    }
	    if (!skip)
		nav_map(leader->x, leader->y,
			type == EF_SHIP
			? !(mchr[(int)leader->type].m_flags & M_SUB) : 1);
	    else
		skip = 0;
	    sprintf(prompt, "<%.1f:%.1f: %s> ", *maxmob,
		    *minmob, xyas(leader->x, leader->y, player->cnum));
	    cp = getstring(prompt, buf);
	    /* Just in case any of our units were shelled while we were
	     * at the prompt, we call shp_nav() or lnd_mar() again.
	     */
	    if (type == EF_SHIP)
		shp_nav(ulist, minmob, maxmob, together, player->cnum);
	    else
		lnd_mar(ulist, minmob, maxmob, together, player->cnum);
	    if (QEMPTY(ulist)) {
		pr("No %s left\n", type == EF_SHIP ? "ships" : "lands");
		if (type == EF_SHIP && strlen(pathtaken) > 1) {
		    pathtaken[strlen(pathtaken) - 1] = '\0';
		    pr("Path taken: %s\n", pathtaken);
		}
		return RET_OK;
	    }
	    leader = get_leader(ulist);
	    if (leader->uid != leader_uid) {
		leader_uid = leader->uid;
		pr_leader_change(leader);
		stopping = 1;
		continue;
	    }
	    if (cp)
		cp = unit_path(*together, leader, cp);
	}
	if (type == EF_SHIP) {
	    rad_map_set(player->cnum, leader->x, leader->y, leader->effic,
			leader->tech, mchr[leader->type].m_vrnge);
	}
	if (cp == NULL || *cp == '\0')
	    cp = &dirch[DIR_STOP];
	dir = chkdir(*cp, DIR_STOP, DIR_LAST);
	if (dir >= 0) {
	    if (type == EF_SHIP) {
		stopping |= shp_nav_one_sector(ulist, dir,
					       player->cnum, *together);
		if (stopping != 2) {
		    *pt++ = dirch[dir];
		    *pt = '\0';
		}
	    } else
		stopping |=
		    lnd_mar_one_sector(ulist, dir, player->cnum,
				       *together);
	    cp++;
	    continue;
	}
	ac = parse(cp, player->argbuf, player->argp, NULL, NULL, NULL);
	if (ac <= 0) {
	    player->argp[0] = "";
	    cp = NULL;
	} else if (ac == 1) {
	    sprintf(dp, "%d", leader->uid);
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
	    display_region_map(bmap_flag, type, leader->x, leader->y,
			       player->argp[1], player->argp[2]);
	    skip = 1;
	    continue;
	case 'f':
	    if (ac <= 1)
		switch_leader(ulist, -1);
	    else
		switch_leader(ulist, atoi(player->argp[1]));
	    leader = get_leader(ulist);
	    if (leader->uid != leader_uid) {
		leader_uid = leader->uid;
		pr_leader_change(leader);
	    }
	    continue;
	case 'i':
	    unit_list(ulist);
	    continue;
	case 'm':
	    if (type == EF_SHIP)
		stopping |= shp_sweep(ulist, 1, 1, player->cnum);
	    else {
		lnd_sweep(ulist, 1, 1, player->cnum);
		stopping |= lnd_check_mines(ulist);
	    }
	    continue;
	case 'r':
	    radar(leader->ef_type);
	    skip = 1;
	    player->btused++;
	    continue;
	case 'l':
	    do_look(type);
	    player->btused++;
	    continue;
	case 's':
	    if (leader->ef_type != EF_SHIP)
		break;
	    sona();
	    player->btused++;
	    skip = 1;
	    continue;
	case 'd':
	    if (ac < 3) {
		player->argp[2] = ac < 2 ? "1" : player->argp[1];
		sprintf(dp, "%d", leader->uid);
		player->argp[1] = dp;
	    }
	    if (type == EF_SHIP)
		mine();
	    else
		landmine();
	    stopping = 1;
	    skip = 1;
	    player->btused++;
	    continue;
	case 'v':
	    unit_view(ulist);
	    continue;
	}
	direrr("`%c' to stop", ", `%c' to view", NULL);
	pr(", `i' to list %s, `f' to change %s,\n",
	    type == EF_SHIP ? "ships" : "units",
	    type == EF_SHIP ? "flagship" : "leader");
	pr("`r' to radar, %s`l' to look, `M' to map, `B' to bmap,\n",
	    type == EF_SHIP ? "`s' to sonar, " : "");
	pr("`d' to drop mines, and `m' to minesweep\n");
	stopping = 1;
    }
    if (type == EF_SHIP && strlen(pathtaken) > 1) {
	pathtaken[strlen(pathtaken) - 1] = '\0';
	pr("Path taken: %s\n", pathtaken);
    }
    return RET_OK;
}

int
nav_map(int x, int y, int show_designations)
{
    char *ptr;
    struct nstr_sect ns;
    struct sctstr sect;
    int i;
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
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
    if (!wmapbuf || !wmap) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in navi\n");
	return RET_FAIL;
    }
    snxtsct_dist(&ns, x, y, 1);
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
	*ptr = player->bmap[sect.sct_uid];
    }
    if (changed)
	writemap(player->cnum);
    for (i = 0; i < ns.range.height; i++)
	pr("%s\n", wmap[i]);
    return RET_OK;
}

static void
pr_leader_change(struct empobj *leader)
{
    pr("Changing %s to %s\n",
	leader->ef_type == EF_SHIP ? "flagship" : "leader",
	unit_nameof(leader));
}

static struct empobj *
get_leader(struct emp_qelem *list)
{
    return &((struct ulist *)(list->q_back))->unit.gen;
}

static void
switch_leader(struct emp_qelem *list, int uid)
{
    struct emp_qelem *qp, *save;
    struct ulist *ulp;

    if (QEMPTY(list))
	return;

    save = qp = list->q_back;
    do {
	emp_remque(qp);
	emp_insque(qp, list);
	qp = list->q_back;
	ulp = (struct ulist *)qp;
	if (ulp->unit.gen.uid == uid || uid == -1)
	    break;
    } while (list->q_back != save);
}
