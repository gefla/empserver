/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *  Copyright (C) 1991-3 Stephen Crane
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
 *  arch.c: architecture-dependant process context code
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1994-2000
 *     Markus Armbruster, 2004-2008
 */

#include <config.h>

#include <stdlib.h>
#include "lwpint.h"

/*
 * Historically, this was implemented by abusing setjmp() and
 * longjump(), which required a fair amount of system-dependent and
 * fragile hackery.  We now use POSIX ucontext.h.
 */

int
lwpNewContext(struct lwpProc *newp)
{
    if (getcontext(&newp->context) < 0)
	return -1;
#ifdef MAKECONTEXT_SP_HIGH
    /*
     * Known systems that are broken that way: Solaris prior to 10,
     * IRIX.
     */
    newp->context.uc_stack.ss_sp = newp->ustack + newp->usize - 8;
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
