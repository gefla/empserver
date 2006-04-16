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
 * Return a pointer to the as_queue structure associated with
 * this coordinate if the coordinate is in the queue.
 */
struct as_queue *
as_iscinq(struct as_data *adp, struct as_coord c)
{
    int hashval;
    struct as_hash *hp;

    hashval = adp->hash(c) % adp->hashsize;

    for (hp = adp->hashtab[hashval]; hp; hp = hp->next)
	if (hp->c.x == c.x && hp->c.y == c.y)
	    return hp->qp;

    return NULL;
}

/*
 * Set the queue structure associated with this coordinate.
 */
void
as_setcinq(struct as_data *adp, struct as_coord c, struct as_queue *qp)
{
    int hashval;
    struct as_hash *hp;
    struct as_hash *new;

    new = (struct as_hash *)malloc(sizeof(struct as_hash));
    new->c = c;
    new->qp = qp;

    hashval = adp->hash(c) % adp->hashsize;
    hp = adp->hashtab[hashval];

    new->next = (hp) ? hp : NULL;
    adp->hashtab[hashval] = new;
}

/*
 * Walk down the hash table array, freeing the chains and zeroing
 * the chain pointers.
 */
void
as_free_hashtab(struct as_data *adp)
{
    int i;
    struct as_hash *hp, *hp2;

    for (i = 0; i < adp->hashsize; i++) {
	for (hp = adp->hashtab[i]; hp; hp = hp2) {
	    hp2 = hp->next;
	    free(hp);
	}
	adp->hashtab[i] = NULL;
    }
}
