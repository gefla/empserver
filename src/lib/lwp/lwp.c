/*
 * lwp.c -- lightweight process creation, destruction and manipulation.
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

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "lwp.h"
#include "lwpint.h"
#include "prototypes.h"

#if defined(_EMPTH_LWP)

#ifdef BOUNDS_CHECK
#include <bounds/fix-args.h>
#include <bounds/unchecked.h>
#endif

struct lwpQueue LwpSchedQ[LWP_MAX_PRIO], LwpDeadQ;

struct lwpProc *LwpCurrent = NULL;
char **LwpContextPtr;
int LwpMaxpri = 0;		/* maximum priority so far */

static sigset_t oldmask;

/* for systems without strdup  */
#ifdef NOSTRDUP
extern char *strdup();
#endif /* NOSTRDUP */

static void lwpStackCheckInit(struct lwpProc *newp);
static void lwpStackCheck(struct lwpProc *newp);
static void lwpStackCheckUsed(struct lwpProc *newp);

/* check stack direction */
static int
growsdown(void *x)
{
    int y;

#ifdef BOUNDS_CHECK
    BOUNDS_CHECKING_OFF;
#endif
    y = (x > (void *)&y);

#ifdef BOUNDS_CHECK
    BOUNDS_CHECKING_ON;
#endif

    return y;
}

/*
 * lwpReschedule -- schedule another process.  we also check for dead
 * processes here and free them.
 */
void
lwpReschedule(void)
{
    static int lcount = LCOUNT;
    static struct lwpProc *nextp;
    static int i;
    static sigset_t tmask;

    if (LwpCurrent && (LwpCurrent->flags & LWP_STACKCHECK)) {
	lwpStackCheck(LwpCurrent);
    }
    if (!--lcount) {
	int p = lwpSetPriority(LWP_MAX_PRIO - 1);
	lcount = LCOUNT;
	sigprocmask(SIG_SETMASK, &oldmask, &tmask);
	sigprocmask(SIG_SETMASK, &tmask, &oldmask);
	LwpCurrent->pri = p;
    }

    /* destroy dead threads */
    lwpStatus(LwpCurrent, "Cleaning dead queue");
    while (NULL != (nextp = lwpGetFirst(&LwpDeadQ))) {
	if (nextp == LwpCurrent) {
	    lwpStatus(nextp, "OOOPS, we are running already dead thread");
	    exit(1);
	}
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
/*				fprintf(stderr,  "Destroying done\n"); */
	    }
	    nextp = 0;
	}
	if (nextp)
	    break;
    }
    if (LwpCurrent == 0 && nextp == 0) {
	fprintf(stderr, "No processes to run!\n");
	exit(1);
    }
    if (LwpCurrent)
	lwpStatus(LwpCurrent, "switch out");
    /* do context switch */
#ifdef BOUNDS_CHECK
    BOUNDS_CHECKING_OFF;
#endif

#if defined(hpc)
    {
	int endpoint;

	endpoint = &endpoint;
	if (initcontext == NULL || endpoint < startpoint) {
	    i = lwpSave(LwpCurrent->context);
	} else {
	    LwpCurrent->size = endpoint - startpoint;
	    LwpCurrent->sbtm = realloc(LwpCurrent->sbtm, LwpCurrent->size);
	    memcpy(LwpCurrent->sbtm, startpoint, LwpCurrent->size);
	    if (i = lwpSave(LwpCurrent->context)) {
		memcpy(startpoint, LwpCurrent->sbtm, LwpCurrent->size);
		i = 1;
	    }
	}
    }
#else
    i = lwpSave(LwpCurrent->context);
#endif
#ifdef BOUNDS_CHECK
    BOUNDS_CHECKING_ON;
#endif

    if (LwpCurrent != nextp && !(LwpCurrent && i)) {
	/* restore previous context */
	lwpStatus(nextp, "switch in %d", nextp->pri);
	LwpCurrent = nextp;
	*LwpContextPtr = LwpCurrent->ud;
#ifdef BOUNDS_CHECK
	BOUNDS_CHECKING_OFF;
#endif
	lwpRestore(LwpCurrent->context);

#ifdef BOUNDS_CHECK
	BOUNDS_CHECKING_ON;
#endif
    }
}

/*
 * lwpEntryPoint -- process entry point.
 */
void
lwpEntryPoint(void)
{
    sigset_t set;

#ifdef BOUNDS_CHECK
    BOUNDS_CHECKING_OFF;
#endif
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_SETMASK, &set, &oldmask);
    *LwpContextPtr = LwpCurrent->ud;

    lwpStatus(LwpCurrent, "starting at entry point");
    (*LwpCurrent->entry)(LwpCurrent->ud);
    lwpExit();
