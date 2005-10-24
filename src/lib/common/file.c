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
 *  file.c: Misc. operations on files
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 2000
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "misc.h"
#include "nsc.h"
#include "file.h"
#include "common.h"
#include "gen.h"


static int fillcache(struct empfile *, int);
static int do_write(struct empfile *, void *, int, int);


/*
 * Open the binary file for table TYPE (EF_SECTOR, ...).
 * HOW are EFF_OPEN flags to control operation.
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
    if (CANT_HAPPEN(how & ~EFF_OPEN))
	how &= EFF_OPEN;
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
    fsiz = fsize(fd);
    if (fsiz % ep->size) {
	logerror("Can't open %s (file size not a multiple of record size %d)",
		 ep->file, ep->size);
	close(fd);
	return 0;
    }
    ep->fids = fsiz / ep->size;
    if (how & EFF_MEM)
	ep->csize = ep->fids;
    else
	ep->csize = max(1, blksize(fd) / ep->size);
    size = ep->csize * ep->size;
    if (size) {
	ep->cache = malloc(size);
	if (ep->cache == NULL) {
	    logerror("Can't open %s: out of memory", ep->file);
	    close(fd);
	    return 0;
	}
    } else {
	ep->cache = NULL;
    }
    ep->baseid = 0;
    ep->cids = 0;
    ep->flags = (ep->flags & ~EFF_OPEN) | (how & ~EFF_CREATE);
    ep->fd = fd;
    if ((how & EFF_MEM) && ep->fids) {
	if (fillcache(ep, 0) != ep->fids) {
	    ep->cids = 0;	/* prevent cache flush */
	    ep->flags &= ~EFF_OPEN; /* maintain invariant */
	    ef_close(type);
	    return 0;
	}
    }
    return 1;
}

/*
 * Close the file containing objects of the type 'type', flushing the cache
 * if applicable.
 */
int
ef_close(int type)
{
    struct empfile *ep;
    int retval;

    retval = ef_flush(type);
    ep = &empfile[type];
    ep->flags &= ~EFF_OPEN;
    free(ep->cache);
    ep->cache = NULL;
    if (close(ep->fd) < 0) {
	logerror("Error closing %s (%s)", ep->name, strerror(errno));
	retval = 0;
    }
    ep->fd = -1;
    return retval;
}

/*
 * Flush the cache of the file containing objects of type 'type' to disk.
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
 * Return a pointer the id 'id' of object of type 'type' in the cache.
 */
void *
ef_ptr(int type, int id)
{
    struct empfile *ep;

    if (ef_check(type) < 0)
	return NULL;
    ep = &empfile[type];
    if (CANT_HAPPEN(!(ep->flags & EFF_MEM)))
	return NULL;
    if (id < 0 || id >= ep->fids)
	return NULL;		/* FIXME can this happen? */
    return ep->cache + ep->size * id;
}

/*
 * buffered read.  Tries to read a large number of items.
 * This system won't work if item size is > sizeof buffer area.
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
    if (id < 0)
	return 0;		/* FIXME can this happen? */
    if (id >= ep->fids)
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
 * buffered write.  Modifies read cache (if applicable)
 * and writes through to disk.
 */
int
ef_write(int type, int id, void *from)
{
    struct empfile *ep;
    char *to;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (CANT_HAPPEN(ep->fd < 0))
	return 0;
    if (ep->prewrite)
	ep->prewrite(id, from);
    if (CANT_HAPPEN((ep->flags & EFF_MEM) ? id >= ep->fids : id > ep->fids))
	return 0;		/* not implemented */
    if (do_write(ep, from, id, 1) < 0)
	return 0;
    if (id >= ep->baseid && id < ep->baseid + ep->cids) {
	/* update the cache if necessary */
	to = ep->cache + (id - ep->baseid) * ep->size;
	memcpy(to, from, ep->size);
    }
    if (id >= ep->fids) {
	/* write beyond end of file extends it, take note */
	ep->fids = id + 1;
    }
    return 1;
}

/*
 * Grow the file containing objects of the type 'type' by 'count' objects.
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
	how = ep->flags & EFF_OPEN;
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
 * Search empfile[0..EF_MAX-1] for element named NAME.
 * Return its index in empfile[] if found, else -1.
 */
int
ef_byname(char *name)
{
    struct empfile *ef;
    int i;
    int len;

    len = strlen(name);
    for (i = 0; i < EF_MAX; i++) {
	ef = &empfile[i];
	if (strncmp(ef->name, name, min(len, strlen(ef->name))) == 0)
	    return i;
    }
    return -1;
}

char *
ef_nameof(int type)
{
    if (type < 0 || type >= EF_MAX)
	return "bad item type";
    return empfile[type].name;
}

int
ef_check(int type)
{
    if (type < 0 || type >= EF_MAX) {
	logerror("ef_ptr: bad EF_type %d\n", type);
	return -1;
    }
    return 0;
}

int
ef_ensure_space(int type, int id, int count)
{
    while (id >= empfile[type].fids) {
	if (!ef_extend(type, count))
	    return 0;
    }
    return 1;
}
