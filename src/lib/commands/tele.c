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
 *  tele.c: Send a telegram
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *    
 */

#include "misc.h"
#include "player.h"
#include "nat.h"
#include "tel.h"
#include "news.h"
#include "deity.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"

int
tele(void)
{
    natid to;
    struct natstr *natp;
    int teltype;
    s_char buf[MAXTELSIZE + 1];
    int n;

    natp = getnatp(player->cnum);
    if (*player->argp[0] == 'a') {
	if (getele("everybody", buf) <= 0) {
	    pr("Announcement aborted\n");
	    return RET_OK;
	}
	pr("\n");
	to = 0;
	if (typed_wu(player->cnum, to, buf, TEL_ANNOUNCE) < 0)
	    logerror("tele: typed_wu failed to #%d", to);
    } else if (*player->argp[0] == 'p') {
	if (getele("your Gracious Deity", buf) <= 0) {
	    pr("Prayor aborted\n");
	    return RET_OK;
	}
	pr("\n");
	if (typed_wu(player->cnum, 0, buf, TEL_NORM) < 0)
	    logerror("tele: typed_wu failed to #0");
    } else {
	int kk;

	kk = 1;
	while (player->argp[kk] != (s_char *)0) {
	    if ((n = natarg(player->argp[kk], "for which country? ")) < 0) {
		if (opt_HIDDEN) {
		    if (n < -1) {
			return RET_OK;
		    } else {
			return RET_SYN;
		    }
		} else {
		    return RET_SYN;
		}
	    }
	    to = n;

	    if (kk == 1) {
		if (player->argp[2]) {
		    if (getele("multiple recipients", buf) < 0) {
			pr("Telegram aborted\n");
			return RET_OK;
		    }
		} else {
		    if (getele(cname(to), buf) < 0) {
			pr("Telegram aborted\n");
			return RET_OK;
		    }
		}
		pr("\n");
	    }

	    natp = getnatp(to);
	    if (((natp->nat_stat & STAT_NORM) == 0) &&
		((natp->nat_stat & STAT_SANCT) == 0)) {
		pr("%s has no \"telegram priveleges\".\n", cname(to));
		kk++;
		continue;
	    }
	    if (!player->god
		&& (getrejects(player->cnum, natp) & REJ_TELE)) {
		pr("%s is rejecting your telegrams.\n", cname(to));
		return RET_SYN;
	    }
	    teltype = /* player->god ? TEL_BULLETIN : */ TEL_NORM;
	    if (typed_wu(player->cnum, to, buf, teltype) < 0) {
		logerror("tele: typed_wu failed to #%d", n);
		return RET_FAIL;
	    }

	    if (!player->god &&
		(natp->nat_stat & GOD) != GOD && player->cnum != to)
		nreport(player->cnum, N_SENT_TEL, to, 1);
	    if (opt_HIDDEN) {
		setcont(to, player->cnum, FOUND_TELE);
	    }
	    kk++;
	}
    }
    return RET_OK;
}
