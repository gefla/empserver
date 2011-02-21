/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  chance.c: return 1 if "roll" is under the chance.
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include <math.h>
#include "prototypes.h"

int
chance(double d)
{
    return d > (random() % 32768) / 32768.0;
}

int
roll(int n)
{
    return 1 + random() % n;
}

/*
 * round value to nearest int (on the average). E.g. rounds up
 * with a chance proportional to the size of the fractional part.
 */
int
roundavg(double val)
{
    double flr = floor(val);
    return (int)(flr + chance(val - flr));
}
