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
 *  queue.c: Various queue routines (for lists)
 * 
 *  Known contributors to this file:
 *     
 */

#include "queue.h"

void
emp_insque(struct emp_qelem *elem, struct emp_qelem *queue)
{
    struct emp_qelem *next;

    next = queue->q_forw;
    queue->q_forw = elem;
    elem->q_forw = next;
    elem->q_back = queue;
    next->q_back = elem;
}

void
emp_remque(struct emp_qelem *elem)
{
    if (elem == (struct emp_qelem *)0)
	return;
    if (elem->q_forw != (struct emp_qelem *)0)
	elem->q_forw->q_back = elem->q_back;
    if (elem->q_back != (struct emp_qelem *)0)
	elem->q_back->q_forw = elem->q_forw;
}

void
emp_initque(struct emp_qelem *elem)
{
    elem->q_forw = elem;
    elem->q_back = elem;
}
