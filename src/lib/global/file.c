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
 *  file.c: Empire game data file descriptions.
 * 
 *  Known contributors to this file:
 *     
 */

#include <stddef.h>
#include "misc.h"
#include "xy.h"
#include "loan.h"
#include "nsc.h"
#include "nuke.h"
#include "plane.h"
#include "ship.h"
#include "land.h"
#include "sect.h"
#include "trade.h"
#include "treaty.h"
#include "file.h"
#include "power.h"
#include "news.h"
#include "nat.h"
#include "lost.h"
#include "product.h"

#include "gamesdef.h"
#include "commodity.h"
#include "nsc.h"

struct empfile empfile[] = {
    /* Dynamic game data */
    {"sect", "sector", EFF_XY | EFF_OWNER,
     0, sizeof(struct sctstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, sect_ca},
    {"ship", "ship", EFF_XY | EFF_OWNER | EFF_GROUP,
     0, sizeof(struct shpstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, ship_ca},
    {"plane", "plane", EFF_XY | EFF_OWNER | EFF_GROUP,
     0, sizeof(struct plnstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, plane_ca},
    {"land", "land", EFF_XY | EFF_OWNER | EFF_GROUP,
     0, sizeof(struct lndstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, land_ca},
    {"nuke", "nuke", EFF_XY | EFF_OWNER,
     0, sizeof(struct nukstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, nuke_ca},
    {"news", "news", 0,
     0, sizeof(struct nwsstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, news_ca},
    {"treaty", "treaty", 0,
     0, sizeof(struct trtstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, treaty_ca},
    {"trade", "trade", 0,
     0, sizeof(struct trdstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, trade_ca},
    {"pow", "power", 0,
     0, sizeof(struct powstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"nat", "nation", EFF_OWNER,
     0, sizeof(struct natstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, nat_ca},
    {"loan", "loan", 0,
     0, sizeof(struct lonstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, loan_ca},
    {"map", "map", 0,
     0, DEF_WORLD_X * DEF_WORLD_Y / 2, NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"bmap", "bmap", 0,
     0, DEF_WORLD_X * DEF_WORLD_Y / 2, NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"commodity", "commodity", 0,
     0, sizeof(struct comstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, commodity_ca},
    {"lost", "lostitems", EFF_OWNER,
     0, sizeof(struct loststr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, lost_ca},

    /* Static game data (configuation) */
    {"sect chr", NULL, EFF_MEM,
     0, sizeof(dchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)dchr, 0, dchr_ca},
    {"ship chr", NULL, EFF_MEM,
     0, sizeof(mchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)mchr, 0, mchr_ca},
    {"plane chr", NULL, EFF_MEM,
     0, sizeof(plchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)plchr, 0, plchr_ca},
    {"land chr", NULL, EFF_MEM,
     0, sizeof(lchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)lchr, 0, lchr_ca},
    {"nuke chr", NULL, EFF_MEM,
     0, sizeof(nchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)nchr, 0, nchr_ca},
#if 0
    /* FIXME rpt[] lacks sentinel, xdchr() doesn't terminate */
    {"news chr", NULL, EFF_MEM,
     0, sizeof(rpt[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)rpt, 0, rpt_ca},
#endif
    {"treaty chr", NULL, EFF_MEM,
     0, sizeof(tchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)tchr, 0, tchr_ca},
    {"item", NULL, EFF_MEM,
     0, sizeof(ichr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)ichr, 0, ichr_ca},
    {"infrastructure", NULL, EFF_MEM,
     0, sizeof(intrchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)intrchr, 0, intrchr_ca},
    {"product", NULL, EFF_MEM,
     0, sizeof(pchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)pchr, 0, pchr_ca},
    {"table", NULL, EFF_MEM,
     0, sizeof(empfile[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)empfile, 0, empfile_ca},
    {"ship chr flags", NULL, EFF_MEM,
     0, sizeof(ship_chr_flags[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)ship_chr_flags, 0, lookup_ca},
    {"plane chr flags", NULL, EFF_MEM,
     0, sizeof(plane_chr_flags[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)plane_chr_flags, 0, lookup_ca},
    {"land chr flags", NULL, EFF_MEM,
     0, sizeof(land_chr_flags[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)land_chr_flags, 0, lookup_ca},
    {"nuke chr flags", NULL, EFF_MEM,
     0, sizeof(nuke_chr_flags[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)nuke_chr_flags, 0, lookup_ca},

    /* Sentinel */
    {NULL, NULL, 0,
     0, 0, NULL, NULL, NULL,
     -1, -1, 0,0,NULL, 0, NULL}
};
