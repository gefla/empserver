/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  filetable.c: Empire game data file descriptions.
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2005-2008
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

static void sct_oninit(void *);
static void pln_oninit(void *);
static void lnd_oninit(void *);
static void nuk_oninit(void *);
static void nat_oninit(void *);
static void realm_oninit(void *);
static void game_oninit(void *);
static void pchr_oninit(void *);
static void mchr_oninit(void *);
static void plchr_oninit(void *);
static void lchr_oninit(void *);
static void nchr_oninit(void *);

/* Number of elements in ARRAY.  */
#define SZ(array) (sizeof(array) / sizeof((array)[0]))

/* Initializers for members flags... */
/* Unmapped cache */
#define UNMAPPED_CACHE(type, nent, flags)	\
    sizeof(type), (nent), (flags), NULL,	\
    0, 0, 0, 0, -1
/*
 * Mapped cache, array with known size.
 * Members cids, fids are zero, i.e. cache is empty.
 */
#define ARRAY_CACHE(array, flags) \
    sizeof(*(array)), -1, (flags), (char *)(array),	\
    SZ((array)), 0, 0, 0, -1
/*
 * Mapped cache, array with unknown size.
 * Member csize gets a bogus value, needs to be fixed up.
 * Members cids, fids are zero, i.e. cache is empty.
 */
#define PTR_CACHE(ptr, flags) \
    sizeof(*(ptr)), -1, (flags), (char *)(ptr),	\
    0, 0, 0, 0, -1
/*
 * Array-backed table of fixed size.
 */
#define ARRAY_TABLE(array, nent, flags)			\
    sizeof(*(array)), (nent), (flags), (char *)(array),	\
    SZ((array)), 0, (nent), (nent), -1

