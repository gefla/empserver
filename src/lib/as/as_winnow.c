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

static struct as_node *as_newnode(struct as_node *backp, struct as_coord c,
				  double inclbcost, double lbcost,
				  double knowncost, double seccost);

/*
 * Take a list of neighbor coordinates and winnow them down into
 * an interesting list of neighbor nodes.  This means:
 *
 *	For each neighbor,
 *		Compute a lower bound on the total cost to target.
 *		If this neighbor is already in our queue,
 *			See if the new neighbor is cheaper.
 *			If so, add it to queue and move the
 *			old node to the subsumed list.
 *			If not, ignore this neighbor.
 *		If this neighbor isn't in the queue, add it.
 *
 */
int
as_winnow(struct as_data *adp, struct as_coord *coords, int ncoords)
{
    int i = 0;
    int fix_pointer;
    double knowncost;
    double incknowncost;
    double lbcost;
    double inclbcost;
    double seccost;
    struct as_coord *cp;
    struct as_coord *end;
    struct as_queue *qp;
    struct as_node *np;

    for (cp = coords, end = coords + ncoords; cp < end; cp++) {
	fix_pointer = 0;
	incknowncost = adp->realcost(adp->head->np->c, *cp, adp->userdata);
	knowncost = adp->head->np->knowncost + incknowncost;
	/*
	 * If this neighbor is already in the queue, we can
	 * save some time.
	 */
	qp = as_iscinq(adp, *cp);
	inclbcost = qp ? qp->np->inclbcost :
	    adp->lbcost(*cp, adp->to, adp->userdata);
	if (inclbcost < 0.0)	/* skip bad cases */
	    continue;
	lbcost = knowncost + inclbcost;
#ifdef DEBUG
	fprintf(stderr, "\tneighbor %d, %d, lbcost %f ", cp->x, cp->y,
		lbcost);
#endif /* DEBUG */
	/*
	 * If this neighbor is already in the queue, check to
	 * see which has the lower cost.  If the one already in
	 * the queue is cheaper, skip this neighbor as bad.  If
	 * the neighbor does, delete the one in the queue.
	 */
	if (qp) {
	    if (qp->np->lbcost <= lbcost) {
#ifdef DEBUG
		fprintf(stderr, "old, loses to %f\n", qp->np->lbcost);
#endif /* DEBUG */
		continue;
	    } else {
#ifdef DEBUG
		fprintf(stderr, "old, wins over %f\n", qp->np->lbcost);
#endif /* DEBUG */
		if (qp->np->flags & AS_TRIED) {
		    /* should "never happen" */
		    return 0;
		}
		/*
		 * The neighbor is better than a previously visited coordinate;
		 * remove the old coordinate from the queue and add it to
		 * the subsumed nodes queue.  To get here at
		 * all we can't be the head, thus qp->prev is defined.
		 */
		/* Delete from main queue */
		qp->prev->next = qp->next;
		if (qp->next)
		    qp->next->prev = qp->prev;

		/* Add to subsumed queue */
		if (adp->subsumed) {
		    adp->subsumed->prev = qp;
		    qp->next = adp->subsumed;
		} else {
		    qp->next = NULL;
		}
		adp->subsumed = qp;
		adp->subsumed->prev = NULL;
		fix_pointer = 1;
		/*
		 * At this point, the as_iscinq code may contain bogus pointer
		 * refs.  They'll be fixed when as_merge merges the new
		 * neighbors into the main queue.
		 */
	    }
	}
#ifdef DEBUG
	else {
	    fprintf(stderr, "new\n");
	}
#endif /* DEBUG */

	if (qp)
	    seccost = qp->np->seccost;
	else
	    seccost = adp->seccost ?
		adp->seccost(*cp, adp->to, adp->userdata) : 0.0;
	np = as_newnode(adp->head->np, *cp, inclbcost, lbcost,
			knowncost, seccost);
	if (np == NULL)
	    return 0;
	if (fix_pointer) {
#ifdef DEBUG
	    fprintf(stderr, "Fixing pointer for %d, %d\n",
		    adp->subsumed->np->c.x, adp->subsumed->np->c.y);
#endif
	    adp->subsumed->np->back = np;
	}
	adp->neighbor_nodes[i++] = np;

    }
    adp->neighbor_nodes[i] = NULL;

    return i;
}


static struct as_node *
as_newnode(struct as_node *backp, struct as_coord c,
	   double inclbcost, double lbcost, double knowncost,
	   double seccost)
{
    struct as_node *np;

    /* Got an interesting coordinate; make a node for it. */
    AS_NEW_MALLOC(np, struct as_node, NULL);
    np->flags = 0;
    np->c = c;
    np->inclbcost = inclbcost;
    np->lbcost = lbcost;
    np->knowncost = knowncost;
    np->seccost = seccost;
    np->step = backp->step;
    np->back = backp;

    return np;
}
