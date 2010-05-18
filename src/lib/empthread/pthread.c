/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  pthread.c: Interface from Empire threads to POSIX threads
 *
 *  Known contributors to this file:
 *     Sasha Mikheev
 *     Steve McClure, 1998
 *     Markus Armbruster, 2005-2009
 *     Ron Koenderink, 2007-2009
 */

/* Required for PTHREAD_STACK_MIN on some systems, e.g. Solaris: */
#define _XOPEN_SOURCE 500

#include <config.h>

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "misc.h"
#include "empthread.h"
#include "file.h"
#include "prototypes.h"

struct empth_t {
    char *name;			/* thread name */
    void *ud;			/* user data */
    int wakeup;
    void (*ep)(void *);		/* entry point */
    pthread_t id;		/* thread id */
};

struct empth_rwlock_t {
    /* Can't use pthread_rwlock_t, because it needn't prefer writers */
    char *name;
    int nread;			/* #active readers */
    int nwrite;			/* total #writers (active and waiting) */
    pthread_cond_t can_read;
    pthread_cond_t can_write;
};

/* Thread-specific data key */
static pthread_key_t ctx_key;

/* Flags that were passed to empth_init() */
static int empth_flags;

/* Pointer to thread context variable */
static void **udata;

/*
 * Non-preemption mutex.
 * Empire code outside this file is only executed while holding this
 * mutex.  This makes sure Empire code is never preempted by Empire
 * code.
 */
static pthread_mutex_t mtx_ctxsw;

static void empth_status(char *format, ...)
    ATTRIBUTE((format (printf, 1, 2)));
static void empth_alarm(int sig);

static void *
empth_start(void *arg)
{
    empth_t *ctx = arg;

    ctx->id = pthread_self();
    pthread_setspecific(ctx_key, ctx);
    pthread_mutex_lock(&mtx_ctxsw);
    *udata = ctx->ud;
    ctx->ep(ctx->ud);
    empth_exit();
    return NULL;
}

static void
empth_status(char *format, ...)
{
    va_list ap;
    static struct timeval startTime;
    struct timeval tv;
    char buf[1024];
    int sec, msec;
    empth_t *a;

    va_start(ap, format);
    if (empth_flags & EMPTH_PRINT) {
	if (startTime.tv_sec == 0)
	    gettimeofday(&startTime, 0);
	gettimeofday(&tv, 0);
	sec = tv.tv_sec - startTime.tv_sec;
	msec = (tv.tv_usec - startTime.tv_usec) / 1000;
	if (msec < 0) {
	    sec++;
	    msec += 1000;
	}
	vsprintf(buf, format, ap);
	a = empth_self();
	printf("%d:%02d.%03d %17s: %s\n", sec / 60, sec % 60, msec / 10,
	       a->name, buf);

    }
    va_end(ap);
}


int
empth_init(void **ctx_ptr, int flags)
{
    empth_t *ctx;
    sigset_t set;
    struct sigaction act;

    empth_flags = flags;
    udata = ctx_ptr;

    empth_init_signals();
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = empth_alarm;
    sigaction(SIGALRM, &act, NULL);

    pthread_key_create(&ctx_key, NULL);
    pthread_mutex_init(&mtx_ctxsw, NULL);

    ctx = malloc(sizeof(empth_t));
    if (!ctx) {
	logerror("pthread init failed: not enough memory");
	exit(1);
    }
    ctx->name = "Main";
    ctx->ep = 0;
    ctx->ud = 0;
    ctx->id = pthread_self();
    ctx->wakeup = 0;
    pthread_setspecific(ctx_key, ctx);
    pthread_mutex_lock(&mtx_ctxsw);
    logerror("pthreads initialized");
    return 0;
}


