/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2006-2012
 */

#include <config.h>

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "chance.h"
#include "mt19937ar.h"

/*
 * Return non-zero with probability D.
 */
int
chance(double d)
{
    return d > genrand_real2();
}

/*
 * Return non-zero with probability PCT%.
 */
int
pct_chance(int pct)
{
    return roll(100) <= pct;
}

static unsigned
round_up_to_pow2(unsigned val)
{
    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;
    return val;
}

/*
 * Return a random number in [0..N-1].
 * N must be in [1..2^31-1].
 */
int
roll0(int n)
{
    unsigned pow2 = round_up_to_pow2(n);
    int r;

    do
	r = genrand_int32() & (pow2 - 1);
    while (r >= n);
    return r;
}

/*
 * Return a random number in [1..N].
 * N must be in [0..2^31-1].
 */
int
roll(int n)
{
    return 1 + roll0(n);
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
    init_genrand(seed);
}

static uint32_t
djb_hash(uint32_t hash, void *buf, size_t sz)
{
    unsigned char *bp;

    for (bp = buf; bp < (unsigned char *)buf + sz; bp++)
	hash = hash * 33 ^ *bp;

    return hash;
}

/*
 * Pick a reasonably random seed for the pseudo-random number generator.
 */
unsigned
pick_seed(void)
{
    int fd;
    uint32_t seed;
    int got_seed = 0;
    struct timeval tv;
    pid_t pid;

    /*
     * Modern systems provide random number devices, but the details
     * vary.  On many systems, /dev/random blocks when the kernel
     * entropy pool has been depleted, while /dev/urandom doesn't.
     * The former should only be used for generating long-lived
     * cryptographic keys.  On other systems, both devices behave
     * exactly the same, or only /dev/random exists.
     *
     * Try /dev/urandom first, and if it can't be opened, blindly try
     * /dev/random.
     */
    fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (fd < 0)
	fd = open("/dev/random", O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
	got_seed = read(fd, &seed, sizeof(seed)) == sizeof(seed);
	close(fd);
    }

    if (!got_seed) {
	/* Kernel didn't provide, fall back to hashing time and PID */
	gettimeofday(&tv, NULL);
	seed = djb_hash(5381, &tv, sizeof(tv));
	pid = getpid();
	seed = djb_hash(seed, &pid, sizeof(pid));
    }

    return seed;
}
