/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#include <stdio.h>
#include "prototypes.h"
#include "empthread.h"

#ifdef _EMPTH_LWP

int
empth_init(char **ctx, int flags)
{
    lwpInitSystem(7, ctx, flags);
    return 0;
}


empth_t *
empth_create(int prio, void (*entry) (), int size, int flags, char *name,
	     char *desc, void *ud)
{
    /* inherit flags */
    if (!flags)
	flags = LwpCurrent->flags;
    return lwpCreate(prio, entry, size, flags, name, desc, 0, 0, ud);
}

#if 0
/* This is unused now? */
void
empth_setctx(void *ctx)
{
    /* lwp does it automatically */
    return;
}
#endif

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
    if (!strcmp(LwpCurrent->name, "Main")) {
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
    /* a hack! */
    lwpReschedule();
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
empth_sleep(long int until)
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

void
empth_alarm(int sig)
{
    /* no way we can be here while using LWP threads */
    panic(sig);
}
#endif
