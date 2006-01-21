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
 *  lwpint.h: lwp internal structures
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2006
 */

#ifndef LWPINT_H
#define LWPINT_H

#ifdef UCONTEXT
#include <ucontext.h>
#else  /* !UCONTEXT */
#include <setjmp.h>
#endif /* !UCONTEXT */

/* `liveness' counter: check signals every `n' visits to the scheduler */
/* note: the lower this value, the more responsive the system but the */
/* more inefficient the context switch time */
#define LCOUNT	-1

/* process control block.  do *not* change the position of context */
struct lwpProc {
#ifdef UCONTEXT
    ucontext_t context;		/* context structure */
#else  /* !UCONTEXT */
    jmp_buf context;		/* processor context area */
#endif /* !UCONTEXT */
    void *sbtm;			/* stack buffer attached to it */
    int size;			/* size of stack buffer */
    char *ustack;		/* lowest usable stack address */
    int usize;			/* size of usable stack */
    void (*entry)(void *);	/* entry point */
    int dead;			/* whether the process can be rescheduled */
    int pri;			/* which scheduling queue we're on */
    long runtime;		/* time at which process is restarted */
    int fd;			/* fd we're blocking on */
    int argc;			/* initial arguments */
    char **argv;
    void *ud;			/* user data */
    char *name;			/* process name and description */
    char *desc;
    int flags;
    struct lwpProc *next;
};

/* queue */
struct lwpQueue {
    struct lwpProc *head;
    struct lwpProc *tail;
};

/* semaphore */
struct lwpSem {
    int count;
    struct lwpQueue q;
    char *name;
};

#define LWP_REDZONE	1024	/* make this a multiple of 1024 */

/* XXX Note that this assumes sizeof(long) == 4 */
#define LWP_CHECKMARK	0x5a5a5a5aL

extern int LwpStackGrowsDown;

int lwpNewContext(struct lwpProc *, int);
void lwpSwitchContext(struct lwpProc *, struct lwpProc *);
void lwpAddTail(struct lwpQueue *, struct lwpProc *);
struct lwpProc *lwpGetFirst(struct lwpQueue *);
void lwpReady(struct lwpProc *);
void lwpReschedule(void);
void lwpEntryPoint(void);
void lwpInitSelect(struct lwpProc * self);
void lwpDestroy(struct lwpProc * proc);

#endif
