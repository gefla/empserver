/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  flash.c: Flash a message to another player
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2004-2009
 */

#include <config.h>

#include "commands.h"

static int chat(struct natstr *, struct natstr *, char *);
static int sendmessage(struct natstr *, struct natstr *, char *, int);

int
flash(void)
{
    struct natstr *us;
    struct natstr *to;
    int tocn;

    us = getnatp(player->cnum);
    if ((tocn = natarg(player->argp[1], "to which country? ")) < 0)
	return RET_SYN;
    to = getnatp(tocn);

    if (us->nat_stat == STAT_GOD) {
	/* We are gods, we can flash anyone */
    } else if (us->nat_stat == STAT_VIS) {
	/* We are a visitor.  We can only flash the gods. :) */
	if (to->nat_stat != STAT_GOD) {
	    pr("Visitors can only flash the gods.\n");
	    return RET_SYN;
	}
    } else {
	/* Ok, we are a normal country, can we flash them? */
	if (to->nat_stat != STAT_GOD && getrel(to, player->cnum) < FRIENDLY) {
	    pr("%s is not a deity or friendly with us.\n", to->nat_cnam);
	    return RET_SYN;
	}
    }

    return chat(us, to, player->comtail[2]);
}

int
wall(void)
{
    return chat(getnatp(player->cnum), NULL, player->comtail[1]);
}

/*
 * Send flash message(s) from US to TO.
 * Null TO broadcasts to all.
 * MESSAGE is UTF-8.  If it is null, prompt for messages interactively.
 * Return RET_OK.
 */
static int
chat(struct natstr *us, struct natstr *to, char *message)
{
    char buf[1024];		/* UTF-8 */

    if (message) {
	buf[0] = ':';
	buf[1] = ' ';
	strcpy(buf+2, message);
	sendmessage(us, to, buf, 1);
    } else {
	sendmessage(us, to, "...", 1);
	while (ugetstring("> ", buf)) {
	    if (*buf == '.')
		break;
	    sendmessage(us, to, buf, 0);
	}
	sendmessage(us, to, "<EOT>", 0);
    }
    return RET_OK;
}

/*
 * Send flash message MESSAGE from US to TO.
 * MESSAGE is UTF-8.
 * Null TO broadcasts to all.
 * A header identifying US is prepended to the message.  It is more
 * verbose if VERBOSE.
 */
static int
sendmessage(struct natstr *us, struct natstr *to, char *message, int verbose)
{
    struct player *other;
    struct tm *tm;
    time_t now;
    int sent = 0;
    struct natstr *wto;

    time(&now);
    tm = localtime(&now);
    for (other = player_next(NULL); other; other = player_next(other)) {
	if (other->state != PS_PLAYING)
	    continue;
	if (to && other->cnum != to->nat_cnum)
	    continue;
	if (!(wto = getnatp(other->cnum)))
	    continue;
	if (!to && !player->god && getrel(wto, player->cnum) != ALLIED)
	    continue;
	if (!player->god && !(wto->nat_flags & NF_FLASH))
	    continue;
	if (player == other)
	    continue;
	if (verbose)
	    if (to)
		pr_flash(other, "FLASH from %s (#%d) @ %02d:%02d%s\n",
			 us->nat_cnam, us->nat_cnum, tm->tm_hour,
			 tm->tm_min, message);
	    else
		pr_flash(other, "BROADCAST from %s (#%d) @ %02d:%02d%s\n",
			 us->nat_cnam, us->nat_cnum, tm->tm_hour,
			 tm->tm_min, message);

	else
	    pr_flash(other, "%s (#%d): %s\n",
		     us->nat_cnam, us->nat_cnum, message);
	sent++;
    }
    if (player->god) {
	if (to)
	    if (sent)
		pr("Flash sent to %s\n", to->nat_cnam);
	    else
		pr("%s is not logged on\n", to->nat_cnam);
	else if (sent)
	    pr("Broadcast sent to %d players\n", sent);
	else
	    pr("No-one is logged in\n");
    }
    if (to && !player->god) {
	/* If they are allied with us, we would normally see that
	 * they are logged in anyway, so just tell us */
	if ((getrel(to, player->cnum) == ALLIED) && !sent) {
	    if (to->nat_flags & NF_FLASH)
		pr("%s is not logged on\n", to->nat_cnam);
	    else
		pr("%s is not accepting flashes\n", to->nat_cnam);
	}
    }
    return 0;
}
