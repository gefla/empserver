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
 *  queue.c: implementation of various queuing routines
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#include <stdlib.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "misc.h"
#include "queue.h"

void
insque(struct qelem *p, struct qelem *q)
{
    p->q_forw = q->q_forw;
    p->q_back = q;
    q->q_forw->q_back = p;
    q->q_forw = p;
}

void
remque(struct qelem *p)
{
    p->q_back->q_forw = p->q_forw;
    p->q_forw->q_back = p->q_back;
}

void
initque(struct qelem *p)
{
    p->q_forw = p;
    p->q_back = p;
}

struct qelem *
makeqt(int nelem)
{
    struct qelem *table;
    struct qelem *qp;
    int i;

    table = malloc(sizeof(*table) * nelem);
    for (i = 0, qp = table; i < nelem; i++, qp++)
	initque(qp);
    return table;
}
