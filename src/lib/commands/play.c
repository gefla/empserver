/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  play.c: Who is logged on?
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "file.h"
#include "nat.h"
#include "optlist.h"
#include "commands.h"

static void play_header(void);
static int play_list(struct player *joe);

int
play(void)
{
    struct player *joe;
    int saw = 0;
    int count = 0;

    for (joe = player_prev(0); joe; joe = player_prev(joe)) {
	if (!count++)
	    play_header();
	saw += play_list(joe);
    }
    if (player->god || opt_BLITZ)
	pr("%d player%s\n", count, splur(count));

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
    s_char *com;
    struct natstr *natp;
    struct natstr *us;

    if (joe->cnum >= MAXNOC || !(natp = getnatp(joe->cnum)))
	return 0;

    us = getnatp(player->cnum);
    if (player->god) {
	/* We are a god, we see everything */
    } else if (opt_BLITZ) {
	/* It's a blitz, we see everything */
    } else if (joe->god) {
	/* This country is a god, so we see it */
    } else if (us->nat_stat == VIS) {
	/* We are a visitor country, we can't see squat, except deities */
	return 0;
    } else if (joe->cnum != player->cnum) {
	/* This isn't us.  Can we see it? */
	if (natp->nat_stat == VIS) {
	    /* Yes, we can see visitors are logged on */
	} else if (getrel(natp, player->cnum) < ALLIED) {
	    /* This is a non-allied country, don't show it. */
	    return 0;
	}
    }

    time(&now);
    if (player->god) {
	if (!joe->combuf || !*joe->combuf)
	    com = "NULL";
	else
	    com = joe->combuf;
    } else
	com = "";


    pr("%-9.9s %3d %32.32s %2d:%02d %4lds %-20.20s\n",
       cname(joe->cnum),
       joe->cnum,
       player->god || joe->cnum == player->cnum ? praddr(joe) : "",
       natp->nat_minused / 60,
       natp->nat_minused % 60,
       (long)(now - joe->curup),
       com);
    return 1;
}
