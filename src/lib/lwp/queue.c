/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  queue.c: queue manipulation routines
 *
 *  Known contributors to this file:
 *
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
lwpAddTail(struct lwpQueue *q, struct lwpProc *p)
{
    if (!q->tail)
	q->head = p;
    else
	q->tail->next = p;
    q->tail = p;
    p->next = 0;
}
