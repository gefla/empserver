/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ship.c: Ship characteristics
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Jeff Bailey
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

/*
 * marine characteristics -- ship capabilities descriptions
 * 
 * Note, that you should not set the flags M_XLIGHT or M_CHOPPER
 * here.  They will be automatically set by init_global() when the server
 * starts if the ship can carry a non-zero amount of those things.
 * Also note that if nplanes > 0 and M_FLY is not set, then M_MSL will
 * automatically be set.
 */

#include <config.h>

#include "misc.h"
#include "ship.h"

struct mchrstr mchr[] = {
/*          a   s   v  v   f       E  K   f  f       		    	      */
/*          r   p   i  r   r   g   |  |   u  u       		    	      */
/* l   h    m   e   s  n   n   l   x  V   e  e       		    	      */
/* c   c    o   e   i  g   g   i   p  p   l  l       		    	      */
/* m   m    r   d   b  e   e   m   l  l   c  u  name                tech  Cost*/

    {{300,  10,   0,   0,   0,   0,   0,   0, 900,   0,   0,   0,  15,   0},
     25, 15, 10, 10, 15, 2, 0, 0, 0, 0, 0, 0, "fb   fishing boat",
     0, 180, M_FOOD | M_CANAL, 0, 0},

    {{300,  10,   0,   0,   0,   0,   0,   0, 900,   0,   0,   0,  15,   0},
     25, 15, 10, 25, 15, 2, 0, 0, 0, 0, 20, 1, "ft   fishing trawler",
     35, 300, M_FOOD | M_CANAL, 0, 0},

    {{600,  50, 300,  50,   0,   0,   0,   0, 900,   0,1400, 900, 250,   0},
     60, 40, 20, 25, 35, 3, 0, 0, 1, 0, 0, 0, "cs   cargo ship",
     20, 500, M_SUPPLY, 0, 2},

    {{30,   5,   0,   0,   0, 990, 990,   0, 200,   0,   0,   0,  45, 990},
     60, 40, 20, 25, 35, 3, 0, 0, 1, 0, 30, 1, "os   ore ship",
     20, 500, 0, 0, 0},

    {{20,  80,   0,   0,   0,   0,   0,   0, 200,   0,   0,   0,1200,   0},
     60, 40, 20, 10, 35, 3, 0, 0, 1, 0, 0, 0, "ss   slave ship",
     0, 300, 0, 0, 0},

    {{50,  50,   0,   0,   0,   0,   0,   0, 100,   0,   0,   0,   0,   0},
     200, 100, 20, 25, 35, 3, 0, 0, 1, 0, 30, 1, "ts   trade ship",
     30, 1750, M_TRADE, 0, 0},

    {{0,  60,  10,   2,   0,   0,   0,   0,  60,   0,   0,   0,   0,   0},
     30, 30, 50, 25, 25, 3, 1, 1, 1, 0, 0, 0, "frg  frigate",
     0, 600, M_SEMILAND, 0, 2},

    {{10,   5,   0,   0,   0,   0,   0,   0, 100,   1,   0,   0,   0,   0},
     25, 15, 10, 25, 15, 2, 0, 0, 0, 0, 20, 1, "oe   oil exploration boat",
     40, 800, M_OIL | M_CANAL, 0, 0},

    {{990,  80,   0,   0,   0,   0,   0,   0, 990, 990,   0,   0, 990,   0},
     60, 60, 30, 15, 65, 3, 0, 0, 2, 0, 0, 0, "od   oil derrick",
     50, 1500, M_OIL, 0, 0},

    {{0,   2,  12,   2,   0,   0,   0,   0,   5,   0,   0,   0,   0,   0},
     20, 10, 10, 38, 10, 2, 1, 1, 0, 0, 4, 1, "pt   patrol boat",
     40, 300, M_TORP | M_CANAL, 0, 0},

    {{0, 100,  40,   5,   0,   0,   0,   0, 100,   0,   0,   0,   0,   0},
     30, 40, 50, 30, 30, 5, 6, 3, 1, 0, 40, 1, "lc   light cruiser",
     45, 800, M_MINE, 0, 2 },

