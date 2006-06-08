/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  lwp.c: lightweight process creation, destruction and manipulation
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2006
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwp.h"
#include "lwpint.h"
#include "prototypes.h"

struct lwpQueue LwpSchedQ[LWP_MAX_PRIO], LwpDeadQ;

struct lwpProc *LwpCurrent = NULL;
void **LwpContextPtr;
int LwpMaxpri = 0;		/* maximum priority so far */
int LwpStackGrowsDown;

static void lwpDestroy(struct lwpProc *proc);
static void lwpStackCheckInit(struct lwpProc *newp);
static void lwpStackCheck(struct lwpProc *newp);
static void lwpStackCheckUsed(struct lwpProc *newp);

/* check stack direction */
static int
growsdown(void *x)
{
    int y;
    y = (x > (void *)&y);
    return y;
}

/*
 * lwpReschedule -- schedule another process.  we also check for dead
 * processes here and free them.
 */
void
lwpReschedule(void)
{
    static struct lwpProc *nextp;
    static int i;

    if (LwpCurrent && (LwpCurrent->flags & LWP_STACKCHECK)) {
	lwpStackCheck(LwpCurrent);
    }

    lwpSigWakeup();

    /* destroy dead threads */
    lwpStatus(LwpCurrent, "Cleaning dead queue");
    while (NULL != (nextp = lwpGetFirst(&LwpDeadQ))) {
	if (CANT_HAPPEN(nextp == LwpCurrent))
	    abort();
	lwpDestroy(nextp);
	lwpStatus(LwpCurrent, "Destroying done");
    }

    for (i = LwpMaxpri + 1; i--;) {
	while (NULL != (nextp = lwpGetFirst(&LwpSchedQ[i]))) {
	    if (!nextp->dead)
		break;
	    /* clean up after dead bodies */
	    lwpStatus(nextp, "got a dead body");
	    if (nextp == LwpCurrent) {
		lwpStatus(nextp, "we are in it -- will bury later");
		lwpAddTail(&LwpDeadQ, nextp);
	    } else {
		lwpDestroy(nextp);
	    }
	    nextp = 0;
	}
	if (nextp)
	    break;
    }
    if (CANT_HAPPEN(LwpCurrent == 0 && nextp == 0))
	abort();
    if (LwpCurrent != nextp) {
	struct lwpProc *oldp = LwpCurrent;
	if (oldp)
	    lwpStatus(oldp, "switch out");
	LwpCurrent = nextp;
	*LwpContextPtr = nextp->ud;
	lwpSwitchContext(oldp, nextp);
	lwpStatus(nextp, "switch in %d", nextp->pri);
    }
}

/*
 * lwpEntryPoint -- process entry point.
 */
void
lwpEntryPoint(void)
{
    lwpStatus(LwpCurrent, "starting at entry point");
    (*LwpCurrent->entry)(LwpCurrent->ud);
    lwpExit();
}

/*
 * lwpCreate -- create a process.
 */
struct lwpProc *
lwpCreate(int priority, void (*entry)(void *), int stacksz, int flags, char *name, char *desc, int argc, char **argv, void *ud)
{
    struct lwpProc *newp;

    if (!(newp = malloc(sizeof(struct lwpProc))))
	return NULL;
    newp->flags = flags;
    newp->name = strdup(name);
    newp->desc = strdup(desc);
    newp->entry = entry;
    newp->argc = argc;
    newp->argv = argv;
    newp->ud = ud;
    newp->dead = 0;
    if (LWP_MAX_PRIO <= priority)
	priority = LWP_MAX_PRIO - 1;
    if (LwpMaxpri < (newp->pri = priority))
	LwpMaxpri = priority;
    if (lwpNewContext(newp, stacksz) < 0) {
	free(newp->name);
	free(newp->desc);
	free(newp);
	return NULL;
    }
    lwpStatus(newp, "creating process structure %p (sbtm %p)",
	      newp, newp->sbtm);
    if (flags & LWP_STACKCHECK)
	lwpStackCheckInit(newp);
    lwpReady(newp);
    lwpReady(LwpCurrent);
    lwpReschedule();
    return newp;
}

void
lwpDestroy(struct lwpProc *proc)
{
    if (proc->flags & LWP_STACKCHECK) {
	lwpStackCheckUsed(proc);
	lwpStackCheck(proc);
    }
    lwpStatus(proc, "destroying sbtm: %p", proc->sbtm);
    free(proc->sbtm);
    free(proc->name);
    free(proc->desc);
    free(proc);
}

/*
 * lwpReady -- put process on ready queue.  if null, assume current.
 */
void
lwpReady(struct lwpProc *p)
{
    if (!p)
	p = LwpCurrent;
    lwpStatus(p, "added to run queue");
    lwpAddTail(&LwpSchedQ[p->pri], p);
}

/*
 * return user's data
 */
void *
lwpGetUD(struct lwpProc *p)
{
    if (!p)
	p = LwpCurrent;
    return p->ud;
}

/*
 * set user's data
 */
void
lwpSetUD(struct lwpProc *p, char *ud)
{
    if (!p)
	p = LwpCurrent;
    p->ud = ud;
}

/*
 * set name & desc
 */
void
lwpSetDesc(struct lwpProc *p, char *name, char *desc)
{
    if (!p)
	p = LwpCurrent;
    free(p->name);
    free(p->desc);
    p->name = strdup(name);
    p->desc = strdup(desc);
}

