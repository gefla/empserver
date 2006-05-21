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

static void as_free_queue(struct as_queue *queue);

/*
 * Free any dynamically allocated data stored in the as_data structure.
 */
void
as_reset(struct as_data *adp)
{
    as_free_queue(adp->head);
    adp->head = NULL;
    as_free_queue(adp->tried);
    adp->tried = NULL;
    as_free_queue(adp->subsumed);
    adp->subsumed = NULL;
    as_free_hashtab(adp);
    as_free_path(adp->path);
    adp->path = NULL;
}

/*
 * Free a queue (either the main, subsumed, or tried).
 */
static void
as_free_queue(struct as_queue *queue)
{
    struct as_queue *qp, *qp2;

    for (qp = queue; qp; qp = qp2) {
	free(qp->np);
	qp2 = qp->next;
	free(qp);
    }
}

/*
 * Free a path.
 */
void
as_free_path(struct as_path *pp)
{
    struct as_path *pp2;

    for (; pp; pp = pp2) {
	pp2 = pp->next;
	free(pp);
    }
}

/*
 * Delete the as_data structure (which includes freeing its data).
 */
void
as_delete(struct as_data *adp)
{
    as_reset(adp);
    free(adp->neighbor_coords);
    free(adp->neighbor_nodes);
    free(adp->hashtab);
    free(adp);
}
