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
 *  ntthread.c: Interface from Empire threads to Windows NT threads
 * 
 *  Known contributors to this file:
 *     Doug Hay, 1998
 *     Steve McClure, 1998
 */

/*
 * EMPTHREADs for Windows NT.
 *
 * Actually, threads for any Win32 platform,
 * like Win95, Win98, WinCE, and whatever other
 * toy OSs are in our future from Microsoft.
 *
 * WIN32 has a full pre-emptive threading environment.
 * But, Empire can not handle pre-emptive threading.
 * Thus, we will use the threads, but limit the preemption
 * using a Mutex semaphore.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include "misc.h"
#include "empthread.h"
#include "prototypes.h"

#if defined(_WIN32) && defined(_EMPTH_WIN32)
#include <winsock2.h>
#undef NS_ALL
#include <windows.h>
#include <process.h>


#define loc_MIN_THREAD_STACK  16384

/************************
 * loc_Thread_t
 *
 * The REAL empth_t thread structure.
 * The external world only gets
 * a void pointer to this.
 */
typedef struct loc_Thread_t {

    /* The thread name, passed in at create time. */
    char szName[17];
    /* The thread description, passed in at create time. */
    char szDesc[80];

    /* True if this is the main line, and not a real thread. */
    BOOL bMainThread;

    /* The user data passed in at create time. */
    void *pvUserData;

    /* True if this thread has been killed. */
    BOOL bKilled;

    /* The entry function for the thread. */
    void (*pfnEntry) (void *);

    /* The system thread ID. */
    unsigned long ulThreadID;

    /* An Event sem that the thread will wait/sleep on. */
    HANDLE hThreadEvent;
} loc_Thread_t;


/************************
 * loc_Sem_t
 *
 * The REAL empth_sem_t structure.
 * The external world only gets
 * a void pointer to this.
 */
typedef struct empth_sem_t {

    char szName[17];

    /* An exclusion semaphore for this sem. */
    HANDLE hMutex;
    /* An Event sem that the thread(s) will sleep on. */

    HANDLE hEvent;
    /* The count variable */
    int count;
} loc_Sem_t;

static struct {
    /* This is the thread exclusion/non-premption mutex. */
    /* The running thread has this MUTEX, and all others are */
    /* either blocked on it, or waiting for some OS response. */
    HANDLE hThreadMutex;

    /* This is the thread startup event sem. */
    /* We use this to lockstep when we are starting up threads. */
    HANDLE hThreadStartEvent;

    /* The Thread Local Storage index.  We store the pThread pointer */
    /* for each thread at this index. */
    DWORD dwTLSIndex;

    /* The current running thread. */
    loc_Thread_t *pCurThread;

    /* Ticks at start */
    unsigned long ulTickAtStart;

    /* Pointer out to global context.  "player". */
    /* From empth_init parameter. */
    char **ppvUserData;

    /* Global flags.  From empth_init parameter. */
    int flags;
} loc_GVAR;


/************************
 * loc_debug
 *
 * Print out the current thread's status??
 */
static void
loc_debug(const char *pszFmt, ...)
{
    va_list vaList;
    unsigned long ulCurTick;
    unsigned long ulRunTick;
    unsigned long ulMs, ulSec, ulMin, ulHr;
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);
    char buf[1024];

    if ((loc_GVAR.flags & EMPTH_PRINT) != 0) {

	/* Ticks are in milliseconds */
	ulCurTick = GetTickCount();

	ulRunTick = ulCurTick - loc_GVAR.ulTickAtStart;
	ulMs = ulRunTick % 1000L;
	ulSec = (ulRunTick / 1000L) % 60L;
	ulMin = (ulRunTick / (60L * 1000L)) % 60L;
	ulHr = (ulRunTick / (60L * 60L * 1000L));

	va_start(vaList, pszFmt);
	vsprintf(buf, pszFmt, vaList);
	va_end(vaList);

	if (pThread) {
	    printf("%ld:%02ld:%02ld.%03ld %17s: %s\n",
		   ulHr, ulMin, ulSec, ulMs, pThread->szName, buf);
	} else {
	    printf("%ld:%02ld:%02ld.%03ld %17s: %s\n",
		   ulHr, ulMin, ulSec, ulMs, "UNKNOWN", buf);
	}

    }
}

/************************
 * loc_FreeThreadInfo
 *
 */
