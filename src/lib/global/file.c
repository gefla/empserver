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

/* Some abbreviations: */
#define SZ(array) (sizeof(array) / sizeof((array)[0]))
/* Unmapped cache initializers for members flags... */
#define UNMAPPED_INIT(type, flags) \
    sizeof(type), flags, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL
/* Same for cache mapped to array with known size */
#define ARRAY_INIT(array, flags) \
    sizeof(*(array)), flags, (char *)(array), \
    SZ((array)), 0, SZ((array)) - 1, SZ((array)) - 1, -1, NULL, NULL, NULL
/* Same for unknown size, size to be filled in by ef_init() */
#define PTR_INIT(ptr, flags) sizeof(*(ptr)), flags, (char *)(ptr), \
    0, 0, 0, 0, -1, NULL, NULL, NULL
/* Common configuration table flags */
#define EFF_CFG (EFF_RDONLY | EFF_MEM | EFF_STATIC)

struct empfile empfile[] = {
    /*
     * How empfile[] is initialized:
     *
     * Members uid, name, file, size, cadef, and the EFF_IMMUTABLE
     * bits of flags get their final value.
     * If flags & EFF_STATIC, the cache is mapped here, and members
     * cache, csize get their final value.
     * Members baseid, cids, fids and the EFF_MEM|EFF_RDONLY bits of
     * flags are initialized according the initial cache contents.
     * Member fd is initialized to -1.
     * Members init, postread, prewrite get initialized to NULL, but
     * that can be changed by users.
     *
     * Whatever of the above can't be done here must be done in
     * ef_init().
     */

    /* Dynamic game data */
    {EF_SECTOR, "sect", "sector", sect_ca,
     UNMAPPED_INIT(struct sctstr, EFF_XY | EFF_OWNER)},
    {EF_SHIP, "ship", "ship", ship_ca,
     UNMAPPED_INIT(struct shpstr, EFF_XY | EFF_OWNER | EFF_GROUP)},
    {EF_PLANE, "plane", "plane", plane_ca,
     UNMAPPED_INIT(struct plnstr, EFF_XY | EFF_OWNER | EFF_GROUP)},
    {EF_LAND, "land", "land", land_ca,
     UNMAPPED_INIT(struct lndstr, EFF_XY | EFF_OWNER | EFF_GROUP)},
    {EF_NUKE, "nuke", "nuke", nuke_ca,
     UNMAPPED_INIT(struct nukstr, EFF_XY | EFF_OWNER)},
    {EF_NEWS, "news", "news", news_ca,
     UNMAPPED_INIT(struct nwsstr, 0)},
    {EF_TREATY, "treaty", "treaty", treaty_ca,
     UNMAPPED_INIT(struct trtstr, 0)},
    {EF_TRADE, "trade", "trade", trade_ca,
     UNMAPPED_INIT(struct trdstr, 0)},
    {EF_POWER, "pow", "power", NULL,
     UNMAPPED_INIT(struct powstr, 0)},
    {EF_NATION, "nat", "nation", nat_ca,
     UNMAPPED_INIT(struct natstr, EFF_OWNER)},
    {EF_LOAN, "loan", "loan", loan_ca,
     UNMAPPED_INIT(struct lonstr, 0)},
    {EF_MAP, "map", "map", NULL,
     0, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL},
    {EF_BMAP, "bmap", "bmap", NULL,
     0, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL},
    {EF_COMM, "commodity", "commodity", commodity_ca,
     UNMAPPED_INIT(struct comstr, 0)},
    {EF_LOST, "lost", "lostitems", lost_ca,
     UNMAPPED_INIT(struct loststr, EFF_OWNER)},

    /* Static game data (configuration) */
    {EF_SECTOR_CHR, "sect chr", NULL, dchr_ca, ARRAY_INIT(dchr, EFF_CFG)},
    {EF_SHIP_CHR, "ship chr", NULL, mchr_ca, PTR_INIT(mchr, EFF_CFG)},
    {EF_PLANE_CHR, "plane chr", NULL, plchr_ca, PTR_INIT(plchr, EFF_CFG)},
    {EF_LAND_CHR, "land chr", NULL, lchr_ca, PTR_INIT(lchr, EFF_CFG)},
    {EF_NUKE_CHR, "nuke chr", NULL, nchr_ca, PTR_INIT(nchr, EFF_CFG)},
    {EF_NEWS_CHR, "news chr", NULL, rpt_ca, ARRAY_INIT(rpt, EFF_CFG)},
    {EF_TREATY_FLAGS, "treaty flags", NULL, symbol_ca,
     PTR_INIT(treaty_flags, EFF_CFG)},
    {EF_ITEM, "item", NULL, ichr_ca, ARRAY_INIT(ichr, EFF_CFG)},
    {EF_INFRASTRUCTURE, "infrastructure", NULL, intrchr_ca,
     ARRAY_INIT(intrchr, EFF_CFG)},
    {EF_PRODUCT, "product", NULL, pchr_ca, ARRAY_INIT(pchr, EFF_CFG)},
    {EF_TABLE, "table", NULL, empfile_ca, ARRAY_INIT(empfile, EFF_CFG)},
    {EF_SHIP_CHR_FLAGS, "ship chr flags", NULL, symbol_ca,
     PTR_INIT(ship_chr_flags, EFF_CFG)},
    {EF_PLANE_CHR_FLAGS, "plane chr flags", NULL, symbol_ca,
     PTR_INIT(plane_chr_flags, EFF_CFG)},
    {EF_LAND_CHR_FLAGS, "land chr flags", NULL, symbol_ca,
     PTR_INIT(land_chr_flags, EFF_CFG)},
    {EF_NUKE_CHR_FLAGS, "nuke chr flags", NULL, symbol_ca,
     PTR_INIT(nuke_chr_flags, EFF_CFG)},
    {EF_META, "meta", NULL, mdchr_ca, PTR_INIT(mdchr_ca, EFF_CFG)},
    {EF_META_TYPE, "meta type", NULL, symbol_ca,
     PTR_INIT(meta_type, EFF_CFG)},
    {EF_META_FLAGS, "meta flags", NULL, symbol_ca,
     PTR_INIT(meta_flags, EFF_CFG)},

    /* Sentinel */
    {EF_BAD, NULL, NULL, NULL, 0, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL},
};
