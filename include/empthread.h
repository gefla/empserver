/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  empthread.h: Definitions for Empire threading
 *
 *  Known contributors to this file:
 *     Sasha Mikheev
 *     Doug Hay, 1998
 *     Steve McClure, 1998
 *     Markus Armbruster, 2005-2012
 *     Ron Koenderink, 2005-2009
 */

/*
 * This header defines Empire's abstract thread interface.  There are
 * several concrete implementations.
 *
 * Empire threads are non-preemptive, i.e. they run until they
 * voluntarily yield the processor.  The thread scheduler then picks
 * one of the runnable threads.  The most common form of yielding the
 * processor is sleeping for some event to happen.
 */

#ifndef EMPTHREAD_H
#define EMPTHREAD_H

#include <sys/time.h>
#include <time.h>

#ifdef EMPTH_LWP
#include "lwp.h"

/* Abstract data types */

/* A thread. */
typedef struct lwpProc empth_t;

/* A read-write lock, perferring writers */
typedef struct lwp_rwlock empth_rwlock_t;

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
typedef struct empth_rwlock_t empth_rwlock_t;

#endif /* EMPTH_POSIX */

#ifdef EMPTH_W32
/* The Windows NT Threads */
#define EMPTH_FD_READ   0x1
#define EMPTH_FD_WRITE  0x2

#define EMPTH_PRINT       0x1
#define EMPTH_STACKCHECK  0x2

typedef struct loc_Thread empth_t;
typedef struct loc_RWLock empth_rwlock_t;

void empth_request_shutdown(void);
#endif /* EMPTH_W32 */

/*
 * Initialize thread package.
 * @ctx points to a thread context variable; see empth_create().
 * @flags request optional features.
 * Should return 0 on success, -1 on error, but currently always
 * returns 0.
 */
int empth_init(void **ctx, int flags);

/*
 * Create a new thread.
 * @entry is the entry point.  It will be called with argument @ud.
 * If it returns, the thread terminates as if it called empth_exit().
 * Thread stack is at least @size bytes.
 * @flags should be the same as were passed to empth_init(), or zero.
 * @name is the thread's name, it is used for logging and debugging.
 * @ud is the value to pass to @entry.  It is also assigned to the
 * context variable defined with empth_init() whenever the thread gets
 * scheduled.
 * Yield the processor.
 * Return the thread, or NULL on error.
 */
empth_t *empth_create(void (*entry)(void *),
		      int size, int flags, char *name, void *ud);

/*
 * Return the current thread, NULL before empth_init().
 * This is the only function that may be called before empth_init().
 */
empth_t *empth_self(void);

/*
 * Return @thread's name.
 */
char *empth_name(empth_t *thread);

/*
 * Set @thread's name to @name.
 */
void empth_set_name(empth_t *thread, char *name);

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
 * Put current thread to sleep until file descriptor @fd is ready for I/O.
 * If @flags & EMPTH_FD_READ, wake up if @fd is ready for input.
 * If @flags & EMPTH_FD_WRITE, wake up if @fd is ready for output.
 * At most one thread may sleep on the same file descriptor.
 * @timeout, if non-null, limits the sleep time.
 * Return one when the @fd is ready, zero on timeout or early wakeup by
 * empth_wakeup(), -1 on error with errno set.
 * Note: Currently, Empire sleeps only on network I/O, i.e. @fd is a
 * socket.  Implementations should not rely on that.
 */
int empth_select(int fd, int flags, struct timeval *timeout);

/*
 * Awaken @thread if it is sleeping in empth_select() or empth_sleep().
 * This does not awaken threads sleeping in other functions.
 * Does not yield the processor.
 */
void empth_wakeup(empth_t *thread);

/*
 * Put current thread to sleep until the time is @until.
 * Return 0 if it slept until that time.
 * Return -1 if woken up early, by empth_wakeup().
 */
int empth_sleep(time_t until);

/*
 * Put current thread to sleep until SIGHUP, SIGINT or SIGTERM is received.
 * Return the signal number.
 */
int empth_wait_for_signal(void);

/*
 * Create a read-write lock.
 * @name is its name, it is used for debugging.
 * Return the read-write lock, or NULL on error.
 */
empth_rwlock_t *empth_rwlock_create(char *name);

/*
 * Destroy @rwlock.
 */
void empth_rwlock_destroy(empth_rwlock_t *rwlock);

/*
 * Lock @rwlock for writing.
 * A read-write lock can be locked for writing only when it is
 * unlocked.  If this is not the case, put the current thread to sleep
 * until it is.
 */
void empth_rwlock_wrlock(empth_rwlock_t *rwlock);

/*
 * Lock @rwlock for reading.
 * A read-write lock can be locked for reading only when it is not
 * locked for writing, and no other thread is attempting to lock it
 * for writing.  If this is not the case, put the current thread to
 * sleep until it is.
 */
void empth_rwlock_rdlock(empth_rwlock_t *rwlock);

/*
 * Unlock read-write lock @rwlock.
 * The current thread must hold @rwlock.
 * Wake up threads that can now lock it.
 */
void empth_rwlock_unlock(empth_rwlock_t *rwlock);


/*
 * Stuff for implementations, not for clients.
 */

void empth_init_signals(void);

#endif