static void
loc_FreeThreadInfo(loc_Thread_t *pThread)
{
    if (pThread) {
	if (pThread->hThreadEvent)
	    CloseHandle(pThread->hThreadEvent);
	memset(pThread, 0, sizeof(*pThread));
	free(pThread);
    }
}

/************************
 * loc_RunThisThread
 *
 * This thread wants to run.
 * When this function returns, the
 * globals are set to this thread info,
 * and the thread owns the MUTEX sem.
 */
static void
loc_RunThisThread()
{
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);

    if (pThread->bKilled) {
	if (!pThread->bMainThread) {
	    TlsSetValue(loc_GVAR.dwTLSIndex, NULL);
	    loc_FreeThreadInfo(pThread);
	    _endthread();
	}
    }

    /* Get the MUTEX semaphore, wait forever. */
    WaitForSingleObject(loc_GVAR.hThreadMutex, INFINITE);

    if (!loc_GVAR.pCurThread) {
	/* Set the globals to this thread. */
	*loc_GVAR.ppvUserData = pThread->pvUserData;

	loc_GVAR.pCurThread = pThread;
    } else {
	/* Hmm, a problem, eh? */
	logerror("RunThisThread, someone already running.");
    }
}

/************************
 * loc_BlockThisThread
 *
 * This thread was running.  It no longer wants to.
 */
static void
loc_BlockThisThread()
{
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);

    if (loc_GVAR.pCurThread == pThread) {
	/* Reset the globals back to original */

	loc_GVAR.pCurThread = NULL;
	*loc_GVAR.ppvUserData = NULL;

	/* Release the MUTEX */
	ReleaseMutex(loc_GVAR.hThreadMutex);
    } else {
	/* Hmm, this thread was not the running one. */
	logerror("BlockThisThread, not running.");
    }
}


/************************
 * loc_SleepThisThread
 */
static void
loc_SleepThisThread(unsigned long ulMillisecs)
{
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);

    /* Make sure the event thread is clean. */
    ResetEvent(pThread->hThreadEvent);

    /* Get the MUTEX semaphore, wait the number of MS */
    WaitForSingleObject(pThread->hThreadEvent, ulMillisecs);
}


/************************
 * empth_threadMain
 *
 * This is the main line of each thread.
 * This is really a static local func....
 */
void
empth_threadMain(void *pvData)
{
    time_t now;

    loc_Thread_t *pThread = (loc_Thread_t *)pvData;

    /* Out of here... */
    if (!pvData)
	return;

    /* Store pThread on this thread. */
    TlsSetValue(loc_GVAR.dwTLSIndex, pvData);

    /* Get the ID of the thread. */
    pThread->ulThreadID = GetCurrentThreadId();

    /* Signal that the thread has started. */
    SetEvent(loc_GVAR.hThreadStartEvent);

    /* seed the rand() function */
    time(&now);
    srand(now ^ (unsigned int)pThread);

    /* Switch to this thread context */
    loc_RunThisThread();

    /* Run the thread. */
    if (pThread->pfnEntry)
	pThread->pfnEntry(pThread->pvUserData);

    /* Kill the thread. */
    empth_exit();
}

/************************
 * empth_init
 *
 * Initialize the thread environment.
 *
 * This is called from the program
 * main line.
 */
int
empth_init(char **ctx_ptr, int flags)
{
    loc_Thread_t *pThread = NULL;

    loc_GVAR.ulTickAtStart = GetTickCount();
    loc_GVAR.ppvUserData = ctx_ptr;
    loc_GVAR.flags = flags;
    loc_GVAR.dwTLSIndex = TlsAlloc();

    /* Create the thread mutex sem. */
    /* Initally unowned. */
    loc_GVAR.hThreadMutex = CreateMutex(NULL, FALSE, NULL);
    if (!loc_GVAR.hThreadMutex) {
	logerror("Failed to create mutex");
	return 0;
    }

    /* Create the thread start event sem. */
    /* Automatic state reset. */
    loc_GVAR.hThreadStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!loc_GVAR.hThreadStartEvent) {
	logerror("Failed to create mutex");
	return 0;
    }

    /* Create the global Thread context. */
    pThread = (loc_Thread_t *)malloc(sizeof(*pThread));
    if (!pThread) {
	logerror("not enough memory to create main thread.");
	return 0;
    }
    memset(pThread, 0, sizeof(*pThread));

    strncpy(pThread->szName, "Main", sizeof(pThread->szName) - 1);
    strncpy(pThread->szDesc, "The main process",
	    sizeof(pThread->szDesc) - 1);
    pThread->ulThreadID = GetCurrentThreadId();
    pThread->bMainThread = TRUE;

    TlsSetValue(loc_GVAR.dwTLSIndex, pThread);

    /* Make this the running thread. */
    loc_RunThisThread();

    logerror("NT pthreads initialized");
    return 0;
}


