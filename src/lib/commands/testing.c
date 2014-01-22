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
 *  testing.c: Commands for test-suite use
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2014
 */

#include <config.h>

#include "commands.h"
#include "testing.h"

int
testing_cmd(void)
{
    char *p;
    char buf[1024];

    p = getstarg(player->argp[1], "? ", buf);
    if (!p)
	return RET_SYN;
    if (!strcmp(p, "added")) {
	p = getstarg(player->argp[2], "number? ", buf);
	test_suite_prng_seed -= strtol(p, NULL, 10);
	p = getstarg(player->argp[3], "BTUs? ", buf);
	player->btused -= strtol(p, NULL, 10);
	p = getstarg(player->argp[4], "money? ", buf);
	player->dolcost -= strtod(p, NULL);
	return RET_OK;
    }
    return RET_SYN;
}