    {{0, 120, 100,   8,   0,   0,   0,   0, 200,   0,   0,   0,   0,   0},
     40, 50, 70, 30, 30, 5, 8, 4, 1, 0, 30, 1, "hc   heavy cruiser",
     50, 1200, 0, 0, 4},

    {{0, 120,  20,   4,   0,   0,   0,   0, 120,   0,   0,   0,   0,   0},
     50, 50, 60, 20, 35, 3, 1, 2, 1, 0, 0, 0, "tt   troop transport",
     10, 800, M_SEMILAND, 0, 2 },

    {{0, 200, 200,  10,   0,   0,   0,   0, 900,   0,   0,   0,   0,   0},
     50, 70, 95, 25, 35, 6, 10, 7, 1, 0, 50, 2, "bb   battleship",
     45, 1800, 0, 0, 2},

    {{0, 180, 100,  10,   0,   0,   0,   0, 400,   0,   0,   0,   0,   0},
     50, 60, 55, 30, 35, 6, 10, 6, 1, 0, 60, 2, "bbc  battlecruiser",
     75, 1500, 0, 0, 2},

    {{30,   5,   0,   0, 990,   0,   0,   0, 200, 990,   0,   0,  25,   0},
     60, 40, 75, 25, 45, 3, 0, 0, 1, 0, 30, 1, "tk   tanker",
     35, 600, M_OILER | M_SUPPLY, 0, 0},

    {{0,  10, 100,   1,   0,   0,   0,   0,  90,   0,   0,   0,   0,   0},
     25, 15, 10, 25, 15, 2, 0, 0, 0, 0, 20, 1, "ms   minesweeper",
     40, 400, M_MINE | M_SWEEP | M_CANAL, 0, 0},

    {{0,  60,  40,   4,   0,   0,   0,   0,  80,   0,   0,   0,   0,   0},
     30, 30, 45, 35, 20, 4, 6, 3, 1, 0, 30, 1, "dd   destroyer",
     70, 600, M_MINE | M_DCH | M_SONAR, 0, 1},

    {{0,  25,  36,   5,   0,   0,   0,   0,  80,   0,   0,   0,   0,   0},
     30, 30, 25, 20, 5, 4, 3, 3, 0, 0, 30, 1, "sb   submarine",
     60, 650, M_TORP | M_SUB | M_MINE | M_SONAR, 0, 0},

    {{5,  10, 104,  20, 100,   0,   0,   0, 900,   0, 500, 300,   0,   0},
     40, 40, 50, 30, 2, 3, 0, 0, 0, 0, 50, 2, "sbc  cargo submarine",
     150, 1200, M_SUPPLY | M_OILER | M_SUB | M_SONAR, 0, 0},

    {{0, 175, 250,   4, 300,   0,   0,   0, 180,   0,   0,   0,   0,   0},
     50, 60, 60, 30, 40, 5, 2, 2, 4, 20, 50, 2, "cal  light carrier",
     80, 2700, M_FLY, 20, 0},

    {{0, 350, 500,   4, 500,   0,   0,   0, 900,   0,   0,   0,   0,   0},
     60, 70, 80, 35, 40, 7, 2, 2, 10, 40, 120, 3, "car  aircraft carrier",
     160, 4500, M_FLY, 40, 0},

    {{0, 350, 999,   4, 999,   0,   0,   0, 900,   0,   0,   0,   0,   0},
     70, 80, 100, 45, 40, 9, 2, 2, 20, 4, 0, 0, "can  nuc carrier",
     305, 8000, M_OILER | M_FLY | M_SUPPLY, 60, 0},

    {{0, 400,  10,   1,   0,   0,   0,   0, 300,   0,   0,   0,   0,   0},
     60, 40, 40, 30, 30, 2, 0, 0, 2, 0, 30, 1, "ls   landing ship",
     145, 1000, M_LAND, 0, 6},

