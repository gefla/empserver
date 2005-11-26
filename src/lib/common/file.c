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
 *  file.c: Operations on Empire tables (`files' for historical reasons)
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 2000
 *     Markus Armbruster, 2005
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "common.h"
#include "file.h"
#include "gen.h"
#include "match.h"
#include "misc.h"
#include "nsc.h"
#include "optlist.h"

static int fillcache(struct empfile *, int);
static int do_write(struct empfile *, void *, int, int);

/*
 * Open the file-backed table TYPE (EF_SECTOR, ...).
 * HOW are flags to control operation.  Naturally, immutable flags are
 * not permitted.
 * Return non-zero on success, zero on failure.
 * You must call ef_close() before the next ef_open().
 */
int
ef_open(int type, int how)
{
    struct empfile *ep;
    int oflags, fd, fsiz, size;

    if (ef_check(type) < 0)
	return 0;
    if (CANT_HAPPEN(how & EFF_IMMUTABLE))
	how &= ~EFF_IMMUTABLE;

    /* open file */
    ep = &empfile[type];
    if (CANT_HAPPEN(ep->fd >= 0))
	return 0;
    oflags = O_RDWR;
    if (how & EFF_RDONLY)
	oflags = O_RDONLY;
    if (how & EFF_CREATE)
	oflags |= O_CREAT | O_TRUNC;
#if defined(_WIN32)
    oflags |= O_BINARY;
#endif
    if ((fd = open(ep->file, oflags, 0660)) < 0) {
	logerror("Can't open %s (%s)", ep->file, strerror(errno));
	return 0;
    }

    /* get file size */
    fsiz = fsize(fd);
    if (fsiz % ep->size) {
	logerror("Can't open %s (file size not a multiple of record size %d)",
		 ep->file, ep->size);
	close(fd);
	return 0;
    }
    ep->fids = fsiz / ep->size;

    /* allocate cache */
    if (ep->flags & EFF_STATIC) {
	/* ep->cache already points to space for e->csize elements */
	if (how & EFF_MEM) {
	    if (ep->fids > ep->csize) {
		logerror("Can't open %s: file larger than %d bytes",
			 ep->file, ep->fids * ep->size);
		close(fd);
		return 0;
	    }
	}
    } else {
	if (how & EFF_MEM)
	    ep->csize = ep->fids;
	else
	    ep->csize = max(1, blksize(fd) / ep->size);
	size = ep->csize * ep->size;
	if (CANT_HAPPEN(ep->cache))
	    free(ep->cache);
	ep->cache = malloc(size);
	if (ep->cache == NULL && size) {
	    logerror("Can't open %s: out of memory", ep->file);
	    close(fd);
	    return 0;
	}
    }
    ep->baseid = 0;
    ep->cids = 0;
    ep->flags = (ep->flags & EFF_IMMUTABLE) | (how & ~EFF_CREATE);
    ep->fd = fd;

    /* map file into cache */
    if ((how & EFF_MEM) && ep->fids) {
	if (fillcache(ep, 0) != ep->fids) {
	    ep->cids = 0;	/* prevent cache flush */
	    ep->flags &= EFF_IMMUTABLE; /* maintain invariant */
	    ef_close(type);
	    return 0;
	}
    }

    /*
     * Could close fd if both EFF_RDONLY and EFF_MEM, but that doesn't
     * happen, so don't bother.
     */

    return 1;
}

/*
 * Close the file-backed table TYPE (EF_SECTOR, ...).
 * Return non-zero on success, zero on failure.
 */
int
ef_close(int type)
{
    struct empfile *ep;
    int retval;

    retval = ef_flush(type);
    ep = &empfile[type];
    ep->flags &= EFF_IMMUTABLE;
    if (!(ep->flags & EFF_STATIC)) {
	free(ep->cache);
	ep->cache = NULL;
    }
    if (close(ep->fd) < 0) {
	logerror("Error closing %s (%s)", ep->name, strerror(errno));
	retval = 0;
    }
    ep->fd = -1;
    return retval;
}

/*
 * Flush file-backed table TYPE (EF_SECTOR, ...) to disk.
 * Return non-zero on success, zero on failure.
 */
int
ef_flush(int type)
{
    struct empfile *ep;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (CANT_HAPPEN(ep->fd < 0))
	return 0;
    /*
     * We don't know which cache entries are dirty.  ef_write() writes
     * through, but direct updates through ef_ptr() don't.  They are
     * allowed only with EFF_MEM.  Assume the whole cash is dirty
     * then.
     */
    if (!(ep->flags & EFF_RDONLY) && (ep->flags & EFF_MEM))
	return do_write(ep, ep->cache, ep->baseid, ep->cids) >= 0;

    return 1;
}

/*
 * Return pointer to element ID in table TYPE if it exists, else NULL.
 * The table must be fully cached, i.e. flags & EFF_MEM.
 * The caller is responsible for flushing changes he makes.
 */
void *
ef_ptr(int type, int id)
{
    struct empfile *ep;

    if (ef_check(type) < 0)
	return NULL;
    ep = &empfile[type];
    if (CANT_HAPPEN(!(ep->flags & EFF_MEM) || !ep->cache))
	return NULL;
    if (id < 0 || id >= ep->fids)
	return NULL;
    return ep->cache + ep->size * id;
}

/*
 * Read element ID from table TYPE into buffer INTO.
 * FIXME pass buffer size!
 * Return non-zero on success, zero on failure.
 */
int
ef_read(int type, int id, void *into)
{
    struct empfile *ep;
    void *from;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (CANT_HAPPEN(!ep->cache))
	return 0;
    if (id < 0 || id >= ep->fids)
	return 0;

    if (ep->flags & EFF_MEM) {
	from = ep->cache + id * ep->size;
    } else {
	if (ep->baseid + ep->cids <= id || ep->baseid > id) {
	    if (fillcache(ep, id) < 1)
		return 0;
	}
	from = ep->cache + (id - ep->baseid) * ep->size;
    }
    memcpy(into, from, ep->size);

    if (ep->postread)
	ep->postread(id, into);
    return 1;
}

/*
 * Fill cache of EP with elements starting at ID.
 * If any were read, return their number.
 * Else return -1 and leave the cache unchanged.
 */
static int
fillcache(struct empfile *ep, int start)
{
    int n, ret;
    char *p;

    if (CANT_HAPPEN(ep->fd < 0 || !ep->cache))
	return -1;

    if (lseek(ep->fd, start * ep->size, SEEK_SET) == (off_t)-1) {
	logerror("Error seeking %s (%s)", ep->file, strerror(errno));
	return -1;
    }

    p = ep->cache;
    n = ep->csize * ep->size;
    while (n > 0) {
	ret = read(ep->fd, p, n);
	if (ret < 0) {
	    if (errno != EAGAIN) {
		logerror("Error reading %s (%s)", ep->file, strerror(errno));
		break;
	    }
	} else if (ret == 0) {
	    break;
	} else {
	    p += ret;
	    n -= ret;
	}
    }

    if (p == ep->cache)
	return -1;		/* nothing read, old cache still ok */

    ep->baseid = start;
    ep->cids = (p - ep->cache) / ep->size;
    return ep->cids;
}

/*
 * Write COUNT elements from BUF to EP, starting at ID.
 * Return 0 on success, -1 on error.
 */
static int
do_write(struct empfile *ep, void *buf, int id, int count)
{
    int n, ret;
    char *p;

    if (CANT_HAPPEN(ep->fd < 0 || id < 0 || count < 0))
	return -1;

    if (lseek(ep->fd, id * ep->size, SEEK_SET) == (off_t)-1) {
	logerror("Error seeking %s (%s)", ep->file, strerror(errno));
	return -1;
    }

    p = buf;
    n = count * ep->size;
    while (n > 0) {
	ret = write(ep->fd, p, n);
	if (ret < 0) {
	    if (errno != EAGAIN) {
		logerror("Error writing %s (%s)", ep->file, strerror(errno));
		/* FIXME if this extended file, truncate back to old size */
		return -1;
	    }
	} else {
	    p += ret;
	    n -= ret;
	}
    }

    return 0;
}

/*
 * Write element ID into file-backed table TYPE from buffer FROM.
 * FIXME pass buffer size!
 * Write through cache straight to disk.
 * Cannot write beyond the end of fully cached table (flags & EFF_MEM).
 * Can write at the end of partially cached table.
 * Return non-zero on success, zero on failure.
 */
int
ef_write(int type, int id, void *from)
{
    struct empfile *ep;
    char *to;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (ep->prewrite)
	ep->prewrite(id, from);
    if (CANT_HAPPEN((ep->flags & EFF_MEM) ? id >= ep->fids : id > ep->fids))
	return 0;		/* not implemented */
    if (do_write(ep, from, id, 1) < 0)
	return 0;
    if (id >= ep->baseid && id < ep->baseid + ep->cids) {
	/* update the cache if necessary */
	to = ep->cache + (id - ep->baseid) * ep->size;
	if (to != from)
	    memcpy(to, from, ep->size);
    }
    if (id >= ep->fids) {
	/* write beyond end of file extends it, take note */
	ep->fids = id + 1;
    }
    return 1;
}

/*
 * Extend the file-backed table TYPE by COUNT elements.
 * Return non-zero on success, zero on failure.
 */
int
ef_extend(int type, int count)
{
    struct empfile *ep;
    char *tmpobj;
    int id, i, how;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (CANT_HAPPEN(ep->fd < 0 || count < 0))
	return 0;

    tmpobj = calloc(1, ep->size);
    id = ep->fids;
    for (i = 0; i < count; i++) {
	if (ep->init)
	    ep->init(id + i, tmpobj);
	if (do_write(ep, tmpobj, id + i, 1) < 0)
	    break;
    }
    free(tmpobj);

    if (ep->flags & EFF_MEM) {
	/* FIXME lazy bastards...  do this right */
	/* XXX this will cause problems if there are ef_ptrs (to the
	 * old allocated structure) active when we do the re-open */
	how = ep->flags & ~EFF_IMMUTABLE;
	ef_close(type);
	ef_open(type, how);
    } else {
	ep->fids += i;
    }

    return i == count;
}

struct castr *
ef_cadef(int type)
{
    return empfile[type].cadef;
}

int
ef_nelem(int type)
{
    return empfile[type].fids;
}

int
ef_flags(int type)
{
    return empfile[type].flags;
}

time_t
ef_mtime(int type)
{
    if (empfile[type].fd <= 0)
	return 0;
    return fdate(empfile[type].fd);
}

/*
 * Search for a table matching NAME, return its table type.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 */
int
ef_byname(char *name)
{
    return stmtch(name, empfile, offsetof(struct empfile, name),
		  sizeof(empfile[0]));
}

/*
 * Search CHOICES[] for a table type matching NAME, return it.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 * CHOICES[] must be terminated with a negative value.
 */
int
ef_byname_from(char *name, int choices[])
{
    int res;
    int *p;

    res = M_NOTFOUND;
    for (p = choices; *p >= 0; p++) {
	if (ef_check(*p) < 0)
	    continue;
	switch (mineq(name, empfile[*p].name)) {
	case ME_MISMATCH:
	    break;
	case ME_PARTIAL:
	    if (res >= 0)
		return M_NOTUNIQUE;
	    res = *p;
	    break;
	case ME_EXACT:
	    return *p;
	}
    }
    return res;
}

char *
ef_nameof(int type)
{
    if (ef_check(type) < 0)
	return "bad ef_type";
    return empfile[type].name;
}

int
ef_check(int type)
{
    if (CANT_HAPPEN((unsigned)type >= EF_MAX))
	return -1;
    return 0;
}

/*
 * Ensure file-backed table contains ID.
 * If necessary, extend it in steps of COUNT elements.
 * Return non-zero on success, zero on failure.
 */
int
ef_ensure_space(int type, int id, int count)
{
    if (ef_check(type) < 0)
	return 0;
    CANT_HAPPEN(id < 0);

    while (id >= empfile[type].fids) {
	if (!ef_extend(type, count))
	    return 0;
    }
    return 1;
}

static void
ef_fix_size(struct empfile *ep, int n)
{
    ep->cids = ep->fids = n;
    ep->csize = n + 1;
}

static void
ef_init_chr(int type, size_t size, ptrdiff_t name_offs)
{
    struct empfile *ep = &empfile[type];
    char *p;

    for (p = ep->cache; *((char **)(p + name_offs)); p += size) ;
    ep->cids = ep->fids = (p - ep->cache) / size;
}

/*
 * Initialize Empire tables.
 * Must be called once, before using anything else from this module.
 */
void
ef_init(void)
{
    struct castr *ca;
    struct empfile *ep;
    struct symbol *lup;
    int i;

    empfile[EF_MAP].size = empfile[EF_BMAP].size = (WORLD_X * WORLD_Y) / 2;

    ef_init_chr(EF_SHIP_CHR,
		sizeof(struct mchrstr), offsetof(struct mchrstr, m_name));
    ef_init_chr(EF_PLANE_CHR,
		sizeof(struct plchrstr), offsetof(struct plchrstr, pl_name));
    ef_init_chr(EF_LAND_CHR,
		sizeof(struct lchrstr), offsetof(struct lchrstr, l_name));
    ef_init_chr(EF_NUKE_CHR,
		sizeof(struct nchrstr), offsetof(struct nchrstr, n_name));

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
