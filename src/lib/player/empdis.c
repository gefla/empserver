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
    FILE *down_fp;
    struct telstr tgm;
    s_char buf[MAXTELSIZE];

    if (player->god)
	return 0;
    if ((down_fp = fopen(downfil, "rb")) == NULL)
	return 0;
    if (fread((void *)&tgm, sizeof(tgm), 1, down_fp) != 1) {
	logerror("bad header on login message (downfil)");
	fclose(down_fp);
	return 1;
    }
    if (tgm.tel_length >= (long)sizeof(buf)) {
	logerror("text length (%ld) is too long for login message (downfil)", tgm.tel_length);
	fclose(down_fp);
	return 1;
    }
    if (fread(buf, tgm.tel_length, 1, down_fp) != 1) {
	logerror("bad length %ld on login message", tgm.tel_length);
	fclose(down_fp);
	return 1;
    }
    buf[tgm.tel_length] = 0;
    prnf(buf);
    pr("\nThe game is down\n");
    fclose(down_fp);
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
getminleft(time_t now, int mpd)
{
    struct tm *tm;
    int nminleft;
    struct natstr *natp;
    int n;

    tm = localtime(&now);
    natp = getnatp(player->cnum);
    nminleft = mpd - natp->nat_minused;
    n = 60 * 24 - (tm->tm_min + tm->tm_hour * 60);
    if (n < nminleft)
	nminleft = n;
    return nminleft;
}
