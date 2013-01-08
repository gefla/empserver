/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *  Copyright (C) 1991-3 Stephen Crane
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
 *  lwpint.h: lwp internal structures
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2009
 */

#ifndef LWPINT_H
#define LWPINT_H

#include <signal.h>
#include <time.h>
#include <ucontext.h>

#include "misc.h"

/* process control block.  do *not* change the position of context */
struct lwpProc {
    ucontext_t context;		/* context structure */
    void *sbtm;			/* stack buffer attached to it */
    int size;			/* size of stack buffer */
    char *ustack;		/* lowest usable stack address */
    int usize;			/* size of usable stack */
    void (*entry)(void *);	/* entry point */
    int dead;			/* whether the process can be rescheduled */
    int pri;			/* which scheduling queue we're on */
    time_t runtime;		/* time at which process is restarted */
    int fd;			/* fd we're blocking on */
    int fd_ready;		/* woken up because fd was ready */
    int argc;			/* initial arguments */
    char **argv;
    void *ud;			/* user data */
    char *name;			/* process name, for debugging */
    int flags;
    struct lwpProc *next;
};

/* queue */
struct lwpQueue {
    struct lwpProc *head;
    struct lwpProc *tail;
};

#define LWP_REDZONE	1024	/* make this a multiple of 1024 */

/* XXX Note that this assumes sizeof(int) == 4 */
#define LWP_CHECKMARK	0x5a5a5a5a

int lwpNewContext(struct lwpProc *);
void lwpSwitchContext(struct lwpProc *, struct lwpProc *);
void lwpAddTail(struct lwpQueue *, struct lwpProc *);
struct lwpProc *lwpGetFirst(struct lwpQueue *);
void lwpReady(struct lwpProc *);
void lwpReschedule(void);
void lwpEntryPoint(void);
void lwpInitSelect(struct lwpProc *);
void lwpWakeupSleep(void);
void lwpSelect(void *);
void lwpInitSigWait(sigset_t *);
void lwpSigWakeup(void);
void lwpStatus(struct lwpProc *, char *, ...)
    ATTRIBUTE((format (printf, 2, 3)));

#endif
