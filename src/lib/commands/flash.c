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
 *  flash.c: Flash a message to another player
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include <time.h>
#include "misc.h"
#include "player.h"
#include "nat.h"
#include "file.h"
#include "commands.h"

int
flash(void)
{
    struct natstr *us;
    struct natstr *to;
    s_char buf[1024];
    int tocn;
    s_char *sp;

    us = getnatp(player->cnum);
    if ((tocn = natarg(player->argp[1], "to which country? ")) < 0)
	return RET_SYN;
    if (!(to = getnatp((natid)tocn))) {
	pr("Bad country number\n");
	return RET_SYN;
    }

    if (us->nat_stat & STAT_GOD) {
	/* We are gods, we can flash anyone */
    } else if (us->nat_stat == VIS) {
	/* We are a visitor.  We can only flash the gods. :) */
	if (!(to->nat_stat & STAT_GOD)) {
	    pr("Visitors can only flash the gods.\n");
	    return RET_SYN;
	}
    } else {
	/* Ok, we are a normal country, can we flash them? */
	if ((!(to->nat_stat & STAT_GOD)) &&
	    (getrel(to, player->cnum) < FRIENDLY)) {
	    pr("%s is not a deity or friendly with us.\n", to->nat_cnam);
	    return RET_SYN;
	}
    }

    if (player->argp[2]) {
	for (sp = &player->combuf[0]; *sp && *sp != ' '; ++sp) ;
	for (++sp; *sp && *sp != ' '; ++sp) ;
	sprintf(buf, ":%s", sp);
	sendmessage(us, to, buf, 1);
    } else {
	sendmessage(us, to, "...", 1);
	while (getstring("> ", buf)) {
	    if (*buf == '.')
		break;
	    sendmessage(us, to, buf, 0);
	}
	sendmessage(us, to, "<EOT>", 0);
    }
    return RET_OK;
}

int
wall(void)
{
    struct natstr *us;
    s_char buf[1024];
    s_char *sp;

    us = getnatp(player->cnum);
    if (player->argp[1]) {
	for (sp = &player->combuf[0]; *sp && *sp != ' '; ++sp) ;
	sprintf(buf, ":%s", sp);
	sendmessage(us, 0, buf, 1);
    } else {
	sendmessage(us, 0, "...", 1);
	while (getstring("> ", buf)) {
	    if (*buf == '.')
		break;
	    sendmessage(us, 0, buf, 0);
	}
	sendmessage(us, 0, "<EOT>", 0);
    }
    return RET_OK;
}

int
sendmessage(struct natstr *us, struct natstr *to, char *message,
	    int oneshot)
{
    struct player *other;
    struct tm *tm;
    char *p;
    char c;
    time_t now;
    int sent = 0;
    struct natstr *wto;

    for (p = message; 0 != (c = *p); p++) {
	if (!isprint(c))
	    *p = '*';
    }
    if (strlen(message) > 60) {
	s_char c = message[60];
	message[60] = '\0';
	sendmessage(us, to, message, oneshot);
	message[60] = c;
	sendmessage(us, to, &message[60], 0);
	return 0;
    }
    time(&now);
    tm = localtime(&now);
    for (other = player_next(0); other != 0; other = player_next(other)) {
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
	if (oneshot)
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
	player_wakeup(other);
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