/************************
 * empth_create
 *
 * Create a new thread.
 *
 * prio  - priority, not particularly useful in our context.
 * entry - entry point function for thread.
 * size  - stack size.
 * flags - debug control.
 *           LWP_STACKCHECK  - not needed
 * name  - name of the thread, for debug.
 * desc  - description of thread, for debug.
 * ud    - "user data".  The "ctx_ptr" gets this value
 *         when the thread is active.
 *         It is also passed to the entry function...
 */
empth_t *
empth_create(int prio, void (*entry)(void *), int size, int flags,
	     char *name, char *desc, void *ud)
{
    loc_Thread_t *pThread = NULL;

    loc_debug("creating new thread %s:%s", name, desc);

    pThread = (loc_Thread_t *)malloc(sizeof(*pThread));
    if (!pThread) {
	logerror("not enough memory to create thread: %s (%s)", name,
		 desc);
	return NULL;
    }
    memset(pThread, 0, sizeof(*pThread));

    strncpy(pThread->szName, name, sizeof(pThread->szName) - 1);
    strncpy(pThread->szDesc, desc, sizeof(pThread->szDesc) - 1);
    pThread->pvUserData = ud;
    pThread->pfnEntry = entry;
    pThread->bMainThread = FALSE;

    /* Create thread event sem, auto reset. */
    pThread->hThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (size < loc_MIN_THREAD_STACK)
	size = loc_MIN_THREAD_STACK;

    pThread->ulThreadID = _beginthread(empth_threadMain, size,
				       (void *)pThread);
    if (pThread->ulThreadID == -1) {
	logerror("can not create thread: %s (%s): %s", name, desc,
		 strerror(errno));
	goto bad;
    }

    loc_debug("new thread id is %ld", pThread->ulThreadID);
    return pThread;

  bad:
    if (pThread) {
	loc_FreeThreadInfo(pThread);
    }
    return NULL;
}


/************************
 * empth_self
 */
empth_t *
empth_self(void)
{
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);

    return pThread;
}

/************************
 * empth_exit
 */
void
empth_exit(void)
{
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);

    loc_BlockThisThread();

    loc_debug("empth_exit");

    if (pThread->bMainThread) {
	/* The main line.  Wait forever. */
	while (1) {
	    if (!debug) {
		if (service_stopped())
		    shutdwn(0);
		Sleep(3);
	    } else {
		char buf[20];
		printf("\nEmpire Server>");
		fgets(buf, sizeof(buf), stdin);
		if (!strnicmp(buf, "quit", 4))
		    shutdwn(0);
	    }
	}
    } else {
	TlsSetValue(loc_GVAR.dwTLSIndex, NULL);
	loc_FreeThreadInfo(pThread);
	_endthread();
    }
}

/************************
 * empth_yield
 *
 * Yield processing to another thread.
 */
void
empth_yield(void)
{
    loc_BlockThisThread();
    loc_RunThisThread();
}

/************************
 * empth_terminate
 *
 * Kill off the thread.
 */
void
empth_terminate(empth_t *a)
{
    loc_Thread_t *pThread = (loc_Thread_t *)a;

    loc_debug("killing thread %s", pThread->szName);
    pThread->bKilled = TRUE;

    SetEvent(pThread->hThreadEvent);
}

/************************
 * empth_select
 *
 * Do a select on the given file.
 * Wait for IO on it.
 *
 * This would be one of the main functions used within
 * gen\io.c
 */