/*
 * lwpYield -- yield the processor to another thread.
 */
void
lwpYield(void)
{
    lwpStatus(LwpCurrent, "yielding control");
    lwpReady(LwpCurrent);
    lwpReschedule();
}

/*
 * cause the current process to be scheduled for deletion.
 */
void
lwpExit(void)
{
    lwpStatus(LwpCurrent, "marking self as dead");
    LwpCurrent->dead = 1;
    lwpYield();
}

/*
 * mark another process as dead, so it will never be rescheduled.
 * remove any lingering FD action
 */
void
lwpTerminate(struct lwpProc *p)
{
    lwpStatus(p, "terminating process");
    p->dead = 1;
    if (p->fd >= 0)
	lwpWakeupFd(p);
}

/*
 * set the thread's priority, returning the old.
 * if the new priority is lower than the old, we reschedule.
 */
int
lwpSetPriority(int new)
{
    int old = LwpCurrent->pri;

    if (LWP_MAX_PRIO <= new)
	new = LWP_MAX_PRIO - 1;
    if (LwpMaxpri < new)
	LwpMaxpri = new;
    LwpCurrent->pri = new;
    lwpStatus(LwpCurrent, "resetting priority (%d -> %d)", old, new);
    if (new < old)
	lwpYield();
    return old;
}

/*
 * initialise the coroutine structures
 */
struct lwpProc *
lwpInitSystem(int pri, void **ctxptr, int flags, sigset_t *waitset)
{
    struct lwpQueue *q;
    int i, *stack, marker;
    struct lwpProc *sel;

    LwpContextPtr = ctxptr;
    if (pri < 1)
	pri = 1;
    LwpStackGrowsDown = growsdown(&marker);
    if (!(LwpCurrent = calloc(1, sizeof(struct lwpProc))))
	return NULL;
    if (!(stack = malloc(64)))
	return NULL;
    if (LWP_MAX_PRIO <= pri)
	pri = LWP_MAX_PRIO - 1;
    if (LwpMaxpri < pri)
	LwpMaxpri = pri;
    LwpCurrent->next = 0;
    LwpCurrent->sbtm = stack;	/* dummy stack for "main" */
    LwpCurrent->pri = pri;
    LwpCurrent->dead = 0;
    LwpCurrent->flags = flags & ~LWP_STACKCHECK;
    LwpCurrent->name = "Main";
    for (i = LWP_MAX_PRIO, q = LwpSchedQ; i--; q++)
	q->head = q->tail = 0;
    LwpDeadQ.head = LwpDeadQ.tail = 0;
    lwpInitSigWait(waitset);
    /* must be lower in priority than us for this to work right */
    sel = lwpCreate(0, lwpSelect, 16384, flags, "EventHandler",
		    "Select (main loop) Event Handler", 0, 0, 0);
    lwpInitSelect(sel);
    return LwpCurrent;
}

/* lwpStackCheckInit
 *
 * Initialize the entire stack (including both redzones) with the stack
 * check mark.  Thus, we can get some indication of how much stack was
 * used.
 */
static void
lwpStackCheckInit(struct lwpProc *newp)
{
    int *p;
    int *lim = (int *)((char *)newp->sbtm + newp->size);

    for (p = newp->sbtm; p < lim; p++)
	*p = LWP_CHECKMARK;
}

/* lwpStackCheck
 *
 * Check if the thread has overflowed/underflowed its stack.
 * Should that happen, abort the process, as we cannot recover.
 */
static void
lwpStackCheck(struct lwpProc *newp)
{
    int *btm = (int *)(newp->ustack - LWP_REDZONE);
    int *top = (int *)(newp->ustack + newp->usize + LWP_REDZONE);
    int n = LWP_REDZONE / sizeof(int);
    int i, lo_clean, hi_clean, overflow, underflow;

    for (i = 0; i < n && btm[i] == LWP_CHECKMARK; i++) ;
    lo_clean = i;

    for (i = 1; i <= n && top[-i] == LWP_CHECKMARK; i++) ;
    hi_clean = i - 1;

    if (LwpStackGrowsDown) {
	overflow = n - lo_clean;
	underflow = n - hi_clean;
    } else {
	overflow = n - hi_clean;
	underflow = n - lo_clean;
    }
    if (overflow)
	logerror("Thread %s stack overflow %d bytes",
		 newp->name, overflow * (int)sizeof(int));
    if (underflow)
	logerror("Thread %s stack underflow %d bytes",
		 newp->name, underflow * (int)sizeof(int));
    if (overflow || underflow)
	abort();
}

/* lwpStackCheckUsed
 *
 * Figure out how much stack was used by this thread.
 */
static void
lwpStackCheckUsed(struct lwpProc *newp)
{
    int *base = (int *)newp->ustack;
    int *lim = (int *)(newp->ustack + newp->usize);
    int total = (lim + 1 - base) * sizeof(int);
    int used, *p;

    if (LwpStackGrowsDown) {
	for (p = base; p < lim && *p == LWP_CHECKMARK; ++p) ;
	used = (lim - p) * sizeof(int);
    } else {
	for (p = lim - 1; p >= base && *p == LWP_CHECKMARK; --p) ;
	used = (p - base + 1) * sizeof(int);
    }
    lwpStatus(newp, "Thread stack %d used, %d left, %d total",
	      used, total - used, total);
}
