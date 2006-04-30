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

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "lwp.h"
#include "lwpint.h"

struct lwpSelect {
    int maxfd;
    int nfds;
    fd_set readmask;
    fd_set writemask;
    struct lwpProc **wait;
    struct lwpQueue delayq;
    struct lwpProc *proc;
};

struct lwpSelect LwpSelect;

void
lwpInitSelect(struct lwpProc *proc)
{
    LwpSelect.maxfd = 0;
    LwpSelect.nfds = 0;
    FD_ZERO(&LwpSelect.readmask);
    FD_ZERO(&LwpSelect.writemask);
    LwpSelect.wait = calloc(FD_SETSIZE, sizeof(struct lwpProc *));
    LwpSelect.delayq.head = 0;
    LwpSelect.delayq.tail = 0;
    LwpSelect.proc = proc;
}

void
lwpSleepFd(int fd, int mask)
{
    lwpStatus(LwpCurrent, "sleeping on fd %d for %d", fd, mask);

    if (CANT_HAPPEN(fd > FD_SETSIZE))
	return;
    if (LwpSelect.wait[fd] != 0) {
	lwpStatus(LwpCurrent,
		  "multiple sleeps attempted on file descriptor %d", fd);
	return;
    }
    if (mask & LWP_FD_READ)
	FD_SET(fd, &LwpSelect.readmask);
    if (mask & LWP_FD_WRITE)
	FD_SET(fd, &LwpSelect.writemask);

    LwpSelect.nfds++;

    if (LwpSelect.maxfd == 0 && LwpSelect.delayq.head == 0) {
	/* select process is sleeping until first waiter arrives */
	lwpStatus(LwpCurrent, "going to resched fd %d", fd);
	lwpReady(LwpSelect.proc);
    }
    lwpStatus(LwpCurrent, "going to wait on fd %d", fd);
    if (fd > LwpSelect.maxfd)
	LwpSelect.maxfd = fd;
    LwpSelect.wait[fd] = LwpCurrent;
    LwpCurrent->fd = fd;
    lwpReschedule();
}

void
lwpWakeupFd(struct lwpProc *proc)
{
    if (proc->fd < 0)
	return;

    lwpStatus(proc, "awakening; was sleeping on fd %d", proc->fd);
    FD_CLR(proc->fd, &LwpSelect.readmask);
    FD_CLR(proc->fd, &LwpSelect.writemask);
    LwpSelect.nfds--;
    LwpSelect.wait[proc->fd] = 0;
    proc->fd = -1;
    lwpReady(proc);
}

void
lwpSleepUntil(long until)
{
    lwpStatus(LwpCurrent, "sleeping for %ld sec", until - time(0));
    LwpCurrent->runtime = until;
    if (LwpSelect.maxfd == 0 && LwpSelect.delayq.head == 0) {
	/* select process is sleeping until first waiter arrives */
	lwpReady(LwpSelect.proc);
    }
    lwpAddTail(&LwpSelect.delayq, LwpCurrent);
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
	    if (LwpSelect.nfds)
		break;
	    if (LwpSelect.delayq.head)
		break;
	    /* wait for someone to lwpSleepFd or lwpSleepUntil */
	    LwpSelect.maxfd = 0;
	    lwpStatus(us, "no fds or sleepers, waiting");
	    lwpReschedule();
	}
	tv.tv_sec = 1000000;
	tv.tv_usec = 0;
	if (LwpSelect.delayq.head) {
	    time(&now);
	    proc = LwpSelect.delayq.head;
	    for (; proc != 0; proc = proc->next) {
		delta = proc->runtime - now;
		if (delta < tv.tv_sec)
		    tv.tv_sec = delta;
	    }
	    if (tv.tv_sec < 0)
		tv.tv_sec = 0;
	}
	lwpStatus(us, "selecting; sleep %ld secs", tv.tv_sec);

	memcpy(&readmask, &LwpSelect.readmask, sizeof(fd_set));
	memcpy(&writemask, &LwpSelect.writemask, sizeof(fd_set));
	n = select(LwpSelect.maxfd + 1, &readmask, &writemask,
		   (fd_set *) 0, &tv);

	if (n < 0) {
	    if (errno == EINTR) {
		/* go handle the signal */
		lwpReady(us);
		lwpReschedule();
		continue;
	    }
	    lwpStatus(us, "select failed (bad file descriptor?)");
	    exit(-1);
	}

	if (LwpSelect.delayq.head) {
	    /* sleeping proecss activity */
	    time(&now);
	    save.tail = save.head = 0;
	    while (NULL != (proc = lwpGetFirst(&LwpSelect.delayq))) {
		if (now >= proc->runtime) {
		    lwpStatus(proc, "sleep done");
		    lwpReady(proc);
		} else {
		    lwpAddTail(&save, proc);
		}
	    }
	    LwpSelect.delayq = save;
	}
	if (n > 0) {
	    /* file descriptor activity */
	    for (fd = 0; fd <= LwpSelect.maxfd; fd++) {
		if (LwpSelect.wait[fd] == 0)
		    continue;
		if (FD_ISSET(fd, &readmask)) {
		    lwpStatus(LwpSelect.wait[fd], "input ready");
		    lwpWakeupFd(LwpSelect.wait[fd]);
		    continue;
		}
		if (FD_ISSET(fd, &writemask)) {
		    lwpStatus(LwpSelect.wait[fd], "output ready");
		    lwpWakeupFd(LwpSelect.wait[fd]);
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
