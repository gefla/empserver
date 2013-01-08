/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fileinit.c: Initialize Empire tables for full server operations.
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2005-2011
 */

#include <config.h>

#include "file.h"
#include "nsc.h"
#include "prototypes.h"
#include "unit.h"

struct fileinit {
    int ef_type;
    void (*postread)(int, void *);
    void (*prewrite)(int, void *, void *);
    void (*onresize)(int);
};

static struct fileinit fileinit[] = {
    {EF_SECTOR, sct_postread, sct_prewrite, NULL},
    {EF_SHIP, shp_postread, shp_prewrite, unit_onresize},
    {EF_PLANE, pln_postread, pln_prewrite, unit_onresize},
    {EF_LAND, lnd_postread, lnd_prewrite, unit_onresize},
    {EF_NUKE, nuk_postread, nuk_prewrite, unit_onresize}
};

static void ef_open_srv(void);
static void ef_close_srv(void);

/*
 * Initialize empfile for full server operations.
 */
void
ef_init_srv(int force_bad_state)
{
    unsigned i;

    for (i = 0; i < sizeof(fileinit) / sizeof(fileinit[0]); i++) {
	empfile[fileinit[i].ef_type].postread = fileinit[i].postread;
	empfile[fileinit[i].ef_type].prewrite = fileinit[i].prewrite;
	empfile[fileinit[i].ef_type].onresize = fileinit[i].onresize;
    }

    nsc_init();
    if (ef_verify_config() < 0)
	exit(EXIT_FAILURE);
    ef_open_srv();
    if (ef_verify_state(1) < 0 && !force_bad_state) {
	fprintf(stderr, "You can try -F to force running anyway,"
		" but that's risky; see the manual page\n");
	exit(EXIT_FAILURE);
    }
    global_init();
    unit_cargo_init();
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
	failed |= ef_open_view(EF_COUNTRY);
    if (failed) {
	logerror("Missing files, giving up");
	exit(EXIT_FAILURE);
    }
}

static void
ef_close_srv(void)
{
    ef_close(EF_COUNTRY);
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
