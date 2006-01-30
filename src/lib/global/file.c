/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  file.c: Empire game data file descriptions.
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

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

#include "commodity.h"
#include "nsc.h"

/* Number of elements in ARRAY.  */
#define SZ(array) (sizeof(array) / sizeof((array)[0]))

/* Initializers for members flags... */
/* Unmapped cache */
#define UNMAPPED_CACHE(type, flags) \
    sizeof(type), flags, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL
/*
 * Mapped cache, array with known size.
 * Members cids, fids are not set.
 */
#define ARRAY_CACHE(array, flags) \
    sizeof(*(array)), (flags), (char *)(array), \
    SZ((array)), 0, 0, 0, -1, NULL, NULL, NULL
/*
 * Mapped cache, array with unknown size.
 * Members csize, cids, fids are not set.
 */
#define PTR_CACHE(ptr, flags) \
    sizeof(*(ptr)), (flags), (char *)(ptr), \
    0, 0, 0, 0, -1, NULL, NULL, NULL
/*
 * Array-backed table.
 * The array's last element is the sentinel.
 */
#define ARRAY_TABLE(array, flags) \
    sizeof(*(array)), (flags), (char *)(array), \
    SZ((array)), 0, SZ((array)) - 1, SZ((array)) - 1, -1, NULL, NULL, NULL

/* Common configuration table flags */
#define EFF_CFG (EFF_RDONLY | EFF_MEM | EFF_STATIC)

struct empfile empfile[] = {
    /*
     * How this initializer works:
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

    /*
     * Keep in mind that player command arguments are matched against
     * member name; no whitespace please.
     */

    /* Dynamic game data */
    {EF_SECTOR, "sect", "sector", sect_ca,
     UNMAPPED_CACHE(struct sctstr, EFF_XY | EFF_OWNER)},
    {EF_SHIP, "ship", "ship", ship_ca,
     UNMAPPED_CACHE(struct shpstr, EFF_XY | EFF_OWNER | EFF_GROUP)},
    {EF_PLANE, "plane", "plane", plane_ca,
     UNMAPPED_CACHE(struct plnstr, EFF_XY | EFF_OWNER | EFF_GROUP)},
    {EF_LAND, "land", "land", land_ca,
     UNMAPPED_CACHE(struct lndstr, EFF_XY | EFF_OWNER | EFF_GROUP)},
    {EF_NUKE, "nuke", "nuke", nuke_ca,
     UNMAPPED_CACHE(struct nukstr, EFF_XY | EFF_OWNER)},
    {EF_NEWS, "news", "news", news_ca,
     UNMAPPED_CACHE(struct nwsstr, 0)},
    {EF_TREATY, "treaty", "treaty", treaty_ca,
     UNMAPPED_CACHE(struct trtstr, 0)},
    {EF_TRADE, "trade", "trade", trade_ca,
     UNMAPPED_CACHE(struct trdstr, 0)},
    {EF_POWER, "pow", "power", NULL,
     UNMAPPED_CACHE(struct powstr, 0)},
    {EF_NATION, "nat", "nation", nat_ca,
     UNMAPPED_CACHE(struct natstr, EFF_OWNER)},
    {EF_LOAN, "loan", "loan", loan_ca,
     UNMAPPED_CACHE(struct lonstr, 0)},
    {EF_MAP, "map", "map", NULL,
     0, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL},
    {EF_BMAP, "bmap", "bmap", NULL,
     0, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL},
    {EF_COMM, "commodity", "commodity", commodity_ca,
     UNMAPPED_CACHE(struct comstr, 0)},
    {EF_LOST, "lost", "lostitems", lost_ca,
     UNMAPPED_CACHE(struct loststr, EFF_OWNER)},
    {EF_REALM, "realm", "realms", realm_ca,
     UNMAPPED_CACHE(struct realmstr, EFF_OWNER)},

