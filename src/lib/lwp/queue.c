/*
 * queue.c -- queue manipulation routines.
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

#include "lwp.h"
#include "lwpint.h"

struct lwpProc *
lwpGetFirst(struct lwpQueue *q)
{
    struct lwpProc *head;

    if ((head = q->head) && !(q->head = head->next))
	q->tail = 0;
    return head;
}

void
lwpAddTail(register struct lwpQueue *q, register struct lwpProc *p)
{
    if (!q->tail)
	q->head = p;
    else
	q->tail->next = p;
    q->tail = p;
    p->next = 0;
}
