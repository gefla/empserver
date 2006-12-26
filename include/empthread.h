/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  empthread.h: Definitions for Empire threading
 * 
 *  Known contributors to this file:
 *     Sasha Mikheev
 *     Doug Hay, 1998
 *     Steve McClure, 1998
 *     Markus Armbruster, 2005-2006
 */

/*
 * This header defines Empire's abstract thread interface.  There are
 * several concrete implementations.
 *
 * Empire threads are non-preemptive, i.e. they run until they
 * voluntarily yield the processor.  The thread scheduler then picks
 * one of the runnable threads with the highest priority.  Priorities
 * are static.  Empire code relies on these properties heavily.  The
 * most common form of yielding the processor is sleeping for some
 * event to happen.
 */

#ifndef EMPTHREAD_H
#define EMPTHREAD_H

#include <time.h>

/* thread priorities */
enum {
    PP_MAIN	 = 7,
    PP_UPDATE	 = 6,
    PP_SHUTDOWN	 = 5,
    PP_SCHED	 = 4,
    PP_TIMESTAMP = 2,
    PP_PLAYER	 = 3,
    PP_ACCEPT	 = 3,
    PP_KILLIDLE	 = 2
};

#ifdef EMPTH_LWP
#include "lwp.h"

/* Abstract data types */

/* empth_t * represents a thread.  */
typedef struct lwpProc empth_t;

/* empth_sem_t * represents a semaphore */
typedef struct lwpSem empth_sem_t;

/* Flags for empth_select(): whether to sleep on input or output */
#define EMPTH_FD_READ     LWP_FD_READ
#define EMPTH_FD_WRITE    LWP_FD_WRITE

/* Flags for empth_init() and empth_create() */
/* Request debug prints */
#define EMPTH_PRINT       LWP_PRINT
/* Request stack checking */
#define EMPTH_STACKCHECK  LWP_STACKCHECK

#endif /* EMPTH_LWP */

#ifdef EMPTH_POSIX
#define EMPTH_FD_READ   0x1
#define EMPTH_FD_WRITE  0x2

#define EMPTH_PRINT       0x1
#define EMPTH_STACKCHECK  0x2

typedef struct empth_t empth_t;
typedef struct empth_sem_t empth_sem_t;

#endif /* EMPTH_POSIX */

#ifdef EMPTH_W32
/* The Windows NT Threads */
#define EMPTH_FD_READ   0x1
#define EMPTH_FD_WRITE  0x2

#define EMPTH_PRINT       0x1
#define EMPTH_STACKCHECK  0x2

typedef struct loc_Thread_t empth_t;
typedef struct loc_Sem_t empth_sem_t;

void empth_request_shutdown(void);
#endif /* EMPTH_W32 */

/*
 * Initialize thread package.
 * CTX points to a thread context variable; see empth_create().
 * FLAGS request optional features.
 * Should return 0 on success, -1 on error, but currently always
 * returns 0.
 */
int empth_init(void **ctx, int flags);

/*
 * Create a new thread.
 * PRIO is the scheduling priority.
 * ENTRY is the entry point.  It will be called with argument UD.
 * Thread stack is at least SIZE bytes.
 * FLAGS should be the same as were passed to empth_init(), or zero.
 * NAME is the threads name, and DESC its description.  These are used
 * for logging and debugging.
 * UD is the value to pass to ENTRY.  It is also assigned to the
 * context variable defined with empth_init() whenever the thread gets
 * scheduled.
 * Yield the processor.
 * Return the thread, or NULL on error.
 */
empth_t *empth_create(int prio, void (*entry)(void *),
		      int size, int flags, char *name, char *desc, void *ud);

/*
 * Return the current thread.
 */
empth_t *empth_self(void);

/*
 * Terminate the current thread.
 * The current thread should not be the thread that executed main().
 * If it is, implementations may terminate the process rather than the
 * thread.
 * Never returns.
 */
void empth_exit(void);

/*
 * Yield the processor.
 */
void empth_yield(void);

/*
 * Terminate THREAD.
 * THREAD will not be scheduled again.  Instead, it will terminate as
 * if it executed empth_exit().  It is unspecified when exactly that
 * happens.
 * THREAD must not be the current thread.
 */
void empth_terminate(empth_t *thread);

/*
 * Put current thread to sleep until file descriptor FD is ready for I/O.
 * If FLAGS & EMPTH_FD_READ, wake up if FD is ready for input.
 * If FLAGS & EMPTH_FD_WRITE, wake up if FD is ready for output.
 * At most one thread may sleep on the same file descriptor.
 * Note: Currently, Empire sleeps only on network I/O, i.e. FD is a
 * socket.  Implementations should not rely on that.
 */
void empth_select(int fd, int flags);

/*
 * Awaken THREAD if it is sleeping in empth_select().
 * Note: This must not awaken threads sleeping in other functions.
 */
void empth_wakeup(empth_t *thread);

/*
 * Put current thread to sleep until the time is UNTIL.
 * May sleep somehwat longer, but never shorter.
 */
void empth_sleep(time_t until);

/*
 * Wait for signal, return the signal number
 */
int empth_wait_for_signal(void);

/*
 * Create a semaphore.
 * NAME is its name, it is used for debugging.
 * COUNT is the initial count value of the semaphore, it must not be
 * negative.
 * Return the semaphore, or NULL on error.
 */
empth_sem_t *empth_sem_create(char *name, int count);

/*
 * Signal SEM.
 * Increase SEM's count.  If threads are sleeping on it, wake up
 * exactly one of them.  If that thread has a higher priority, yield
 * the processor.
 * This semaphore operation is often called `down' or `V' otherwhere.
 */
void empth_sem_signal(empth_sem_t *sem);

/*
 * Wait for SEM.
 * If SEM has a zero count, put current thread to sleep until
 * empth_sem_signal() awakens it.  SEM will have non-zero value then.
 * Decrement SEM's count.
 * This semaphore operation is often called `up' or `P' otherwhere.
 */
void empth_sem_wait(empth_sem_t *sem);


/*
 * Stuff for implementations, not for clients.
 */

void empth_init_signals(void);

#endif
