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
 *  sel.c: arrange to block on read/write file descriptors using lwp
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 */

#include <config.h>

#include <errno.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "lwp.h"
#include "lwpint.h"
#include "prototypes.h"

/* Largest fd in LwpReadfds, LwpWritefds */
static int LwpMaxfd;

/* Number of file descriptors in LwpReadfds, LwpWritefds */
static int LwpNfds;

/* File descriptors waited for in lwpSleepFd() */
static fd_set LwpReadfds, LwpWritefds;

/* Map file descriptor to thread sleeping in lwpSleepFd() */
static struct lwpProc **LwpFdwait;

/* Threads sleeping in lwpSleepUntil(), in no particular order */
static struct lwpQueue LwpDelayq;

/* The thread executing lwpSelect() */
static struct lwpProc *LwpSelProc;	

void
lwpInitSelect(struct lwpProc *proc)
{
    LwpMaxfd = 0;
    LwpNfds = 0;
    FD_ZERO(&LwpReadfds);
    FD_ZERO(&LwpWritefds);
    LwpFdwait = calloc(FD_SETSIZE, sizeof(struct lwpProc *));
    LwpDelayq.head = 0;
    LwpDelayq.tail = 0;
    LwpSelProc = proc;
}

void
lwpSleepFd(int fd, int mask)
{
    lwpStatus(LwpCurrent, "sleeping on fd %d for %d", fd, mask);

    if (CANT_HAPPEN(fd > FD_SETSIZE))
	return;
    if (LwpFdwait[fd] != 0) {
	lwpStatus(LwpCurrent,
		  "multiple sleeps attempted on file descriptor %d", fd);
	return;
    }
    if (mask & LWP_FD_READ)
	FD_SET(fd, &LwpReadfds);
    if (mask & LWP_FD_WRITE)
	FD_SET(fd, &LwpWritefds);

    LwpNfds++;

    if (LwpMaxfd == 0 && LwpDelayq.head == 0) {
	/* select process is sleeping until first waiter arrives */
	lwpStatus(LwpCurrent, "going to resched fd %d", fd);
	lwpReady(LwpSelProc);
    }
    lwpStatus(LwpCurrent, "going to wait on fd %d", fd);
    if (fd > LwpMaxfd)
	LwpMaxfd = fd;
    LwpFdwait[fd] = LwpCurrent;
    LwpCurrent->fd = fd;
    lwpReschedule();
}

void
lwpWakeupFd(struct lwpProc *proc)
{
    if (proc->fd < 0)
	return;

    lwpStatus(proc, "awakening; was sleeping on fd %d", proc->fd);
    FD_CLR(proc->fd, &LwpReadfds);
    FD_CLR(proc->fd, &LwpWritefds);
    LwpNfds--;
    LwpFdwait[proc->fd] = 0;
    proc->fd = -1;
    lwpReady(proc);
}

void
lwpSleepUntil(long until)
{
    lwpStatus(LwpCurrent, "sleeping for %ld sec", until - time(0));
    LwpCurrent->runtime = until;
    if (LwpMaxfd == 0 && LwpDelayq.head == 0) {
	/* select process is sleeping until first waiter arrives */
	lwpReady(LwpSelProc);
    }
    lwpAddTail(&LwpDelayq, LwpCurrent);
    lwpReschedule();
}

/*ARGSUSED*/
void
lwpSelect(void *arg)
{
    struct lwpProc *us = LwpCurrent;
    fd_set readmask;
    fd_set writemask;
    int n;
    int fd;
    time_t now;
    time_t delta;
    struct lwpProc *proc;
    struct timeval tv;
    struct lwpQueue save;

    lwpStatus(us, "starting select loop");
    FD_ZERO(&readmask);
    FD_ZERO(&writemask);
    while (1) {
	while (1) {
	    if (LwpNfds)
		break;
	    if (LwpDelayq.head)
		break;
	    /* wait for someone to lwpSleepFd or lwpSleepUntil */
	    LwpMaxfd = 0;
	    lwpStatus(us, "no fds or sleepers, waiting");
	    lwpReschedule();
	}
	tv.tv_sec = 1000000;
	tv.tv_usec = 0;
	if (LwpDelayq.head) {
	    time(&now);
	    proc = LwpDelayq.head;
	    for (; proc != 0; proc = proc->next) {
		delta = proc->runtime - now;
		if (delta < tv.tv_sec)
		    tv.tv_sec = delta;
	    }
	    if (tv.tv_sec < 0)
		tv.tv_sec = 0;
	}
	lwpStatus(us, "selecting; sleep %ld secs", tv.tv_sec);

	memcpy(&readmask, &LwpReadfds, sizeof(fd_set));
	memcpy(&writemask, &LwpWritefds, sizeof(fd_set));
	n = select(LwpMaxfd + 1, &readmask, &writemask, NULL, &tv);
	if (n < 0) {
	    if (errno != EINTR) {
		logerror("select failed (%s)", strerror(errno));
		exit(1);
	    }
	    /* go handle the signal */
	    lwpReady(us);
	    lwpReschedule();
	    continue;
	}

	if (LwpDelayq.head) {
	    /* sleeping proecss activity */
	    time(&now);
	    save.tail = save.head = 0;
	    while (NULL != (proc = lwpGetFirst(&LwpDelayq))) {
		if (now >= proc->runtime) {
		    lwpStatus(proc, "sleep done");
		    lwpReady(proc);
		} else {
		    lwpAddTail(&save, proc);
		}
	    }
	    LwpDelayq = save;
	}
	if (n > 0) {
	    /* file descriptor activity */
	    for (fd = 0; fd <= LwpMaxfd; fd++) {
		if (LwpFdwait[fd] == 0)
		    continue;
		if (FD_ISSET(fd, &readmask)) {
		    lwpStatus(LwpFdwait[fd], "input ready");
		    lwpWakeupFd(LwpFdwait[fd]);
		    continue;
		}
		if (FD_ISSET(fd, &writemask)) {
		    lwpStatus(LwpFdwait[fd], "output ready");
		    lwpWakeupFd(LwpFdwait[fd]);
		    continue;
		}
	    }
	}
	lwpStatus(us, "fd dispatch completed");
	lwpReady(LwpCurrent);
	lwpReschedule();
    }
    /*NOTREACHED*/
}
