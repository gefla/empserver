/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  relo.c: Re-read (some) configuration files
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include "commands.h"
#include "journal.h"
#include "server.h"

int
relo(void)
{
    /*
     * Like SIGHUP, plus friendly chatter.  If you change anything
     * here, also update the code that handles SIGHUP!
     */

    if (journal_reopen() < 0) {
	pr("Can't reopen journal");
	return RET_FAIL;
    }
    pr("Journal reopened.\n");

    update_reschedule();
    pr("Reload of update schedule requested.\n");

    if (logreopen() < 0) {
	pr("Can't reopen log");
	return RET_FAIL;
    }
    pr("Log reopened.\n");

    return RET_OK;
}
