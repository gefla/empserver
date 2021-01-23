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
 *  natarg.c: Return countr # given country name or country #
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2016
 */

#include <config.h>

#include <ctype.h>
#include <stdlib.h>
#include "match.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"

/*
 * Get nation argument.
 * If @arg is not empty, use it
 * Else prompt for input using @prompt.
 * If no input is provided, return NULL.
 * If the argument identifies a country, return its getnatp() value.
 * Else complain and return NULL.
 * Caution: this function doesn't care for lack of contact.
 */
struct natstr *
natargp(char *arg, char *prompt)
{
    char buf[1024];
    int n;
    struct natstr *np;

    arg = getstarg(arg, prompt, buf);
    if (!arg || !*arg)
	return NULL;
    if (isdigit(*arg))
	n = atoi(arg);
    else {
	n = cnumb(arg);
	if (n == M_NOTUNIQUE) {
	    pr("Country '%s' is ambiguous\n", arg);
	    return NULL;
	}
    }
    np = getnatp(n);
    if (!np || np->nat_stat == STAT_UNUSED) {
	pr("Country '%s' doesn't exist.\n", arg);
	return NULL;
    }
    return np;
}

/*
 * Get nation argument.
 * If @arg is not empty, use it.
 * Else prompt for input using @prompt.
 * If no input is provided, return -1.
 * If the argument identifies a country, return its number.  getnatp()
 * can be assumed to succeed for this number.
 * Else complain and return -1.
 * If HIDDEN is enabled, countries not contacted are not eligible
 * unless the player is a deity.
 */
int
natarg(char *arg, char *prompt)
{
    struct natstr *np = natargp(arg, prompt);
    if (!np)
	return -1;
    if (opt_HIDDEN) {
	if (!player->god
	    && !in_contact(player->cnum, np->nat_cnum)) {
	    if (np->nat_stat != STAT_GOD) {
		pr("Country '%s' has not been contacted.\n", arg);
		return -1;
	    }
	}
    }
    return np->nat_cnum;
}
