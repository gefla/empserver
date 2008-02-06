/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fileinit.c: Initialize Empire tables for full server operations.
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 */

#include <config.h>

#include "file.h"
#include "prototypes.h"

struct fileinit {
    int ef_type;
    void (*init) (int, void *);
    int (*postread) (int, void *);
    int (*prewrite) (int, void *);
};

static struct fileinit fileinit[] = {
    {EF_SECTOR, NULL, sct_postread, sct_prewrite},
    {EF_SHIP, shp_init, shp_postread, shp_prewrite},
    {EF_PLANE, pln_init, pln_postread, pln_prewrite},
    {EF_LAND, lnd_init, lnd_postread, lnd_prewrite},
    {EF_NUKE, nuk_init, nuk_postread, nuk_prewrite}
};

static void ef_open_srv(void);
static void ef_close_srv(void);
static int ef_init_view(int, int);
static void ef_fina_view(int);

/*
 * Initialize empfile for full server operations.
 */
void
ef_init_srv(void)
{
    unsigned i;

    for (i = 0; i < sizeof(fileinit) / sizeof(fileinit[0]); i++) {
	empfile[fileinit[i].ef_type].init = fileinit[i].init;
	empfile[fileinit[i].ef_type].postread = fileinit[i].postread;
	empfile[fileinit[i].ef_type].prewrite = fileinit[i].prewrite;
    }

    nsc_init();
    ef_open_srv();
    if (ef_verify() < 0)
	exit(EXIT_FAILURE);
    global_init();
}

void
ef_fin_srv(void)
{
    ef_close_srv();
}

static void
ef_open_srv(void)
{
    int failed = 0;

    failed |= !ef_open(EF_NATION, EFF_MEM);
    failed |= !ef_open(EF_SECTOR, EFF_MEM);
    failed |= !ef_open(EF_SHIP, EFF_MEM);
    failed |= !ef_open(EF_PLANE, EFF_MEM);
    failed |= !ef_open(EF_LAND, EFF_MEM);
    failed |= !ef_open(EF_GAME, EFF_MEM);
    failed |= !ef_open(EF_NEWS, 0);
    failed |= !ef_open(EF_LOAN, 0);
    failed |= !ef_open(EF_TREATY, 0);
    failed |= !ef_open(EF_NUKE, EFF_MEM);
    failed |= !ef_open(EF_POWER, 0);
    failed |= !ef_open(EF_TRADE, 0);
    failed |= !ef_open(EF_MAP, EFF_MEM);
    failed |= !ef_open(EF_BMAP, EFF_MEM);
    failed |= !ef_open(EF_COMM, 0);
    failed |= !ef_open(EF_LOST, 0);
    failed |= !ef_open(EF_REALM, EFF_MEM);
    if (!failed)
	failed |= ef_init_view(EF_COUNTRY, EF_NATION);
    if (failed) {
	logerror("Missing files, giving up");
	exit(EXIT_FAILURE);
    }
}

static void
ef_close_srv(void)
{
    ef_fina_view(EF_COUNTRY);
    ef_close(EF_NATION);
    ef_close(EF_SECTOR);
    ef_close(EF_SHIP);
    ef_close(EF_PLANE);
    ef_close(EF_LAND);
    ef_close(EF_GAME);
    ef_close(EF_NEWS);
    ef_close(EF_LOAN);
    ef_close(EF_TREATY);
    ef_close(EF_NUKE);
    ef_close(EF_POWER);
    ef_close(EF_TRADE);
    ef_close(EF_MAP);
    ef_close(EF_COMM);
    ef_close(EF_BMAP);
    ef_close(EF_LOST);
    ef_close(EF_REALM);
}

static int
ef_init_view(int type, int base)
{
    if (CANT_HAPPEN(!(empfile[base].flags & EFF_MEM)))
	return -1;
    empfile[type].cache = empfile[base].cache;
    empfile[type].csize = empfile[base].csize;
    empfile[type].flags |= EFF_MEM;
    empfile[type].baseid = empfile[base].baseid;
    empfile[type].cids = empfile[base].cids;
    empfile[type].fids = empfile[base].fids;
    return 0;
}

static void
ef_fina_view(int type)
{
    empfile[type].cache = NULL;
}
