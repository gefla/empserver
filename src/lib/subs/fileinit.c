/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  fileinit.c: Stuff that ef_init uses to initialize the ca pointers and
 *              the pre/post i/o calls.
 * 
 *  Known contributors to this file:
 *  
 */

#include <fcntl.h>
#include "misc.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "loan.h"
#include "lost.h"
#include "commodity.h"
#include "prototypes.h"
#include "optlist.h"

struct fileinit fileinit[EF_MAX] = {
    {0, sct_postread, sct_prewrite, sect_ca},
    {shp_init, shp_postread, shp_prewrite, ship_ca},
    {pln_init, pln_postread, pln_prewrite, plane_ca},
    {lnd_init, lnd_postread, lnd_prewrite, land_ca},
    {nuk_init, nuk_postread, nuk_prewrite, nuke_ca},
    {0, 0, 0, news_ca},
    {0, 0, 0, treaty_ca},
    {0, 0, 0, trade_ca},
    {0, 0, 0, 0},		/* power */
    {0, 0, 0, 0},		/* nation */
    {0, 0, 0, loan_ca},
    {0, 0, 0, 0},		/* map */
    {0, 0, 0, 0},		/* map */
    {0, 0, 0, commodity_ca},
    {0, 0, 0, lost_ca}
};

void
ef_init(void)
{
    int i;
    struct empfile *ef;
    struct fileinit *fi;

    ef = empfile;
    fi = fileinit;
    for (i = 0; i < EF_MAX; i++, ef++, fi++) {
	ef->init = fi->init;
	ef->postread = fi->postread;
	ef->prewrite = fi->prewrite;
	ef->cadef = fi->cadef;
	/* We have to set the size for the map and bmap files at
	   runtime. */
	if (i == EF_MAP || i == EF_BMAP)
	    ef->size = (WORLD_X * WORLD_Y) / 2;
    }
}
