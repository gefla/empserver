/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  minmax.c: Misc min and max routines
 * 
 *  Known contributors to this file:
 *      Doug Hay, 1998
 *      Steve McClure, 1998
 */

#include "gen.h"

double
dmax(double n1, double n2)
{
    if (n1 > n2)
	return n1;
    return n2;
}

double
dmin(double n1, double n2)
{
    if (n1 < n2)
	return n1;
    return n2;
}

#if !defined(_WIN32)
int
max(int n1, int n2)
{
    if (n1 > n2)
	return n1;
    return n2;
}


int
min(int n1, int n2)
{
    if (n1 < n2)
	return n1;
    return n2;
}

#endif