empth_t *
empth_create(void (*entry)(void *), int size, int flags,
	     char *name, void *ud)
{
    pthread_t t;
    pthread_attr_t attr;
    empth_t *ctx;
    int eno;

    empth_status("creating new thread %s", name);
    ef_make_stale();

    ctx = malloc(sizeof(empth_t));
    if (!ctx) {
	logerror("not enough memory to create thread %s", name);
	return NULL;
    }
    ctx->name = strdup(name);
    ctx->ud = ud;
    ctx->wakeup = 0;
    ctx->ep = entry;

    eno = pthread_attr_init(&attr);
    if (eno) {
	logerror("can not create thread attribute %s: %s",
		 name, strerror(eno));
	goto bad;
    }
    if (size < PTHREAD_STACK_MIN)
	size = PTHREAD_STACK_MIN;
    pthread_attr_setstacksize(&attr, size);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    eno = pthread_create(&t, &attr, empth_start, ctx);
    if (eno) {
	logerror("can not create thread: %s: %s", name, strerror(eno));
	goto bad;
    }
    empth_status("new thread id is %ld", (long)t);
    empth_yield();
    return ctx;

  bad:
    pthread_attr_destroy(&attr);
    free(ctx);
    return NULL;
}


static void
empth_restorectx(void)
{
    empth_t *ctx_ptr;

    ctx_ptr = pthread_getspecific(ctx_key);
    *udata = ctx_ptr->ud;
    ctx_ptr->wakeup = 0;
    empth_status("context restored");
}

empth_t *
empth_self(void)
{
    return pthread_getspecific(ctx_key);
}

char *
empth_name(empth_t *thread)
{
    return thread->name;
}

void
empth_set_name(empth_t *thread, char *name)
{
    if (thread->name)
	free(thread->name);
    thread->name = strdup(name);
}

void
empth_exit(void)
{
    empth_t *ctx = pthread_getspecific(ctx_key);

    empth_status("empth_exit");
    ef_make_stale();
    pthread_mutex_unlock(&mtx_ctxsw);
    free(ctx->name);
    free(ctx);
    pthread_exit(0);
}

void
empth_yield(void)
{
    ef_make_stale();
    pthread_mutex_unlock(&mtx_ctxsw);
    pthread_mutex_lock(&mtx_ctxsw);
    empth_restorectx();
}

int
empth_select(int fd, int flags, struct timeval *timeout)
{
    fd_set readmask;
    fd_set writemask;
    struct timeval tv;
    int n;
    empth_t *ctx;
    int res = 0;

    ef_make_stale();
    pthread_mutex_unlock(&mtx_ctxsw);
    empth_status("select on %d for %d", fd, flags);

again:
    FD_ZERO(&readmask);
    FD_ZERO(&writemask);
    if (flags & EMPTH_FD_READ)
	FD_SET(fd, &readmask);
    if (flags & EMPTH_FD_WRITE)
	FD_SET(fd, &writemask);

    if (timeout)
	tv = *timeout;
    n = select(fd + 1, &readmask, &writemask, NULL, timeout ? &tv : NULL);
    if (n < 0) {
	ctx = pthread_getspecific(ctx_key);
	if (ctx->wakeup) {
	    empth_status("select woken up");
	    res = 0;
	} else if (errno == EINTR) {
	    empth_status("select broken by signal");
	    goto again;
	} else {
	    empth_status("select failed (%s)", strerror(errno));
	    res = -1;
	}
    } else if (n == 0) {
	empth_status("select timed out");
	res = 0;
    } else if (flags == EMPTH_FD_READ && FD_ISSET(fd, &readmask)) {
	empth_status("input ready");
	res = 1;
    } else if (flags == EMPTH_FD_WRITE && FD_ISSET(fd, &writemask)) {
	empth_status("output ready");
	res = 1;
    }

    pthread_mutex_lock(&mtx_ctxsw);
    empth_restorectx();
    return res;
}

static void
empth_alarm(int sig)
{
    /*
     * Nothing to do --- we handle this signal just to let
     * empth_wakeup() interrupt system calls.
     */
}

