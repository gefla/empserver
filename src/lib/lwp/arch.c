/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *  Copyright (C) 1991-3 Stephen Crane
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
 *  arch.c: architecture-dependant process context code
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1994-2000
 *     Markus Armbruster, 2004-2007
 */

#include <config.h>

#include <stdlib.h>
#include "lwp.h"
#include "lwpint.h"

/*
 * Historically, this was implemented by abusing setjmp() and
 * longjump(), which required a fair amount of system-dependent and
 * fragile hackery.  We now use POSIX ucontext.h.
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
