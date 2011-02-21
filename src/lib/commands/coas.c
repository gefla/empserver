/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  coas.c: Look at all the ships in the world
 *
 *  Known contributors to this file:
 *     Keith Muller, 1983
 *     Dave Pare, 1986 (rewrite)
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"
#include "ship.h"

#define TSIZE	200

struct coast {
    struct coast *c_next;
    int c_spotted;
    int c_number;
    struct shpstr c_shp;
};

static int showship(struct coast **cpp, int x, int y);

/*
 * format: coastwatch [<SECTS>]
 */
int
coas(void)
{
    struct sctstr sect;
    struct nstr_sect nstr;
    struct coast *cp;
    struct coast *list[TSIZE];
    int i, n;
    int vrange, see;
    int x, y, dx, dy, dxmax;
    int nship = 0;
    double tech;
    struct nstr_item ni;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    for (i = 0; i < TSIZE; i++)
	list[i] = NULL;
    cp = malloc(sizeof(*cp));
    snxtitem_all(&ni, EF_SHIP);
    while (nxtitem(&ni, &cp->c_shp)) {
	if (cp->c_shp.shp_own == 0 || cp->c_shp.shp_own == player->cnum)
	    continue;
	/*
	 * don't bother putting subs in the table...
	 * unless they're in a sector you own (harbor or such)
	 */
	getsect(cp->c_shp.shp_x, cp->c_shp.shp_y, &sect);
	if ((mchr[(int)cp->c_shp.shp_type].m_flags & M_SUB) &&
	    (sect.sct_own != player->cnum))
	    continue;
	n = scthash(cp->c_shp.shp_x, cp->c_shp.shp_y, TSIZE);
	cp->c_spotted = 0;
	cp->c_number = i;
	cp->c_next = list[n];
	list[n] = cp;
	cp = malloc(sizeof(*cp));
	nship++;
    }
    /* get that last one! */
    free(cp);
    pr("- = [ Coastwatch report for %s ] = -\n", cname(player->cnum));
    pr("  Country            Ship          Location\n");
    tech = tfact(player->cnum, 1.0);
    while (nxtsct(&nstr, &sect) && nship) {
	if (sect.sct_own != player->cnum)
	    continue;
	see = sect.sct_type == SCT_RADAR ? 14 : 4;
	vrange = (int)(sect.sct_effic / 100.0 * see * tech);
	if (vrange < 1)
	    vrange = 1;
	for (dy = -vrange; dy <= vrange; dy++) {
	    y = ynorm(sect.sct_y + dy);
	    dxmax = 2 * vrange - abs(dy);
	    for (dx = -dxmax; dx <= dxmax; dx += 2) {
		x = xnorm(sect.sct_x + dx);
		n = scthash(x, y, TSIZE);
		if (!list[n])
		    continue;
		nship -= showship(&list[n], x, y);
	    }
	}
    }
    /* free up the coast structs calloc'ed above */
    for (i = 0; i < TSIZE; i++) {
	while (NULL != (cp = list[i])) {
	    list[i] = cp->c_next;
	    free(cp);
	}
    }
    return RET_OK;
}

static int
showship(struct coast **cpp, int x, int y)
{
    struct coast *cp;
    struct coast *todelete = NULL;
    struct coast **prev;
    int nship = 0;

    prev = NULL;
    cp = *cpp;
    prev = cpp;
    do {
	/* we delete it, we free it. */
	if (todelete) {
	    free(todelete);
	    todelete = NULL;
	}
	if (cp->c_shp.shp_x != x || cp->c_shp.shp_y != y) {
	    prev = &(*prev)->c_next;
	    continue;
	}
	pr(" %12.12s (#%3d) %s @ %s\n",
	   cname(cp->c_shp.shp_own), cp->c_shp.shp_own,
	   prship(&cp->c_shp), xyas(x, y, player->cnum));
	if (opt_HIDDEN) {
	    setcont(player->cnum, cp->c_shp.shp_own, FOUND_COAST);
	}
	*prev = cp->c_next;
	todelete = cp;
	nship++;
    } while (NULL != (cp = cp->c_next));
    /* check that last one! */
    if (todelete)
	free(todelete);
    return nship;
}
