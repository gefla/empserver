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
 *  nuke.c: Nuke post-read and pre-write data massage
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "nuke.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "deity.h"
#include "prototypes.h"

/*ARGSUSED*/
int
nuk_postread(int n, s_char *ptr)
{
    struct nukstr *np = (struct nukstr *)ptr;

    if (np->nuk_uid != n) {
	logerror("nuk_postread: Error - %d != %d, zeroing.\n", np->nuk_uid,
		 n);
	bzero(ptr, sizeof(struct nukstr));
    }
    player->owner = (player->god || np->nuk_own == player->cnum);
    return 1;
}

/*ARGSUSED*/
int
nuk_prewrite(int n, s_char *ptr)
{
    struct nukstr *np = (struct nukstr *)ptr;
    struct nukstr nuke;

    np->ef_type = EF_NUKE;
    np->nuk_uid = n;

    time(&np->nuk_timestamp);

    getnuke(n, &nuke);

    return 1;
}

void
nuk_init(int n, s_char *ptr)
{
    struct nukstr *np = (struct nukstr *)ptr;

    np->ef_type = EF_NUKE;
    np->nuk_uid = n;
    np->nuk_own = 0;
}

void
nuk_add(coord x, coord y, int type, int num)
{
    struct nukstr nuke;
    struct sctstr sect;
    int nuk_uid;
    natid own;
    int n;

    /* getsect in case of world wraparound */
    getsect(x, y, &sect);
    if (!(own = sect.sct_own))
	return;
    x = sect.sct_x;
    y = sect.sct_y;

    /*
     * either find a stockpile in x,y or add a new one
     */
    nuk_uid = -1;
    for (n = 0; getnuke(n, &nuke); n++) {
	if (nuke.nuk_own == own) {
	    if (nuke.nuk_x == x && nuke.nuk_y == y) {
		nuk_uid = n;
		break;
	    }
	} else if (nuke.nuk_own == 0 && nuk_uid == -1)
	    nuk_uid = n;
    }
    if (nuk_uid == -1)
	nuk_uid = n;
    if (getnuke(nuk_uid, &nuke) == 0) {
	ef_extend(EF_NUKE, 10);
	bzero((s_char *)&nuke, sizeof(struct nukstr));
	nuke.nuk_uid = nuk_uid;
	nuke.nuk_ship = -1;
	nuke.nuk_n = 0;
	makenotlost(EF_NUKE, nuke.nuk_own, nuke.nuk_uid, nuke.nuk_x,
		    nuke.nuk_y);
    }
    nuke.nuk_x = x;
    nuke.nuk_y = y;
    nuke.nuk_own = own;
    nuke.nuk_types[type] += num;
    nuke.nuk_n += num;

    if (!putnuke(nuke.nuk_uid, &nuke))
	pr("Problem with the nuclear stockpiles, tell the deity.\n");

}

void
nuk_delete(struct nukstr *np, int type, int num)
{
    if (np->nuk_types[type] < num)
	num = np->nuk_types[type];
    np->nuk_types[type] -= num;
    np->nuk_n -= num;
    if (np->nuk_n <= 0) {
	makelost(EF_NUKE, np->nuk_own, np->nuk_uid, np->nuk_x, np->nuk_y);
	np->nuk_own = 0;
    }
    putnuke(np->nuk_uid, np);
}
