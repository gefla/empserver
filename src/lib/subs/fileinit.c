/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fileinit.c: Fill the empfile[] with function pointers only required for
 *              full server operations.  This allows the empfile[] to be
 *              used in files and fairland.
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 *  
 */

#include <fcntl.h>
#include "misc.h"
#include "nsc.h"
#include "file.h"
#include "prototypes.h"
#include "optlist.h"

struct fileinit fileinit[] = {
    {EF_SECTOR, NULL, sct_postread, sct_prewrite},
    {EF_SHIP, shp_init, shp_postread, shp_prewrite},
    {EF_PLANE, pln_init, pln_postread, pln_prewrite},
    {EF_LAND, lnd_init, lnd_postread, lnd_prewrite},
    {EF_NUKE, nuk_init, nuk_postread, nuk_prewrite}
};

void
ef_init(void)
{
    int i;

    for (i = 0; i < sizeof(fileinit) / sizeof(fileinit[0]); i++) {
	empfile[fileinit[i].ef_type].init = fileinit[i].init;
	empfile[fileinit[i].ef_type].postread = fileinit[i].postread;
	empfile[fileinit[i].ef_type].prewrite = fileinit[i].prewrite;
    }

    empfile[EF_MAP].size = empfile[EF_BMAP].size = (WORLD_X * WORLD_Y) / 2;
}