void
empth_select(int fd, int flags)
{
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);
    fd_set readmask;
    fd_set writemask;
    struct lwpProc *proc;
    struct timeval tv;
    int n;

    loc_debug("%s select on %d",
	      flags == EMPTH_FD_READ ? "read" : "write", fd);
    loc_BlockThisThread();

    while (1) {
	tv.tv_sec = 1000000;
	tv.tv_usec = 0;

	FD_ZERO(&readmask);
	FD_ZERO(&writemask);

	switch (flags) {
	case EMPTH_FD_READ:
	    FD_SET(fd, &readmask);
	    break;
	case EMPTH_FD_WRITE:
	    FD_SET(fd, &writemask);
	    break;
	default:
	    logerror("bad flag %d passed to empth_select", flags);
	    empth_exit();
	}

	n = select(fd + 1, &readmask, &writemask, (fd_set *) 0, &tv);

	if (n < 0) {
	    if (errno == EINTR) {
		/* go handle the signal */
		loc_debug("select broken by signal");
		goto done;
		return;
	    }
	    /* strange but we dont get EINTR on select broken by signal */
	    loc_debug("select failed (%s)", strerror(errno));
	    goto done;
	    return;
	}

	if (flags == EMPTH_FD_READ && FD_ISSET(fd, &readmask)) {
	    loc_debug("input ready");
	    break;
	}
	if (flags == EMPTH_FD_WRITE && FD_ISSET(fd, &writemask)) {
	    loc_debug("output ready");
	    break;
	}
    }

  done:
    loc_RunThisThread();
}

/************************
 * empth_alarm
 */
void
empth_alarm(int sig)
{
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);

    loc_debug("got alarm signal  %d", sig);

    /* Let it run if it is blocked like... */
    SetEvent(pThread->hThreadEvent);
}

/************************
 * empth_wakeup
 *
 * Wake up the specified thread.
 */
void
empth_wakeup(empth_t *a)
{
    loc_Thread_t *pThread = (loc_Thread_t *)a;

    loc_debug("waking up thread %s", pThread->szName);

    /* Let it run if it is blocked... */
    SetEvent(pThread->hThreadEvent);
}

/************************
 * empth_sleep
 *
 * Put the given thread to sleep...
 */
void
empth_sleep(long until)
{
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);
    unsigned long ulSec;

    ulSec = until - time(0);

    loc_debug("going to sleep %ld sec", ulSec);

    loc_BlockThisThread();

    WaitForSingleObject(pThread->hThreadEvent, (ulSec * 1000));

    loc_debug("sleep done. Waiting to run.");

    loc_RunThisThread();
}


/************************
 * empth_sem_create
 *
 * Create a signalling semaphore.
 */
empth_sem_t *
empth_sem_create(char *name, int cnt)
{
    loc_Sem_t *pSem;

    pSem = (loc_Sem_t *)malloc(sizeof(*pSem));
    if (!pSem) {
	logerror("out of memory at %s:%d", __FILE__, __LINE__);
	return NULL;
    }

    memset(pSem, 0, sizeof(pSem));
    strncpy(pSem->szName, name, sizeof(pSem->szName) - 1);

    pSem->hMutex = CreateMutex(NULL, FALSE, NULL);
    pSem->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    pSem->count = cnt;

    return pSem;
}

/************************
 * empth_sem_signal
 *
 * Hit/signal the specified semaphore.
 */
void
empth_sem_signal(empth_sem_t *sm)
{
    loc_Sem_t *pSem = (loc_Sem_t *)sm;

    loc_debug("signal on semaphore %s:%d", pSem->szName, pSem->count);

    /* Wait for the Semaphore */
    WaitForSingleObject(pSem->hMutex, INFINITE);

    if (pSem->count++ < 0) {
	SetEvent(pSem->hEvent);
    }

    ReleaseMutex(pSem->hMutex);
}

/************************
 * empth_sem_wait
 *
 * Wait for the specified signal semaphore
 * to be signaled.
 */
void
empth_sem_wait(empth_sem_t *sm)
{
    loc_Thread_t *pThread =
	(loc_Thread_t *)TlsGetValue(loc_GVAR.dwTLSIndex);
    loc_Sem_t *pSem = (loc_Sem_t *)sm;

    loc_debug("wait on semaphore %s:%d", pSem->szName, pSem->count);

    /* Remove the thread from the running state. */
    loc_BlockThisThread();

    /* Wait for the Semaphore */
    WaitForSingleObject(pSem->hMutex, INFINITE);
    if (--pSem->count < 0) {
	loc_debug("blocking");
	ReleaseMutex(pSem->hMutex);

	WaitForSingleObject(pSem->hEvent, INFINITE);

	loc_debug("waking up");
    } else
	ReleaseMutex(pSem->hMutex);

    loc_RunThisThread();
}

#endif /* _WIN32 */
