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
/*
 * 11/09/98 - Steve McClure
 *  Added path list caching structures
 */
#include <stdio.h>
#include <stdlib.h>
#include "as.h"

/*
 * Basic A* search function.  "adp" should have been initialized by
 * as_init (any previously allocated data will be freed by as_reset here),
 * and adp->from and adp->to should be set accordingly.  On success,
 * returns 0, with adp->path set to a linked list of coordinates to target.
 * If we can't find target, return -1; if malloc fails, return -2.
 */
int
as_search(struct as_data *adp)
{
    int iter = 0;
    struct as_queue *head;
    struct as_node *np;
#ifdef DEBUG
    int i;
    struct as_queue *qp;
    struct as_path *pp;
#endif /* DEBUG */

    as_reset(adp);

    /*
     * Jump start the queue by making first element the zero-cost
     * node where we start.
     */
    AS_NEW_MALLOC(head, struct as_queue, -2);
    adp->head = head;
    head->next = head->prev = NULL;
    AS_NEW(np, struct as_node, -2);
    np->c = adp->from;
    head->np = np;
    as_setcinq(adp, head->np->c, adp->head);

    for (;;) {
	iter++;
#ifdef DEBUG
	fprintf(stderr, "Iteration %d, head at %d, %d\n", iter,
		head->np->c.x, head->np->c.y);
#endif /* DEBUG */

	/* see if we're done, one way or another */
	if (head == NULL)
	    break;

	/* Add it to the cache */
	as_add_cachepath(adp);

	if (head->np->c.x == adp->to.x && head->np->c.y == adp->to.y)
	    break;

	/* extend queue by neighbors */
#ifdef DEBUG
	fprintf(stderr, "\tExtending queue\n");
#endif /* DEBUG */
	adp->head = head = as_extend(adp);

#ifdef DEBUG
	fprintf(stderr, "queue:\n");
	i = 0;
	for (qp = head; qp; qp = qp->next) {
	    fprintf(stderr, "\t%d, %d so far %f lb %f sec %f\n",
		    qp->np->c.x, qp->np->c.y,
		    qp->np->knowncost, qp->np->lbcost, qp->np->seccost);
	    i++;
	}
	fprintf(stderr, "\tqueue len %d\n", i);
#endif /* DEBUG */

    }

    if (head == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Failed\n");
#endif /* DEBUG */
	return (-1);
    }

    as_makepath(adp);

#ifdef DEBUG
    fprintf(stderr, "Succeeded, iter %d, cost %f!\n", iter,
	    head->np->knowncost);
    fprintf(stderr, "Path:\n");
    for (pp = adp->path; pp; pp = pp->next) {
	fprintf(stderr, "\t%d, %d\n", pp->c.x, pp->c.y);
    }
    fprintf(stderr, "Tried queue:\n");
    for (qp = adp->tried; qp; qp = qp->next) {
	fprintf(stderr, "\t%d, %d\n", qp->np->c.x, qp->np->c.y);
    }
    fprintf(stderr, "Subsumed queue:\n");
    for (qp = adp->subsumed; qp; qp = qp->next) {
	fprintf(stderr, "\t%d, %d\n", qp->np->c.x, qp->np->c.y);
    }
#endif /* DEBUG */

    return (0);
}

/*
 * Work backwards through the list of nodes (starting at head)
 * to produce a path.
 */
void
as_makepath(struct as_data *adp)
{
    struct as_path *pp;
    struct as_node *np;

    for (np = adp->head->np; np; np = np->back) {
	pp = (struct as_path *)malloc(sizeof(struct as_path));
	pp->c = np->c;
	pp->next = adp->path;
	adp->path = pp;
    }
}