#ifdef BOUNDS_CHECK
    BOUNDS_CHECKING_ON;
#endif


}

/*
 * lwpCreate -- create a process.
 */
struct lwpProc *
lwpCreate(int priority, void (*entry)(void *), int size, int flags, char *name, char *desc, int argc, char **argv, void *ud)
{
    struct lwpProc *newp;
    int *s, x;
#ifdef UCONTEXT
    stack_t sp;
#else  /* UCONTEXT */
    void *sp;
#endif /* UCONTEXT */
    unsigned long stackp;

    if (!(newp = malloc(sizeof(struct lwpProc))))
	return (0);
    if (flags & LWP_STACKCHECK) {
	/* Add a 1K buffer on each side of the stack */
	size += 2 * LWP_REDZONE;
    }
    size += LWP_EXTRASTACK;
    size += sizeof(stkalign_t);
    if (!(s = malloc(size)))
	return (0);
    newp->flags = flags;
    newp->name = strdup(name);
    newp->desc = strdup(desc);
    newp->entry = entry;
    newp->argc = argc;
    newp->argv = argv;
    newp->ud = ud;
    if ((newp->flags & LWP_STACKCHECK) == 0) {
	stackp = growsdown((void *)&x)
	    ? (unsigned long)s + size - sizeof(stkalign_t) - LWP_EXTRASTACK
	    : (unsigned long)s + LWP_EXTRASTACK;
#ifdef UCONTEXT
	sp.ss_sp = (void *)(stackp & -sizeof(stkalign_t));
	sp.ss_size = size;
	sp.ss_flags = 0;
#else  /* UCONTEXT */
	sp = (void *)(stackp & -sizeof(stkalign_t));
#endif /* UCONTEXT */
    } else {
	if (growsdown(&x)) {
	    /* round address off to stkalign_t */
	    stackp = ((long)s) + size - LWP_REDZONE -
		LWP_EXTRASTACK - sizeof(stkalign_t);
#ifdef UCONTEXT
	    sp.ss_sp = (void *)(stackp & -sizeof(stkalign_t));
	    sp.ss_size = size;
	    sp.ss_flags = 0;
	    newp->lowmark = (void *)(((long)sp.ss_sp) + LWP_EXTRASTACK);
#else  /* UCONTEXT */
	    sp = (void *)(stackp & -sizeof(stkalign_t));
	    newp->lowmark = (void *)(((long)sp) + LWP_EXTRASTACK);
#endif /* UCONTEXT */
	    newp->himark = s;
	} else {
	    stackp = ((long)s) + LWP_REDZONE + LWP_EXTRASTACK;
#ifdef UCONTEXT
	    sp.ss_sp = (void *)(((long)stackp) & -sizeof(stkalign_t));
	    sp.ss_size = size;
	    sp.ss_flags = 0;
#else  /* UCONTEXT */
	    sp = (void *)(((long)stackp) & -sizeof(stkalign_t));
#endif /* UCONTEXT */
	    newp->lowmark = (void *)s;
	    newp->himark = (void *)(((long)s) + size - LWP_REDZONE);
	}
    }
    if (LWP_MAX_PRIO <= priority)
	priority = LWP_MAX_PRIO - 1;
    if (LwpMaxpri < (newp->pri = priority))
	LwpMaxpri = priority;
    newp->sbtm = (void *)s;
    newp->size = size;
    newp->dead = 0;
    if (flags & LWP_STACKCHECK)
	lwpStackCheckInit(newp);
    lwpStatus(newp, "creating process structure sbtm: %p",
	      newp->sbtm);
    lwpReady(newp);
    lwpReady(LwpCurrent);
#ifdef UCONTEXT
    lwpInitContext(newp, &sp);	/* architecture-dependent: from arch.c */
#else  /* UCONTEXT */
    lwpInitContext(newp, sp);	/* architecture-dependent: from arch.c */
#endif /* UCONTEXT */
    lwpReschedule();
    return (newp);
}

void
lwpDestroy(struct lwpProc *proc)
{
    if (proc->flags & LWP_STACKCHECK) {
	lwpStackCheckUsed(proc);
	lwpStackCheck(proc);
    }
    lwpStatus(proc, "destroying sbtm: %p", proc->sbtm);
    proc->entry = 0;
    proc->ud = 0;
    proc->argv = 0;
    free(proc->sbtm);
    free(proc->name);
    free(proc->desc);
    proc->name = 0;
    proc->desc = 0;
    proc->sbtm = 0;
    proc->lowmark = 0;
    proc->himark = 0;
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
    return (p->ud);
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
    return (old);
}

