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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include "as.h"

/*
 * Extend the queue by neighbors.  This entails getting the
 * coordinates of all the neighbors, figuring out their lower bound
 * costs, throwing away ones that are more expensive than ones we
 * already have, sorting, tand then merging into the queue.
 */
struct as_queue *
as_extend(struct as_data *adp)
{
    struct as_queue *qp;
    int i;
    struct as_queue *head;

    head = adp->head;

    /* Find the neighboring coordinates. */
    i = (*adp->neighbor) (head->np->c, adp->neighbor_coords,
			  adp->userdata);
    if (i == 0)
	return NULL;
    /*
     * Get rid of neighbors that are more costly than ones we already have,
     * and sort the rest into an array of as_nodes.
     */
    i = as_winnow(adp, adp->neighbor_coords, i);
    if (i < 0)
	return NULL;
    if (i > 1)
	qsort(adp->neighbor_nodes, i,
	      sizeof(*adp->neighbor_nodes), as_costcomp);

    /* remove old coord from head of queue and add to list of tried */
    qp = head;
    head = head->next;
    if (head)
	head->prev = NULL;
    if (adp->tried) {
	adp->tried->prev = qp;
	qp->next = adp->tried;
	adp->tried = qp;
    } else
	adp->tried = qp;
    adp->tried->np->flags |= AS_TRIED;

    head = as_merge(adp, head, adp->neighbor_nodes);
    return head;
}
