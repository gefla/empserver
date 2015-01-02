/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  empdis.c: Empire dispatcher stuff
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 2000
 *     Markus Armbruster, 2006-2010
 *     Ron Koenderink, 2004-2009
 */

#include <config.h>

#include <stdio.h>
#include <time.h>
#include "com.h"
#include "file.h"
#include "game.h"
#include "match.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"


#define KEEP_COMMANDS 50

/* ring buffer of most recent command prompts and commands, user text */
static char player_commands[KEEP_COMMANDS][1024 + 8];

/* the slot holding the most recent command in player_commands[] */
static int player_commands_index = 0;

static void disable_coms(void);

/*
 * Get a command from the current player into COMBUFP[1024], in UTF-8.
 * This may block for input, yielding the processor.  Flush buffered
 * output when blocking, to make sure player sees the prompt.
 * Return command's byte length on success, -1 on error.
 */
int
getcommand(char *combufp)
{
    char buf[1024];		/* user text */

    if (recvclient(buf, sizeof(buf)) < 0)
	return -1;

    if (++player_commands_index >= KEEP_COMMANDS)
	player_commands_index = 0;
    sprintf(player_commands[player_commands_index], "%3d %3d %s",
	    player_commands_index, player->cnum, buf);

    if (player->flags & PF_UTF8)
	return copy_utf8_no_funny(combufp, buf);
    return copy_ascii_no_funny(combufp, buf);
}

void
init_player_commands(void)
{
    int i;

    for (i = 0; i < KEEP_COMMANDS; ++i)
	*player_commands[i] = 0;

    disable_coms();
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
    struct cmndstr *com;

    pr("\t\tCurrent EMPIRE Command List\n"
       "\t\t------- ------ ------- ----\n"
       "Initial number is cost in B.T.U. units.\n"
       "Next 2 chars (if present) are:\n"
       "$ - must be non-broke\tc -- must have capital\n"
       "Args in [brackets] are optional.\n"
       "All-caps args in <angle brackets>"
       " have the following meanings:\n"
       /* FIXME incomplete */
       "  <NUM> :: a number in unspecified units\n"
       "  <COMM> :: a commodity such as `food', `guns', etc\n"
       "  <TYPE> :: an item type such as `ship', `plane', etc\n");
    for (com = player_coms; com->c_form; com++) {
	if ((com->c_permit & player->nstat) == com->c_permit) {
	    pr("%2d ", com->c_cost);
	    if (com->c_permit & MONEY)
		pr("$");
	    else
		pr(" ");
	    if (com->c_permit & CAP)
		pr("c");
	    else
		pr(" ");
	    pr(" %s\n", com->c_form);
	}
    }
    pr("For further info on command syntax see \"info Syntax\".\n");
    return RET_OK;
}

static void
disable_coms(void)
{
    char *tmp = strdup(disabled_commands);
    char *name;
    int cmd;

    for (name = strtok(tmp, " \t"); name; name = strtok(NULL, " \t")) {
	cmd = comtch(name, player_coms, -1);
	if (cmd < 0) {
	    logerror("Warning: not disabling %s command %s\n",
		     cmd == M_NOTUNIQUE ? "ambiguous" : "unknown", name);
	    continue;
	}
	player_coms[cmd].c_permit |= GOD;
    }

    free(tmp);
}

static int
seconds_since_midnight(time_t time)
{
    struct tm *tm = localtime(&time);

    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    tm->tm_isdst = -1;
    return time - mktime(tm);
}

void
update_timeused_login(time_t now)
{
    struct natstr *natp = getnatp(player->cnum);
    time_t midnight_secs = seconds_since_midnight(now);

    if (now - natp->nat_last_logout > midnight_secs) {
	natp->nat_timeused = 0;
	putnat(natp);
    }
    player->lasttime = now;
}

void
update_timeused(time_t now)
{
    struct natstr *natp = getnatp(player->cnum);
    time_t midnight_secs = seconds_since_midnight(now);
    time_t dt = now - player->lasttime;

    if (dt > midnight_secs)
	natp->nat_timeused = midnight_secs;
    else
	natp->nat_timeused += dt;
    player->lasttime = now;
    putnat(natp);
}

void
enforce_minimum_session_time(void)
{
    struct natstr *natp = getnatp(player->cnum);

    time_t dt = natp->nat_last_logout - natp->nat_last_login;
    if (dt > seconds_since_midnight(natp->nat_last_logout))
	dt = seconds_since_midnight(natp->nat_last_logout);
    if (dt < 15)
	natp->nat_timeused += 15 - dt;
    putnat(natp);
}

int
may_play_now(struct natstr *natp, time_t now)
{
    if (CANT_HAPPEN(natp->nat_cnum != player->cnum))
	return 0;

    if (gamehours(now)) {
	if (player->flags & PF_HOURS) {
	    pr("\nEmpire hours restriction lifted\n");
	    player->flags &= ~PF_HOURS;
	}
    } else {
	if (!(player->flags & PF_HOURS)) {
	    pr("\nEmpire hours restriction in force\n");
	    player->flags |= PF_HOURS;
	}
	if (natp->nat_stat != STAT_GOD)
	    return 0;
    }

    if (game_play_disabled()) {
	if (!(player->flags & PF_DOWN)) {
	    show_first_tel(downfil);
	    pr("\nThe game is down\n");
	    player->flags |= PF_DOWN;
	}
	if (natp->nat_stat != STAT_GOD)
	    return 0;
    } else
	player->flags &= ~PF_DOWN;

    if ((natp->nat_stat != STAT_GOD && natp->nat_stat != STAT_VIS)
	&& natp->nat_timeused > m_m_p_d * 60) {
	pr("Max minutes per day limit exceeded.\n");
	return 0;
    }
    return 1;
}
