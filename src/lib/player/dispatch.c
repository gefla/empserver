/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  dispatch.c: Actually execute the command given
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1998
 */

#include <config.h>

#include "prototypes.h"
#include "misc.h"
#include "player.h"
#include "com.h"
#include "match.h"
#include "nat.h"
#include "file.h"
#include "proto.h"
#include "empio.h"
#include "optlist.h"
#include "subs.h"
#include "common.h"
#include "server.h"

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

    cmd = comtch(player->argp[0], player_coms, player->ncomstat);
    if (cmd < 0) {
	if (cmd == M_NOTUNIQUE)
	    pr("Command \"%s\" is ambiguous -- ", player->argp[0]);
	else if (cmd == M_IGNORE)
	    return 0;
	else {
	    pr("\"%s\" is not a legal command ", player->argp[0]);
	    if (player->nstat != player->ncomstat)
		pr("now ");
	    pr("\n");
	}
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
    if (command->c_addr == 0) {
	pr("Command not implemented\n");
	return 0;
    }
    if (update_pending) {
	pr("Update in progress...command failed\n");
	return 0;
    }
    if (redir) {
	prredir(redir);
	uprnf(buf);
	pr("\n");
    }
    player->command = command;
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
	logerror("%s: returned bad value", command->c_form);
	break;
    }
    player->command = 0;
    return 0;
}
