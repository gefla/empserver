/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mapdist.c: Return the distance between two sectors
 * 
 *  Known contributors to this file:
 *     
 */

/*
 * mapdist returns (integer) distance between two sectors.
 * mapdsq returns the square of the distance -- more efficient.
 */

#include "misc.h"
#include "gen.h"
#include "optlist.h"

int
diffx(int x1, int x2)
{
	int	dx;

	dx = x1 - x2;
	dx = dx % WORLD_X;
	if (dx > WORLD_X/2)
		dx = dx - WORLD_X;
	if (dx < -WORLD_X/2)
		dx = dx + WORLD_X;
	return dx; 
}

int
diffy(int y1, int y2)
{
	int	dy;

	dy = y1 - y2;
	dy = dy % WORLD_Y;
	if (dy > WORLD_Y/2)
		dy = dy - WORLD_Y;
	if (dy < -WORLD_Y/2)
		dy = dy + WORLD_Y;
	return dy; 
}

int
deltax(int x1, int x2)
{
	int	dx;

	dx = abs(x1 - x2);
	dx = dx % WORLD_X;
	if (dx > WORLD_X/2)
		dx = WORLD_X - dx;
	return dx; 
}

int
deltay(int y1, int y2)
{
	int	dy;

	dy = abs(y1 - y2);
	dy = dy % WORLD_Y;
	if (dy > WORLD_Y/2)
		dy = WORLD_Y - dy;
	return dy; 
}

int
mapdist(int x1, int y1, int x2, int y2)
{
	int	dx, dy;

	x1 = x1 % WORLD_X;
	y1 = y1 % WORLD_Y;
	x2 = x2 % WORLD_X;
	y2 = y2 % WORLD_Y;
	dx = deltax(x1, x2);
	dy = deltay(y1, y2);
	if (dx > dy)
		return (dx - dy) / 2 + dy;
	return dy;
}

int
mapdsq(int x1, int y1, int x2, int y2)
{
	int	sq;

	sq = mapdist(x1, y1, x2, y2);
	return sq * sq;
}
