/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  distrea.c: Display treaty information
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "file.h"
#include "nat.h"
#include "nsc.h"
#include "player.h"
#include "prototypes.h"
#include "treaty.h"

int
distrea(int n, struct trtstr *tp)
{
    int i;
    int acond, bcond, cond;
    time_t now;

    if (tp->trt_status == TS_FREE)
	return 0;
    if (tp->trt_cna != player->cnum &&
	tp->trt_cnb != player->cnum && !player->god)
	return 0;
    (void)time(&now);
    if (now > tp->trt_exp) {
	tp->trt_status = TS_FREE;
	if (!puttre(n, tp)) {
	    pr("Couldn't save treaty; get help!\n");
	    return 0;
	}
	pr("Treaty #%d expired %s", n, ctime(&tp->trt_exp));
	return 0;
    }
    pr("\n      * * *  Empire Treaty #%d  * * *\n", n);
    if (tp->trt_status == TS_PROPOSED)
	pr("(proposed)\n");
    pr("between %s and ", cname(tp->trt_cna));
    pr("%s  expires %s", cname(tp->trt_cnb), ctime(&tp->trt_exp));
    pr("%24.24s terms", cname(tp->trt_cna));
    pr(" - %s terms\n", cname(tp->trt_cnb));
    for (i = 0; 0 != (cond = treaty_flags[i].value); i++) {
	acond = tp->trt_acond & cond;
	bcond = tp->trt_bcond & cond;
	if (acond | bcond) {
	    if (acond)
		pr("%30s", treaty_flags[i].name);
	    else
		pr("%30s", "");
	    if (bcond)
		pr(" - %s\n", treaty_flags[i].name);
	    else
		pr(" -\n");
	}
    }
    return 1;
}
