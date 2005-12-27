/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  marc.c: March units around
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak
 *     Ken Stevens, 1995 (rewrite)
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "land.h"
#include "xy.h"
#include "nsc.h"
#include "path.h"
#include "file.h"
#include "map.h"
#include "commands.h"

static int set_leader(struct emp_qelem *list, struct lndstr **leaderp);

int
march(void)
{
    struct nstr_item ni_land;
    struct emp_qelem land_list;
    double minmob, maxmob;
    int together;
    s_char *cp = 0;
    struct lndstr *lnd = 0;	/* leader */
    struct nstr_sect ns;
    s_char origin;
    int dir;
    int stopping = 0;
    int skip = 0;
    s_char buf[1024];
    s_char prompt[128];

    if (!snxtitem(&ni_land, EF_LAND, player->argp[1]))
	return RET_SYN;
    lnd_sel(&ni_land, &land_list);
    lnd_mar(&land_list, &minmob, &maxmob, &together, player->cnum);
    if (QEMPTY(&land_list)) {
	pr("No lands\n");
	return RET_FAIL;
    }
    set_leader(&land_list, &lnd);
    if (player->argp[2]) {
	strcpy(buf, player->argp[2]);
	if (!(cp = lnd_path(together, lnd, buf)))
	    cp = player->argp[2];
    }

    while (!QEMPTY(&land_list)) {
	s_char *bp, dp[80];

	if (cp == 0 || *cp == '\0' || stopping) {
	    stopping = 0;
	    lnd_mar(&land_list, &minmob, &maxmob, &together, player->cnum);
	    if (QEMPTY(&land_list)) {
		pr("No lands left\n");
		return RET_OK;
	    }
	    if (set_leader(&land_list, &lnd)) {
		stopping = 1;
		continue;
	    }
	    if (!skip)
		nav_map(lnd->lnd_x, lnd->lnd_y, 1);
	    else
		skip = 0;
	    sprintf(prompt, "<%.1f:%.1f: %s> ", maxmob,
		    minmob, xyas(lnd->lnd_x, lnd->lnd_y, player->cnum));
	    cp = getstring(prompt, buf);
/* Just in case any of our lands were shelled while we were at the
 * prompt, we call lnd_mar() again.
 */
	    lnd_mar(&land_list, &minmob, &maxmob, &together, player->cnum);
	    if (QEMPTY(&land_list)) {
		pr("No lands left\n");
		return RET_OK;
	    }
	    if (set_leader(&land_list, &lnd)) {
		stopping = 1;
		continue;
	    }
	}
	if (cp == 0 || *cp == '\0')
	    cp = &dirch[DIR_STOP];
	if (*cp == 'M' ||
	    *cp == 'B' || *cp == 'f' || *cp == 'i' || *cp == 'm') {
	    ++cp;
	    if (cp[-1] == 'M') {
		unit_map(EF_LAND, lnd->lnd_uid, &ns, &origin);
		draw_map(0, origin, 0, &ns);
		skip = 1;
	    } else if (cp[-1] == 'B') {
		unit_map(EF_LAND, lnd->lnd_uid, &ns, &origin);
		draw_map('b', origin, 0, &ns);
		skip = 1;
	    } else if (cp[-1] == 'f') {
		struct emp_qelem *qp;
		qp = land_list.q_back;
		emp_remque(land_list.q_back);
		emp_insque(qp, &land_list);
		set_leader(&land_list, &lnd);
	    } else if (cp[-1] == 'i') {
		lnd_list(&land_list);
	    } else {
		lnd_sweep(&land_list, 1, 1, player->cnum);
		stopping |= lnd_check_mines(&land_list);
	    }
	    continue;
	} else if (*cp == 'r' || *cp == 'l') {
	    bp = ++cp;
	    while ((*bp != ' ') && (*bp))
		bp++;
	    while ((*bp == ' ') && (*bp))
		bp++;
	    if ((bp != (s_char *)0) && (*bp))
		player->argp[1] = bp;
	    else {
		sprintf(dp, "%d", lnd->lnd_uid);
		player->argp[1] = dp;
	    }
	    if (cp[-1] == 'r') {
		player->argp[0] = "lradar";
		rada();
		skip = 1;
	    } else
		llook();
	    *cp = 0;
	    player->btused++;
	    continue;
	} else {
	    dir = chkdir(*cp++, DIR_STOP, DIR_LAST);
	    if (dir < 0) {
		if (NULL != (cp = lnd_path(together, lnd, buf)))
		    continue;
		direrr("`%c' to stop", 0, 0);
		pr(", `i' to list units, `f' to change leader,\n");
		pr("`r' to radar, `l' to look, `M' to map, `B' to bmap,\n");
		pr("and `m' to minesweep\n");
		stopping = 1;
		continue;
	    }
	}
	stopping |=
	    lnd_mar_one_sector(&land_list, dir, player->cnum, together);
    }
    return RET_OK;
}

static int
set_leader(struct emp_qelem *list, struct lndstr **leaderp)
{
    struct llist *llp = (struct llist *)(list->q_back);

    if (!*leaderp)
	pr("Leader is ");
    else if ((*leaderp)->lnd_uid != llp->land.lnd_uid)
	pr("Changing leader to ");
    else
	return 0;
    *leaderp = &llp->land;
    pr("%s\n", prland(&llp->land));
    return 1;
}
