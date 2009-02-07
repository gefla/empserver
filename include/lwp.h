/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  lwp.h -- prototypes and structures for lightweight processes
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2008
 *     Ron Koenderink, 2007-2008
 */

/*
 * author: Stephen Crane, (jsc@doc.ic.ac.uk), Department of Computing,
 * Imperial College of Science, Technology and Medicine, 180 Queen's
 * Gate, London SW7 2BZ, England.
 */

#ifndef LWP_H
#define LWP_H

#include <signal.h>
#include <sys/types.h>

#define LWP_STACKCHECK	0x1
#define LWP_PRINT	0x2

struct lwpProc;
struct lwp_rwlock;

#define LWP_FD_READ	0x1
#define LWP_FD_WRITE	0x2

#define LWP_MAX_PRIO	8

struct lwpProc *lwpInitSystem(int prio, void **ctxp, int flags, sigset_t *);
struct lwpProc *lwpCreate(int prio, void (*)(void *), int size,
			  int flags, char *name,
			  int argc, char **argv, void *ud);
void lwpExit(void);
void lwpTerminate(struct lwpProc * p);
void lwpYield(void);
int lwpSleepFd(int fd, int flags, struct timeval *timeout);
int lwpSleepUntil(time_t until);
void lwpWakeup(struct lwpProc *);
int lwpSigWait(sigset_t *set, int *sig);
void *lwpGetUD(struct lwpProc * p);
void lwpSetUD(struct lwpProc * p, char *ud);
int lwpSetPriority(int prio);
char *lwpName(struct lwpProc * p);
void lwpSetName(struct lwpProc * p, char *name);

struct lwp_rwlock *lwp_rwlock_create(char *);
void lwp_rwlock_destroy(struct lwp_rwlock *);
void lwp_rwlock_wrlock(struct lwp_rwlock *);
void lwp_rwlock_rdlock(struct lwp_rwlock *);
void lwp_rwlock_unlock(struct lwp_rwlock *);

extern struct lwpProc *LwpCurrent;

#endif
