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
static char sccsid[] = "@(#)as_init.c	1.4	11/13/90";
#endif /* not lint */

/*
 * Return an as_data structure with the necessary fields filled in
 * and space malloced.  Return NULL if malloc fails.
 */
struct as_data *
as_init(int maxneighbors,
	int hashsize,
	int (*hashfunc) (struct as_coord),
	int (*neighborfunc) (struct as_coord, struct as_coord *, s_char *),
	double (*lbcostfunc) (struct as_coord, struct as_coord, s_char *),
	double (*realcostfunc) (struct as_coord, struct as_coord,
				s_char *),
	double (*seccostfunc) (struct as_coord, struct as_coord, s_char *),
	s_char *userdata)
{
    struct as_data *adp;

    AS_NEW(adp, struct as_data, NULL);
    AS_NEW_ARRAY(adp->neighbor_coords, struct as_coord,
		 maxneighbors, NULL);
    AS_NEW_ARRAY(adp->neighbor_nodes, struct as_node *,
		 maxneighbors + 1, NULL);
    AS_NEW_ARRAY(adp->hashtab, struct as_hash *, hashsize, NULL);

    adp->maxneighbors = maxneighbors;
    adp->hashsize = hashsize;
    adp->hash = hashfunc;
    adp->neighbor = neighborfunc;
    adp->lbcost = lbcostfunc;
    adp->realcost = realcostfunc;
    adp->seccost = seccostfunc;
    adp->userdata = userdata;

    return (adp);
}