/* Common configuration table flags */
#define EFF_CFG (EFF_PRIVATE | EFF_MEM | EFF_STATIC | EFF_SENTINEL)

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
     * All caches unmapped.  EF_SECTOR gets bogus nent here.  EF_MAP
     * and EF_BMAP get a bogus size here.  Fixed up by
     * empfile_fixup().
     */
    {EF_SECTOR, "sect", "sector", sect_ca, EF_BAD,
     UNMAPPED_CACHE(struct sctstr, -1, EFF_TYPED | EFF_XY | EFF_OWNER),
     sct_oninit, NULL, NULL, NULL},
    {EF_SHIP, "ship", "ship", ship_ca, EF_BAD,
     UNMAPPED_CACHE(struct shpstr, -1,
		    EFF_TYPED | EFF_XY | EFF_OWNER | EFF_GROUP),
     NULL, NULL, NULL, NULL},
    {EF_PLANE, "plane", "plane", plane_ca, EF_BAD,
     UNMAPPED_CACHE(struct plnstr, -1,
		    EFF_TYPED | EFF_XY | EFF_OWNER | EFF_GROUP),
     pln_oninit, NULL, NULL, NULL},
    {EF_LAND, "land", "land", land_ca, EF_BAD,
     UNMAPPED_CACHE(struct lndstr, -1,
		    EFF_TYPED | EFF_XY | EFF_OWNER | EFF_GROUP),
     lnd_oninit, NULL, NULL, NULL},
    {EF_NUKE, "nuke", "nuke", nuke_ca, EF_BAD,
     UNMAPPED_CACHE(struct nukstr, -1, EFF_TYPED | EFF_XY | EFF_OWNER),
     nuk_oninit, NULL, NULL, NULL},
    {EF_NEWS, "news", "news", news_ca, EF_BAD,
     UNMAPPED_CACHE(struct nwsstr, -1, 0),
     NULL, NULL, NULL, NULL},
    {EF_TREATY, "treaty", "treaty", treaty_ca, EF_BAD,
     UNMAPPED_CACHE(struct trtstr, -1, EFF_TYPED),
     NULL, NULL, NULL, NULL},
    {EF_TRADE, "trade", "trade", trade_ca, EF_BAD,
     UNMAPPED_CACHE(struct trdstr, -1, EFF_TYPED | EFF_OWNER),
     NULL, NULL, NULL, NULL},
    {EF_POWER, "pow", "power", NULL, EF_BAD,
     UNMAPPED_CACHE(struct powstr, -1, 0),
     NULL, NULL, NULL, NULL},
    {EF_NATION, "nat", "nation", nat_ca, EF_BAD,
     UNMAPPED_CACHE(struct natstr, MAXNOC, EFF_TYPED | EFF_OWNER),
     nat_oninit, NULL, NULL, NULL},
    {EF_LOAN, "loan", "loan", loan_ca, EF_BAD,
     UNMAPPED_CACHE(struct lonstr, -1, EFF_TYPED),
     NULL, NULL, NULL, NULL},
    {EF_MAP, "map", "map", NULL, EF_BAD,
     0, MAXNOC, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL, NULL},
    {EF_BMAP, "bmap", "bmap", NULL, EF_BAD,
     0, MAXNOC, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL, NULL},
    {EF_COMM, "commodity", "commodity", commodity_ca, EF_BAD,
     UNMAPPED_CACHE(struct comstr, -1, EFF_TYPED | EFF_OWNER),
     NULL, NULL, NULL, NULL},
    {EF_LOST, "lost", "lostitems", lost_ca, EF_BAD,
     UNMAPPED_CACHE(struct loststr, -1, EFF_TYPED | EFF_OWNER),
     NULL, NULL, NULL, NULL},
    {EF_REALM, "realm", "realms", realm_ca, EF_BAD,
     UNMAPPED_CACHE(struct realmstr, MAXNOC * MAXNOR,
		    EFF_TYPED | EFF_OWNER),
     realm_oninit, NULL, NULL, NULL},
    {EF_GAME, "game", "game", game_ca, EF_BAD,
     UNMAPPED_CACHE(struct gamestr, 1, EFF_TYPED),
     game_oninit, NULL, NULL, NULL},

    /*
     * Static game data (configuration)
     */
    /*
     * Characteristics tables.  Characteristics with a null file
     * member are compiled in.  The others are empty; use
     * read_builtin_tables() to fill them.
     */
    {EF_ITEM, "item", "item.config", ichr_ca, EF_BAD,
     ARRAY_CACHE(ichr, EFF_CFG), NULL, NULL, NULL, NULL},
    {EF_PRODUCT, "product", "product.config", pchr_ca, EF_BAD,
     ARRAY_CACHE(pchr, EFF_CFG), pchr_oninit, NULL, NULL, NULL},
    {EF_SECTOR_CHR, "sect-chr", "sect.config", dchr_ca, EF_BAD,
     ARRAY_CACHE(dchr, EFF_CFG), NULL, NULL, NULL, NULL},
    {EF_SHIP_CHR, "ship-chr", "ship.config", mchr_ca, EF_BAD,
     ARRAY_CACHE(mchr, EFF_CFG), mchr_oninit, NULL, NULL, NULL},
    {EF_PLANE_CHR, "plane-chr", "plane.config", plchr_ca, EF_BAD,
     ARRAY_CACHE(plchr, EFF_CFG), plchr_oninit, NULL, NULL, NULL},
    {EF_LAND_CHR, "land-chr", "land.config", lchr_ca, EF_BAD,
     ARRAY_CACHE(lchr, EFF_CFG), lchr_oninit, NULL, NULL, NULL},
    {EF_NUKE_CHR, "nuke-chr", "nuke.config", nchr_ca, EF_BAD,
     ARRAY_CACHE(nchr, EFF_CFG), nchr_oninit, NULL, NULL, NULL},
    {EF_NEWS_CHR, "news-chr", NULL, rpt_ca, EF_BAD,
     ARRAY_TABLE(rpt, N_MAX_VERB + 1, EFF_CFG), NULL, NULL, NULL, NULL},
    {EF_INFRASTRUCTURE, "infrastructure", "infra.config", intrchr_ca, EF_BAD,
     ARRAY_CACHE(intrchr, EFF_CFG), NULL, NULL, NULL, NULL},
    /*
     * Update schedule table.  Use read_schedule() to fill.
     */
    {EF_UPDATES, "updates", NULL, update_ca, EF_BAD,
     ARRAY_CACHE(update_time, EFF_CFG),
     NULL, NULL, NULL, NULL},
    /*
     * Special tables.  EF_META gets bogus size, cids and fids here.
     * Fixed up by empfile_init().  EF_VERSION's cadef is set by
     * nsc_init().
     */
    {EF_TABLE, "table", NULL, empfile_ca, EF_BAD,
     ARRAY_TABLE(empfile, EF_MAX, EFF_CFG),
     NULL, NULL, NULL, NULL},
    {EF_VERSION, "version", NULL, NULL, EF_BAD,
     sizeof(PACKAGE_STRING), -1, EFF_STATIC, version, 1, 0, 1, 1, -1,
     NULL, NULL, NULL, NULL},
    {EF_META, "meta", NULL, mdchr_ca, EF_BAD,
     PTR_CACHE(mdchr_ca, EFF_CFG),
     NULL, NULL, NULL, NULL},

    /*
     * Symbol tables
     *
     * These get bogus csize, cids and fids here.  Fixed up by
     * empfile_init().
     */
