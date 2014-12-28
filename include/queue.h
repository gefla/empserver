/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
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
 *  queue.h: Generic vax-like doubly linked list queues
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2014
 */

#ifndef QUEUE_H
#define QUEUE_H

#define QEMPTY(p)	((p)->q_forw == (p))

struct emp_qelem {
    struct emp_qelem *q_forw;
    struct emp_qelem *q_back;
};

extern void emp_remque(struct emp_qelem *);
extern void emp_initque(struct emp_qelem *);
extern int emp_quelen(struct emp_qelem *);
extern struct emp_qelem *emp_searchque(struct emp_qelem *, void *,
				int (*)(struct emp_qelem *, void *));
extern void emp_insque(struct emp_qelem *, struct emp_qelem *);

#endif
