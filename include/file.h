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
    char *name;			/* Empire name (e.g., "treaty") */
    char *file;			/* file name (relative to data directory) */
    int flags;			/* misc stuff */
    int mode;			/* O_flags */
    int size;			/* size of object */
    void (*init) (int, char *);	/* call this when object is created */
    int (*postread) (int, char *);	/* specific massage routines for items */
    int (*prewrite) (int, char *);
    ptrdiff_t itemoffs;		/* offset of item[] in struct */
    int fd;			/* file descriptor */
    int baseid;			/* starting item in cache */
    int cids;			/* # ids in cache */
    int csize;			/* size of cache in bytes */
    char *cache;		/* pointer to cache */
    int fids;			/* # of ids in file */
    struct castr *cadef;	/* ca defs selection list */
};

#define EFF_XY		bit(0)	/* has location */
#define EFF_MEM		bit(1)	/* stored entirely in-memory */
#define EFF_OWNER	bit(2)	/* has concept of owner */
#define EFF_GROUP	bit(3)	/* has concept of group */

#define EF_BAD		-1	/* illegal file type */
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

#define EF_NMAP         222	/* Kinda bogus, but used to describe a newdesmap
				   instead of bmap or map. */

struct fileinit {
    void (*init) (int, char *);
    int (*postread) (int, char *);
    int (*prewrite) (int, char *);
    struct castr *cadef;
};

extern struct castr *ef_cadef(int);
extern int ef_read(int, int, void *);
extern char *ef_ptr(int, int);
extern char *ef_nameof(int);
extern time_t ef_mtime(int);
extern int ef_open(int, int, int);
extern int ef_check(int);
extern int ef_close(int);
extern int ef_flush(int);
extern int ef_write(int, int, void *);
extern int ef_extend(int, int);
extern int ef_ensure_space(int, int, int);
extern void ef_zapcache(int);
extern int ef_nelem(int);
extern int ef_flags(int);
extern int ef_byname(char *);

extern int ef_nbread(int type, int id, void *ptr);
extern struct empfile empfile[];

#endif /* _FILE_H_ */
