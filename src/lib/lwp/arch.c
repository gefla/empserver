/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  arch.c: architecture-dependant process context code
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1994-2000
 */

#include <stdlib.h>
#include "lwp.h"
#include "lwpint.h"

#if defined(_EMPTH_LWP)

/*
 * Implement machine-dependent functions lwpNewContext(),
 * lwpSwitchContext().
 */


#if defined UCONTEXT

/*
 * Alternate aproach using setcontext and getcontext instead of setjmp
 * and longjump.  This should work on any SVr4 machine independant of
 * architecture.
 */

int
lwpNewContext(struct lwpProc *newp, int stacksz)
{
    char *s;
    int size, redsize;

    /* Make size a multiple of sizeof(long) to keep things aligned */
    stacksz = (stacksz + sizeof(long) - 1) & -sizeof(long);
    /* Add a red zone on each side of the stack for LWP_STACKCHECK */
    redsize = newp->flags & LWP_STACKCHECK ? LWP_REDZONE : 0;
    size = stacksz + 2 * redsize;

    s = malloc(size);
    if (!s)
	return -1;

    newp->sbtm = s;
    newp->size = size;
    newp->ustack = s + redsize;
    newp->usize = stacksz;

    if (getcontext(&newp->context) < 0) {
	free(s);
	return -1;
    }
#ifdef MAKECONTEXT_SP_HIGH
    /*
     * Known systems that are broken that way: Solaris prior to 10,
     * IRIX.
     */
    newp->context.uc_stack.ss_sp = newp->ustack + stacksz - 8;
#else
    newp->context.uc_stack.ss_sp = newp->ustack;
#endif
    newp->context.uc_stack.ss_size = newp->usize;
    newp->context.uc_stack.ss_flags = 0;
    newp->context.uc_link = NULL;
    makecontext(&newp->context, lwpEntryPoint, 0);
    return 0;
}

void
lwpSwitchContext(struct lwpProc *oldp, struct lwpProc *nextp)
{
    if (!oldp) {
	setcontext(&nextp->context);
	abort();
    } else {
	if (swapcontext(&oldp->context, &nextp->context) < 0)
	    abort();
    }
}

#else  /* !UCONTEXT */

/*
 * If lwpSave() and lwpRestore() are #def'd to setjmp() and longjmp(),
 * then lwpInitContext() needs to set up the jmp_buf for a longjmp(),
 * similar to setjmp().  To figure that out for another machine, check
 * their source or reverse engineer.
 */

#if defined(hpc)

#define STKALIGN 64

static struct lwpProc *tempcontext;
static struct lwpProc *initcontext = NULL;
static int startpoint;

static void
startcontext(void)
{
    int space[10000];
    int x;

    startpoint = (void *)&x;
    if (!setjmp(initcontext->context))
	longjmp(tempcontext->context, 1);

    if (!setjmp(tempcontext->context))
	longjmp(LwpCurrent->context, 1);

    lwpEntryPoint();
}

static void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    struct lwpProc holder;
    int endpoint;

    if (initcontext == NULL) {
	initcontext = malloc(sizeof(struct lwpProc));
	tempcontext = &holder;
	if (!setjmp(tempcontext->context))
	    startcontext();
    }

    tempcontext = newp;
    endpoint = &endpoint;
    if (endpoint < startpoint) {
	if (!setjmp(LwpCurrent->context))
	    longjmp(initcontext->context, 1);
    } else {
	LwpCurrent->size = endpoint - startpoint;
	LwpCurrent->sbtm = realloc(LwpCurrent->sbtm, LwpCurrent->size);
	memcpy(LwpCurrent->sbtm, startpoint, LwpCurrent->size);
	if (!setjmp(LwpCurrent->context))
	    longjmp(initcontext->context, 1);
	memcpy(startpoint, LwpCurrent->sbtm, LwpCurrent->size);
    }
}

static int
lwpSave(jmp_buf jb)
{
    int endpoint;

    endpoint = &endpoint;
    if (initcontext == NULL || endpoint < startpoint)
	return setjmp(jb, 1);

    LwpCurrent->size = endpoint - startpoint;
    LwpCurrent->sbtm = realloc(LwpCurrent->sbtm, LwpCurrent->size);
    memcpy(LwpCurrent->sbtm, startpoint, LwpCurrent->size);
    if (setjmp(jb, 1)) {
	memcpy(startpoint, LwpCurrent->sbtm, LwpCurrent->size);
	return 1;
    }
    return 0;
}

#elif defined(hpux)

#define STKALIGN 64

