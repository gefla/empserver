/*
 * lwpint.h -- lwp internal structures
 *
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
#ifndef _LWPINT_H
#define _LWPINT_H

/* `liveness' counter: check signals every `n' visits to the scheduler */
/* note: the lower this value, the more responsive the system but the */
/* more inefficient the context switch time */
#define LCOUNT	-1

#ifdef hpc
extern struct lwpProc *initcontext;
extern int startpoint;
#endif

#ifdef hpux
int lwpSave(jmp_buf);
void lwpRestore(jmp_buf);
#endif

#if defined(MIPS) || defined(AIX32) || defined(ALPHA) || defined(__vax)
int lwpSave(jmp_buf);
void lwpRestore(jmp_buf);
#elif defined(SUN4)
#define	lwpSave(x)	_setjmp(x)
#define lwpRestore(x)	_longjmp(x, 1)
#elif defined (UCONTEXT)
#define lwpSave(x)    getcontext(&(x))
#define lwpRestore(x) setcontext(&(x))
#else
#ifdef hpc
#define	lwpSave(x)	setjmp(x)
#define lwpRestore(x)	longjmp(x, 1)
#else
#ifndef hpux
#define	lwpSave(x)	setjmp(x)
#define lwpRestore(x)	longjmp(x, 1)
#endif /* hpux */
#endif /* hpc */
#endif

#ifdef AIX32
/* AIX needs 12 extra bytes above the stack; we add it here */
#define	LWP_EXTRASTACK	3*sizeof(long)
#else
#define LWP_EXTRASTACK	0
#endif

#define LWP_REDZONE	1024	/* make this a multiple of 1024 */

/* XXX Note that this assumes sizeof(long) == 4 */
#define LWP_CHECKMARK	0x5a5a5a5aL

#define SIGNALS	sigmask(SIGALRM)

#ifndef hpux
typedef double stkalign_t;
#else
typedef struct {
    char x[64];
} stkalign_t;
#endif

/* internal routines */
void lwpAddTail(struct lwpQueue *, struct lwpProc *);
struct lwpProc *lwpGetFirst(struct lwpQueue *);
void lwpReschedule(void);
void lwpReady(struct lwpProc *);
void lwpOnalarm(void);

#ifdef UCONTEXT
void lwpInitContext(struct lwpProc *, stack_t *);
#else  /* GETCONTEXT */
#ifdef hpc
void lwpInitContext(struct lwpProc *, void *);
#else
#ifdef hpux
void lwpInitContext(volatile struct lwpProc * volatile, void *);
#else
void lwpInitContext(struct lwpProc *, void *);
#endif /* hpux */
#endif /* hpc */
#endif /* GETCONTEXT */
void lwpEntryPoint(void);
void lwpInitSelect(struct lwpProc * self);
void lwpDestroy(struct lwpProc * proc);

#endif /* _LWP_H */
