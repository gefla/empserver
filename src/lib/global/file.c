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
    {EF_SECTOR, "sect", "sector", EFF_XY | EFF_OWNER,
     sizeof(struct sctstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, sect_ca},
    {EF_SHIP, "ship", "ship", EFF_XY | EFF_OWNER | EFF_GROUP,
     sizeof(struct shpstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, ship_ca},
    {EF_PLANE, "plane", "plane", EFF_XY | EFF_OWNER | EFF_GROUP,
     sizeof(struct plnstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, plane_ca},
    {EF_LAND, "land", "land", EFF_XY | EFF_OWNER | EFF_GROUP,
     sizeof(struct lndstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, land_ca},
    {EF_NUKE, "nuke", "nuke", EFF_XY | EFF_OWNER,
     sizeof(struct nukstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, nuke_ca},
    {EF_NEWS, "news", "news", 0,
     sizeof(struct nwsstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, news_ca},
    {EF_TREATY, "treaty", "treaty", 0,
     sizeof(struct trtstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, treaty_ca},
    {EF_TRADE, "trade", "trade", 0,
     sizeof(struct trdstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, trade_ca},
    {EF_POWER, "pow", "power", 0,
     sizeof(struct powstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {EF_NATION, "nat", "nation", EFF_OWNER,
     sizeof(struct natstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, nat_ca},
    {EF_LOAN, "loan", "loan", 0,
     sizeof(struct lonstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, loan_ca},
    {EF_MAP, "map", "map", 0,
     DEF_WORLD_X * DEF_WORLD_Y / 2, NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {EF_BMAP, "bmap", "bmap", 0,
     DEF_WORLD_X * DEF_WORLD_Y / 2, NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {EF_COMM, "commodity", "commodity", 0,
     sizeof(struct comstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, commodity_ca},
    {EF_LOST, "lost", "lostitems", EFF_OWNER,
     sizeof(struct loststr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, lost_ca},

    /* Static game data (configuation) */
#define EFF_CFG (EFF_RDONLY | EFF_MEM | EFF_STATIC)
    {EF_SECTOR_CHR, "sect chr", NULL, EFF_CFG,
     sizeof(dchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)dchr, 0, dchr_ca},
    {EF_SHIP_CHR, "ship chr", NULL, EFF_CFG,
     sizeof(mchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)mchr, 0, mchr_ca},
    {EF_PLANE_CHR, "plane chr", NULL, EFF_CFG,
     sizeof(plchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)plchr, 0, plchr_ca},
    {EF_LAND_CHR, "land chr", NULL, EFF_CFG,
     sizeof(lchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)lchr, 0, lchr_ca},
    {EF_NUKE_CHR, "nuke chr", NULL, EFF_CFG,
     sizeof(nchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)nchr, 0, nchr_ca},
#if 0
    /* FIXME rpt[] lacks sentinel, xdchr() doesn't terminate */
    {EF_, "news chr", NULL, EFF_CFG,
     sizeof(rpt[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)rpt, 0, rpt_ca},
#endif
    {EF_TREATY_CHR, "treaty chr", NULL, EFF_CFG,
     sizeof(tchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)tchr, 0, tchr_ca},
    {EF_ITEM, "item", NULL, EFF_CFG,
     sizeof(ichr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)ichr, 0, ichr_ca},
    {EF_INFRASTRUCTURE, "infrastructure", NULL, EFF_CFG,
     sizeof(intrchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)intrchr, 0, intrchr_ca},
    {EF_PRODUCT, "product", NULL, EFF_CFG,
     sizeof(pchr[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)pchr, 0, pchr_ca},
    {EF_TABLE, "table", NULL, EFF_CFG,
     sizeof(empfile[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)empfile, 0, empfile_ca},
    {EF_SHIP_CHR_FLAGS, "ship chr flags", NULL, EFF_CFG,
     sizeof(ship_chr_flags[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)ship_chr_flags, 0, lookup_ca},
    {EF_PLANE_CHR_FLAGS, "plane chr flags", NULL, EFF_CFG,
     sizeof(plane_chr_flags[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)plane_chr_flags, 0, lookup_ca},
    {EF_LAND_CHR_FLAGS, "land chr flags", NULL, EFF_CFG,
     sizeof(land_chr_flags[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)land_chr_flags, 0, lookup_ca},
    {EF_NUKE_CHR_FLAGS, "nuke chr flags", NULL, EFF_CFG,
     sizeof(nuke_chr_flags[0]), NULL, NULL, NULL,
     -1, -1, 0, 0, (char *)nuke_chr_flags, 0, lookup_ca},

    /* Sentinel */
    {EF_BAD, NULL, NULL, 0,
     0, NULL, NULL, NULL,
     -1, -1, 0,0,NULL, 0, NULL}
};
