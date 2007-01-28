/*
 * lwp.h -- prototypes and structures for lightweight processes
 * Copyright (C) 1991-3 Stephen Crane.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
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
struct lwpSem;
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
void lwpSleepFd(int fd, int flags);
void lwpSleepUntil(time_t until);
void lwpWakeupFd(struct lwpProc * p);
int lwpSigWait(sigset_t *set, int *sig);
void *lwpGetUD(struct lwpProc * p);
void lwpSetUD(struct lwpProc * p, char *ud);
int lwpSetPriority(int prio);

struct lwpSem *lwpCreateSem(char *name, int count);
void lwpSignal(struct lwpSem *);
void lwpWait(struct lwpSem *);

struct lwp_rwlock *lwp_rwlock_create(char *);
void lwp_rwlock_destroy(struct lwp_rwlock *);
void lwp_rwlock_wrlock(struct lwp_rwlock *);
void lwp_rwlock_rdlock(struct lwp_rwlock *);
void lwp_rwlock_unlock(struct lwp_rwlock *);

extern struct lwpProc *LwpCurrent;

#endif
