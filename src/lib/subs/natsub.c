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
 *  nat.c: Nation subroutines
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2009-2013
 *     Ron Koenderink, 2008-2009
 */

#include <config.h>

#include <ctype.h>
#include "file.h"
#include "nat.h"
#include "prototypes.h"

int
check_nat_name(char *cname, natid cnum)
{
    struct natstr *natp;
    natid cn;
    int allblank;
    char *p;

    if (strlen(cname) >= sizeof(natp->nat_cnam)) {
	pr("Country name too long\n");
	return 0;
    }

    allblank = 1;
    for (p = cname; *p != '\0'; p++) {
	if (iscntrl(*p)) {
	    pr("No control characters allowed in country names!\n");
	    return 0;
	} else if (!isspace(*p))
	    allblank = 0;
    }
    if (allblank) {
	pr("Country name can't be all blank\n");
	return 0;
    }

    for (cn = 0; NULL != (natp = getnatp(cn)); cn++) {
	if (cn != cnum && !strcmp(cname, natp->nat_cnam)) {
	    pr("Country #%d is already called `%s'\n", cn, cname);
	    return 0;
	}
    }
    return 1;
}

char *
prnat(struct natstr *np)
{
    return prbuf("%s (#%d)", np->nat_cnam, np->nat_cnum);
}

char *
prnatid(natid cnum)
{
    struct natstr *np = getnatp(cnum);

    if (CANT_HAPPEN(!np))
	return prbuf("%d (#%d)", cnum, cnum);
    return prnat(np);
}
