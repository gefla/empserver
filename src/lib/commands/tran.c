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
 *  tran.c: Transport nuclear devices and planes
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     Markus Armbruster, 2006-2011
 */

#include <config.h>

#include "commands.h"
#include "land.h"
#include "map.h"
#include "nuke.h"
#include "plane.h"
#include "ship.h"

static int tran_pmap(coord, coord, char *, char *);
static int tran_nmap(coord, coord, char *, char *);
static int tran_nuke(void);
static int tran_plane(void);

int
tran(void)
{
    char *what;
    char buf[1024];

    what = getstarg(player->argp[1], "transport what (nuke or plane): ",
		    buf);
    if (!what)
	return RET_SYN;
    if (*what == 'n')
	return tran_nuke();
    else if (*what == 'p')
	return tran_plane();
    return RET_SYN;
}

static int
tran_nuke(void)
{
    coord srcx, srcy;
    coord dstx, dsty;
    int mcost;
    int weight, count;
    int type, dam;
    struct nstr_item nstr;
    struct nukstr nuke;
    struct sctstr sect;
    struct sctstr endsect;

    weight = 0;
    count = 0;
    if (!snxtitem(&nstr, EF_NUKE, player->argp[2], NULL))
	return RET_SYN;
    while (nxtitem(&nstr, &nuke)) {
	if (!player->owner)
	    continue;
	type = nuke.nuk_type;
	if (nuke.nuk_plane >= 0) {
	    pr("%s is armed and can't be transported\n", prnuke(&nuke));
	    return RET_FAIL;
	}
	if (count == 0) {
	    srcx = nuke.nuk_x;
	    srcy = nuke.nuk_y;
	} else {
	    if (nuke.nuk_x != srcx || nuke.nuk_y != srcy) {
		pr("All nukes must be in the same sector.\n");
		return RET_FAIL;
	    }
	}
	weight += nchr[type].n_weight;
	++count;
    }
    if (count == 0) {
	pr("No nukes\n");
	return RET_FAIL;
    }
    if (!getsect(srcx, srcy, &sect) || !player->owner) {
	pr("You don't own %s\n", xyas(srcx, srcy, player->cnum));
	return RET_FAIL;
    }
    if (!military_control(&sect)) {
	pr("Military control required to move nukes.\n");
	return RET_FAIL;
    }
    dam = 0;
    mcost = move_ground(&sect, &endsect, weight,
			player->argp[3], tran_nmap, 0, &dam);
    if (mcost < 0)
	return 0;

    dstx = endsect.sct_x;
    dsty = endsect.sct_y;
    snxtitem_rewind(&nstr);
    while (nxtitem(&nstr, &nuke)) {
	if (!player->owner)
	    continue;
	/* TODO apply dam */
	nuke.nuk_x = dstx;
	nuke.nuk_y = dsty;
	nuke.nuk_mission = 0;
	putnuke(nuke.nuk_uid, &nuke);
    }
    if (mcost > 0)
	pr("Total movement cost = %d\n", mcost);
    else
	pr("No mobility used\n");
    getsect(srcx, srcy, &sect);
    if (sect.sct_mobil >= mcost)
	sect.sct_mobil -= mcost;
    else
	sect.sct_mobil = 0;
    putsect(&sect);
    return RET_OK;
}

static int
tran_plane(void)
{
    coord srcx, srcy;
    coord dstx, dsty;
    int mcost;
    int weight, count;
    int type, dam;
    struct nstr_item nstr;
    struct plnstr plane;
    struct sctstr sect;
    struct sctstr endsect;

    weight = 0;
    count = 0;
    if (!snxtitem(&nstr, EF_PLANE, player->argp[2], NULL))
	return RET_SYN;
    /*
     * First do some sanity checks: make sure that they are all in the,
     * same sector, not on ships, owned, etc.
     * No one could seriously want to move planes in parallel from
     * several sectors!
     */
    while (nxtitem(&nstr, &plane)) {
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
	} else if (pln_is_in_orbit(&plane)) {
	    pr("%s is in space and can't be transported\n",
	       prplane(&plane));
	    return RET_FAIL;
	}
	if (count == 0) {
	    srcx = plane.pln_x;
	    srcy = plane.pln_y;
	} else {
	    if (plane.pln_x != srcx || plane.pln_y != srcy) {
		pr("All planes must be in the same sector.\n");
		return RET_FAIL;
	    }
	}
	weight += plchr[type].pl_lcm + (plchr[type].pl_hcm * 2);
	++count;
    }
    if (count == 0) {
	pr("No planes\n");
	return RET_FAIL;
    }
    if (!getsect(srcx, srcy, &sect) || !player->owner) {
	pr("You don't own %s\n", xyas(srcx, srcy, player->cnum));
	return RET_FAIL;
    }
    if (!military_control(&sect)) {
	pr("Military control required to move planes.\n");
	return RET_FAIL;
    }
    dam = 1;
    mcost = move_ground(&sect, &endsect, weight,
			player->argp[3], tran_pmap, 0, &dam);
    dam /= count;
    if (mcost < 0)
	return 0;

    dstx = endsect.sct_x;
    dsty = endsect.sct_y;
    snxtitem_rewind(&nstr);
    while (nxtitem(&nstr, &plane)) {
	if (!player->owner)
	    continue;
	if (dam)
	    planedamage(&plane, dam);
	plane.pln_x = dstx;
	plane.pln_y = dsty;
	plane.pln_mission = 0;
	putplane(plane.pln_uid, &plane);
    }
    if (mcost > 0)
	pr("Total movement cost = %d\n", mcost);
    else
	pr("No mobility used\n");
    getsect(srcx, srcy, &sect);
    if (sect.sct_mobil >= mcost)
	sect.sct_mobil -= mcost;
    else
	sect.sct_mobil = 0;
    putsect(&sect);
    return RET_OK;
}

/*
 * Pretty tacky, but it works.
 * If more commands start doing this, then
 * rewrite map to do the right thing.
 */
/*ARGSUSED*/
static int
tran_pmap(coord curx, coord cury, char *arg1, char *arg2)
{
    return display_region_map(0, EF_PLANE, curx, cury, arg1, arg2);
}

static int
tran_nmap(coord curx, coord cury, char *arg1, char *arg2)
{
    return display_region_map(0, EF_NUKE, curx, cury, arg1, arg2);
}
