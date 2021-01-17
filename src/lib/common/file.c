/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  file.c: Operations on Empire tables (`files' for historical reasons)
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 2000
 *     Markus Armbruster, 2005-2014
 */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef _WIN32
#include <io.h>
#include <share.h>
#endif
#include "file.h"
#include "match.h"
#include "misc.h"
#include "prototypes.h"

static int open_locked(char *, int, mode_t);
static int ef_realloc_cache(struct empfile *, int);
static int fillcache(struct empfile *, int);
static int do_read(struct empfile *, void *, int, int);
static int do_write(struct empfile *, void *, int, int);
static unsigned get_seqno(struct empfile *, int);
static void new_seqno(struct empfile *, void *);
static void must_be_fresh(struct empfile *, void *);
static int do_extend(struct empfile *, int);
static void do_blank(struct empfile *, void *, int, int);
static int ef_check(int);

static unsigned ef_generation;

/*
 * Open the file-backed table @type (EF_SECTOR, ...).
 * @how are flags to control operation.  Naturally, immutable flags are
 * not permitted.
 * The table must not be already open.
 * Return non-zero on success, zero on failure.
 */
int
ef_open(int type, int how)
{
    struct empfile *ep;
    int oflags, fd, fsiz, fids, nslots, fail;

    if (ef_check(type) < 0)
	return 0;
    if (CANT_HAPPEN(how & EFF_IMMUTABLE))
	how &= ~EFF_IMMUTABLE;

    /* open file */
    ep = &empfile[type];
    if (CANT_HAPPEN(!ep->file || ep->base != EF_BAD || ep->fd >= 0))
	return 0;
    if (CANT_HAPPEN(ep->prewrite && !(how & EFF_MEM)))
	return 0;		/* not implemented */
    oflags = O_RDWR;
    if (how & EFF_PRIVATE)
	oflags = O_RDONLY;
    if (how & EFF_CREATE)
	oflags |= O_CREAT | O_TRUNC;
    fd = open_locked(ep->file, oflags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (fd < 0) {
	logerror("Can't open %s (%s)", ep->file, strerror(errno));
	return 0;
    }

    /* get file size */
    if (how & EFF_CREATE) {
	fids = ep->nent >= 0 ? ep->nent : 0;
    } else {
	fsiz = fsize(fd);
	if (fsiz % ep->size) {
	    logerror("Can't open %s (file size not a multiple of record size %d)",
		     ep->file, ep->size);
	    close(fd);
	    return 0;
	}
	fids = fsiz / ep->size;
	if (ep->nent >= 0 && ep->nent != fids) {
	    logerror("Can't open %s (got %d records instead of %d)",
		     ep->file, fids, ep->nent);
	    close(fd);
	    return 0;
	}
    }

    /* allocate cache */
    if (ep->flags & EFF_STATIC) {
	/* ep->cache already points to space for ep->csize elements */
	if (how & EFF_MEM) {
	    if (fids > ep->csize) {
		CANT_HAPPEN(ep->nent >= 0); /* insufficient static cache */
		logerror("Can't open %s (file larger than %d records)",
			 ep->file, ep->csize);
		close(fd);
		return 0;
	    }
	}
    } else {
	if (CANT_HAPPEN(ep->cache))
	    free(ep->cache);
	if (how & EFF_MEM)
	    nslots = fids;
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

    if (how & EFF_CREATE) {
	/* populate new file */
	ep->fids = 0;
	fail = !do_extend(ep, fids);
    } else {
	ep->fids = fids;
	if ((how & EFF_MEM) && fids)
	    fail = fillcache(ep, 0) != fids;
	else
	    fail = 0;
    }
    if (fail) {
	ep->cids = 0;	/* prevent cache flush */
	ef_close(type);
	return 0;
    }

    if (ep->onresize)
	ep->onresize(type);
    return 1;
}

static int
open_locked(char *name, int oflags, mode_t mode)
{
    int rdlonly = (oflags & O_ACCMODE) == O_RDONLY;
    int fd;

#ifdef _WIN32
    fd = _sopen(name, oflags | O_BINARY, rdlonly ? SH_DENYNO : SH_DENYWR,
		mode);
    if (fd < 0)
	return -1;
#else  /* !_WIN32 */
    struct flock lock;

    fd = open(name, oflags, mode);
    if (fd < 0)
	return -1;

    lock.l_type = rdlonly ? F_RDLCK : F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = lock.l_len = 0;
    if (fcntl(fd, F_SETLK, &lock) == -1) {
	close(fd);
	return -1;
    }
#endif	/* !_WIN32 */
    return fd;
}

/*
 * Reallocate cache for table @ep to hold @count slots.
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
 * Open the table @type, which is a view of a base table
 * The table must not be already open.
 * Return non-zero on success, zero on failure.
 * Beware: views work only as long as the base table doesn't change size!
 * You must close the view before closing its base table.
 */
int
ef_open_view(int type)
{
    struct empfile *ep;
    int base;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    base = ep->base;
    if (ef_check(base) < 0)
	return 0;
    if (CANT_HAPPEN(!(ef_flags(base) & EFF_MEM)
		    || ep->file
		    || ep->size != empfile[base].size
		    || ep->nent != empfile[base].nent
		    || ep->cache || ep->oninit || ep->postread
		    || ep->prewrite || ep->onresize))
	return -1;

    ep->cache = empfile[base].cache;
    ep->csize = empfile[base].csize;
    ep->flags |= EFF_MEM;
    ep->baseid = empfile[base].baseid;
    ep->cids = empfile[base].cids;
    ep->fids = empfile[base].fids;
    return 0;
}

/*
 * Close the open table @type (EF_SECTOR, ...).
 * Return non-zero on success, zero on failure.
 */
int
ef_close(int type)
{
    struct empfile *ep;
    int retval = 1;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];

    if (EF_IS_VIEW(type)) {
	ep->cache = NULL;
	ep->csize = 0;
    } else {
	if (!ef_flush(type))
	    retval = 0;
	if (!(ep->flags & EFF_STATIC)) {
	    free(ep->cache);
	    ep->cache = NULL;
	    ep->csize = 0;
	}
	if (close(ep->fd) < 0) {
	    logerror("Error closing %s (%s)", ep->file, strerror(errno));
	    retval = 0;
	}
	ep->fd = -1;
    }
    ep->flags &= EFF_IMMUTABLE;
    ep->baseid = ep->cids = ep->fids = 0;
    if (ep->onresize)
	ep->onresize(type);
    return retval;
}

/*
 * Flush file-backed table @type (EF_SECTOR, ...) to its backing file.
 * Do nothing if the table is privately mapped.
 * Update timestamps of written elements if table is EFF_TYPED.
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
	if (do_write(ep, ep->cache, ep->baseid, ep->cids) < 0)
	    return 0;
    }

    return 1;
}

/*
 * Return pointer to element @id in table @type if it exists, else NULL.
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
 * Read element @id from table @type into buffer @into.
 * FIXME pass buffer size!
 * @into is marked fresh with ef_mark_fresh().
 * Return non-zero on success, zero on failure.
 */
int
ef_read(int type, int id, void *into)
{
    struct empfile *ep;
    void *cachep;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (CANT_HAPPEN(!ep->cache))
	return 0;
    if (id < 0 || id >= ep->fids)
	return 0;

    if (ep->flags & EFF_MEM) {
	cachep = ep->cache + id * ep->size;
    } else {
	if (ep->baseid + ep->cids <= id || ep->baseid > id) {
	    if (fillcache(ep, id) < 1)
		return 0;
	}
	cachep = ep->cache + (id - ep->baseid) * ep->size;
    }
    memcpy(into, cachep, ep->size);
    ef_mark_fresh(type, into);

    if (ep->postread)
	ep->postread(id, into);
    return 1;
}

/*
 * Fill cache of file-backed @ep with elements starting at @id.
 * If any were read, return their number.
 * Else return -1 and leave the cache unchanged.
 */
static int
fillcache(struct empfile *ep, int id)
{
    int ret;

    if (CANT_HAPPEN(!ep->cache))
	return -1;

    ret = do_read(ep, ep->cache, id, MIN(ep->csize, ep->fids - id));
    if (ret >= 0) {
	/* cache changed */
	ep->baseid = id;
	ep->cids = ret;
    }
    return ret;
}

static int
do_read(struct empfile *ep, void *buf, int id, int count)
{
    int n, ret;
    char *p;

    if (CANT_HAPPEN(ep->fd < 0 || id < 0 || count < 0))
	return -1;

    if (lseek(ep->fd, id * ep->size, SEEK_SET) == (off_t)-1) {
	logerror("Error seeking %s to elt %d (%s)",
		 ep->file, id, strerror(errno));
	return -1;
    }

    p = buf;
    n = count * ep->size;
    while (n > 0) {
	ret = read(ep->fd, p, n);
	if (ret < 0) {
	    if (errno != EINTR) {
		logerror("Error reading %s elt %d (%s)",
			 ep->file,
			 id + (int)((p - (char *)buf) / ep->size),
			 strerror(errno));
		break;
	    }
	} else if (ret == 0) {
	    logerror("Unexpected EOF reading %s elt %d",
		     ep->file, id + (int)((p - (char *)buf) / ep->size));
	    break;
	} else {
	    p += ret;
	    n -= ret;
	}
    }

    return (p - (char *)buf) / ep->size;
}

/*
 * Write @count elements starting at @id from @buf to file-backed @ep.
 * Update the timestamp if the table is EFF_TYPED.
 * Don't actually write if table is privately mapped.
 * Return 0 on success, -1 on error (file may be corrupt then).
 */
static int
do_write(struct empfile *ep, void *buf, int id, int count)
{
    int i, n, ret;
    char *p;
    struct ef_typedstr *elt;
    time_t now;

    if (CANT_HAPPEN(ep->fd < 0 || id < 0 || count < 0))
	return -1;

    if (ep->flags & EFF_TYPED) {
	now = ep->flags & EFF_NOTIME ? (time_t)-1 : time(NULL);
	for (i = 0; i < count; i++) {
	    /*
	     * TODO Oopses here could be due to bad data corruption.
	     * Fail instead of attempting to recover?
	     */
	    elt = (struct ef_typedstr *)((char *)buf + i * ep->size);
	    if (CANT_HAPPEN(elt->ef_type != ep->uid))
		elt->ef_type = ep->uid;
	    if (CANT_HAPPEN(elt->uid != id + i))
		elt->uid = id + i;
	    if (now != (time_t)-1)
		elt->timestamp = now;
	}
    }

    if (ep->flags & EFF_PRIVATE)
	return 0;

    if (lseek(ep->fd, id * ep->size, SEEK_SET) == (off_t)-1) {
	logerror("Error seeking %s to elt %d (%s)",
		 ep->file, id, strerror(errno));
	return -1;
    }

    p = buf;
    n = count * ep->size;
    while (n > 0) {
	ret = write(ep->fd, p, n);
	if (ret < 0) {
	    if (errno != EINTR) {
		logerror("Error writing %s elt %d (%s)",
			 ep->file,
			 id + (int)((p - (char *)buf) / ep->size),
			 strerror(errno));
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
 * Write element @id into table @type from buffer @from.
 * FIXME pass buffer size!
 * Update timestamp in @from if table is EFF_TYPED.
 * If table is file-backed and not privately mapped, write through
 * cache straight to disk.
 * Cannot write beyond the end of fully cached table (flags & EFF_MEM).
 * Can write at the end of partially cached table.
 * @from must be fresh; see ef_make_stale().
 * Return non-zero on success, zero on failure.
 */
int
ef_write(int type, int id, void *from)
{
    struct empfile *ep;
    char *cachep;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (CANT_HAPPEN((ep->flags & (EFF_MEM | EFF_PRIVATE)) == EFF_PRIVATE))
	return 0;
    if (CANT_HAPPEN(id < 0))
	return 0;
    if (CANT_HAPPEN(ep->nent >= 0 && id >= ep->nent))
	return 0;		/* beyond fixed size */
    new_seqno(ep, from);
    if (id >= ep->fids) {
	/* beyond end of file */
	if (CANT_HAPPEN((ep->flags & EFF_MEM) || id > ep->fids))
	    return 0;		/* not implemented */
	/* write at end of file extends it */
	ep->fids = id + 1;
	if (ep->onresize)
	    ep->onresize(type);
    }
    if (id >= ep->baseid && id < ep->baseid + ep->cids) {
	cachep = ep->cache + (id - ep->baseid) * ep->size;
	if (cachep != from)
	    must_be_fresh(ep, from);
    } else
	cachep = NULL;
    if (ep->prewrite)
	ep->prewrite(id, cachep, from);
    if (ep->fd >= 0) {
	if (do_write(ep, from, id, 1) < 0)
	    return 0;
    }
    if (cachep && cachep != from)	/* update the cache if necessary */
	memcpy(cachep, from, ep->size);
    return 1;
}

/*
 * Change element ID.
 * @buf is an element of table @type.
 * @id is its new element ID.
 * If table is EFF_TYPED, change ID and sequence number stored in @buf.
 * Else do nothing.
 */
void
ef_set_uid(int type, void *buf, int uid)
{
    struct ef_typedstr *elt;
    struct empfile *ep;

    if (ef_check(type) < 0)
	return;
    ep = &empfile[type];
    if (!(ep->flags & EFF_TYPED))
	return;
    elt = buf;
    if (elt->uid == uid)
	return;
    elt->uid = uid;
    elt->seqno = get_seqno(ep, uid);
}

/*
 * Are *@a and *@b equal, except for timestamps and such?
 */
int
ef_typedstr_eq(struct ef_typedstr *a, struct ef_typedstr *b)
{
    return a->ef_type == b->ef_type
	&& a->seqno == b->seqno
	&& a->uid == b->uid
	&& !memcmp((char *)a + sizeof(*a), (char *)b + sizeof(*a),
		   empfile[a->ef_type].size - sizeof(*a));
}

/*
 * Return sequence number of element @id in table @ep.
 * Return zero if table is not EFF_TYPED (it has no sequence number
 * then).
 */
static unsigned
get_seqno(struct empfile *ep, int id)
{
    struct ef_typedstr *elt;

    if (!(ep->flags & EFF_TYPED))
	return 0;
    if (id < 0 || id >= ep->fids)
	return 0;
    if (id >= ep->baseid && id < ep->baseid + ep->cids)
	elt = (void *)(ep->cache + (id - ep->baseid) * ep->size);
    else {
	/* need a buffer, steal last cache slot */
	if (ep->cids == ep->csize)
	    ep->cids--;
	elt = (void *)(ep->cache + ep->cids * ep->size);
	if (do_read(ep, elt, id, 1) < 0)
	    return 0;		/* deep trouble */
    }
    return elt->seqno;
}

/*
 * Increment sequence number in @buf, which is about to be written to @ep.
 * Do nothing if table is not EFF_TYPED (it has no sequence number
 * then).
 * Else, @buf's sequence number must match the one in @ep's cache.  If
 * it doesn't, we're about to clobber a previous write.
 */
static void
new_seqno(struct empfile *ep, void *buf)
{
    struct ef_typedstr *elt = buf;
    unsigned old_seqno;

    if (!(ep->flags & EFF_TYPED))
	return;
    old_seqno = get_seqno(ep, elt->uid);
    CANT_HAPPEN(old_seqno != elt->seqno);
    elt->seqno = old_seqno + 1;
}

/*
 * Make all copies stale.
 * Only fresh copies may be written back to the cache.
 * To be called by functions that may yield the processor.
 * Writing an copy when there has been a yield since it was read is
 * unsafe, because we could clobber another thread's write then.
 * Robust code must assume the that any function that may yield does
 * yield.  Marking copies stale there lets us catch unsafe writes.
 */
void
ef_make_stale(void)
{
    ef_generation++;
}

/* Mark copy of an element of table @type in @buf fresh. */
void
ef_mark_fresh(int type, void *buf)
{
    struct empfile *ep;

    if (ef_check(type) < 0)
	return;
    ep = &empfile[type];
    if (!(ep->flags & EFF_TYPED))
	return;
    ((struct ef_typedstr *)buf)->generation = ef_generation;
}

static void
must_be_fresh(struct empfile *ep, void *buf)
{
    struct ef_typedstr *elt = buf;

    if (!(ep->flags & EFF_TYPED))
	return;
    CANT_HAPPEN(elt->generation != (ef_generation & 0xfff));
}

/*
 * Extend table @type by @count elements.
 * Any pointers obtained from ef_ptr() become invalid.
 * Return non-zero on success, zero on failure.
 */
int
ef_extend(int type, int count)
{
    struct empfile *ep;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (ep->nent >= 0) {
	logerror("Can't extend %s, its size is fixed", ep->name);
	return 0;
    }
    if (!do_extend(ep, count))
	return 0;
    if (ep->onresize)
	ep->onresize(type);
    return 1;
}

static int
do_extend(struct empfile *ep, int count)
{
    char *p;
    int need_sentinel, i, id;

    if (CANT_HAPPEN(EF_IS_VIEW(ep->uid)) || count < 0)
	return 0;

    id = ep->fids;
    if (ep->flags & EFF_MEM) {
	need_sentinel = (ep->flags & EFF_SENTINEL) != 0;
	if (id + count + need_sentinel > ep->csize) {
	    if (ep->flags & EFF_STATIC) {
		logerror("Can't extend %s beyond %d elements",
			 ep->name, ep->csize - need_sentinel);
		return 0;
	    }
	    if (!ef_realloc_cache(ep, id + count + need_sentinel)) {
		logerror("Can't extend %s to %d elements (%s)",
			 ep->name, id + count, strerror(errno));
		return 0;
	    }
	}
	p = ep->cache + id * ep->size;
	do_blank(ep, p, id, count);
	if (ep->fd >= 0) {
	    if (do_write(ep, p, id, count) < 0)
		return 0;
	}
	if (need_sentinel)
	    memset(ep->cache + (id + count) * ep->size, 0, ep->size);
	ep->cids = id + count;
    } else {
	/* need a buffer, steal last cache slot */
	if (ep->cids == ep->csize)
	    ep->cids--;
	p = ep->cache + ep->cids * ep->size;
	for (i = 0; i < count; i++) {
	    do_blank(ep, p, id + i, 1);
	    if (do_write(ep, p, id + i, 1) < 0)
		return 0;
	}
    }
    ep->fids = id + count;
    return 1;
}

/*
 * Initialize element @id for table @type in @buf.
 * FIXME pass buffer size!
 * @buf is marked fresh with ef_mark_fresh().
 */
void
ef_blank(int type, int id, void *buf)
{
    struct empfile *ep;
    struct ef_typedstr *elt;

    if (ef_check(type) < 0)
	return;
    ep = &empfile[type];
    do_blank(ep, buf, id, 1);
    if (ep->flags & EFF_TYPED) {
	elt = buf;
	elt->seqno = get_seqno(ep, elt->uid);
    }
    ef_mark_fresh(type, buf);
}

/*
 * Initialize @count elements of @ep in @buf, starting with element @id.
 */
static void
do_blank(struct empfile *ep, void *buf, int id, int count)
{
    int i;
    struct ef_typedstr *elt;

    memset(buf, 0, count * ep->size);
    for (i = 0; i < count; i++) {
	elt = (struct ef_typedstr *)((char *)buf + i * ep->size);
	if (ep->flags & EFF_TYPED) {
	    elt->ef_type = ep->uid;
	    elt->uid = id + i;
	}
	if (ep->oninit)
	    ep->oninit(elt);
    }
}

/*
 * Truncate table @type to @count elements.
 * Any pointers obtained from ef_ptr() become invalid.
 * Return non-zero on success, zero on failure.
 */
int
ef_truncate(int type, int count)
{
    struct empfile *ep;
    int need_sentinel;

    if (ef_check(type) < 0 || CANT_HAPPEN(EF_IS_VIEW(type)))
	return 0;
    ep = &empfile[type];
    if (ep->nent >= 0) {
	logerror("Can't truncate %s, its size is fixed", ep->name);
	return 0;
    }
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
	need_sentinel = (ep->flags & EFF_SENTINEL) != 0;
	if (!(ep->flags & EFF_STATIC)) {
	    if (!ef_realloc_cache(ep, count + need_sentinel)) {
		logerror("Can't shrink %s cache after truncate (%s)",
			 ep->name, strerror(errno));
		/* continue with unshrunk cache */
	    }
	}
	if (need_sentinel)
	    memset(ep->cache + count * ep->size, 0, ep->size);
	ep->cids = count;
    } else {
	if (ep->baseid >= count)
	    ep->cids = 0;
	else if (ep->cids > count - ep->baseid)
	    ep->cids = count - ep->baseid;
    }

    if (ep->onresize)
	ep->onresize(type);
    return 1;
}

struct castr *
ef_cadef(int type)
{
    if (ef_check(type) < 0)
	return NULL;
    return empfile[type].cadef;
}

int
ef_nelem(int type)
{
    if (ef_check(type) < 0)
	return 0;
    return empfile[type].fids;
}

int
ef_flags(int type)
{
    if (ef_check(type) < 0)
	return 0;
    return empfile[type].flags;
}

time_t
ef_mtime(int type)
{
    if (ef_check(type) < 0)
	return 0;
    if (empfile[type].fd <= 0)
	return 0;
    return fdate(empfile[type].fd);
}

/*
 * Search for a table matching @name, return its table type.
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
 * Search @choices[] for a table type matching @name, return it.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 * @choices[] must be terminated with a negative value.
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

/*
 * Return name of table @type.  Always a single, short word.
 */
char *
ef_nameof(int type)
{
    if (ef_check(type) < 0)
	return "bad";
    return empfile[type].name;
}

/*
 * Return "pretty" name of table @type.
 */
char *
ef_nameof_pretty(int type)
{
    if (ef_check(type) < 0)
	return "bad";
    return empfile[type].pretty_name;
}

static int
ef_check(int type)
{
    if (CANT_HAPPEN((unsigned)type >= EF_MAX))
	return -1;
    return 0;
}

/*
 * Ensure table @type contains element @id.
 * If necessary, extend it in steps of @count elements.
 * Return non-zero on success, zero on failure.
 */
int
ef_ensure_space(int type, int id, int count)
{
    if (ef_check(type) < 0 || CANT_HAPPEN(id < 0))
	return 0;

    while (id >= empfile[type].fids) {
	if (!ef_extend(type, count))
	    return 0;
    }
    return 1;
}

/*
 * Return maximum ID acceptable for table @type.
 * Assuming infinite memory and disk space.
 */
int
ef_id_limit(int type)
{
    struct empfile *ep;

    if (ef_check(type) < 0)
	return -1;
    ep = &empfile[type];
    if (ep->nent >= 0)
	return ep->nent - 1;
    if (ep->flags & EFF_MEM) {
	if (ep->flags & EFF_STATIC)
	    return ep->csize - 1 - ((ep->flags & EFF_SENTINEL) != 0);
    }
    return INT_MAX;
}
