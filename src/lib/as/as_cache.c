/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  as_cache.c: Routines used to create/delete caches of A* paths.
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include "as.h"
#include "optlist.h"

/* The way this works is interesting. :) */
/* We keep a pointer to a list of pointers.  The index into this list
 * is the y coordinate of the from sector.  This member points to a list
 * of from sectors on that y coordinate.  So, we march that list checking
 * the x value to find the from x,y we want. */
/* Once we find the from x,y, that node has the same type of pointer to
 * a list of pointers.  The index into this list is the y coordinate of
 * the to sector.  This member points to a list of to sectors on that y
 * coordinate.  So, we march that list checking the x value to find the
 * to x,y we want. */
/* These lists are dynamically created since the world is dynamically sized. */
/* See, I told you it was interesting. :) */

static struct as_frompath **fromhead = (struct as_frompath **)0;

/* Note that we only want to cache during updates.  Other times, it
 * probably doesn't make much sense, but can be done. */

static int as_cachepath_on = 0;	/* Default to off */

void
as_enable_cachepath(void)
{
    as_cachepath_on = 1;
}

void
as_disable_cachepath(void)
{
    as_cachepath_on = 0;
}

/* Note we want these to be as fast as possible */

void
as_add_cachepath(struct as_data *adp)
{
    struct as_frompath *from;
    struct as_topath *to = (struct as_topath *)0;
    struct as_node *np;

    /* Don't do anything if we aren't cacheing these */
    if (as_cachepath_on == 0)
	return;

    /* Note we will only allocate this once.  Afterwards, we just keep
     * zeroing it since it's rather small and we don't need to re-allocate
     * each time. */
    if (fromhead == NULL) {
	fromhead = calloc(1, sizeof(struct as_frompath *) * WORLD_Y);
	if (fromhead == NULL)
	    return;
    }

    np = adp->head->np;
    for (from = fromhead[adp->from.y]; from; from = from->next)
	if (from->x == adp->from.x)
	    break;
    if (from) {
	for (to = from->tolist[np->c.y]; to; to = to->next) {
	    if (to->x == np->c.x) {
		/* It is already here!  Don't bother adding it again */
		return;
	    }
	}
    } else {
	/* We must make a new one of these */
	from = malloc(sizeof(struct as_frompath));
	if (from == NULL)
	    return;
	/* And set some stuff */
	from->x = adp->from.x;
	/* Here we malloc a whole bunch of tolist pointers. */
	from->tolist = calloc(1, sizeof(struct as_topath *) * WORLD_Y);
	/* Now, add from to the global list */
	from->next = fromhead[adp->from.y];
	fromhead[adp->from.y] = from;
    }
    if (!to) {
	/* We must make a new one */
	to = malloc(sizeof(struct as_topath));
	/* We can't, sorry */
	if (to == NULL)
	    return;
	/* Now set some stuff */
	to->x = np->c.x;
	/* Now add it to the list we are in */
	to->next = from->tolist[np->c.y];
	from->tolist[np->c.y] = to;
    }
    /* Now, make the path */
    as_makepath(adp);
    /* Now, take the path */
    to->path = adp->path;
    /* And clear the path in the adp */
    adp->path = NULL;
}

void
as_clear_cachepath(void)
{
    struct as_frompath *from, *from2;
    struct as_topath *to, *to2;
    int i, j;

    /* Cache not used yet :) */
    if (fromhead == NULL)
	return;

    for (j = 0; j < WORLD_Y; j++) {
	for (from = fromhead[j]; from; from = from2) {
	    for (i = 0; i < WORLD_Y; i++) {
		for (to = from->tolist[i]; to; to = to2) {
		    to2 = to->next;
		    /* Free this path */
		    as_free_path(to->path);
		    /* Free this node */
		    free(to);
		}
	    }
	    /* Now, free the list of lists */
	    free(from->tolist);
	    /* Save the next pointer */
	    from2 = from->next;
	    /* now, free this from node */
	    free(from);
	}
    }
    /* Note we don't free the fromhead here, we just zero it.  That way,
       we can use it next time without mallocing int */
    memset(fromhead, 0, (sizeof(struct as_frompath *) * WORLD_Y));
}

struct as_path *
as_find_cachepath(coord fx, coord fy, coord tx, coord ty)
{
    struct as_frompath *from;
    struct as_topath *to;

    /* Is the cache on?  if not, return NULL */
    if (as_cachepath_on == 0)
	return NULL;

    /* Do we have any cached? */
    if (fromhead == NULL)
	return NULL;

    /* Yes! */
    for (from = fromhead[fy]; from; from = from->next) {
	if (from->x == fx) {
	    for (to = from->tolist[ty]; to; to = to->next) {
		if (to->x == tx)
		    return to->path;
	    }
	}
    }
    return NULL;
}
