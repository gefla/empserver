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
 *  empthread.h: Definitions for Empire threading
 * 
 *  Known contributors to this file:
 *     Sasha Mikheev
 *     Doug Hay, 1998
 *     Steve McClure, 1998
 */

#ifndef _EMTHREAD_H_
#define _EMTHREAD_H_

#include "prototype.h"
#include "misc.h"

#if defined(_WIN32)
#undef _EMPTH_LWP
#undef _EMPTH_POSIX
#define _EMPTH_WIN32
#endif

#ifdef _EMPTH_LWP
#include "lwp.h"
typedef struct lwpProc empth_t;
typedef struct lwpSem empth_sem_t;
#define EMPTH_FD_READ     LWP_FD_READ
#define EMPTH_FD_WRITE    LWP_FD_WRITE
#define EMPTH_PRINT       LWP_PRINT
#define EMPTH_STACKCHECK  LWP_STACKCHECK
#endif

#ifdef _EMPTH_POSIX
#ifdef __linux__
#define _MIT_POSIX_THREADS 1
#endif
#include <pthread.h>
#define EMPTH_FD_READ   0x1
#define EMPTH_FD_WRITE  0x2

#define EMPTH_PRINT       0x1
#define EMPTH_STACKCHECK  0x2

typedef void (*vf_ptr)();
#define EMPTH_KILLED  1
typedef struct empth_ctx_t {
    char *name;           /* thread name */
    char *desc;           /* description */
    void *ud;             /* user data */
    int  state;           /* my state */
    vf_ptr ep;            /* entry point */
    pthread_t id;         /* thread id */
}empth_t;

typedef struct {
    pthread_mutex_t mtx_update; /* use it to update count */
    int count;
    char name[80];
    pthread_mutex_t mtx_sem;
    pthread_cond_t cnd_sem;
}empth_sem_t;

#endif

/* DEC has slightly different names for whatever reason... */
#ifdef _DECTHREADS_
#define pthread_key_create  pthread_keycreate 
#define pthread_attr_init   pthread_attr_create
#define pthread_attr_destroy pthread_attr_delete

#endif


#if defined(_EMPTH_WIN32)
/* The Windows NT Threads */
#define EMPTH_FD_READ   0x1
#define EMPTH_FD_WRITE  0x2

#define EMPTH_PRINT       0x1
#define EMPTH_STACKCHECK  0x2

typedef void empth_t;

typedef void empth_sem_t;

#endif

int empth_init _PROTO((char **ctx, int flags));
empth_t *empth_create _PROTO((int, void (*)(), int,
			int, char *, char *, void *));
empth_t *empth_self();
void empth_exit _PROTO((void));
void empth_yield _PROTO((void));
void empth_terminate _PROTO((empth_t *));
void empth_select _PROTO((int fd, int flags));
void empth_wakeup _PROTO((empth_t *));
void empth_sleep  _PROTO((long until));
empth_sem_t *empth_sem_create _PROTO((char *name, int count));
void empth_sem_signal _PROTO((empth_sem_t *));
void empth_sem_wait _PROTO((empth_sem_t *));
emp_sig_t empth_alarm _PROTO((int));


#include "prototypes.h" /* must come at end, after defines and typedefs */
#endif	