    /* Static game data (configuration) */
    {EF_SECTOR_CHR, "sect-chr", "sect_def", dchr_ca, ARRAY_TABLE(dchr, EFF_CFG)},
    {EF_SHIP_CHR, "ship-chr", "ship_def", mchr_ca, ARRAY_CACHE(mchr, EFF_CFG)},
    {EF_PLANE_CHR, "plane-chr", "plane_def", plchr_ca, ARRAY_CACHE(plchr, EFF_CFG)},
    {EF_LAND_CHR, "land-chr", "land_def", lchr_ca, ARRAY_CACHE(lchr, EFF_CFG)},
    {EF_NUKE_CHR, "nuke-chr", "nuke_def", nchr_ca, ARRAY_CACHE(nchr, EFF_CFG)},
    {EF_NEWS_CHR, "news-chr", NULL, rpt_ca, ARRAY_TABLE(rpt, EFF_CFG)},
    {EF_TREATY_FLAGS, "treaty-flags", NULL, symbol_ca,
     PTR_CACHE(treaty_flags, EFF_CFG)},
    {EF_ITEM, "item", "item_def", ichr_ca, ARRAY_TABLE(ichr, EFF_CFG)},
    {EF_INFRASTRUCTURE, "infrastructure", "infrastructure_def", intrchr_ca,
     ARRAY_TABLE(intrchr, EFF_CFG)},
    {EF_PRODUCT, "product", "product_def", pchr_ca, ARRAY_TABLE(pchr, EFF_CFG)},
    {EF_TABLE, "table", NULL, empfile_ca, ARRAY_TABLE(empfile, EFF_CFG)},
    {EF_SHIP_CHR_FLAGS, "ship-chr-flags", NULL, symbol_ca,
     PTR_CACHE(ship_chr_flags, EFF_CFG)},
    {EF_PLANE_CHR_FLAGS, "plane-chr-flags", NULL, symbol_ca,
     PTR_CACHE(plane_chr_flags, EFF_CFG)},
    {EF_LAND_CHR_FLAGS, "land-chr-flags", NULL, symbol_ca,
     PTR_CACHE(land_chr_flags, EFF_CFG)},
    {EF_NUKE_CHR_FLAGS, "nuke-chr-flags", NULL, symbol_ca,
     PTR_CACHE(nuke_chr_flags, EFF_CFG)},
    {EF_META, "meta", NULL, mdchr_ca, PTR_CACHE(mdchr_ca, EFF_CFG)},
    {EF_META_TYPE, "meta-type", NULL, symbol_ca,
     PTR_CACHE(meta_type, EFF_CFG)},
    {EF_META_FLAGS, "meta-flags", NULL, symbol_ca,
     PTR_CACHE(meta_flags, EFF_CFG)},
    {EF_MISSIONS, "missions", NULL, symbol_ca, PTR_CACHE(missions, EFF_CFG)},
    {EF_PLANE_FLAGS, "plane-flags", NULL, symbol_ca,
     PTR_CACHE(plane_flags, EFF_CFG)},
    {EF_RETREAT_FLAGS, "retreat-flags", NULL, symbol_ca,
     PTR_CACHE(retreat_flags, EFF_CFG)},
    {EF_NATION_FLAGS, "nation-flags", NULL, symbol_ca,
     PTR_CACHE(nation_flags, EFF_CFG)},
    {EF_NATION_RELATIONS, "nation-relationships", NULL, symbol_ca,
     PTR_CACHE(nation_relations, EFF_CFG)},
    {EF_LEVEL, "level", NULL, symbol_ca, PTR_CACHE(level, EFF_CFG)},
    {EF_AGREEMENT_STATUS, "agreement-status", NULL, symbol_ca,
     PTR_CACHE(agreement_statuses, EFF_CFG)},
    {EF_PLAGUE_STAGES, "plague-stages", NULL, symbol_ca,
     PTR_CACHE(plague_stages, EFF_CFG)},
    {EF_PACKING, "packing", NULL, symbol_ca, PTR_CACHE(packing, EFF_CFG)},
    {EF_RESOURCES, "resources", NULL, symbol_ca, PTR_CACHE(resources, EFF_CFG)},
    {EF_NATION_STATUS, "nation-status", NULL, symbol_ca,
     PTR_CACHE(nation_status, EFF_CFG)},
    {EF_SECTOR_NAVIGATION, "sector-navigation", NULL, symbol_ca,
     PTR_CACHE(sector_navigation, EFF_CFG)},
    {EF_PAGE_HEADINGS, "page-headings", NULL, symbol_ca,
     PTR_CACHE(page_headings, EFF_CFG)},

    /* Views */
    {EF_COUNTRY, "country", NULL, cou_ca, UNMAPPED_CACHE(struct natstr, 0)},

    /* Sentinel */
    {EF_BAD, NULL, NULL, NULL, 0, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL},
};
