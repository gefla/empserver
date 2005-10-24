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
 *  file.h: Describes Empire files and their contents
 * 
 *  Known contributors to this file:
 *    
 */

#ifndef _FILE_H_
#define _FILE_H_

#include <stddef.h>

struct empfile {
    int ef_uid;			/* Table ID */
    char *name;			/* Empire name (e.g., "treaty") */
    char *file;			/* file name (relative to data directory) */
    int flags;			/* misc stuff */
    int size;			/* size of object */
    void (*init) (int, char *);	/* call this when object is created */
    int (*postread) (int, char *);	/* specific massage routines for items */
    int (*prewrite) (int, char *);
    int fd;			/* file descriptor, -1 if not open */
    int baseid;			/* starting item in cache */
    int cids;			/* # ids in cache */
    int csize;			/* size of cache in bytes */
    char *cache;		/* pointer to cache */
    int fids;			/* # of ids in file */
    struct castr *cadef;	/* ca defs selection list */
};

/*
 * struct empfile flags
 *
 * EFF_XY / EFF_OWNER / EFF_GROUP assert that coordinates / owner /
 * group of such a file's record can be safely obtained by
 * dereferencing its memory address cast to struct genitem *.
 */
#define EFF_XY		bit(0)
#define EFF_OWNER	bit(1)
#define EFF_GROUP	bit(2)
/* Table is entirely in memory */
#define EFF_MEM		bit(3)
/* Table is read-only */
#define EFF_RDONLY	bit(4)
/* Create table file, clobbering any existing file */
#define EFF_CREATE	bit(5)
/* Table is allocated statically */

/* Flags that may be passed to ef_open() */
#define EFF_OPEN	(EFF_MEM | EFF_RDONLY | EFF_CREATE)

/*
 * Empire `file types'
 * These are really table IDs.  Some tables are backed by files, some
 * are compiled into the server.
 * Historically, only table IDs 0..EF_MAX-1 existed.  All the
 * functions operating on table IDs still reject the new indexes >=
 * EF_MAX.  This needs to be rectified, carefully checking existing
 * code, which could rely on unspoken assumptions about these tables.
 */
/* Error value */
#define EF_BAD		-1
/* Dynamic game data tables: 0..EF_MAX-1 */
#define EF_SECTOR	0
#define EF_SHIP		1
#define EF_PLANE	2
#define	EF_LAND		3
#define EF_NUKE		4
#define EF_NEWS		5
#define EF_TREATY	6
#define EF_TRADE	7
#define EF_POWER	8
#define EF_NATION	9
#define EF_LOAN		10
#define	EF_MAP		11
#define EF_BMAP		12
#define EF_COMM         13
#define EF_LOST         14
#define EF_MAX		15
/* Static game data (configuation): EF_MAX.. */
#define EF_SECTOR_CHR	15
#define EF_SHIP_CHR	16
#define EF_PLANE_CHR	17
#define EF_LAND_CHR	18
#define EF_NUKE_CHR	19
#if 0 /* doesn't exist yet */
#define EF_NEWS_CHR
#endif
#define EF_TREATY_CHR	20
#define EF_ITEM		21
#define EF_INFRASTRUCTURE   22
#define EF_PRODUCT	23
#define EF_TABLE	24
#define EF_SHIP_CHR_FLAGS	25
#define EF_PLANE_CHR_FLAGS	26
#define EF_LAND_CHR_FLAGS	27
#define EF_NUKE_CHR_FLAGS	28

struct fileinit {
    int ef_type;
    void (*init) (int, char *);
    int (*postread) (int, char *);
    int (*prewrite) (int, char *);
};

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

extern struct empfile empfile[];

#endif /* _FILE_H_ */
