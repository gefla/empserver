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
 *  file.h: Describes Empire tables (`files' for historical reasons)
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2005
 */

#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <time.h>

struct empfile {
    /* Members with immutable values */
    int uid;			/* Table ID */
    char *name;			/* Empire name (e.g., "treaty") */
    char *file;			/* if backed by file, file name relative to
				   data directory */
    struct castr *cadef;	/* table column selectors (column meta-data) */
    int size;			/* size of a table entry */
    int flags;			/* only EFF_IMMUTABLE immutable, see below
				   for use of remaining bits */
    /* Members whose values are fixed when the cache is mapped */
    char *cache;		/* pointer to cache */
    int csize;			/* cache size, in entries */
    /* and flags bit EFF_MEM */
    /* Members whose values may vary throughout operation */
    int baseid;			/* id of first entry in cache */
    int cids;			/* # entries in cache */
    int fids;			/* # entries in table */
    int fd;			/* file descriptor, -1 if not open */
    /* and flags bit EFF_RDONLY */
    /* User callbacks */
    void (*init)(int, void *);	/* called after entry creation, unless null */
    int (*postread)(int, void *); /* called after read, unless null */
    int (*prewrite)(int, void *); /* called before write, unless null */
};

/*
 * Flag bits for struct empfile member flags
 * Immutable flags are properties of the table and thus cannot change.
 * The remaining flags record how the table is being used.
 */
/* Immutable flags, fixed at compile-time */
/*
 * EFF_XY / EFF_OWNER / EFF_GROUP assert that coordinates / owner /
 * group of such a table's entries can be safely obtained by
 * dereferencing entry address cast to struct genitem *.
 */
#define EFF_XY		bit(0)
#define EFF_OWNER	bit(1)
#define EFF_GROUP	bit(2)
/* Table is allocated statically */
#define EFF_STATIC	bit(3)
/* All the immutable flags */
#define EFF_IMMUTABLE	(EFF_XY | EFF_OWNER | EFF_GROUP | EFF_STATIC)
/* Flags set when table contents is mapped */
/* Table is entirely in memory */
#define EFF_MEM		bit(8)
/* Table is read-only */
#define EFF_RDONLY	bit(9)
/* Table is customized (configuration tables only) */
#define EFF_CUSTOM	bit(10)
/* Transient flags, only occur in argument of ef_open() */
/* Create table file, clobbering any existing file */
#define EFF_CREATE	bit(11)

/*
 * Empire `file types'
 * These are really table IDs.  Some tables are backed by files, some
 * are compiled into the server.
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
    /* Static game data (configuration) */
    EF_SECTOR_CHR,
    EF_SHIP_CHR,
    EF_PLANE_CHR,
    EF_LAND_CHR,
    EF_NUKE_CHR,
    EF_NEWS_CHR,
    EF_ITEM,
    EF_INFRASTRUCTURE,
    EF_PRODUCT,
    EF_TABLE,
    EF_META,
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

#define EF_IS_GAME_STATE(type) (EF_SECTOR <= (type) && (type) <= EF_REALM)

extern struct castr *ef_cadef(int);
extern int ef_read(int, int, void *);
extern void *ef_ptr(int, int);
extern char *ef_nameof(int);
extern time_t ef_mtime(int);
extern int ef_open(int, int);
extern int ef_check(int);
extern int ef_close(int);
extern int ef_flush(int);
extern int ef_write(int, int, void *);
extern int ef_extend(int, int);
extern int ef_ensure_space(int, int, int);
extern int ef_nelem(int);
extern int ef_flags(int);
extern int ef_byname(char *);
extern int ef_byname_from(char *, int *);
extern void ef_init(void);
extern int ef_verify(void);

extern struct empfile empfile[EF_MAX + 1];

#endif
