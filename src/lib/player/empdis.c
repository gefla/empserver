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
 *  empdis.c: Empire dispatcher stuff
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 2000
 */

#include "prototypes.h"
#include <stdio.h>
#include "misc.h"
#include "player.h"
#include "nat.h"
#include "tel.h"
#include "proto.h"
#include "com.h"
#include "keyword.h"
#include "file.h"
#include "empio.h"
#include "subs.h"
#include "common.h"
#include "optlist.h"

#include <fcntl.h>
#include <time.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <signal.h>

#define KEEP_COMMANDS 50
s_char player_commands[KEEP_COMMANDS][1024 + 8];
int player_commands_index = 0;

int
getcommand(s_char *combufp)
{
    struct natstr *natp;
    s_char buf[1024];

/* Note this now assumes a 1024 byte buffer is being passed in */
    natp = getnatp(player->cnum);
    if (++player_commands_index >= KEEP_COMMANDS)
	player_commands_index = 0;
    sprintf(player_commands[player_commands_index], "%3d %3d [prompt]",
	    player_commands_index, player->cnum);
    do {
	prprompt(natp->nat_minused, natp->nat_btu);
	buf[0] = 0;
	if (recvclient(buf, 1024) < 0) {
	    return -1;
	}
    } while (buf[0] == 0);
    if (++player_commands_index >= KEEP_COMMANDS)
	player_commands_index = 0;
    sprintf(player_commands[player_commands_index], "%3d %3d %s",
	    player_commands_index, player->cnum, buf);
    strcpy(combufp, buf);
    return (strlen(combufp));
}

void
init_player_commands(void)
{
    int i;

    for (i = 0; i < KEEP_COMMANDS; ++i)
	*player_commands[i] = 0;
}

void
log_last_commands(void)
{
    int i;

    logerror("Most recent player commands:");
    for (i = player_commands_index; i >= 0; --i)
	if (*player_commands[i])
	    logerror("%s", player_commands[i] + 4);
    for (i = KEEP_COMMANDS - 1; i > player_commands_index; --i)
	if (*player_commands[i])
	    logerror("%s", player_commands[i] + 4);
}

int
explain(void)
{
    register s_char *format;
    register int i;

    pr("\t\tCurrent EMPIRE Command List\n");
    pr("\t\t------- ------ ------- ----\n");
    pr("Initial number is cost in B.T.U. units.\n");
    pr("Next 2 chars (if present) are:\n");
    pr("$ - must be non-broke\tc -- must have capital\n");
    pr("Args in [brackets] are optional.\n");
    if (player->nstat > 4) {
	pr("All-caps args in <angle brackets>");
	pr(" have the following meanings:\n");
	pr("  <NUM> :: a number in unspecified units\n");
	pr("  <COMM> :: a commodity such as `food', `guns', etc\n");
	pr("  <VAR> :: a commodity such as `food', `guns', etc\n");
	pr("  <TYPE> :: an item type such as `ship', `plane', etc\n");
    }
    for (i = 0; (format = player_coms[i].c_form) != 0; i++) {
	if ((player_coms[i].c_permit & player->ncomstat) ==
	    player_coms[i].c_permit) {
	    pr("%2d ", player_coms[i].c_cost);
	    if ((player_coms[i].c_permit & MONEY) == MONEY)
		pr("$");
	    else
		pr(" ");
	    if ((player_coms[i].c_permit & CAP) == CAP)
		pr("c");
	    else
		pr(" ");
	    pr(" %s\n", format);
	}
    }
    pr("For further info on command syntax see \"info Syntax\".\n");
    return RET_OK;
}

/*
 * returns true if down
 */
int
gamedown(void)
{
    int downf;
    struct telstr tgm;
    s_char buf[1024];

    if (player->god)
	return 0;
#if !defined(_WIN32)
    if ((downf = open(downfil, O_RDONLY, 0)) < 0)
#else
    if ((downf = open(downfil, O_RDONLY | O_BINARY, 0)) < 0)
#endif
	return 0;
    if (read(downf, (s_char *)&tgm, sizeof(tgm)) != sizeof(tgm)) {
	logerror("bad header on login message (downfil)");
	close(downf);
	return 1;
    }
    if (read(downf, buf, tgm.tel_length) != tgm.tel_length) {
	logerror("bad length %ld on login message", tgm.tel_length);
	close(downf);
	return 1;
    }
    if (tgm.tel_length >= (long)sizeof(buf))
	tgm.tel_length = sizeof(buf) - 1;
    buf[tgm.tel_length] = 0;
    pr(buf);
    pr("\nThe game is down\n");
    (void)close(downf);
    return 1;
}

void
daychange(time_t now)
{
    struct natstr *natp;
    struct tm *tm;

    natp = getnatp(player->cnum);
    tm = localtime(&now);
    if ((tm->tm_yday % 128) != natp->nat_dayno) {
	natp->nat_dayno = tm->tm_yday % 128;
	natp->nat_minused = 0;
    }
}

int
getminleft(time_t now, int *hour, int *mpd)
{
    s_char *bp;
    struct tm *tm;
    int nminleft;
    int curtime;
    struct natstr *natp;
    int n;

    tm = localtime(&now);
    curtime = tm->tm_min + tm->tm_hour * 60;
    if (NULL != (bp = kw_find("minutes")))
	kw_parse(CF_VALUE, bp, mpd);
    natp = getnatp(player->cnum);
    nminleft = *mpd - natp->nat_minused;
    if (NULL != (bp = kw_find("hours"))) {
	/*
	 * assume hours has already been set; just verify
	 * that it is present
	 */
	n = hour[1] - curtime;
	if (n < nminleft)
	    nminleft = n;
    }
    n = 60 * 24 - (tm->tm_min + tm->tm_hour * 60);
    if (n < nminleft)
	nminleft = n;
    return nminleft;
}