void
empth_wakeup(empth_t *a)
{
    empth_status("waking up thread %s", a->name);
    a->wakeup = 1;
    pthread_kill(a->id, SIGALRM);
}

int
empth_sleep(time_t until)
{
    empth_t *ctx = pthread_getspecific(ctx_key);
    time_t now;
    struct timeval tv;
    int res;

    ef_make_stale();
    pthread_mutex_unlock(&mtx_ctxsw);
    do {
	now = time(NULL);
	tv.tv_sec = until >= now ? until - now : 0;
	tv.tv_usec = 0;
	empth_status("going to sleep %ld sec", (long)tv.tv_sec);
	res = select(0, NULL, NULL, NULL, &tv);
    } while (res < 0 && !ctx->wakeup);
    empth_status("sleep done. Waiting for lock");
    pthread_mutex_lock(&mtx_ctxsw);
    empth_restorectx();
    return res;
}

int
empth_wait_for_signal(void)
{
    sigset_t set;
    int sig, err;

    ef_make_stale();
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_mutex_unlock(&mtx_ctxsw);
    for (;;) {
	empth_status("waiting for signals");
	err = sigwait(&set, &sig);
	if (CANT_HAPPEN(err)) {
	    sleep(60);
	    continue;
	}
	empth_status("got awaited signal %d", sig);
	pthread_mutex_lock(&mtx_ctxsw);
	empth_restorectx();
	return sig;
    }
}

empth_rwlock_t *
empth_rwlock_create(char *name)
{
    empth_rwlock_t *rwlock;

    rwlock = malloc(sizeof(*rwlock));
    if (!rwlock)
	return NULL;

    if (pthread_cond_init(&rwlock->can_read, NULL) != 0
	|| pthread_cond_init(&rwlock->can_write, NULL) != 0) {
	free(rwlock);
	return NULL;
    }

    rwlock->name = strdup(name);
    rwlock->nread = rwlock->nwrite = 0;
    return rwlock;
}

void
empth_rwlock_destroy(empth_rwlock_t *rwlock)
{
    pthread_cond_destroy(&rwlock->can_read);
    pthread_cond_destroy(&rwlock->can_write);
    free(rwlock->name);
    free(rwlock);
}

void
empth_rwlock_wrlock(empth_rwlock_t *rwlock)
{
    empth_status("wrlock %s %d %d",
		 rwlock->name, rwlock->nread, rwlock->nwrite);
    ef_make_stale();
    rwlock->nwrite++;
    while (rwlock->nread != 0 || rwlock->nwrite != 1) {
	empth_status("waiting for wrlock %s", rwlock->name);
	pthread_cond_wait(&rwlock->can_write, &mtx_ctxsw);
	empth_status("got wrlock %s %d %d",
		     rwlock->name, rwlock->nread, rwlock->nwrite);
	empth_restorectx();
    }
}

void
empth_rwlock_rdlock(empth_rwlock_t *rwlock)
{
    empth_status("rdlock %s %d %d",
		 rwlock->name, rwlock->nread, rwlock->nwrite);
    ef_make_stale();
    while (rwlock->nwrite) {
	empth_status("waiting for rdlock %s", rwlock->name);
	pthread_cond_wait(&rwlock->can_read, &mtx_ctxsw);
	empth_status("got rdlock %s %d %d",
		     rwlock->name, rwlock->nread, rwlock->nwrite);
	empth_restorectx();
    }
    rwlock->nread++;
}

void
empth_rwlock_unlock(empth_rwlock_t *rwlock)
{
    if (CANT_HAPPEN(!rwlock->nread && !rwlock->nwrite))
	return;
    if (rwlock->nread) {	/* holding read lock */
	if (!--rwlock->nread)
	    pthread_cond_signal(&rwlock->can_write);
    } else {
	rwlock->nwrite--;
	pthread_cond_signal(&rwlock->can_write);
    }
    if (rwlock->nwrite == 0)
	pthread_cond_broadcast(&rwlock->can_read);
}
