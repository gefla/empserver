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
#include "as.h"

/*
 * Compare the lower bound costs of two nodes.  If the two nodes have
 * equal lower bound costs, sort on the secondary field.
 * Used as comparision function for qsort.
 */
int
as_costcomp(struct as_node **n1, struct as_node **n2)
{
    double diff;

    diff = (*n1)->lbcost - (*n2)->lbcost;
    if (diff < -0.0001)
	return (-1);
    if (diff > 0.0001)
	return (1);

    /* equal, check secondary cost */
    diff = (*n1)->seccost - (*n2)->seccost;
    if (diff < -0.0001)
	return (-1);
    if (diff > 0.0001)
	return (1);
    return (0);
}
