/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  dispatch.c: Actually execute the command given
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1998
 *     Markus Armbruster, 2007-2014
 */

#include <config.h>

#include "chance.h"
#include "com.h"
#include "empio.h"
#include "file.h"
#include "journal.h"
#include "match.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "server.h"

/*
 * Last command's PRNG seed.
 * Only used when running_test_suite.
 */
int test_suite_prng_seed;

/*
 * Execute command named by player->argp[0].
 * BUF is the raw UTF-8 command line.  It should have been passed to
 * parse() to set up player->argp.
 * If REDIR is not null, it's the command's redirection, in UTF-8.
 * Return -1 if the command is not unique or doesn't exist, else 0.
 */
int
dispatch(char *buf, char *redir)
{
    struct natstr *np;
    struct cmndstr *command;
    int cmd;

    cmd = comtch(player->argp[0], player_coms, player->nstat);
    if (cmd < 0) {
	if (cmd == M_NOTUNIQUE)
	    pr("Command \"%s\" is ambiguous -- ", player->argp[0]);
	else if (cmd == M_IGNORE)
	    return 0;
	else
	    pr("\"%s\" is not a legal command\n", player->argp[0]);
	return -1;
    }
    command = &player_coms[cmd];

    np = getnatp(player->cnum);
    if (np->nat_btu < command->c_cost && command->c_cost > 0) {
	if (player->god || opt_BLITZ)
	    np->nat_btu = max_btus;
	else {
	    pr("You don't have the BTU's, bozo\n");
	    return 0;
	}
    }
    if (!command->c_addr) {
	pr("Command not implemented\n");
	return 0;
    }
    player->may_sleep = command->c_flags & C_MOD
	? PLAYER_SLEEP_ON_INPUT : PLAYER_SLEEP_FREELY;
    player->command = command;
    empth_rwlock_rdlock(update_lock);

    /*
     * When running the test suite, reseed PRNG for each command with
     * a counter, to keep results stable even when the number of PRNs
     * consumed changes.
     */
    if (running_test_suite)
	seed_prng(++test_suite_prng_seed);

    if (redir) {
	prredir(redir);
	uprnf(buf);
	pr("\n");
    }
    journal_command(command->c_form);
    switch (command->c_addr()) {
    case RET_OK:
	player->btused += command->c_cost;
	break;
    case RET_FAIL:
	pr("command failed\n");
	player->btused += command->c_cost;
	break;
    case RET_SYN:
	pr("Usage: %s\n", command->c_form);
	break;
    default:
	CANT_REACH();
	break;
    }

    empth_rwlock_unlock(update_lock);
    player->command = NULL;
    if (player->may_sleep != PLAYER_SLEEP_NEVER || !io_eof(player->iop))
	player->may_sleep = PLAYER_SLEEP_FREELY;
    /* else we're being kicked out */
    return 0;
}