/*
 * initialise the coroutine structures
 */
struct lwpProc *
lwpInitSystem(int pri, char **ctxptr, int flags)
{
    struct lwpQueue *q;
    int i, *stack;
    struct lwpProc *sel;

    LwpContextPtr = ctxptr;
    if (pri < 1)
	pri = 1;
    /* *LwpContextPtr = 0; */
    if (!(LwpCurrent = calloc(1, sizeof(struct lwpProc))))
	return (0);
    if (!(stack = malloc(64)))
	return (0);
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
    /* must be lower in priority than us for this to work right */
    sel = lwpCreate(0, lwpSelect, 16384, flags, "EventHandler",
		    "Select (main loop) Event Handler", 0, 0, 0);
    lwpInitSelect(sel);
    return (LwpCurrent);
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
    register int i;
    register long *lp;

    int lim = newp->size / sizeof(long);
    if (!newp || !newp->sbtm)
	return;
    for (lp = newp->sbtm, i = 0; i < lim; i++, lp++) {
	*lp = LWP_CHECKMARK;
    }
}

/* lwpStackCheck
 *
 * Check if the thread has overflowed/underflowed its stack.
 * Should that happen, abort the process, as we cannot recover.
 */
static void
lwpStackCheck(struct lwpProc *newp)
{
    register int end, amt;
    register unsigned int i;
    register long *lp;
    register int growsDown;
    int marker;

    if (CANT_HAPPEN(!newp || !newp->himark || !newp->lowmark))
	return;
    growsDown = growsdown(&marker);
    for (lp = newp->himark, i = 0; i < LWP_REDZONE / sizeof(long);
	 i++, lp++) {
	if (*lp == LWP_CHECKMARK)
	    continue;
	/* Stack overflow. */
	if (growsDown) {
	    end = i;
	    while (i < LWP_REDZONE / sizeof(long)) {
		if (*lp++ != LWP_CHECKMARK)
		    end = i;
		i++;
	    }
	    amt = (end + 1) * sizeof(long);
	} else {
	    amt = (i + 1) * sizeof(long);
	}
	logerror("Thread %s stack overflow %d bytes (of %u)",
		 newp->name, amt,
		 newp->size - 2 * LWP_REDZONE - (int)sizeof(stkalign_t));
	abort();
    }
    for (lp = newp->lowmark, i = 0; i < LWP_REDZONE / sizeof(long);
	 i++, lp++) {
	if (*lp == LWP_CHECKMARK)
	    continue;
	/* Stack underflow. */
	if (growsDown) {
	    end = i;
	    while (i < LWP_REDZONE / sizeof(long)) {
		if (*lp++ != LWP_CHECKMARK)
		    end = i;
		i++;
	    }
	    amt = (end + 1) * sizeof(long);
	} else {
	    amt = (LWP_REDZONE - i + 1) * sizeof(long);
	}
	logerror("Thread %s stack underflow %d bytes (of %u)",
		  newp->name, amt,
		 newp->size - 2 * LWP_REDZONE - (int)sizeof(stkalign_t));
	abort();
    }
}

/* lwpStackCheckUsed
 *
 * Figure out how much stack was used by this thread.
 */
static void
lwpStackCheckUsed(struct lwpProc *newp)
{
    register int i;
    register long *lp;
    register int lim;
    int marker;

    if (!newp || !newp->sbtm)
	return;
    lim = newp->size / sizeof(long);
    if (growsdown(&marker)) {
	/* Start at the bottom and find first non checkmark. */
	for (lp = newp->sbtm, i = 0; i < lim; i++, lp++) {
	    if (*lp != LWP_CHECKMARK) {
		break;
	    }
	}
    } else {
	/* Start at the top and find first non checkmark. */
	lp = newp->sbtm;
	lp += newp->size / sizeof(long);
	lp--;
	for (i = 0; i < lim; i++, lp--) {
	    if (*lp != LWP_CHECKMARK) {
		break;
	    }
	}
    }
    lwpStatus(newp, "Thread stack %lu used %lu left %lu total",
	      labs((char *)lp - (char *)newp->lowmark) - LWP_REDZONE,
	      labs((char *)newp->himark - (char *)lp) - LWP_REDZONE,
	      labs((char *)newp->himark - (char *)newp->lowmark) - LWP_REDZONE);
}

#endif
