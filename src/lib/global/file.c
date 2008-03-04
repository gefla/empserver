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
 *  file.c: Empire game data file descriptions.
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2005-2007
 */

#include <config.h>

#include <stddef.h>
#include "commodity.h"
#include "file.h"
#include "game.h"
#include "land.h"
#include "loan.h"
#include "lost.h"
#include "nat.h"
#include "news.h"
#include "nsc.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"
#include "power.h"
#include "product.h"
#include "sect.h"
#include "ship.h"
#include "server.h"
#include "trade.h"
#include "treaty.h"
#include "version.h"
#include "xy.h"

/* Number of elements in ARRAY.  */
#define SZ(array) (sizeof(array) / sizeof((array)[0]))

/* Initializers for members flags... */
/* Unmapped cache */
#define UNMAPPED_CACHE(type, flags) \
    sizeof(type), (flags), NULL, 0, 0, 0, 0, -1, NULL, NULL
/*
 * Mapped cache, array with known size.
 * Members cids, fids are not set.
 */
#define ARRAY_CACHE(array, flags) \
    sizeof(*(array)), (flags), (char *)(array), \
    SZ((array)), 0, 0, 0, -1, NULL, NULL
/*
 * Mapped cache, array with unknown size.
 * Members csize, cids, fids are not set.
 */
#define PTR_CACHE(ptr, flags) \
    sizeof(*(ptr)), (flags), (char *)(ptr), \
    0, 0, 0, 0, -1, NULL, NULL
/*
 * Array-backed table.
 * The array's last element is the sentinel.
 */
#define ARRAY_TABLE(array, flags) \
    sizeof(*(array)), (flags), (char *)(array), \
    SZ((array)), 0, SZ((array)) - 1, SZ((array)) - 1, -1, NULL, NULL

/* Common configuration table flags */
#define EFF_CFG (EFF_PRIVATE | EFF_MEM | EFF_STATIC)

struct empfile empfile[] = {
    /*
     * How this initializer works:
     *
     * Members uid, name, file, cadef, size, and the EFF_IMMUTABLE
     * bits of flags get their final value.
     * If flags & EFF_STATIC, the cache is mapped here, and members
     * cache, csize get their final value.
     * Members baseid, cids, fids and the EFF_MEM|EFF_PRIVATE bits of
     * flags are initialized according the initial cache contents.
     * Member fd is initialized to -1.
     * Members init, postread, prewrite get initialized to NULL, but
     * that can be changed by users.
     *
     * Whatever of the above can't be done here must be done in
     * empfile_init() or empfile_fixup().  Except cadef may be set in
     * nsc_init() instead.
     */

    /*
     * Keep in mind that player command arguments are matched against
     * values of member name: no whitespace there, please.
     */

    /*
     * Dynamic game data
     *
     * All caches unmapped.  EF_MAP and EF_BMAP get a bogus size here.
     * Fixed up by empfile_fixup().
     */
    {EF_SECTOR, "sect", "sector", sect_ca,
     UNMAPPED_CACHE(struct sctstr, EFF_TYPED | EFF_XY | EFF_OWNER)},
    {EF_SHIP, "ship", "ship", ship_ca,
     UNMAPPED_CACHE(struct shpstr,
		    EFF_TYPED | EFF_XY | EFF_OWNER | EFF_GROUP)},
    {EF_PLANE, "plane", "plane", plane_ca,
     UNMAPPED_CACHE(struct plnstr,
		    EFF_TYPED | EFF_XY | EFF_OWNER | EFF_GROUP)},
    {EF_LAND, "land", "land", land_ca,
     UNMAPPED_CACHE(struct lndstr,
		    EFF_TYPED | EFF_XY | EFF_OWNER | EFF_GROUP)},
    {EF_NUKE, "nuke", "nuke", nuke_ca,
     UNMAPPED_CACHE(struct nukstr, EFF_TYPED | EFF_XY | EFF_OWNER)},
    {EF_NEWS, "news", "news", news_ca,
     UNMAPPED_CACHE(struct nwsstr, EFF_TYPED)},
    {EF_TREATY, "treaty", "treaty", treaty_ca,
     UNMAPPED_CACHE(struct trtstr, EFF_TYPED)},
    {EF_TRADE, "trade", "trade", trade_ca,
     UNMAPPED_CACHE(struct trdstr, EFF_TYPED | EFF_OWNER)},
    {EF_POWER, "pow", "power", NULL,
     UNMAPPED_CACHE(struct powstr, 0)},
    {EF_NATION, "nat", "nation", nat_ca,
     UNMAPPED_CACHE(struct natstr, EFF_TYPED | EFF_OWNER)},
    {EF_LOAN, "loan", "loan", loan_ca,
     UNMAPPED_CACHE(struct lonstr, EFF_TYPED)},
    {EF_MAP, "map", "map", NULL,
     0, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL},
    {EF_BMAP, "bmap", "bmap", NULL,
     0, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL},
    {EF_COMM, "commodity", "commodity", commodity_ca,
     UNMAPPED_CACHE(struct comstr, EFF_TYPED | EFF_OWNER)},
    {EF_LOST, "lost", "lostitems", lost_ca,
     UNMAPPED_CACHE(struct loststr, EFF_TYPED | EFF_OWNER)},
    {EF_REALM, "realm", "realms", realm_ca,
     UNMAPPED_CACHE(struct realmstr, EFF_TYPED | EFF_OWNER)},
    {EF_GAME, "game", "game", game_ca,
     UNMAPPED_CACHE(struct gamestr, EFF_TYPED)},

