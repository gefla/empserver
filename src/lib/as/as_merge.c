/*
 *  A* Search - A search library used in Empire to determine paths between
 *              objects.
 *  Copyright (C) 1990-1998 Phil Lapsley
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
 */
#include <stdio.h>
#include <stdlib.h>
#include "as.h"

#if !defined(lint) && !defined(SABER)
static	char	sccsid[] = "@(#)as_merge.c	1.2	11/13/90";
#endif /* not lint */

/*
 * Merge neighbors into queue, keeping it sorted.  "neighbors" is sorted,
 * both by lower bound cost and then by secondary cost.
 */
struct as_queue *
as_merge(struct as_data *adp, struct as_queue *head, struct as_node **neighbors)
{
	struct as_queue	*qp;
	struct as_queue	*pp;		/* previous pointer */
	struct as_queue	*ip;		/* insert pointer */
	struct as_node	*np;
	int		i;

	qp = head;
	pp = NULL;
	for (i = 0; neighbors[i]; i++) {
		np = neighbors[i];
		/* scan until qp points to a node we should go in front of */
		while (qp && (qp->np->lbcost < np->lbcost)) {
			pp = qp;
			qp = qp->next;
		}
		/* check for equal lower bounds, and use secondary cost if = */
		if (qp && qp->np->lbcost == np->lbcost) {
			while (qp && (qp->np->lbcost == np->lbcost) &&
				(qp->np->seccost < np->seccost)) {
				pp = qp;
				qp = qp->next;
			}
		}
		AS_NEW_MALLOC(ip, struct as_queue, NULL);
		/* if there was such a node, insert us in front of it */
		if (qp) {
			ip->prev = qp->prev;
			if (ip->prev)
				ip->prev->next = ip;
			ip->next = qp;
			qp->prev = ip;
			if (qp == head)
				head = ip;
		} else {	/* otherwise add us to end of queue */
		        ip->next = NULL;
			ip->prev = pp;
			if (ip->prev)
				ip->prev->next = ip;
			else {
				head = ip;
			}
			pp = ip;
		}
		ip->np = np;
		as_setcinq(adp, np->c, ip);
		np->step++;
	}

	return (head);
}
