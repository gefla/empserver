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
 *  arch.c: architecture-dependant process context code
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1994-2000
 */

#include "prototypes.h"

#if defined(_EMPTH_LWP)

#if (!defined(AIX32))

#include "lwp.h"

#include "lwpint.h"

#if defined(hpc)

static struct lwpProc *tempcontext;
struct lwpProc *initcontext = NULL;
int startpoint;

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

void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    struct lwpProc holder;
    int endpoint;

    if (initcontext == NULL) {
	initcontext = (struct lwpProc *)malloc(sizeof(struct lwpProc));
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

#elif defined(hpux)

void
lwpInitContext(volatile struct lwpProc *volatile newp, void *sp)
{
    static jmp_buf *cpp;

    if (!lwpSave(LwpCurrent->context)) {
	cpp = (jmp_buf *) & newp->context;
	asm volatile ("ldw	%0, %%sp"::"o" (sp));
	if (!lwpSave(*cpp))
	    lwpRestore(LwpCurrent->context);
	lwpEntryPoint();
    }
}

int
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

void
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

#elif defined(BSD386)
void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    newp->context[2] = (int)sp;
    newp->context[0] = (int)lwpEntryPoint;
}

#elif defined(FBSD)

void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    setjmp(newp->context);
    newp->context->_jb[2] = (int)sp;
    newp->context->_jb[3] = (int)sp;
    newp->context->_jb[0] = (int)lwpEntryPoint;
}

#elif defined(__linux__)

void
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

void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    newp->context[2] = (int)sp;
    newp->context[3] = (int)lwpEntryPoint;
}

#elif defined(__vax)

#include <stdio.h>

void
lwpInitContext(struct lwpProc *newp, void *stack)
{
    int *sp = (int *)stack;
    int *fp = 0;

    /* Build root frame on new stack for lwpEntryPoint */
    *--sp = 0;			/* pc */
    *--sp = (int)fp;		/* fp */
    *--sp = 0;			/* ap */
    *--sp = 0;			/* psw  */
    *--sp = 0;			/* condition handler */
    fp = sp;

    /* Build stack frame to return from. */
    *--sp = (int)lwpEntryPoint + 2;	/* pc */
    *--sp = (int)fp;		/* fp */
    *--sp = 0;			/* ap */
    *--sp = 0;			/* psw  */
    *--sp = 0;			/* condition handler */
    fp = sp;

    /* Fill in the context */
    /* Note: This is *not* how libc fills out jump buffers. */
    newp->context[0] = 0;	/* r6 */
    newp->context[1] = 0;
    newp->context[2] = 0;
    newp->context[3] = 0;
    newp->context[4] = 0;
    newp->context[5] = 0;	/* r11 */
    newp->context[6] = 0;	/* ap */
    newp->context[7] = (int)fp;	/* fp */
    return;
}

int
lwpSave(jmp_buf jb)
{
    asm("movl 4(ap), r0");	/* r0 = &jb */
    asm("movl r6, (r0)");	/* jb[0] = r6 */
    asm("movl r7, 4(r0)");
    asm("movl r8, 8(r0)");
    asm("movl r9, 12(r0)");
    asm("movl r10, 16(r0)");
    asm("movl r11, 20(r0)");
    asm("movl ap, 24(r0)");
    asm("movl fp, 28(r0)");	/* jb[7] = fp */
    return 0;
}

void
lwpRestore(jmp_buf jb)
{
    asm("movl 4(ap), r0");	/* r0 = &jb */
    asm("movl (r0), r6");	/* r6 = jb[0] */
    asm("movl 4(r0), r7");
    asm("movl 8(r0), r8");
    asm("movl 12(r0), r9");
    asm("movl 16(r0), r10");
    asm("movl 20(r0), r11");
    asm("movl 24(r0), ap");
    asm("movl 28(r0), fp");	/* fp = jb[7] */
    asm("movl $1, r0");		/* faked return 1 from lwpSave() */
    asm("ret");
    return;
}


#elif defined(SUN4)

void
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

#elif defined(__USLC__) && defined(i386)

/* USL/Unixware on an Intel 386/486/... processor.
 * Tested on Unixware v1.1.2, based on SYSV R4.2
 */

/* As per normal empire documentation, there is none.
 *
 * But, what we are attempting to do here is set up a longjump
 * context buffer so that the lwpEntryPoint is called when
 * the thread starts.
 *
 * I.E., what a setjmp/longjmp call set would do.
 *
 * How to figure this out?  Well, without the setjmp code, you
 * need to reverse engineer it by printing out the context buffer
 * and the processor registers, and mapping which ones need
 * to be set.
 *
 * Alternatively, you can single instruction step through the longjmp
 * function, and figure out the offsets that it uses.
 *
 * Using offsets in bytes,
 * context + 0x04 [1] -> esi  (general purpose reg)
 * context + 0x08 [2] -> edi  (general purpose reg)
 * context + 0x0C [3] -> ebp  (general purpose or parameter passing)
 * context + 0x10 [4] -> esp  (stack)
 * context + 0x14 [5] -> jump location for return
 */

void
lwpInitContext(struct lwpProc *newp, void *sp)
{
    newp->context[4] = (int)sp;
    newp->context[5] = (int)lwpEntryPoint;
}

#elif defined UCONTEXT

/*
 * Alternate aproach using setcontext en getcontext in stead of setjmp and
 * longjump. This should work on any SVr4 machine independant of
 * architecture. Unfortunaltely some changes are still nessesary in lwp.c.
 * Tested on IRIX 5.3
 */

void
lwpInitContext(struct lwpProc *newp, stack_t *spp)
{
    getcontext(&(newp->context));
    newp->context.uc_stack.ss_sp = spp->ss_sp;
    newp->context.uc_stack.ss_size = spp->ss_size;
    makecontext(&(newp->context), lwpEntryPoint, 0);
}

#elif defined(ALPHA)

#include <c_asm.h>

void
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

int
lwpSave(jmp_buf jb)
{
    return _setjmp(jb);
}

void
lwpRestore(jmp_buf jb)
{
    /* resume, but get the pv from the jmp_buf */
    asm("ldq	%pv, 248(%a0)");
    asm("stq	%a0, 16(%sp)");
    /* generates a warning, but functions just fine */
    asm("bsr	%ra, __longjump_resume");
}

#endif

#endif

#endif