    /*
     * Static game data (configuration)
     */
    /*
     * Characteristics tables.  Characteristics with a null file
     * member are compiled in.  The others are empty; use
     * read_builtin_tables() to fill them.
     */
    {EF_ITEM, "item", "item.config", ichr_ca,
     ARRAY_CACHE(ichr, EFF_CFG)},
    {EF_PRODUCT, "product", "product.config", pchr_ca,
     ARRAY_CACHE(pchr, EFF_CFG)},
    {EF_SECTOR_CHR, "sect-chr", "sect.config", dchr_ca,
     ARRAY_CACHE(dchr, EFF_CFG)},
    {EF_SHIP_CHR, "ship-chr", "ship.config", mchr_ca,
     ARRAY_CACHE(mchr, EFF_CFG)},
    {EF_PLANE_CHR, "plane-chr", "plane.config", plchr_ca,
     ARRAY_CACHE(plchr, EFF_CFG)},
    {EF_LAND_CHR, "land-chr", "land.config", lchr_ca,
     ARRAY_CACHE(lchr, EFF_CFG)},
    {EF_NUKE_CHR, "nuke-chr", "nuke.config", nchr_ca,
     ARRAY_CACHE(nchr, EFF_CFG)},
    {EF_NEWS_CHR, "news-chr", NULL, rpt_ca,
     ARRAY_TABLE(rpt, EFF_CFG)},
    {EF_INFRASTRUCTURE, "infrastructure", "infra.config", intrchr_ca,
     ARRAY_CACHE(intrchr, EFF_CFG)},
    /*
     * Update schedule table.  Use read_schedule() to fill.
     */
    {EF_UPDATES, "updates", NULL, update_ca,
     ARRAY_TABLE(update_time, EFF_CFG)},
    /*
     * Special tables.  EF_META gets bogus size, cids and fids here.
     * Fixed up by empfile_init().  EF_VERSION's cadef is set by
     * nsc_init().
     */
    {EF_TABLE, "table", NULL, empfile_ca,
     ARRAY_TABLE(empfile, EFF_CFG)},
    {EF_VERSION, "version", NULL, NULL,
     sizeof(PACKAGE_STRING), 0, version, 0, 0, 1, 1, -1, NULL, NULL},
    {EF_META, "meta", NULL, mdchr_ca,
     PTR_CACHE(mdchr_ca, EFF_CFG)},

    /*
     * Symbol tables
     *
     * These get bogus size, cids and fids here.  Fixed up by
     * empfile_init().
     */
#define SYMTAB(type, name, tab) \
	{(type), (name), NULL, symbol_ca, PTR_CACHE((tab), EFF_CFG)}
    SYMTAB(EF_AGREEMENT_STATUS, "agreement-status", agreement_statuses),
    SYMTAB(EF_LAND_CHR_FLAGS, "land-chr-flags", land_chr_flags),
    SYMTAB(EF_LEVEL, "level", level),
    SYMTAB(EF_META_FLAGS, "meta-flags", meta_flags),
    SYMTAB(EF_META_TYPE, "meta-type", meta_type),
    SYMTAB(EF_MISSIONS, "missions", missions),
    SYMTAB(EF_NATION_FLAGS, "nation-flags", nation_flags),
    SYMTAB(EF_NATION_REJECTS, "nation-rejects", nation_rejects),
    SYMTAB(EF_NATION_RELATIONS, "nation-relationships", nation_relations),
    SYMTAB(EF_NATION_STATUS, "nation-status", nation_status),
    SYMTAB(EF_NUKE_CHR_FLAGS, "nuke-chr-flags", nuke_chr_flags),
    SYMTAB(EF_PACKING, "packing", packing),
    SYMTAB(EF_PAGE_HEADINGS, "page-headings", page_headings),
    SYMTAB(EF_PLAGUE_STAGES, "plague-stages", plague_stages),
    SYMTAB(EF_PLANE_CHR_FLAGS, "plane-chr-flags", plane_chr_flags),
    SYMTAB(EF_PLANE_FLAGS, "plane-flags", plane_flags),
    SYMTAB(EF_RESOURCES, "resources", resources),
    SYMTAB(EF_RETREAT_FLAGS, "retreat-flags", retreat_flags),
    SYMTAB(EF_SECTOR_NAVIGATION, "sector-navigation", sector_navigation),
    SYMTAB(EF_SHIP_CHR_FLAGS, "ship-chr-flags", ship_chr_flags),
    SYMTAB(EF_TREATY_FLAGS, "treaty-flags", treaty_flags),

    /* Views */
    {EF_COUNTRY, "country", NULL, cou_ca,
     UNMAPPED_CACHE(struct natstr, EFF_TYPED | EFF_OWNER)},

    /* Sentinel */
    {EF_BAD, NULL, NULL, NULL, 0, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL},
};

static void
ef_fix_size(struct empfile *ep, int n)
{
    ep->cids = ep->fids = n;
    ep->csize = n + 1;
}

void
empfile_init(void)
{
    struct castr *ca;
    struct empfile *ep;
    struct symbol *lup;
    int i;

    ca = (struct castr *)empfile[EF_META].cache;
    for (i = 0; ca[i].ca_name; i++) ;
    ef_fix_size(&empfile[EF_META], i);

    for (ep = empfile; ep->uid >= 0; ep++) {
	if (ep->cadef == symbol_ca) {
	    lup = (struct symbol *)ep->cache;
	    for (i = 0; lup[i].name; i++) ;
	    ef_fix_size(ep, i);
	}
    }
}

void
empfile_fixup(void)
{
    empfile[EF_MAP].size = empfile[EF_BMAP].size = WORLD_SZ();
}
