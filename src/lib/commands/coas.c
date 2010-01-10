/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#define	TSIZE	200

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
    int i, k, j, n;
    int vrange, see;
    int x, y;
    int mink, minj, maxk, maxj;
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
	maxk = vrange;
	maxj = vrange * 2;
	vrange *= vrange;
	mink = -maxk;
	minj = -maxj;
	for (j = minj; j <= maxj && nship; j++) {
	    x = xnorm(sect.sct_x + j);
	    for (k = mink; k <= maxk && nship; k++) {
		if ((j + k) & 01)
		    continue;
		/* quick range check to save time... */
		if (vrange < (j * j + 3 * k * k) / 4)
		    continue;
		y = ynorm(sect.sct_y + k);
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
