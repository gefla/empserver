/*
 * lwp.h -- prototypes and structures for lightweight processes
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
#ifndef _LWP_H
#define _LWP_H

#ifndef _PROTO
#ifdef __cplusplus
#define _PROTO(x)	x
#else
#define _PROTO(x)	()
#endif
#endif

#include <setjmp.h>
#include <sys/time.h>

/* process control block.  do *not* change the position of context */
struct lwpProc {
	jmp_buf	context;	/* processor context area */
	void	*sbtm;		/* bottom of stack attached to it */
	int     size;           /* size of stack */
	void	(*entry)();	/* entry point */
	int	dead;		/* whether the process can be rescheduled */
	int	pri;		/* which scheduling queue we're on */
	long	runtime;	/* time at which process is restarted */
	int	fd;		/* fd we're blocking on */
	int	argc;		/* initial arguments */
	char	**argv;
	void	*ud;		/* user data */
	struct lwpProc *next;
};

/* queue */
struct lwpQueue {
	struct lwpProc *head;
	struct lwpProc *tail;
};

/* semaphore */
struct lwpSem {
	int	count;
	struct lwpQueue q;
};

#define LWP_FD_READ	0x1
#define LWP_FD_WRITE	0x2

#define LWP_MAX_PRIO	8

struct lwpProc	*lwpInitSystem _PROTO((int));
struct lwpProc	*lwpCreate _PROTO((int, void (*)(), int, int, char **, void *));
void		lwpExit _PROTO((void));
void		lwpTerminate _PROTO((struct lwpProc *));
void		lwpYield _PROTO((void));
void		lwpSleepFd _PROTO((int fd, int flags));
void		lwpSleepUntil _PROTO((long until));
void		lwpWakeupFd _PROTO((struct lwpProc *));
void		*lwpGetUD _PROTO((struct lwpProc *));
void		lwpSetUD _PROTO((struct lwpProc *, char *));
int		lwpSetPriority _PROTO((int));
void		lwpReschedule _PROTO(());

struct lwpSem	*lwpCreateSem _PROTO((int));
void		lwpSignal _PROTO((struct lwpSem *));
void		lwpWait _PROTO((struct lwpSem *));
void		lwpSelect _PROTO((int argc, char **argv));

extern struct lwpProc	*LwpCurrent;

#endif	/* _LWP_H */
