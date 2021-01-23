/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sig.c: Wait for signals
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2020
 */

#include <config.h>

#include <errno.h>
#include <stddef.h>
#include <signal.h>
#include "lwp.h"
#include "lwpint.h"

/*
 * Signals caught so far.
 * Access only with signals blocked!
 */
static sigset_t LwpSigCaught;

/*
 * LwpSigCaught changed since last
 */
static sig_atomic_t LwpSigCheck;

/* The thread waiting for signals in lwpSigWait() */
static struct lwpProc *LwpSigWaiter;

static void lwpCatchAwaitedSig(int);

/*
 * Initialize waiting for signals in @set.
 */
void
lwpInitSigWait(sigset_t *set)
{
    struct sigaction act;
    int i;

    sigemptyset(&LwpSigCaught);

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
    sigaddset(&LwpSigCaught, sig);
    LwpSigCheck = 1;
}

/*
 * Test whether a signal from @set has been caught.
 * If yes, delete that signal from the set of caught signals, and
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
	if (sigismember(set, i) && sigismember(&LwpSigCaught, i)) {
	    lwpStatus(LwpCurrent, "Got awaited signal %d", i);
	    sigdelset(&LwpSigCaught, i);
	    break;
	}
    }
    sigprocmask(SIG_SETMASK, &save, NULL);
    return i;
}

/*
 * Wait until a signal from @set arrives.
 * Assign its number to *@sig and return 0.
 * If another thread is already waiting for signals, return EBUSY
 * without waiting.
 */
int
lwpSigWait(sigset_t *set, int *sig)
{
    int res;

    if (LwpSigWaiter)
	return EBUSY;
    for (;;) {
	LwpSigCheck = 0;
	res = lwpGetSig(set);
	if (res > 0)
	    break;
	lwpStatus(LwpCurrent, "Waiting for signals");
	LwpSigWaiter = LwpCurrent;
	lwpReschedule();
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
    if (LwpSigWaiter && LwpSigCheck) {
	lwpReady(LwpSigWaiter);
	LwpSigWaiter = NULL;
    }
}
