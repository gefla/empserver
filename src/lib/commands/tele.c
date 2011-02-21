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
 *  tele.c: Send a telegram
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#include <config.h>

#include "commands.h"
#include "news.h"
#include "optlist.h"
#include "tel.h"

int
tele(void)
{
    natid to;
    struct natstr *natp;
    char buf[MAXTELSIZE + 1];	/* UTF-8 */
    int n;

    if (*player->argp[0] == 'a') {
	if (getele("everybody", buf) <= 0) {
	    pr("Announcement aborted\n");
	    return RET_FAIL;
	}
	pr("\n");
	to = 0;
	if (typed_wu(player->cnum, to, buf, TEL_ANNOUNCE) < 0)
	    logerror("tele: typed_wu failed to #%d", to);
    } else if (*player->argp[0] == 'p') {
	if (getele("your Gracious Deity", buf) <= 0) {
	    pr("Prayer aborted\n");
	    return RET_FAIL;
	}
	pr("\n");
	if (typed_wu(player->cnum, 0, buf, TEL_NORM) < 0)
	    logerror("tele: typed_wu failed to #0");
    } else {
	int kk;

	kk = 1;
	do {
	    if ((n = natarg(player->argp[kk], "for which country? ")) < 0)
		return RET_SYN;
	    to = n;

	    if (kk == 1) {
		if (player->argp[2]) {
		    if (getele("multiple recipients", buf) < 0) {
			pr("Telegram aborted\n");
			return RET_FAIL;
		    }
		} else {
		    if (getele(cname(to), buf) < 0) {
			pr("Telegram aborted\n");
			return RET_FAIL;
		    }
		}
		pr("\n");
	    }

	    natp = getnatp(to);
	    if (natp->nat_stat < STAT_SANCT) {
		pr("%s has no \"telegram privileges\".\n", cname(to));
		kk++;
		continue;
	    }
	    if (!player->god
		&& (getrejects(player->cnum, natp) & REJ_TELE)) {
		pr("%s is rejecting your telegrams.\n", cname(to));
		return RET_SYN;
	    }
	    if (typed_wu(player->cnum, to, buf, TEL_NORM) < 0) {
		logerror("tele: typed_wu failed to #%d", n);
		return RET_FAIL;
	    }

	    if (!player->god && natp->nat_stat != STAT_GOD
		&& player->cnum != to)
		nreport(player->cnum, N_SENT_TEL, to, 1);
	    if (opt_HIDDEN) {
		setcont(to, player->cnum, FOUND_TELE);
	    }
	    kk++;
	} while (player->argp[kk] != NULL);
    }
    return RET_OK;
}
