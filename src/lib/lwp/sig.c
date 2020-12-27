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
#include <stdlib.h>
#include <signal.h>
#include "lwp.h"
#include "lwpint.h"

/*
 * Awaited signal numbers, terminated with 0.
 */
static int *LwpAwaitedSig;

/*
 * Pending awaited signals.
 * Access only with signals blocked!
 */
static volatile int *LwpSigCaught;

/*
 * Is there anything in LwpSigCaught[]?
 */
static volatile sig_atomic_t LwpSigCheck;

/* The thread waiting for signals in lwpSigWait() */
static struct lwpProc *LwpSigWaiter;

static void lwpCatchAwaitedSig(int);

/*
 * Initialize waiting for signals in @sig[].
 * @sig[] contains signal numbers, terminated with 0.  It must have
 * static storage duration.
 */
void
lwpInitSigWait(int sig[])
{
    struct sigaction act;
    int i;

    LwpAwaitedSig = sig;

    act.sa_flags = 0;
    act.sa_handler = lwpCatchAwaitedSig;
    sigemptyset(&act.sa_mask);
    for (i = 0; sig[i]; i++)
	sigaddset(&act.sa_mask, sig[i]);

    LwpSigCaught = calloc(i, sizeof(*LwpSigCaught));

    for (i = 0; sig[i]; i++)
	sigaction(sig[i], &act, NULL);
}

/*
 * Signal handler for awaited signals.
 * Set @LwpSigCaught[] for @sig, and set @LwpSigCheck.
 * Not reentrant; lwpInitSigWait() guards.
 */
static void
lwpCatchAwaitedSig(int sig)
{
    int i;

    for (i = 0; LwpAwaitedSig[i]; i++) {
	if (sig == LwpAwaitedSig[i])
	    LwpSigCaught[i] = 1;
    }
    LwpSigCheck = 1;
}

/*
 * Test whether an awaited signal is pending.
 * If yes, remove that signal from the set of pending signals, and
 * return its number.
 * Else return 0.
 */
static int
lwpGetSig(void)
{
    int ret = 0;
    sigset_t set, save;
    int i;

    sigemptyset(&set);
    for (i = 0; LwpAwaitedSig[i]; i++)
	sigaddset(&set, LwpAwaitedSig[i]);
    sigprocmask(SIG_BLOCK, &set, &save);

    for (i = 0; LwpAwaitedSig[i]; i++) {
	if (LwpSigCaught[i]) {
	    lwpStatus(LwpCurrent, "Got awaited signal %d", LwpSigCaught[i]);
	    ret = LwpAwaitedSig[i];
	    LwpSigCaught[i] = 0;
	}
    }

    for (; LwpAwaitedSig[i] && LwpSigCaught[i]; i++) ;
    if (!LwpSigCaught[i])
	LwpSigCheck = 0;

    sigprocmask(SIG_SETMASK, &save, NULL);
    return ret;
}

/*
 * Wait until one of the signals passed to lwpInitSigWait() arrives.
 * Assign its number to *@sig and return 0.
 * If another thread is already waiting for signals, return EBUSY
 * without waiting.
 */
int
lwpSigWait(int *sig)
{
    int res;

    if (LwpSigWaiter)
	return EBUSY;
    for (;;) {
	res = lwpGetSig();
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
