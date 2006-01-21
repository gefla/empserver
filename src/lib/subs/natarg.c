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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  natarg.c: Return countr # given country name or country #
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2006
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "nat.h"
#include "player.h"
#include "file.h"
#include "match.h"
#include "prototypes.h"
#include "optlist.h"

/*
 * Get nation argument.
 * If ARG is not empty, use it, else prompt for input using PROMPT.
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
    if (arg == 0 || *arg == 0)
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
 * If ARG is not empty, use it, else prompt for input using PROMPT.
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
	if (!player->god && !getcontact(getnatp(player->cnum), np->nat_cnum)) {
	    if (np->nat_stat != STAT_GOD) {
		pr("Country '%s' has not been contacted.\n", arg);
		return -1;
	    }
	}
    }
    return np->nat_cnum;
}