#define SYMTAB(type, name, tab) {(type), (name), NULL, symbol_ca, EF_BAD, \
				 PTR_CACHE((tab), EFF_CFG),		\
				 NULL, NULL, NULL, NULL}
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
    {EF_COUNTRY, "country", NULL, cou_ca, EF_NATION,
     UNMAPPED_CACHE(struct natstr, MAXNOC, EFF_TYPED | EFF_OWNER),
     NULL, NULL, NULL, NULL},

    /* Sentinel */
    {EF_BAD, NULL, NULL, NULL, EF_BAD,
     0, -1, 0, NULL, 0, 0, 0, 0, -1, NULL, NULL, NULL, NULL},
};

static void
sct_oninit(void *ptr)
{
    struct sctstr *sp = (struct sctstr *)ptr;

    sp->sct_y = sp->sct_uid * 2 / WORLD_X;
    sp->sct_x = sp->sct_uid * 2 % WORLD_X + sp->sct_y % 2;
    sp->sct_dist_x = sp->sct_x;
    sp->sct_dist_y = sp->sct_y;
    sp->sct_newtype = sp->sct_type = SCT_WATER;
    sp->sct_coastal = 1;
}

static void
pln_oninit(void *ptr)
{
    struct plnstr *pp = ptr;

    pp->pln_ship = pp->pln_land = -1;
}

static void
lnd_oninit(void *ptr)
{
    struct lndstr *lp = ptr;

    lp->lnd_ship = lp->lnd_land = -1;
}

static void
nuk_oninit(void *ptr)
{
    struct nukstr *np = ptr;

    np->nuk_plane = -1;
}

static void
nat_oninit(void *ptr)
{
    struct natstr *np = ptr;

    np->nat_cnum = np->nat_uid;
}

static void
realm_oninit(void *ptr)
{
    struct realmstr *realm = ptr;

    realm->r_cnum = realm->r_uid / MAXNOR;
    realm->r_realm = realm->r_uid % MAXNOR;
}

static void
game_oninit(void *ptr)
{
    ((struct gamestr *)ptr)->game_turn = 1;
}

static void
pchr_oninit(void *ptr)
{
    ((struct pchrstr *)ptr)->p_sname = "";
}

static void
mchr_oninit(void *ptr)
{
    ((struct mchrstr *)ptr)->m_name = "";
}

static void
plchr_oninit(void *ptr)
{
    ((struct plchrstr *)ptr)->pl_name = "";
}

static void
lchr_oninit(void *ptr)
{
    ((struct lchrstr *)ptr)->l_name = "";
}

static void
nchr_oninit(void *ptr)
{
    ((struct nchrstr *)ptr)->n_name = "";
}

static void
ef_fix_size(struct empfile *ep, int n)
{
    ep->nent = ep->cids = ep->fids = n;
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
    empfile[EF_SECTOR].nent = WORLD_SZ();
    empfile[EF_MAP].size = empfile[EF_BMAP].size = WORLD_SZ();
}