static void
lwpInitContext(volatile struct lwpProc *volatile newp, void *sp)
{
    static jmp_buf *cpp;

    if (!lwpSave(LwpCurrent->context)) {
	cpp = (jmp_buf *)&newp->context;
	asm volatile ("ldw	%0, %%sp"::"o" (sp));
	if (!lwpSave(*cpp))
	    lwpRestore(LwpCurrent->context);
	lwpEntryPoint();
    }
}

static int
lwpSave(jmp_buf jb)
{
    /* save stack pointer and return program counter */
    asm("stw	%sp, 4(%arg0)");
    asm("stw	%rp, 8(%arg0)");

    /* save "callee save" registers */
    asm("stw	%r3, 12(%arg0)");
    asm("stw	%r4, 16(%arg0)");
    asm("stw	%r5, 20(%arg0)");
    asm("stw	%r6, 24(%arg0)");
    asm("stw	%r7, 28(%arg0)");
    asm("stw	%r8, 32(%arg0)");
    asm("stw	%r9, 36(%arg0)");
    asm("stw	%r10, 40(%arg0)");
    asm("stw	%r11, 44(%arg0)");
    asm("stw	%r12, 48(%arg0)");
    asm("stw	%r13, 52(%arg0)");
    asm("stw	%r14, 56(%arg0)");
    asm("stw	%r15, 60(%arg0)");
    asm("stw	%r16, 64(%arg0)");
    asm("stw	%r17, 68(%arg0)");
    asm("stw	%r18, 72(%arg0)");

    /* save "callee save" space register */
    asm volatile ("mfsp	%sr3, %r1");
    asm("stw	%r1, 0(%arg0)");

    /* indicate "true return" from saved() */
    asm("ldi	0, %ret0");

    asm(".LABEL _comefrom_");
}

static void
lwpRestore(jmp_buf jb)
{
    /* restore stack pointer and program counter */
    asm volatile ("ldw	4(%arg0), %sp");
    asm volatile ("ldw	8(%arg0), %rp");

    /* restore "callee save" space register */
    asm volatile ("ldw	0(%arg0), %r1");
    asm volatile ("mtsp	%r1, %sr3");

    /* restore "callee save" registers */
    asm volatile ("ldw	12(%arg0), %r3");
    asm volatile ("ldw	16(%arg0), %r4");
    asm volatile ("ldw	20(%arg0), %r5");
    asm volatile ("ldw	24(%arg0), %r6");
    asm volatile ("ldw	28(%arg0), %r7");
    asm volatile ("ldw	32(%arg0), %r8");
    asm volatile ("ldw	36(%arg0), %r9");
    asm volatile ("ldw	40(%arg0), %r10");
    asm volatile ("ldw	44(%arg0), %r11");
    asm volatile ("ldw	48(%arg0), %r12");
    asm volatile ("ldw	52(%arg0), %r13");
    asm volatile ("ldw	56(%arg0), %r14");
    asm volatile ("ldw	60(%arg0), %r15");
    asm volatile ("ldw	64(%arg0), %r16");
    asm volatile ("ldw	68(%arg0), %r17");
    asm volatile ("ldw	72(%arg0), %r18");

    /* warp to saved() to unwind the frame correctly */
    asm volatile ("bl	_comefrom_, %r0");
    asm volatile ("ldi	1, %ret0");
}

#elif defined(FBSD)

static void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    setjmp(newp->context);
    newp->context->_jb[2] = (int)sp;
    newp->context->_jb[3] = (int)sp;
    newp->context->_jb[0] = (int)lwpEntryPoint;
}

#elif defined(__linux__)

static void
lwpInitContext(struct lwpProc *newp, void *sp)
{
#if defined(__GLIBC__) && (__GLIBC__ >= 2)
#if defined(__PPC__)
    newp->context->__jmpbuf[JB_GPR1] = (int)sp;
    newp->context->__jmpbuf[JB_LR] = (int)lwpEntryPoint;
#else
    newp->context->__jmpbuf[JB_SP] = (int)sp;
    newp->context->__jmpbuf[JB_BP] = (int)sp;
    newp->context->__jmpbuf[JB_PC] = (int)lwpEntryPoint;
#endif
#else
    newp->context->__sp = sp;
    newp->context->__bp = sp;
    newp->context->__pc = (void *)lwpEntryPoint;
#endif
}

#elif defined(SUN3)

static void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    newp->context[2] = (int)sp;
    newp->context[3] = (int)lwpEntryPoint;
}

#elif defined(SUN4)

