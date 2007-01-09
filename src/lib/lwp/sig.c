/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sig.c: Wait for signals
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2006
 */

#include <config.h>

#include <errno.h>
#include <signal.h>
#include "lwp.h"
#include "lwpint.h"

/* Signals awaited by lwpSigWait() */
static sigset_t *LwpSigAwaited;

/*
 * Signals from LwpSigAwaited catched so far
 * Access only with signals blocked!
 */
static sigset_t LwpSigCatched;

/* The thread waiting for signals in lwpSigWait() */
static struct lwpProc *LwpSigWaiter;

/* Where to return the signal number to the thread in lwpSigWait() */
static int *LwpSigPtr;

static void lwpCatchAwaitedSig(int);

/*
 * Initialize waiting for signals in SET.
 */
void
lwpInitSigWait(sigset_t *set)
{
    struct sigaction act;
    int i;

    sigemptyset(&LwpSigCatched);

    act.sa_flags = 0;
    act.sa_mask = *set;
    sigemptyset(&act.sa_mask);
    act.sa_handler = lwpCatchAwaitedSig;
    for (i = 0; i < NSIG; i++) {
	if (sigismember(set, i))
	    sigaction(i, &act, NULL);
    }
}

static void
lwpCatchAwaitedSig(int sig)
{
    sigaddset(&LwpSigCatched, sig);
}

/*
 * Test whether a signal from SET has been catched.
 * If yes, delete that signal from the set of catched signals, and
 * return its number.
 * Else return 0.
 */
static int
lwpGetSig(sigset_t *set)
{
    sigset_t save;
    int i;

    sigprocmask(SIG_BLOCK, set, &save);
    for (i = NSIG - 1; i > 0; i--) {
	if (sigismember(set, i) && sigismember(&LwpSigCatched, i)) {
	    lwpStatus(LwpCurrent, "Got awaited signal %d", i);
	    sigdelset(&LwpSigCatched, i);
	    break;
	}
    }
    sigprocmask(SIG_SETMASK, &save, NULL);
    return i;
}

/*
 * Wait until a signal from SET arrives.
 * Assign its number to *SIG and return 0.
 * If another thread is already waiting for signals, return EBUSY
 * without waiting.
 */
int
lwpSigWait(sigset_t *set, int *sig)
{
    int res;

    if (CANT_HAPPEN(LwpSigWaiter))
	return EBUSY;
    res = lwpGetSig(set);
    if (res <= 0) {
	lwpStatus(LwpCurrent, "Waiting for signals");
	LwpSigAwaited = set;
	LwpSigPtr = sig;
	LwpSigWaiter = LwpCurrent;
	lwpReschedule();
	return 0;
    }
    *sig = res;
    return 0;
}

/*
 * Wake up the thread awaiting signals if one arrived.
 * To be called from lwpReschedule().
 */
void
lwpSigWakeup(void)
{
    int res;

    if (!LwpSigWaiter)
	return;
    res = lwpGetSig(LwpSigAwaited);
    if (res > 0) {
	*LwpSigPtr = res;
	lwpReady(LwpSigWaiter);
	LwpSigWaiter = NULL;
    }
}
