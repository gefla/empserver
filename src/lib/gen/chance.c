/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  chance.c: Roll dice
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include <math.h>
#include <stdlib.h>
#include "chance.h"

/*
 * Return non-zero with probability D.
 */
int
chance(double d)
{
    return d > (random() % 32768) / 32768.0;
}

/*
 * Return a random number in [0..N-1].
 */
int
roll0(int n)
{
    return random() % n;
}

/*
 * Return a random number in [1..N].
 */
int
roll(int n)
{
    return 1 + random() % n;
}

/*
 * Round VAL to nearest integer (on the average).
 * VAL's fractional part is chance to round up.
 */
int
roundavg(double val)
{
    double flr = floor(val);
    return (int)(flr + chance(val - flr));
}

/*
 * Seed the pseudo-random number generator with SEED.
 * The sequence of pseudo-random numbers is repeatable by seeding it
 * with the same value.
 */
void
seed_prng(unsigned seed)
{
    srandom(seed);
}