    {{0,  60,  60,   4,   0,   0,   0,   0, 120,   0,   0,   0,   0,   0},
     40, 30, 50, 35, 30, 5, 2, 2, 4, 0, 40, 1, "af   asw frigate",
     220, 800, M_TORP | M_SUBT | M_DCH | M_SONAR, 0, 0},

    {{0,  25,  60,   6,   0,   0,   0,   0, 500,   0,   0,   0,   0,   0},
     30, 40, 45, 40, 3, 6, 5, 3, 0, 0, 0, 0, "na   nuc attack sub",
     260, 1200, M_TORP | M_SUB | M_MINE | M_SONAR | M_SUBT, 0, 0},

    {{0, 100,  80,   6,  40,   0,   0,   0, 500,   0,   0,   0,   0,   0},
     40, 40, 60, 40, 35, 6, 8, 3, 10, 2, 80, 2, "ad   asw destroyer",
     240, 1500, M_TORP | M_SUBT | M_DCH | M_SONAR, 0, 0},

    {{0,  25, 200,   1,   0,   0,   0,   0, 500,   0,   0,   0,   0,   0},
     30, 40, 55, 35, 2, 6, 0, 0, 0, 0, 0, 0, "nm   nuc miss sub",
     270, 1500, M_SUB | M_SONAR, 20, 0},

    {{0,  25, 100,   1,   0,   0,   0,   0, 500,   0,   0,   0,   0,   0},
     30, 30, 35, 30, 3, 3, 0, 0, 0, 0, 30, 1, "msb  missile sub",
     230, 1200, M_SUB | M_SONAR, 10, 0},

    {{0,   5, 100,   3,   0,   0,   0,   0, 500,   0,   0,   0,   0,   0},
     20, 20, 15, 40, 15, 3, 2, 2, 0, 0, 7, 1, "mb   missile boat",
     180, 500, 0, 10, 0},

    {{0,  60, 220,   4,   0,   0,   0,   0, 120,   0,   0,   0,   0,   0},
     40, 30, 50, 35, 30, 5, 2, 2, 2, 0, 30, 1, "mf   missile frigate",
     280, 1000, 0, 20, 0},

    {{0, 120, 500,   6, 160,   0,   0,   0, 200,   0,   0,   0,   0,   0},
     50, 50, 70, 35, 35, 8, 8, 6, 8, 8, 35, 1, "mc   missile cruiser",
     290, 1500, M_ANTIMISSILE, 40, 0},

    {{0, 100, 100,  15,   0,   0,   0,   0, 200,   0,   0,   0,   0,   0},
     50, 60, 80, 35, 30, 6, 1, 8, 1, 0, 60, 2, "aac  aa cruiser",
     130, 1500, M_ANTIMISSILE, 0, 4},

    {{0, 200, 400,  25,  40,   0,   0,   0, 900,   0,   0,   0,   0,   0},
     50, 60, 80, 35, 30, 6, 1, 16, 30, 2, 0, 0, "agc  aegis cruiser",
     265, 4000, M_ANTIMISSILE, 32, 0},

    {{0, 200, 400,   8,  40,   0,   0,   0, 900,   0,   0,   0,   0,   0},
     50, 50, 100, 45, 35, 6, 14, 7, 10, 2, 0, 0, "ncr  nuc cruiser",
     325, 1800, M_ANTIMISSILE, 20, 0},

    {{0, 200, 120,   6, 160,   0,   0,   0, 500,   0,   0,   0,   0,   0},
     50, 50, 80, 45, 35, 9, 10, 4, 25, 8, 0, 0, "nas  nuc asw cruiser",
     330, 1800, M_TORP | M_SUBT | M_DCH | M_SONAR, 0, 0},

/*     c    m    s    g    p    i    d    b    f    o    l    h    u    r */
    {{50,  50, 600,  50, 999,   0,   0,   0, 999,   0,1500, 900,   0,   0},
     60, 40, 40, 45, 35, 6, 0, 0, 10, 2, 0, 0, "nsp  nuc supply ship",
     360, 1500, M_SUPPLY, 0, 2},

    {{0},
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,
     0, 0, 0, 0, 0}
};
