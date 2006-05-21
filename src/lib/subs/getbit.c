/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  getbit.c: Replaces old bitmap code
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "nsc.h"
#include "sect.h"
#include "nat.h"
#include "prototypes.h"
#include "optlist.h"



/*
 *
 * the bit offsets for each bit pattern based on the efficiency of
 * the sector.
 *
 * bitmap0:  0-20%
 * bitmap1: 21-40%
 * bitmap2: 41-60%
 * bitmap3: 61-80%
 * bitmap4: 81-100%
 */

#define bitoff(x, y) x, y

static int bitmap0[] = {
    bitoff(-1, -1), bitoff(1, -1),
    bitoff(-2, 0), bitoff(0, 0), bitoff(2, 0),
    bitoff(-1, 1), bitoff(1, 1),
    bitoff(9999, 9999),
};

static int bitmap1[] = {
    bitoff(0, -2),
    bitoff(-3, -1), bitoff(-1, -1), bitoff(1, -1), bitoff(3, -1),
    bitoff(-2, 0), bitoff(0, 0), bitoff(2, 0),
    bitoff(-3, 1), bitoff(-1, 1), bitoff(1, 1), bitoff(3, 1),
    bitoff(0, 2),
    bitoff(9999, 9999),
};

static int bitmap2[] = {
    bitoff(-2, -2), bitoff(0, -2), bitoff(2, -2),
    bitoff(-3, -1), bitoff(-1, -1), bitoff(1, -1), bitoff(3, -1),
    bitoff(-4, 0), bitoff(-2, 0), bitoff(0, 0), bitoff(2, 0), bitoff(4, 0),
    bitoff(-3, 1), bitoff(-1, 1), bitoff(1, 1), bitoff(3, 1),
    bitoff(-2, 2), bitoff(0, 2), bitoff(2, 2),
    bitoff(9999, 9999),
};

static int bitmap3[] = {
    bitoff(-1, -3), bitoff(1, -3),
    bitoff(-4, -2), bitoff(-2, -2), bitoff(0, -2), bitoff(2, -2), bitoff(4,
									 -2),
    bitoff(-5, -1), bitoff(-3, -1), bitoff(-1, -1), bitoff(1, -1),
    bitoff(3, -1), bitoff(5, -1),
    bitoff(-4, 0), bitoff(-2, 0), bitoff(0, 0), bitoff(2, 0), bitoff(4, 0),
    bitoff(-5, 1), bitoff(-3, 1), bitoff(-1, 1), bitoff(1, 1),
    bitoff(3, 1), bitoff(5, 1),
    bitoff(-4, 2), bitoff(-2, 2), bitoff(0, 2), bitoff(2, 2), bitoff(4, 2),
    bitoff(-1, 3), bitoff(1, 3),
    bitoff(9999, 9999),
};

static int bitmap4[] = {
    bitoff(-3, -3), bitoff(-1, -3), bitoff(1, -3), bitoff(3, -3),
    bitoff(-4, -2), bitoff(-2, -2), bitoff(0, -2), bitoff(2, -2), bitoff(4,
									 -2),
    bitoff(-5, -1), bitoff(-3, -1), bitoff(-1, -1), bitoff(1, -1),
    bitoff(3, -1), bitoff(5, -1),
    bitoff(-6, 0), bitoff(-4, 0), bitoff(-2, 0), bitoff(0, 0), bitoff(2,
								      0),
    bitoff(4, 0), bitoff(6, 0),
    bitoff(-5, 1), bitoff(-3, 1), bitoff(-1, 1), bitoff(1, 1),
    bitoff(3, 1), bitoff(5, 1),
    bitoff(-4, 2), bitoff(-2, 2), bitoff(0, 2), bitoff(2, 2), bitoff(4, 2),
    bitoff(-3, 3), bitoff(-1, 3), bitoff(1, 3), bitoff(3, 3),
    bitoff(9999, 9999),
};

static int *bitmaps[5] = {
    bitmap0,
    bitmap1,
    bitmap2,
    bitmap3,
    bitmap4,
};

#define GCFx(x) ((x + WORLD_X) % WORLD_X)
#define GCFy(y) ((y + WORLD_Y) % WORLD_Y)

int
emp_getbit(int x, int y, unsigned char *bitmap)
{
    int id;

    id = (GCFy(y)) * WORLD_X / 2 + GCFx(x) / 2;
    return bitmap[id / 8] & bit(id & 07);
}

void
emp_setbit(int x, int y, unsigned char *bitmap)
{
    int id;

    id = (GCFy(y)) * WORLD_X / 2 + GCFx(x) / 2;
    bitmap[id / 8] |= bit(id & 07);
}

static void
emp_setbitmap(int x, int y, unsigned char *bitmap, int *bitmaps)
{
    int *mp;
    int id;
    int dx, dy;

    for (mp = bitmaps; *mp != 9999;) {
	dx = x + *mp++;
	dy = y + *mp++;
	id = (GCFy(dy)) * WORLD_X / 2 + GCFx(dx) / 2;
	bitmap[id / 8] |= bit(id & 07);
    }
}

void
bitinit2(struct nstr_sect *np, unsigned char *bitmap, int country)
{
    struct sctstr sect;
    int eff;

    while (nxtsct(np, &sect)) {
	if (sect.sct_own != country)
	    continue;
	eff = sect.sct_effic / 20;
	if (eff > 4)
	    eff = 4;
	emp_setbitmap(np->x, np->y, bitmap, bitmaps[eff]);
    }
    snxtsct_rewind(np);
}
