/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  lwpSem.c: lwpSemaphore manipulation
 * 
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "lwp.h"
#include "lwpint.h"

/*
 * create a lwpSemaphore.
 */
struct lwpSem *
lwpCreateSem(char *name, int count)
{
    struct lwpSem *new;

    if (!(new = malloc(sizeof(struct lwpSem))))
	return 0;
    new->name = strdup(name);
    new->count = count;
    new->q.head = new->q.tail = 0;
    return new;
}

/*
 * signal a lwpSemaphore.  We only yield here if
 * the blocked process has a higher priority than ours'.
 */
void
lwpSignal(struct lwpSem *s)
{
    lwpStatus(LwpCurrent, "done with semaphore %s", s->name);
    if (s->count++ < 0) {
	struct lwpProc *p = lwpGetFirst(&s->q);
	lwpStatus(LwpCurrent, "activating first waiter");
	lwpReady(p);
	if (LwpCurrent->pri < p->pri) {
	    lwpStatus(p, "priority is higher");
	    lwpYield();
	}
    }
}

/*
 * wait on a lwpSemaphore
 */
void
lwpWait(struct lwpSem *s)
{
    lwpStatus(LwpCurrent, "checking semaphore %s", s->name);
    if (--s->count < 0) {
	lwpStatus(LwpCurrent, "blocking");
	lwpAddTail(&s->q, LwpCurrent);
	lwpReschedule();
    }
}
