/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#ifdef Rel4
#include <string.h>
#endif /* Rel4 */
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "misc.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "match.h"
#include "struct.h"
#include "common.h"
#include "gen.h"

#ifdef Rel4
#include <unistd.h>
#endif

static void fillcache(struct empfile *ep, int start);

int
ef_open(int type, int mode, int how)
{
    register struct empfile *ep;
    static int block;
    int size;

#if defined(_WIN32)
    mode |= _O_BINARY;
#endif
    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if ((ep->fd = open(ep->file, mode, 0660)) < 0) {
	logerror("%s: open failed", ep->file);
	return 0;
    }
    if (block == 0)
	block = blksize(ep->fd);
    ep->baseid = 0;
    ep->cids = 0;
    ep->mode = mode;
    ep->flags |= how;
    ep->fids = fsize(ep->fd) / ep->size;
    if (ep->flags & EFF_MEM)
	ep->csize = ep->fids;
    else
	ep->csize = block / ep->size;
    size = ep->csize * ep->size;
    ep->cache = (s_char *)malloc(size);
    if ((ep->cache == 0) && (size != 0)) {
	logerror("ef_open: %s malloc(%d) failed\n", ep->file, size);
	return 0;
    }
    if (ep->flags & EFF_MEM) {
	if (read(ep->fd, ep->cache, size) != size) {
	    logerror("ef_open: read(%s) failed\n", ep->file);
	    return 0;
	}
	ep->cids = size / ep->size;
    }
    return 1;
}

int
ef_close(int type)
{
    register struct empfile *ep;
    int r;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (ep->cache == 0) {
	/* no cache implies never opened */
	return 0;
    }
    ef_flush(type);
    ep->flags &= ~EFF_MEM;
    free(ep->cache);
    ep->cache = 0;
    if ((r = close(ep->fd)) < 0) {
	logerror("ef_close: %s close(%d) -> %d", ep->name, ep->fd, r);
    }
    return 1;
}

int
ef_flush(int type)
{
    register struct empfile *ep;
    int size;
    int r;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (ep->cache == 0) {
	/* no cache implies never opened */
	return 0;
    }
    size = ep->csize * ep->size;
    if (ep->mode > 0 && (ep->flags & EFF_MEM)) {
	if ((r = lseek(ep->fd, 0L, 0)) < 0) {
	    logerror("ef_flush: %s cache lseek(%d, 0L, 0) -> %d",
		     ep->name, ep->fd, r);
	    return 0;
	}
	if (write(ep->fd, ep->cache, size) != size) {
	    logerror("ef_flush: %s cache write(%d, %x, %d) -> %d",
		     ep->name, ep->fd, ep->cache, ep->size, r);
	    return 0;
	}
    }
    /*ef_zapcache(type); */
    return 1;
}

s_char *
ef_ptr(int type, int id)
{
    register struct empfile *ep;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (id < 0 || id >= ep->fids)
	return 0;
    if ((ep->flags & EFF_MEM) == 0) {
	logerror("ef_ptr: (%s) only valid for EFF_MEM entries", ep->file);
	return 0;
    }
    return (s_char *)(ep->cache + ep->size * id);
}

/*
 * buffered read.  Tries to read a large number of items.
 * This system won't work if item size is > sizeof buffer area.
 */
int
ef_read(int type, int id, caddr_t ptr)
{
    register struct empfile *ep;
    caddr_t from;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (id < 0)
	return 0;
    if (ep->flags & EFF_MEM) {
	if (id >= ep->fids)
	    return 0;
	from = ep->cache + (id * ep->size);
    } else {
	if (id >= ep->fids) {
	    ep->fids = fsize(ep->fd) / ep->size;
	    if (id >= ep->fids)
		return 0;
	}
	if (ep->baseid + ep->cids <= id || ep->baseid > id)
	    fillcache(ep, id);
	from = ep->cache + (id - ep->baseid) * ep->size;
    }
    memcpy(ptr, from, ep->size);

    if (ep->postread)
	ep->postread(id, ptr);
    return 1;
}

static void
fillcache(struct empfile *ep, int start)
{
    int n;

    ep->baseid = start;
    lseek(ep->fd, start * ep->size, 0);
    n = read(ep->fd, ep->cache, ep->csize * ep->size);
    ep->cids = n / ep->size;
}

#ifdef notdef
/*
 * no-buffered read
 * zaps read cache
 */
int
ef_nbread(int type, int id, caddr_t ptr)
{
    register struct empfile *ep;
    int r;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (id < 0)
	return 0;
    if (id >= ep->fids) {
	ep->fids = fsize(ep->fd) / ep->size;
	if (id >= ep->fids)
	    return 0;
    }
    if ((r = lseek(ep->fd, id * ep->size, 0)) < 0) {
	logerror("ef_nbread: %s #%d lseek(%d, %d, 0) -> %d",
		 ep->name, id, ep->fd, id * ep->size, r);
	return 0;
    }
    if ((r = read(ep->fd, ptr, ep->size)) != ep->size) {
	logerror("ef_nbread: %s #%d read(%d, %x, %d) -> %d",
		 ep->name, id, ep->fd, ptr, ep->size, r);
	return 0;
    }
    ef_zapcache(type);
    if (ep->postread)
	ep->postread(id, ptr);
    return 1;
}
#endif

/*
 * buffered write.  Modifies read cache (if applicable)
 * and writes through to disk.
 */
