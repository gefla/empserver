/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *  Copyright (C) 1991-3 Stephen Crane
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
 *  rwlock.c: Read-write locks
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "lwp.h"
#include "lwpint.h"

struct lwp_rwlock {
    /*
     * Lock counter
     * 0: unlocked
     * -1: locked for writing
     * >0: locked for reading that many times
     */
    int count;
    struct lwpQueue rq;		/* read lock sleepers */
    struct lwpQueue wq;		/* write lock sleepers */
    char *name;
};

struct lwp_rwlock *
lwp_rwlock_create(char *name)
{
    struct lwp_rwlock *rwlock;

    rwlock = malloc(sizeof(*rwlock));
    if (!rwlock)
	return NULL;

    memset(rwlock, 0, sizeof(*rwlock));
    rwlock->name = strdup(name);
    return rwlock;
}

void
lwp_rwlock_destroy(struct lwp_rwlock *rwlock)
{
    if (CANT_HAPPEN(rwlock->count))
	return;
    free(rwlock);
}

void
lwp_rwlock_wrlock(struct lwp_rwlock *rwlock)
{
    if (rwlock->count) {
	lwpAddTail(&rwlock->wq, LwpCurrent);
	lwpStatus(LwpCurrent, "blocked to acquire rwlock %s for writing",
		  rwlock->name);
	lwpReschedule();
    }
    CANT_HAPPEN(rwlock->count != 0);
    rwlock->count = -1;
    lwpStatus(LwpCurrent, "acquired rwlock %s for writing", rwlock->name);
}

void
lwp_rwlock_rdlock(struct lwp_rwlock *rwlock)
{
    if (rwlock->count < 0 || rwlock->wq.head) {
	lwpStatus(LwpCurrent, "blocked to acquire rwlock %s for reading",
		  rwlock->name);
	lwpAddTail(&rwlock->rq, LwpCurrent);
	lwpReschedule();
    }
    CANT_HAPPEN(rwlock->count < 0);
    rwlock->count++;
    lwpStatus(LwpCurrent, "acquired rwlock %s for reading", rwlock->name);
}

void
lwp_rwlock_unlock(struct lwp_rwlock *rwlock)
{
    struct lwpProc *p;

    lwpStatus(LwpCurrent, "unlocking rwlock %s", rwlock->name);
    if (CANT_HAPPEN(rwlock->count == 0))
	return;
    if (rwlock->count < 0)
	rwlock->count = 0;
    else
	rwlock->count--;

    if (rwlock->count == 0 && rwlock->wq.head) {
	p = lwpGetFirst(&rwlock->wq);
	lwpStatus(p, "wake up next writer of rwlock %s", rwlock->name);
    } else if (rwlock->count >= 0 && rwlock->rq.head && !rwlock->wq.head) {
	p = lwpGetFirst(&rwlock->rq);
	lwpStatus(p, "wake up next reader of rwlock %s", rwlock->name);
    } else
	return;

    lwpReady(p);
    if (LwpCurrent->pri < p->pri) {
	lwpStatus(LwpCurrent, "yielding to thread with higher priority");
	lwpYield();
    }
}
