/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#define EMPTH_KILLED  1
typedef struct {
    char *name;			/* thread name */
    char *desc;			/* description */
    void *ud;			/* user data */
    int state;			/* my state */
    void (*ep)(void *);		/* entry point */
    pthread_t id;		/* thread id */
} empth_t;

typedef struct {
    pthread_mutex_t mtx_update;	/* use it to update count */
    int count;
    char name[80];
    pthread_mutex_t mtx_sem;
    pthread_cond_t cnd_sem;
} empth_sem_t;

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

typedef struct loc_Thread_t empth_t;
typedef struct loc_Sem_t empth_sem_t;

void empth_request_shutdown(void);
#endif

int empth_init(char **ctx, int flags);
empth_t *empth_create(int, void (*)(void *), int, int, char *, char *, void *);
empth_t *empth_self(void);
void empth_exit(void);
void empth_yield(void);
void empth_terminate(empth_t *);
void empth_select(int fd, int flags);
void empth_wakeup(empth_t *);
void empth_sleep(long until);
empth_sem_t *empth_sem_create(char *name, int count);
void empth_sem_signal(empth_sem_t *);
void empth_sem_wait(empth_sem_t *);
void empth_alarm(int);

#endif
