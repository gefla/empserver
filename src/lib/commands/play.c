/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  play.c: Who is logged on?
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 *     Markus Armbruster, 2005-2011
 */

#include <config.h>

#include "optlist.h"
#include "commands.h"

static void play_header(void);
static int play_list(struct player *joe);

int
play(void)
{
    struct player *joe;
    int saw = 0;

    play_header();
    for (joe = player_prev(NULL); joe; joe = player_prev(joe)) {
	saw += play_list(joe);
    }
    if (player->god || opt_BLITZ)
	pr("%d player%s\n", saw, splur(saw));

    return RET_OK;
}

static void
play_header(void)
{
    prdate();
    pr("%9s %3s %-32s %5s %5s %-20s\n",
       "", "#", "", "time", "idle", "last command");
}

static int
play_list(struct player *joe)
{
    time_t now;
    char com[1 + 6*20 + 2];	/* UTF-8 */
    struct natstr *natp;
    struct natstr *us;
    int n;

    if (joe->cnum >= MAXNOC || !(natp = getnatp(joe->cnum)))
	return 0;

    us = getnatp(player->cnum);
    if (player->god) {
	/* We are a god, we see everything */
    } else if (opt_BLITZ) {
	/* It's a blitz, we see everything */
    } else if (joe->god) {
	/* This country is a god, so we see it */
    } else if (us->nat_stat == STAT_VIS) {
	/* We are a visitor country, we can't see squat, except deities */
	return 0;
    } else if (joe->cnum != player->cnum) {
	/* This isn't us.  Can we see it? */
	if (natp->nat_stat == STAT_VIS) {
	    /* Yes, we can see visitors are logged on */
	} else if (relations_with(joe->cnum, player->cnum) < ALLIED) {
	    /* This is a non-allied country, don't show it. */
	    return 0;
	}
    }

    time(&now);
    pr("%-9.9s %3d %32.32s %2d:%02d %4lds",
       cname(joe->cnum),
       joe->cnum,
       player->god || joe->cnum == player->cnum ? praddr(joe) : "",
       natp->nat_timeused / 3600,
       (natp->nat_timeused % 3600) / 60,
       (long)(now - joe->curup));

    if (player->god) {
	n = ufindpfx(joe->combuf, 20);
	if (CANT_HAPPEN(n + 3u > sizeof(com))) {
	    pr(" BUGGY\n");
	    return 1;
	}
	sprintf(com, " %.*s\n", n, joe->combuf);
	uprnf(com);
    } else
	pr("\n");

    return 1;
}
