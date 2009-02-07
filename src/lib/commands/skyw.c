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
 *  skyw.c: Look at satellites in the sky.
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"
#include "plane.h"

#define	TSIZE	200

struct sky {
    struct sky *s_next;
    int s_spotted;
    struct plnstr s_sat;
};

static int showsat(struct sky **skypp, int x, int y);

/*
 * format: skywatch [<SECTS>]
 */
int
skyw(void)
{
    struct sctstr sect;
    struct nstr_sect nstr;
    struct sky *skyp;
    struct sky *list[TSIZE];
    int i, k, j, n;
    int vrange, see;
    int x, y;
    int mink, minj, maxk, maxj;
    int nsat = 0;
    double tech;
    struct nstr_item ni;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    for (i = 0; i < TSIZE; i++)
	list[i] = 0;
    skyp = malloc(sizeof(*skyp));
    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &skyp->s_sat)) {
	if (!skyp->s_sat.pln_own)
	    continue;
	if (!pln_is_in_orbit(&skyp->s_sat))
	    continue;
	getsect(skyp->s_sat.pln_x, skyp->s_sat.pln_y, &sect);
	n = scthash(skyp->s_sat.pln_x, skyp->s_sat.pln_y, TSIZE);
	skyp->s_spotted = 0;
	skyp->s_next = list[n];
	list[n] = skyp;
	skyp = malloc(sizeof(*skyp));
	nsat++;
    }
    /* get that last one! */
    free(skyp);
    pr("- = [ Skywatch report for %s ] = -\n", cname(player->cnum));
    pr(" %18s%20s        %s\n", "Country", "Satellite", "Location");
    tech = tfact(player->cnum, 1.0);
    while (nxtsct(&nstr, &sect) && nsat) {
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
	for (j = minj; j <= maxj && nsat; j++) {
	    x = xnorm(sect.sct_x + j);
	    for (k = mink; k <= maxk && nsat; k++) {
		if ((j + k) & 01)
		    continue;
		/* quick range check to save time... */
		if (vrange < (j * j + 3 * k * k) / 4)
		    continue;
		y = ynorm(sect.sct_y + k);
		n = scthash(x, y, TSIZE);
		if (list[n] == 0)
		    continue;
		nsat -= showsat(&list[n], x, y);
	    }
	}
    }
    /* free up the sky structs calloc'ed above */
    for (i = 0; i < TSIZE; i++) {
	while (NULL != (skyp = list[i])) {
	    list[i] = skyp->s_next;
	    free(skyp);
	}
    }
    return RET_OK;
}

static int
showsat(struct sky **skypp, int x, int y)
{
    struct sky *skyp;
    struct sky *todelete = 0;
    struct sky **prev;
    struct plchrstr *pcp;
    char *name;
    int nsat = 0;

    prev = 0;
    skyp = *skypp;
    prev = skypp;
    do {
	/* we delete it, we free it. */
	if (todelete) {
	    free(todelete);
	    todelete = 0;
	}
	if (skyp->s_sat.pln_x != x || skyp->s_sat.pln_y != y) {
	    prev = &(*prev)->s_next;
	    continue;
	}
	pcp = &plchr[(int)skyp->s_sat.pln_type];
	name = pcp->pl_name;
	pr(" %12.12s (#%d) %20s            %s\n",
	   cname(skyp->s_sat.pln_own), skyp->s_sat.pln_own,
	   name, xyas(x, y, player->cnum));
	if (opt_HIDDEN) {
	    /* FOUND_COAST should probably be changed to FOUND_SKY -KHS */
	    setcont(player->cnum, skyp->s_sat.pln_own, FOUND_COAST);
	}
	*prev = skyp->s_next;
	todelete = skyp;
	nsat++;
    } while (NULL != (skyp = skyp->s_next));
    /* check that last one! */
    if (todelete)
	free(todelete);
    return nsat;
}
