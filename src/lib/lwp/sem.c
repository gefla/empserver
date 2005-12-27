/*
 * lwpSem.c -- lwpSemaphore manipulation.
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

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "lwp.h"
#include "lwpint.h"

#if defined(_EMPTH_LWP)

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
#endif