static void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    static jmp_buf *cpp;

    memset(newp->context, 0, sizeof(newp->context));
    newp->context[0] = (int)sp;
    /* preserve cpp for new context */
    cpp = (jmp_buf *) & newp->context;
    if (!_setjmp(LwpCurrent->context)) {
	/* create new context */
	/* flush registers */
	asm("ta	0x03");
	/* %o0 <- newp */
	asm("ld	[%fp+0x44], %o0");
	/* %o1 <- newp->context[0] */
	asm("ld	[%o0], %o1");
	/* create min frame on new stack */
	asm("save	%o1,-96, %sp");
	if (!_setjmp(*cpp))
	    _longjmp(LwpCurrent->context, 1);
	lwpEntryPoint();
    }
}

#define	lwpSave(x)	_setjmp(x)
#define lwpRestore(x)	_longjmp(x, 1)

#elif defined(ALPHA)

#include <c_asm.h>

static void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    extern long *_gp;

    /* register values obtained from setjmp.h */
    _setjmp(newp->context);
    newp->context[2] = (long)lwpEntryPoint;	/* program counter */
    newp->context[30] = (long)lwpEntryPoint;	/* return address */
    newp->context[31] = (long)lwpEntryPoint;	/* fake program value (!) */
    newp->context[34] = (long)sp;	/* stack pointer */
}

static int
lwpSave(jmp_buf jb)
{
    return _setjmp(jb);
}

static void
lwpRestore(jmp_buf jb)
{
    /* resume, but get the pv from the jmp_buf */
    asm("ldq	%pv, 248(%a0)");
    asm("stq	%a0, 16(%sp)");
    /* generates a warning, but functions just fine */
    asm("bsr	%ra, __longjump_resume");
}

#elif defined(AIX32)

#define	LWP_EXTRASTACK 12

/* Code is in .s files, as compiler doesn't grok asm */
extern int lwpSave(jmp_buf);
extern void lwpRestore(jmp_buf);
extern void lwpInitContext(struct lwpProc *);

#endif

#ifndef LWP_EXTRASTACK
#define LWP_EXTRASTACK 0
#endif
#ifndef STKALIGN
#define STKALIGN sizeof(double)
#endif
#ifndef lwpSave
#define lwpSave(x)	setjmp(x)
#endif
#ifndef lwpRestore
#define lwpRestore(x)	longjmp(x, 1)
#endif

int
lwpNewContext(struct lwpProc *newp, int stacksz)
{
    char *s, *sp;
    int size, redsize;

    if (CANT_HAPPEN(STKALIGN == 0|| (STKALIGN & (STKALIGN - 1))))
	return -1;		/* STKALIGN not power of 2 */

    /* Make size a multiple of sizeof(long) to keep things aligned */
    stacksz = (stacksz + sizeof(long) - 1) & -sizeof(long);
    /* Add a red zone on each side of the stack for LWP_STACKCHECK */
    redsize = newp->flags & LWP_STACKCHECK ? LWP_REDZONE : 0;
    size = stacksz + 2 * redsize + LWP_EXTRASTACK + STKALIGN - 1;

    s = malloc(size);
    if (!s)
	return -1;

    if (LwpStackGrowsDown) {
	/*
	 * Stack layout for stack growing downward:
	 *     ptr        block      size
	 *     --------------------------------------
	 *                red zone   LWP_REDZONE
	 *     sp      -> extra      LWP_EXTRASTACK
	 *     ustack  -> stack      stacksz
	 *                red zone   LWP_REDZONE
	 *                waste      STKALIGN - 1 - x
	 * sp is aligned to a multiple of STKALIGN.
	 */
	sp = s + redsize + stacksz;
	sp = (char *)0 + (((sp + STKALIGN - 1) - (char *)0) & -STKALIGN);
	newp->ustack = sp - stacksz;
    } else {
	/*
	 * Stack layout for stack growing upward:
	 *     ptr        block      size
	 *     --------------------------------------
	 *                waste      x
	 *     		  red zone   LWP_REDZONE
	 *     sp      -> stack      stacksz
	 *     ustack  -> extra      LWP_EXTRASTACK
	 *     		  red zone   LWP_REDZONE
	 *                waste      STKALIGN - 1 - x
	 * sp is aligned to a multiple of STKALIGN.
	 */
	sp = s + redsize + LWP_EXTRASTACK;
	sp = (char *)0 + (((sp + STKALIGN - 1) - (char *)0) & -STKALIGN);
	newp->ustack = sp - LWP_EXTRASTACK;
    }
    newp->sbtm = s;
    newp->size = size;
    newp->usize = stacksz + LWP_EXTRASTACK;
    lwpInitContext(newp, sp);
    return 0;
}

void
lwpSwitchContext(struct lwpProc *oldp, struct lwpProc *nextp)
{
    if (!(oldp && lwpSave(oldp->context)))
	lwpRestore(nextp->context);
}

#endif /* !UCONTEXT */

#endif
