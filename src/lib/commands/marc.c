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
 *  marc.c: March units around
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak
 *     Ken Stevens, 1995 (rewrite)
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "file.h"
#include "map.h"
#include "path.h"
#include "empobj.h"
#include "unit.h"

int
march(void)
{
    struct nstr_item ni_land;
    struct emp_qelem land_list;
    double minmob, maxmob;
    int together;
    char *cp = NULL;
    int leader_uid;
    struct empobj *leader;
    int dir;
    int stopping = 0;
    int skip = 0;
    char buf[1024];
    char prompt[128];
    char scanspace[1024];
    char bmap_flag;
    int ac;

    if (!snxtitem(&ni_land, EF_LAND, player->argp[1]))
	return RET_SYN;
    lnd_sel(&ni_land, &land_list);
    lnd_mar(&land_list, &minmob, &maxmob, &together, player->cnum);
    if (QEMPTY(&land_list)) {
	pr("No lands\n");
	return RET_FAIL;
    }
    leader = get_leader(&land_list);
    leader_uid = leader->uid;
    pr("Leader is %s\n", obj_nameof(leader));
    if (player->argp[2]) {
	strcpy(buf, player->argp[2]);
	if (!(cp = lnd_path(together, (struct lndstr *)leader, buf)))
	    cp = player->argp[2];
    }

    while (!QEMPTY(&land_list)) {
	char dp[80];

	if (cp == NULL || *cp == '\0' || stopping) {
	    stopping = 0;
	    lnd_mar(&land_list, &minmob, &maxmob, &together, player->cnum);
	    if (QEMPTY(&land_list)) {
		pr("No lands left\n");
		return RET_OK;
	    }
	    leader = get_leader(&land_list);
	    if (leader->uid != leader_uid) {
		leader_uid = leader->uid;
		pr_leader_change(leader);
		stopping = 1;
		continue;
	    }
	    if (!skip)
		nav_map(leader->x, leader->y, 1);
	    else
		skip = 0;
	    sprintf(prompt, "<%.1f:%.1f: %s> ", maxmob,
		    minmob, xyas(leader->x, leader->y, player->cnum));
	    cp = getstring(prompt, buf);
/* Just in case any of our lands were shelled while we were at the
 * prompt, we call lnd_mar() again.
 */
	    lnd_mar(&land_list, &minmob, &maxmob, &together, player->cnum);
	    if (QEMPTY(&land_list)) {
		pr("No lands left\n");
		return RET_OK;
	    }
	    leader = get_leader(&land_list);
	    if (leader->uid != leader_uid) {
		leader_uid = leader->uid;
		pr_leader_change(leader);
		stopping = 1;
		continue;
	    }
	    if (cp && !(cp = lnd_path(together, (struct lndstr *)leader, buf)))
		cp = buf;
	}
	if (cp == NULL || *cp == '\0')
	    cp = &dirch[DIR_STOP];
	dir = chkdir(*cp, DIR_STOP, DIR_LAST);
	if (dir >= 0) {
	    stopping |=
		lnd_mar_one_sector(&land_list, dir, player->cnum, together);
	    cp++;
	    continue;
	}
	ac = parse(cp, player->argp, NULL, scanspace, NULL);
	if (ac <= 1) {
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
	    do_map(bmap_flag, EF_LAND, player->argp[1], player->argp[2]);
	    skip = 1;
	    break;
	case 'f':
	    if (ac <= 1)
		switch_leader(&land_list, -1);
	    else
		switch_leader(&land_list, atoi(player->argp[1]));
	    leader = get_leader(&land_list);
	    if (leader->uid != leader_uid) {
		leader_uid = leader->uid;
		pr_leader_change(leader);
	    }
	    break;
	case 'i':
	    lnd_list(&land_list);
	    break;
	case 'm':
	    lnd_sweep(&land_list, 1, 1, player->cnum);
	    stopping |= lnd_check_mines(&land_list);
	    break;
	case 'r':
	    radar(EF_LAND);
	    skip = 1;
	    player->btused++;
	    break;
	case 'l':
	    llook();
	    player->btused++;
	    break;
	case 'd':
	    if (ac == 2) {
		player->argp[2] = player->argp[1];
		sprintf(dp, "%d", leader->uid);
		player->argp[1] = dp;
	    }
	    landmine();
	    skip = 1;
	    player->btused++;
	    break;
	default:
	    direrr("`%c' to stop", 0, 0);
	    pr(", `i' to list units, `f' to change leader,\n");
	    pr("`r' to radar, `l' to look, `M' to map, `B' to bmap,\n");
	    pr("`d' to drop mines, and `m' to minesweep\n");
	    stopping = 1;
	}
    }
    return RET_OK;
}

void
pr_leader_change(struct empobj *leader)
{
    pr("Changing %s to %s\n",
	leader->ef_type == EF_SHIP ? "flagship" : "leader",
	obj_nameof(leader));
}

struct empobj *
get_leader(struct emp_qelem *list)
{
    return &((struct ulist *)(list->q_back))->unit.gen;
}

void
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

