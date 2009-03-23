/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sarg.c: Parse selection arguments
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Markus Armbruster, 2004-2008
 */

#include <config.h>

#include <ctype.h>
#include "file.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "xy.h"

/*
 * returns one of
 *
 *  #1, lx:ly,hx:hy --> NS_AREA
 *  @x,y:dist  --> NS_DIST
 *  %d or %d/%d/%d --> NS_LIST
 *  ~ or letter --> NS_GROUP
 *  * --> NS_ALL
 *
 * or 0 for none of the above.
 */
enum ns_seltype
sarg_type(char *str)
{
    int c;

    c = *str;
    if (c == '@')
	return NS_DIST;
    if (c == '*')
	return NS_ALL;
    if (c == '#' || strchr(str, ','))
	return NS_AREA;
    if (isdigit(c))
	return NS_LIST;
    if (c == '~' || isalpha(c))
	return NS_GROUP;
    return NS_UNDEF;
}

int
sarg_xy(char *str, coord *xp, coord *yp)
{
    coord x, y;
    struct natstr *np;

    x = strtox(str, &str);
    if (x < 0 || *str++ != ',')
	return 0;
    y = strtoy(str, &str);
    if (y < 0 || (*str != 0 && !isspace(*str)))
      return 0;
    if ((x ^ y) & 1)
	return 0;
    np = getnatp(player->cnum);
    *xp = xabs(np, x);
    *yp = yabs(np, y);
    return 1;
}

/*
 * translate #1 or lx:ly,hx:hy into a result range struct
 * returns absolute coords
 */
int
sarg_area(char *str, struct range *rp)
{
    long rlm;
    struct natstr *np;
    struct realmstr realm;
    char *end;

    if (*str == '#') {
	/*
	 * realm #X where (X > 0 && X < MAXNOR)
	 * Assumes realms are in abs coordinates
	 */
	if (*++str) {
	    rlm = strtol(str, &end, 10);
	    if (end == str || (*end != 0 && !isspace(*end))
		|| rlm < 0 || MAXNOR <= rlm)
		return 0;
	} else
	    rlm = 0;
	getrealm(rlm, player->cnum, &realm);
	rp->lx = realm.r_xl;
	rp->hx = realm.r_xh;
	rp->ly = realm.r_yl;
	rp->hy = realm.r_yh;
    } else {
	/*
	 * full map specification
	 * LX:LY,HX:HY where
	 * ly, hy are optional.
	 */
	rp->lx = rp->hx = strtox(str, &str);
	if (rp->lx < 0)
	    return 0;
	if (*str == ':') {
	    rp->hx = strtox(str + 1, &str);
	    if (rp->hx < 0)
		return 0;
	}
	if (*str++ != ',')
	    return 0;
	rp->ly = rp->hy = strtoy(str, &str);
	if (rp->ly < 0)
	    return 0;
	if (*str == ':') {
	    rp->hy = strtoy(str + 1, &str);
	    if (rp->hy < 0)
		return 0;
	}
	if (*str != 0 && !isspace(*str))
	    return 0;
	np = getnatp(player->cnum);
	rp->lx = xabs(np, rp->lx);
	rp->hx = xabs(np, rp->hx);
	rp->ly = yabs(np, rp->ly);
	rp->hy = yabs(np, rp->hy);
    }
    xysize_range(rp);
    return 1;
}

/*
 * translate @x,y:int into
 * result params
 */
int
sarg_range(char *str, coord *xp, coord *yp, int *dist)
{
    coord x, y;
    long d;
    char *end;
    struct natstr *np;

    if (*str++ != '@')
	return 0;
    x = strtox(str, &str);
    if (x < 0 || *str++ != ',')
	return 0;
    y = strtoy(str, &str);
    if (y < 0 || *str++ != ':')
	return 0;
    d = strtol(str, &end, 10);
    if (end == str || (*end != 0 && !isspace(*end)) || d < 0)
	return 0;
    *dist = d;
    np = getnatp(player->cnum);
    *xp = xabs(np, x);
    *yp = yabs(np, y);
    return 1;
}

/*
 * list of idents; id/id/id/id/id
 */
int
sarg_list(char *str, int *list, int max)
{
    int i, j;
    long n;
    char *end;

    i = 0;
    do {
	n = strtol(str, &end, 10);
	if (end == str || n < 0) {
	    pr("Illegal character '%c'\n", *str);
	    return 0;
	}
	for (j = 0; j < i; j++) {
	    if (list[j] == n)
		break;
	}
	if (j == i) {
	    if (i >= max) {
		pr("List too long (limit is %d)\n", max);
		return 0;
	    }
	    list[i++] = n;
	}
	str = end;
    } while (*str++ == '/');

    if (str[-1] != 0 && !isspace(str[-1])) {
	pr("Expecting '/', got '%c'\n", str[-1]);
	return 0;
    }
    return i;
}
