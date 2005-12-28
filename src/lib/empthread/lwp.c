/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  lwp.c: Interface from Empire threads to LWP threads
 * 
 *  Known contributors to this file:
 *     Sasha Mikheev
 */

#include <config.h>

#include <stdio.h>
#include "prototypes.h"
#include "empthread.h"

/* The thread `created' by lwpInitSystem() */
static empth_t *empth_main;

/* Flags that were passed to empth_init() */
static int empth_flags;


int
empth_init(void **ctx, int flags)
{
    empth_flags = flags;
    empth_main = lwpInitSystem(PP_MAIN, (char **)ctx, flags);
    return 0;
}


empth_t *
empth_create(int prio, void (*entry)(void *), int size, int flags,
	     char *name, char *desc, void *ud)
{
    if (!flags)
	flags = empth_flags;
    return lwpCreate(prio, entry, size, flags, name, desc, 0, 0, ud);
}

empth_t *
empth_self(void)
{
    return LwpCurrent;
}

void
empth_exit(void)
{
    time_t now;

    /* We want to leave the main thread around forever, until it's time
       for it to die for real (in a shutdown) */
    if (LwpCurrent == empth_main) {
	while (1) {
	    time(&now);
	    lwpSleepUntil(now + 60);
	}
    }
    lwpExit();
}

void
empth_yield(void)
{
    lwpYield();
}

void
empth_terminate(empth_t *a)
{
    lwpTerminate(a);
}

void
empth_select(int fd, int flags)
{
    lwpSleepFd(fd, flags);
}

void
empth_wakeup(empth_t *a)
{
    lwpWakeupFd(a);
}

void
empth_sleep(time_t until)
{
    lwpSleepUntil(until);
}


empth_sem_t *
empth_sem_create(char *name, int cnt)
{
    return lwpCreateSem(name, cnt);
}

void
empth_sem_signal(empth_sem_t *sm)
{
    lwpSignal(sm);
}

void
empth_sem_wait(empth_sem_t *sm)
{
    lwpWait(sm);
}
