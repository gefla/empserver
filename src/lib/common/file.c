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
 *  file.c: Operations on Empire tables (`files' for historical reasons)
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 2000
 *     Markus Armbruster, 2005-2008
 */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "file.h"
#include "match.h"
#include "misc.h"
#include "nsc.h"
#include "prototypes.h"

static int ef_realloc_cache(struct empfile *, int);
static int fillcache(struct empfile *, int);
static int do_write(struct empfile *, void *, int, int, time_t);
static void do_blank(struct empfile *, void *, int, int);

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
    struct flock lock;
    int oflags, fd, fsiz, nslots;

    if (ef_check(type) < 0)
	return 0;
    if (CANT_HAPPEN(how & EFF_IMMUTABLE))
	how &= ~EFF_IMMUTABLE;

    /* open file */
    ep = &empfile[type];
    if (CANT_HAPPEN(ep->fd >= 0))
	return 0;
    oflags = O_RDWR;
    if (how & EFF_PRIVATE)
	oflags = O_RDONLY;
    if (how & EFF_CREATE)
	oflags |= O_CREAT | O_TRUNC;
#if defined(_WIN32)
    oflags |= O_BINARY;
#endif
    if ((fd = open(ep->file, oflags, S_IRWUG)) < 0) {
	logerror("Can't open %s (%s)", ep->file, strerror(errno));
	return 0;
    }

    lock.l_type = how & EFF_PRIVATE ? F_RDLCK : F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = lock.l_len = 0;
    if (fcntl(fd, F_SETLK, &lock) == -1) {
	logerror("Can't lock %s (%s)", ep->file, strerror(errno));
	close(fd);
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
	/* ep->cache already points to space for ep->csize elements */
	if (how & EFF_MEM) {
	    if (ep->fids > ep->csize) {
		logerror("Can't open %s: file larger than %d bytes",
			 ep->file, ep->fids * ep->size);
		close(fd);
		return 0;
	    }
	}
    } else {
	if (CANT_HAPPEN(ep->cache))
	    free(ep->cache);
	if (how & EFF_MEM)
	    nslots = ep->fids;
	else
	    nslots = blksize(fd) / ep->size;
	if (!ef_realloc_cache(ep, nslots)) {
	    logerror("Can't map %s (%s)", ep->file, strerror(errno));
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

    return 1;
}

/*
 * Reallocate cache for table EP to hold COUNT slots.
 * The table must not be allocated statically.
 * The cache may still be unmapped.
 * If reallocation succeeds, any pointers obtained from ef_ptr()
 * become invalid.
 * If it fails, the cache is unchanged, and errno is set.
 * Return non-zero on success, zero on failure.
 */
static int
ef_realloc_cache(struct empfile *ep, int count)
{
    void *cache;

    if (CANT_HAPPEN(ep->flags & EFF_STATIC))
	return 0;
    if (CANT_HAPPEN(count < 0))
	count = 0;

    /*
     * Avoid zero slots, because that can lead to null cache, which
     * would be interpreted as unmapped cache.
     */
    if (count == 0)
	count++;
    cache = realloc(ep->cache, count * ep->size);
    if (!cache)
	return 0;

    ep->cache = cache;
    ep->csize = count;
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
 * Flush table TYPE (EF_SECTOR, ...) to disk.
 * Does nothing if the table is privately mapped.
 * Return non-zero on success, zero on failure.
 */
int
ef_flush(int type)
{
    struct empfile *ep;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (ep->flags & EFF_PRIVATE)
	return 1;		/* nothing to do */
    if (CANT_HAPPEN(ep->fd < 0))
	return 0;
    /*
     * We don't know which cache entries are dirty.  ef_write() writes
     * through, but direct updates through ef_ptr() don't.  They are
     * allowed only with EFF_MEM.  Assume the whole cash is dirty
     * then.
     */
    if (ep->flags & EFF_MEM) {
	if (do_write(ep, ep->cache, ep->baseid, ep->cids, time(NULL)) < 0)
	    return 0;
    }

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
 * Set the timestamp to NOW if the table has those.
 * Return 0 on success, -1 on error.
 */
static int
do_write(struct empfile *ep, void *buf, int id, int count, time_t now)
{
    int i, n, ret;
    char *p;
    struct emptypedstr *elt;

    if (CANT_HAPPEN(ep->fd < 0 || (ep->flags & EFF_PRIVATE)
		    || id < 0 || count < 0))
	return -1;

    if (ep->flags & EFF_TYPED) {
	for (i = 0; i < count; i++) {
	    /*
	     * TODO Oopses here could be due to bad data corruption.
	     * Fail instead of attempting to recover?
	     */
	    elt = (struct emptypedstr *)((char *)buf + i * ep->size);
	    if (CANT_HAPPEN(elt->ef_type != ep->uid))
		elt->ef_type = ep->uid;
	    if (CANT_HAPPEN(elt->uid != id + i))
		elt->uid = id + i;
	    elt->timestamp = now;
	}
    }

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
 * Write element ID into table TYPE from buffer FROM.
 * FIXME pass buffer size!
 * If table is file-backed and not privately mapped, write through
 * cache straight to disk.
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
    if (CANT_HAPPEN((ep->flags & (EFF_MEM | EFF_PRIVATE)) == EFF_PRIVATE))
	return 0;
    if (ep->prewrite)
	ep->prewrite(id, from);
    if (CANT_HAPPEN((ep->flags & EFF_MEM) ? id >= ep->fids : id > ep->fids))
	return 0;		/* not implemented */
    if (!(ep->flags & EFF_PRIVATE)) {
	if (do_write(ep, from, id, 1, time(NULL)) < 0)
	    return 0;
    }
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
 * Extend table TYPE by COUNT elements.
 * Any pointers obtained from ef_ptr() become invalid.
 * Return non-zero on success, zero on failure.
 */
int
ef_extend(int type, int count)
{
    struct empfile *ep;
    char *p;
    int i, id;
    time_t now = time(NULL);

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (CANT_HAPPEN(count < 0))
	return 0;

    id = ep->fids;
    if (ep->flags & EFF_MEM) {
	if (id + count > ep->csize) {
	    if (ep->flags & EFF_STATIC) {
		logerror("Can't extend %s beyond %d elements",
			 ep->file, ep->csize);
		return 0;
	    }
	    if (!ef_realloc_cache(ep, id + count)) {
		logerror("Can't extend %s to %d elements (%s)",
			 ep->file, id + count, strerror(errno));
		return 0;
	    }
	}
	p = ep->cache + id * ep->size;
	do_blank(ep, p, id, count);
	if (ep->fd >= 0 && !(ep->flags & EFF_PRIVATE)) {
	    if (do_write(ep, p, id, count, now) < 0)
		return 0;
	}
	ep->cids += count;
    } else {
	/* need a buffer, steal last cache slot */
	if (ep->cids == ep->csize)
	    ep->cids--;
	p = ep->cache + ep->cids * ep->size;
	for (i = 0; i < count; i++) {
	    do_blank(ep, p, id + i, 1);
	    if (do_write(ep, p, id + i, 1, now) < 0)
		return 0;
	}
    }
    ep->fids += count;
    return 1;
}

/*
 * Initialize element ID for EP in BUF.
 * FIXME pass buffer size!
 */
void
ef_blank(int type, int id, void *buf)
{
    if (ef_check(type) < 0)
	return;
    do_blank(&empfile[type], buf, id, 1);
}

/*
 * Initialize COUNT elements of EP in BUF, starting with element ID.
 */
static void
do_blank(struct empfile *ep, void *buf, int id, int count)
{
    int i;
    struct emptypedstr *elt;

    memset(buf, 0, count * ep->size);
    if (ep->flags & EFF_TYPED) {
	for (i = 0; i < count; i++) {
	    elt = (struct emptypedstr *)((char *)buf + i * ep->size);
	    elt->ef_type = ep->uid;
	    elt->uid = id + i;
	}
    }
}

/*
 * Truncate table TYPE to COUNT elements.
 * Any pointers obtained from ef_ptr() become invalid.
 * Return non-zero on success, zero on failure.
 */
int
ef_truncate(int type, int count)
{
    struct empfile *ep;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (CANT_HAPPEN(count < 0 || count > ep->fids))
	return 0;

    if (ep->fd >= 0 && !(ep->flags & EFF_PRIVATE)) {
	if (ftruncate(ep->fd, count * ep->size) < 0) {
	    logerror("Can't truncate %s to %d elements (%s)",
		     ep->file, count, strerror(errno));
	    return 0;
	}
    }
    ep->fids = count;

    if (ep->flags & EFF_MEM) {
	if (!(ep->flags & EFF_STATIC)) {
	    if (!ef_realloc_cache(ep, count)) {
		logerror("Can't shrink cache after truncate");
		/* continue with unshrunk cache */
	    }
	}
	ep->cids = count;
    } else {
	if (ep->baseid >= count)
	    ep->cids = 0;
	else if (ep->cids > count - ep->baseid)
	    ep->cids = count - ep->baseid;
    }

    return 1;
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
