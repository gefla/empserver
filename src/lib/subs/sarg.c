/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sarg.c: Parse selection arguments
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include <ctype.h>
#include <string.h>
#include "misc.h"
#include "player.h"
#include "xy.h"
#include "nsc.h"
#include "sect.h"
#include "nat.h"
#include "file.h"
#include "prototypes.h"
#include "optlist.h"

/*
 * returns one of
 *
 *  #1, lx:ly,hx:hy --> NS_AREA
 *  @x,y:dist  --> NS_DIST
 *  %d or %d/%d/%d --> NS_LIST
 *  * --> NS_ALL
 *
 * or 0 for none of the above.
 */
int
sarg_type(s_char *ptr)
{
    int c;

    c = *ptr;
    if (c == '@')
	return NS_DIST;
    if (c == '*')
	return NS_ALL;
    if (c == '#' || strchr(ptr, ',') != 0)
	return NS_AREA;
    if (isdigit(c))
	return NS_LIST;
    if (c == '~' || isupper(c) || islower(c))
	return NS_GROUP;
    return 0;
}

int
sarg_xy(s_char *ptr, coord *xp, coord *yp)
{
    if (sarg_type(ptr) != NS_AREA)
	return 0;
    *xp = atoip(&ptr);
    if (*ptr++ != ',')
	return 0;
    if (!isdigit(*ptr) && *ptr != '-')
	return 0;
    *yp = atoi(ptr);
    inputxy(xp, yp, player->cnum);
    if ((*xp ^ *yp) & 01)
	return 0;
    return 1;
}

/* returns absolute coords */
static int
sarg_getrange(s_char *buf, register struct range *rp)
{
    register int rlm;
    register int c;
    struct natstr *np;
    s_char *bp;

    bp = buf;
    c = *bp;
    if (c == '#') {
	/*
	 * realm #X where (X > 0 && X < MAXNOR)
	 * Assumes realms are in abs coordinates
	 */
	bp++;
	rlm = atoi(bp);
	if (rlm < 0 || rlm >= MAXNOR)
	    return 0;
	np = getnatp(player->cnum);
	rp->lx = np->nat_b[rlm].b_xl;
	rp->hx = np->nat_b[rlm].b_xh;
	rp->ly = np->nat_b[rlm].b_yl;
	rp->hy = np->nat_b[rlm].b_yh;
    } else {
	/*
	 * full map specification
	 * LX:LY,HX:HY where
	 * ly, hy are optional.
	 */
	if (!isdigit(c) && c != '-')
	    return 0;
	rp->lx = rp->hx = atoip(&bp);
	if (*bp == ':') {
	    bp++;
	    rp->hx = atoip(&bp);
	}
	if (*bp++ != ',')
	    return 0;
	if (!isdigit(c) && c != '-')
	    return 0;
	rp->ly = rp->hy = atoip(&bp);
	if (*bp == ':') {
	    bp++;
	    rp->hy = atoip(&bp);
	}
	inputxy(&rp->lx, &rp->ly, player->cnum);
	inputxy(&rp->hx, &rp->hy, player->cnum);
    }
    xysize_range(rp);
    return 1;
}

/*
 * translate #1 or lx:ly,hx:hy into
 * a result range struct
 */
int
sarg_area(s_char *buf, register struct range *rp)
{
    if (!sarg_getrange(buf, rp))
	return 0;
    rp->hx += 1;
    if (rp->hx >= WORLD_X)
	rp->hx = 0;
    rp->hy += 1;
    if (rp->hy >= WORLD_Y)
	rp->hy = 0;
    xysize_range(rp);
    return 1;
}

/*
 * translate @x,y:int into
 * result params
 */
int
sarg_range(s_char *buf, coord *xp, coord *yp, int *dist)
{
    s_char *bp;

    bp = buf;
    if (bp == 0 || *bp == 0)
	return 0;
    if (*bp++ != '@')
	return 0;
    *xp = atoip(&bp);
    if (*bp++ != ',')
	return 0;
    *yp = atoip(&bp);
    if (*bp++ != ':')
	return 0;
    inputxy(xp, yp, player->cnum);
    *dist = atoi(bp);
    return 1;
}

/*
 * list of idents; id/id/id/id/id
 */
int
sarg_list(s_char *str, register int *list, int max)
{
    register int i;
    register int j;
    register int n;
    s_char *arg;

    arg = str;
    for (i = 0; i < max; i++) {
	if (!isdigit(*arg)) {
	    pr("Illegal character '%c'\n", *arg);
	    return 0;
	}
	n = atoip(&arg);
	for (j = 0; j < i; j++) {
	    if (list[j] == n)
		break;
	}
	if (j != i) {
	    /* duplicate; ignore */
	    i--;
	} else
	    list[i] = n;
	if (*arg == 0)
	    break;
	if (*arg != '/') {
	    pr("Expecting '/', got '%c'\n", *arg);
	    return 0;
	}
	arg++;
	if (*arg == 0)
	    break;
    }
    return i + 1;
}