int
ef_write(int type, int id, caddr_t ptr)
{
    register int r;
    register struct empfile *ep;
    s_char *to;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (id > 65536) {
	/* largest unit id; this may bite us in large games */
	logerror("ef_write: type %d id %d is too large!\n", type, id);
	return 0;
    }
    if ((r = lseek(ep->fd, id * ep->size, 0)) < 0) {
	logerror("ef_write: %s #%d lseek(%d, %d, 0) -> %d",
		 ep->name, id, ep->fd, id * ep->size, r);
	return 0;
    }
    if (ep->prewrite)
	ep->prewrite(id, ptr);
    if ((r = write(ep->fd, ptr, ep->size)) != ep->size) {
	logerror("ef_write: %s #%d write(%d, %x, %d) -> %d",
		 ep->name, id, ep->fd, ptr, ep->size, r);
	return 0;
    }
    if (id >= ep->baseid && id < ep->baseid + ep->cids) {
	/* update the cache if necessary */
	to = ep->cache + (id - ep->baseid) * ep->size;
	memcpy(to, ptr, ep->size);
    }
    if (id > ep->fids) {
	logerror("WARNING ef_write: expanded %s by more than one id",
		 ep->name);
	log_last_commands();
    }
    if (id >= ep->fids) {
	if (ep->flags & EFF_MEM) {
	    logerror
		("file %s went beyond %d items; won't be able toread item w/o restart",
		 ep->name, ep->fids);
	} else {
	    /* write expanded file; ep->fids = last id + 1 */
	    ep->fids = id + 1;
	}
    }
    return 1;
}

#ifdef notdef
/*
 * no-buffered write
 * zaps read cache
 */
int
ef_nbwrite(int type, int id, caddr_t ptr)
{
    register struct empfile *ep;
    register int r;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    if (id > 65536) {
	/* largest unit id; this may bite us in large games */
	logerror("ef_nbwrite: %s id %d is too large!\n", ep->name, id);
	return 0;
    }
    if ((r = lseek(ep->fd, id * ep->size, 0)) < 0) {
	logerror("ef_nbwrite: %s #%d lseek(%d, %d, 0) -> %d",
		 ep->name, id, ep->fd, id * ep->size, r);
	return 0;
    }
    if (ep->prewrite)
	ep->prewrite(id, ptr);
    if ((r = write(ep->fd, ptr, ep->size)) != ep->size) {
	logerror("ef_nbwrite: %s #%d write(%d, %x, %d) -> %d",
		 ep->name, id, ep->fd, ptr, ep->size, r);
	return 0;
    }
    ef_zapcache(type);
    if (id >= ep->fids) {
	/* write expanded file; ep->fids = last id + 1 */
	ep->fids = id + 1;
    }
    return 1;
}
#endif

int
ef_extend(int type, int count)
{
    register struct empfile *ep;
    char *ptr;
    int cur, max;
    int mode, how;
    int r;

    if (ef_check(type) < 0)
	return 0;
    ep = &empfile[type];
    max = ep->fids + count;
    cur = ep->fids;
    ptr = (s_char *)calloc(1, ep->size);
    if ((r = lseek(ep->fd, ep->fids * ep->size, 0)) < 0) {
	logerror("ef_extend: %s +#%d lseek(%d, %d, 0) -> %d",
		 ep->name, count, ep->fd, ep->fids * ep->size, r);
	return 0;
    }
    for (cur = ep->fids; cur < max; cur++) {
	if (ep->init)
	    ep->init(cur, ptr);
	if ((r = write(ep->fd, ptr, ep->size)) != ep->size) {
	    logerror("ef_extend: %s +#%d write(%d, %x, %d) -> %d",
		     ep->name, count, ep->fd, ptr, ep->size, r);
	    return 0;
	}
    }
    free(ptr);
    if (ep->flags & EFF_MEM) {
	/* XXX this will cause problems if there are ef_ptrs (to the
	 * old allocated structure) active when we do the re-open */
	mode = ep->mode;
	how = ep->flags;
	ef_close(type);
	ef_open(type, mode, how);
    } else {
	ep->fids += count;
    }
    return 1;
}

void
ef_zapcache(int type)
{
    register struct empfile *ep = &empfile[type];
    if ((ep->flags & EFF_MEM) == 0) {
	ep->cids = 0;
	ep->baseid = -1;
    }
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
    extern time_t fdate(int fd);

    if (empfile[type].fd <= 0)
	return 0;
    return fdate(empfile[type].fd);
}

int
ef_vars(int type, register s_char *sp, u_char **nvp, u_char **vp,
	u_short **ap)
{
    register struct empfile *ef;

    if (ef_check(type) < 0)
	return 0;
    ef = &empfile[type];
    if ((ef->flags & EFF_COM) == 0)
	return -1;
    *nvp = (u_char *)(sp + ef->varoffs[0]);
    *vp = (u_char *)(sp + ef->varoffs[1]);
    *ap = (u_short *)(sp + ef->varoffs[2]);
    return ef->maxvars;
}

int
ef_byname(s_char *name)
{
    register struct empfile *ef;
    register int i;
    int len;

    len = strlen(name);
    for (i = 0; i < EF_MAX; i++) {
	ef = &empfile[i];
	if (strncmp(ef->name, name, min(len, strlen(ef->name))) == 0)
	    return i;
    }
    return -1;
}

s_char *
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
