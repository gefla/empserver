/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  file.h: Describes Empire tables (`files' for historical reasons)
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2005-2010
 */

#ifndef FILE_H
#define FILE_H

#include <time.h>

struct empfile {
    /* Members with immutable values */
    int uid;			/* Table ID */
    char *name;			/* Empire name (e.g., "treaty") */
    char *file;			/* file name, relative to gamedir for
				   game state, to builtindir for config */
    struct castr *cadef;	/* table column selectors (column meta-data) */
    int base;			/* view's base table, else EF_BAD */
    int size;			/* size of a table entry */
    int nent;			/* #table entries, -1 if variable */
    int flags;			/* only EFF_IMMUTABLE immutable, see below
				   for use of remaining bits */

    /* Members whose values are fixed when the cache is mapped */
    char *cache;		/* pointer to cache */
    int csize;			/* cache size, in entries */
    /* flags bits EFF_MEM, EFF_PRIVATE, EFF_NOTIME also fixed then */

    /* Members whose values may vary throughout operation */
    int baseid;			/* id of first entry in cache */
    int cids;			/* # entries in cache */
    int fids;			/* # entries in table */
    int fd;			/* file descriptor, -1 if not open */
    /* flags bit EFF_CUSTOM also varies */

    /* User callbacks, may all be null */
    /*
     * Called after element initialization.  ELT is the element.
     * May modify the element.
     */
    void (*oninit)(void *elt);
    /*
     * Called after read.  ID is the element id, and ELT is the
     * element read.  May modify the element.  Modifications are
     * visible to caller of ef_read(), but have no effect on the file.
     */
    void (*postread)(int id, void *elt);
    /*
     * Called before write.  ID is the element id, OLD is the element
     * being updated (null unless it is cached) and ELT is the element
     * being written.  May modify the element.  Modifications will be
     * visible to caller of ef_write() and are written to the file.
     */
    void (*prewrite)(int id, void *old, void *elt);
    /*
     * Called after table size changed, with file type as argument.
     */
    void (*onresize)(int type);
};

struct emptypedstr {
    signed ef_type: 8;
    unsigned seqno: 12;
    unsigned generation: 12;
    int uid;
    time_t timestamp;
};

/*
 * Flag bits for struct empfile member flags
 * Immutable flags are properties of the table and thus cannot change.
 * The remaining flags record how the table is being used.
 */
/* Immutable flags, fixed at compile-time */
/* Dereferencing entry address cast to struct emptypedstr * is safe */
#define EFF_TYPED	bit(0)
/*
 * EFF_XY / EFF_OWNER / EFF_GROUP assert that coordinates / owner /
 * group of such a table's entries can be safely obtained by
 * dereferencing the entry's address cast to struct empobj *.
 */
#define EFF_XY		bit(1)
#define EFF_OWNER	bit(2)
#define EFF_GROUP	bit(3)
/* Table cache is allocated statically */
#define EFF_STATIC	bit(4)
/* Table has a sentinel (all zero, not counted as elt), implies EFF_MEM */
#define EFF_SENTINEL	bit(5)
/* All the immutable flags */
#define EFF_IMMUTABLE \
    (EFF_TYPED | EFF_XY | EFF_OWNER | EFF_GROUP | EFF_STATIC | EFF_SENTINEL)
/* Flags set when table contents is mapped */
/* Table is entirely in memory */
#define EFF_MEM		bit(8)
/* Table is privately mapped: changes don't affect the underlying file */
#define EFF_PRIVATE	bit(9)
/* Table is customized (configuration tables only) */
#define EFF_CUSTOM	bit(10)
/* Don't update timestamps */
#define EFF_NOTIME	bit(11)
/* Transient flags, only occur in argument of ef_open() */
/* Create table file, clobbering any existing file */
#define EFF_CREATE	bit(16)

/*
 * Empire `file types'
 * These are really table IDs.  Some tables are backed by files, some
 * are compiled into the server, some initialized from configuration
 * files.
 */
enum {
    /* Error value */
    EF_BAD = -1,
    /* Dynamic game data tables */
    EF_SECTOR,
    EF_SHIP,
    EF_PLANE,
    EF_LAND,
    EF_NUKE,
    EF_NEWS,
    EF_TREATY,
    EF_TRADE,
    EF_POWER,
    EF_NATION,
    EF_LOAN,
    EF_MAP,
    EF_BMAP,
    EF_COMM,
    EF_LOST,
    EF_REALM,
    EF_GAME,
    EF_DYNMAX = EF_GAME,
    /* Static game data (configuration) */
    /* Order is relevant; see read_builtin_tables() */
    EF_ITEM,
    EF_PRODUCT,
    EF_SECTOR_CHR,
    EF_SHIP_CHR,
    EF_PLANE_CHR,
    EF_LAND_CHR,
    EF_NUKE_CHR,
    EF_NEWS_CHR,
    EF_INFRASTRUCTURE,
    EF_UPDATES,			/* not actually static */
    EF_TABLE,
    EF_VERSION,
    EF_META,			/* not really configuration */
    /* Symbol tables */
    EF_AGREEMENT_STATUS,
    EF_LAND_CHR_FLAGS,
    EF_LEVEL,
    EF_META_FLAGS,
    EF_META_TYPE,
    EF_MISSIONS,
    EF_NATION_FLAGS,
    EF_NATION_REJECTS,
    EF_NATION_RELATIONS,
    EF_NATION_STATUS,
    EF_NUKE_CHR_FLAGS,
    EF_PACKING,
    EF_PAGE_HEADINGS,
    EF_PLAGUE_STAGES,
    EF_PLANE_CHR_FLAGS,
    EF_PLANE_FLAGS,
    EF_RESOURCES,
    EF_RETREAT_FLAGS,
    EF_SECTOR_NAVIGATION,
    EF_SHIP_CHR_FLAGS,
    EF_TREATY_FLAGS,
    /* Views */
    EF_COUNTRY,
    /* Number of types: */
    EF_MAX
};

#define EF_IS_GAME_STATE(type) (EF_SECTOR <= (type) && (type) <= EF_DYNMAX)
#define EF_IS_VIEW(type) (empfile[(type)].base != EF_BAD)

extern struct castr *ef_cadef(int);
extern int ef_read(int, int, void *);
extern void ef_make_stale(void);
extern void ef_mark_fresh(int, void *);
extern void *ef_ptr(int, int);
extern char *ef_nameof(int);
extern time_t ef_mtime(int);
extern int ef_open(int, int);
extern int ef_open_view(int);
extern int ef_close(int);
extern int ef_flush(int);
extern void ef_blank(int, int, void *);
extern int ef_write(int, int, void *);
extern void ef_set_uid(int, void *, int);
extern int ef_extend(int, int);
extern int ef_ensure_space(int, int, int);
extern int ef_id_limit(int);
extern int ef_truncate(int, int);
extern int ef_nelem(int);
extern int ef_flags(int);
extern int ef_byname(char *);
extern int ef_byname_from(char *, int *);
extern int ef_verify(int);
extern int ef_elt_byname(int, char *);

extern struct empfile empfile[EF_MAX + 1];
extern void empfile_init(void);
extern void empfile_fixup(void);

#endif
