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
 *  tran.c: Transport nuclear devices and planes
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *  
 */

#ifdef Rel4
#include <string.h>
#endif /* Rel4 */
#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "nuke.h"
#include "xy.h"
#include "nsc.h"
#include "plane.h"
#include "ship.h"
#include "file.h"
#include "nat.h"
#include "land.h"
#include "commands.h"

#include <stdio.h>

static int tran_map(s_char *what, coord curx, coord cury, s_char *arg);
static int tran_nuke(void);
static int tran_plane(void);

int
tran(void)
{
    s_char *what;
    s_char buf[1024];

    what =
	getstarg(player->argp[1], "transport what (nuke or plane): ", buf);
    if (what == 0)
	return RET_SYN;
    if (*what == 'n')
	return tran_nuke();
    else if (*what == 'p')
	return tran_plane();
    return RET_SYN;
}

/*
 * Kinda silly; only moves the first nuke.
 * Maybe nukes should be made into commodities?
 */
static int
tran_nuke(void)
{
    struct nchrstr *ncp;
    int len;
    coord x, y;
    coord dstx, dsty;
    int found;
    s_char *p;
    int i;
    int nuketype;
    int moving;
    struct nukstr nuke;
    struct sctstr sect;
    struct sctstr endsect;
    int mcost, dam;
    struct nstr_item nstr;
    s_char buf[1024];

    if (!(p = getstarg(player->argp[2], "from sector : ", buf)))
	return RET_SYN;
    if (!sarg_xy(p, &x, &y))
	return RET_SYN;
    if (!getsect(x, y, &sect) || !player->owner) {
	pr("Not yours\n");
	return RET_FAIL;
    }
#if 0
    if (!snxtitem_xy(&nstr, EF_NUKE, sect.sct_x, sect.sct_y)) {
	pr("There are no nukes in %s\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_FAIL;
    }
#else
    snxtitem_xy(&nstr, EF_NUKE, sect.sct_x, sect.sct_y);
#endif
    found = 0;
    while (nxtitem(&nstr, (caddr_t)&nuke)) {
	if (player->owner) {
	    found = 1;
	    break;
	}
    }
    if (!found) {
	pr("There are no nukes in %s\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_FAIL;
    }
    if (!(p = getstarg(player->argp[3], "warhead type : ", buf)))
	return RET_SYN;
    if (!check_sect_ok(&sect))
	return RET_FAIL;
    len = strlen(p);
    for (i = 0, ncp = nchr; i < N_MAXNUKE; i++, ncp++) {
	if (strncmp(ncp->n_name, p, len) == 0)
	    break;
    }
    if (i >= N_MAXNUKE) {
	pr("No such nuke type!\n");
	return RET_SYN;
    }
    nuketype = i;
    if (!nuke.nuk_types[nuketype]) {
	pr("No %s nukes in %s\n",
	   ncp->n_name, xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_FAIL;
    }
    p = getstarg(player->argp[4], "number of warheads : ", buf);
    if (!check_sect_ok(&sect))
	return RET_FAIL;
    if (p == 0 || *p == 0 || (moving = atoi(p)) < 0)
	return RET_FAIL;
    if (moving > nuke.nuk_types[nuketype]) {
	moving = nuke.nuk_types[nuketype];
	if (moving)
	    pr("only moving %d\n", moving);
	else
	    return RET_FAIL;
    }
    /*
     * military control necessary to move
     * goodies in occupied territory.
     */
    if (sect.sct_oldown != player->cnum) {
	int tot_mil = 0;
	struct nstr_item ni;
	struct lndstr land;
	snxtitem_xy(&ni, EF_LAND, sect.sct_x, sect.sct_y);
	while (nxtitem(&ni, (s_char *)&land)) {
	    if (land.lnd_own == player->cnum)
		tot_mil += total_mil(&land);
	}
	if ((getvar(V_MILIT, (s_char *)&sect, EF_SECTOR) + tot_mil) * 10
	    < getvar(V_CIVIL, (s_char *)&sect, EF_SECTOR)) {
	    pr("Military control required to move goods.\n");
	    return RET_FAIL;
	}
    }
    dam = 0;
    mcost = move_ground((s_char *)&nuke, &sect, &endsect,
			(double)ncp->n_weight * moving,
			player->argp[5], tran_map, 0, &dam);

    if (mcost < 0)
	return 0;

    if (mcost > 0)
	pr("Total movement cost = %d\n", mcost);
    else
	pr("No mobility used\n");

    dstx = endsect.sct_x;
    dsty = endsect.sct_y;
    /*
     * decrement mobility from src sector
     */
    getsect(nuke.nuk_x, nuke.nuk_y, &sect);
    sect.sct_mobil -= mcost;
    if (sect.sct_mobil < 0)
	sect.sct_mobil = 0;
    putsect(&sect);
    /*
     * update old nuke
     */
    if (!getnuke(nuke.nuk_uid, &nuke)) {
	pr("Could not find that stockpile again.\n");
	return RET_FAIL;
    }
    if (nuke.nuk_types[nuketype] < moving || nuke.nuk_own != player->cnum) {
	pr("Stockpile changed!\n");
	return RET_FAIL;
    }
    nuk_delete(&nuke, nuketype, moving);
    nuk_add(dstx, dsty, nuketype, moving);
    return RET_OK;
}

static int
tran_plane(void)
{
    int srcx, srcy;
    int dstx, dsty;
    int mcost;
    int weight, count;
    int first;
    int type, dam;
    struct nstr_item nstr;
    struct plnstr plane;
    struct sctstr sect;
    struct sctstr endsect;

    first = 1;
    weight = 0;
    count = 0;
    if (!snxtitem(&nstr, EF_PLANE, player->argp[2]))
	return RET_SYN;
    /*
     * First do some sanity checks: make sure that they are all in the,
     * same sector, not on ships, owned, etc.
     * No one could seriously want to move planes in parallel from
     * several sectors!
     */
    while (nxtitem(&nstr, (s_char *)&plane)) {
	if (!player->owner)
	    continue;
	type = plane.pln_type;
	if (plane.pln_ship >= 0) {
	    pr("%s is at sea and can't be transported\n", prplane(&plane));
	    return RET_FAIL;
	} else if (plane.pln_harden != 0) {
	    pr("%s has been hardened and can't be transported\n",
	       prplane(&plane));
	    return RET_FAIL;
	} else if ((plane.pln_flags & PLN_LAUNCHED) &&
		   (plchr[type].pl_flags & P_O)) {
	    pr("%s is in space and can't be transported\n",
	       prplane(&plane));
	    return RET_FAIL;
	}
	if (first == 1) {
	    srcx = plane.pln_x;
	    srcy = plane.pln_y;
	    first = 0;
	} else {
	    if (plane.pln_x != srcx || plane.pln_y != srcy) {
		pr("All planes must be in the same sector.\n");
		return RET_FAIL;
	    }
	}
	weight += plchr[type].pl_lcm + (plchr[type].pl_hcm * 2);
	++count;
    }
    if (first == 1) {
	/* no planes */
	return RET_FAIL;
    }
    getsect(srcx, srcy, &sect);
    /*
     * military control necessary to move
     * goodies in occupied territory.
     */
    if (sect.sct_oldown != player->cnum) {
	int tot_mil = 0;
	struct nstr_item ni;
	struct lndstr land;
	snxtitem_xy(&ni, EF_LAND, sect.sct_x, sect.sct_y);
	while (nxtitem(&ni, (s_char *)&land))
	    tot_mil += total_mil(&land);
	if ((getvar(V_MILIT, (s_char *)&sect, EF_SECTOR) + tot_mil) * 10
	    < getvar(V_CIVIL, (s_char *)&sect, EF_SECTOR)) {
	    pr("Military control required to move goods.\n");
	    return RET_FAIL;
	}
    }
    dam = 1;
    mcost = move_ground((s_char *)&plane, &sect, &endsect,
			(double)weight,
			player->argp[3], tran_map, 0, &dam);
    dam /= count;
    if (mcost < 0)
	return 0;

    dstx = endsect.sct_x;
    dsty = endsect.sct_y;
    snxtitem_rewind(&nstr);
    while (nxtitem(&nstr, (s_char *)&plane)) {
	if (!player->owner)
	    continue;
	if (dam) {
	    planedamage(&plane, dam);
	    pr("\t%s takes %d\n", prplane(&plane), dam);
	}
	plane.pln_x = dstx;
	plane.pln_y = dsty;
	plane.pln_mission = 0;
	putplane(plane.pln_uid, &plane);
    }
    if (mcost > 0)
	pr("Total movement cost = %d\n", mcost);
    else
	pr("No mobility used\n");
    sect.sct_mobil -= mcost;
    if (sect.sct_mobil < 0)
	sect.sct_mobil = 0;
    putsect(&sect);
    return RET_OK;
}

/*
 * Pretty tacky, but it works.
 * If more commands start doing this, then
 * rewrite map to do the right thing.
 */
/* I think this is no longer used, check subs/move.c:move_ground() */
/*ARGSUSED*/
static int
tran_map(s_char *what, coord curx, coord cury, s_char *arg)
{
    player->argp[1] = arg;
    player->condarg = 0;
    return map();
}
