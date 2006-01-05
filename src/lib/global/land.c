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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  land.c: Land unit characteristics
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#include <config.h>

#include "misc.h"
#include "land.h"

struct lchrstr lchr[] = {

/* name
   lcm, hcm, mil, gun, shell, tech,   $,
   att, def, vul, spd, vis, spy, rad, frg, acc, dam, amm, aaf, fc, fu, xpl, mxl
   flags, nv
   cargo
*/

    {{0,  20,   0,   0,   0,   0,   0,   0,  12,   0,   0,   0,   0,   0},
     "cav  cavalry",
     10, 5, 0, 0, 0, 30, 500,
     1.2, 0.5, 80, 32, 18, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_RECON | L_LIGHT,
     },

    {{0,  25,   1,   0,   0,   0,   0,   0,  15,   0,   0,   0,   0,   0},
     "linf light infantry",
     8, 4, 0, 0, 0, 40, 300,
     1.0, 1.5, 60, 28, 15, 2, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0,
     L_ASSAULT | L_LIGHT,
     },

    {{0, 100,   0,   0,   0,   0,   0,   0,  24,   0,   0,   0,   0,   0},
     "inf  infantry",
     10, 5, 0, 0, 0, 50, 500,
     1.0, 1.5, 60, 25, 15, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_ASSAULT | L_LIGHT,
     },

    {{0, 100,   8,   0,   0,   0,   0,   0,  60,   0,   0,   0,   0,   0},
     "mtif motor inf",
     15, 10, 0, 0, 0, 190, 400,
     1.2, 2.2, 60, 33, 17, 1, 3, 0, 0, 0, 2, 3, 13, 1, 0, 0,
     L_LIGHT,
     },

    {{0, 100,   8,   0,   0,   0,   0,   0,  60,   0,   0,   0,   0,   0},
     "mif  mech inf",
     15, 10, 0, 0, 0, 190, 800,
     1.5, 2.5, 50, 33, 17, 1, 3, 0, 0, 0, 2, 3, 13, 1, 0, 0,
     L_LIGHT,
     },

    {{0, 100,   4,   0,   0,   0,   0,   0,  60,   0,   0,   0,   0,   0},
     "mar  marines",
     10, 5, 0, 0, 0, 140, 1000,
     1.4, 2.4, 60, 25, 14, 2, 1, 0, 0, 0, 1, 2, 0, 0, 0, 0,
     L_MARINE | L_ASSAULT | L_LIGHT,
     },

    {{0,  25, 200,  10, 300, 100, 100,  10, 300,   0, 200, 100,   0,   0},
     "sup  supply",
     10, 5, 0, 0, 0, 50, 500,
     0.1, 0.2, 80, 25, 20, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_LIGHT | L_SUPPLY,
     },

    {{0, 990, 990, 200, 990, 500, 500, 100, 990, 990, 990, 990,   0, 150},
     "tra  train",
     100, 50, 0, 0, 0, 40, 3500,
     0.0, 0.0, 120, 10, 25, 3, 0, 0, 0, 0, 0, 0, 0, 0, 5, 12,
     L_XLIGHT | L_TRAIN | L_HEAVY | L_SUPPLY,
     },

    {{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
     "spy  infiltrator",
     10, 5, 0, 0, 0, 40, 750,
     0.0, 0.0, 80, 32, 18, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_ASSAULT | L_RECON | L_LIGHT | L_SPY,
     },

    {{0,   0,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
     "com  commando",
     10, 5, 0, 0, 0, 55, 1500,
     0.0, 0.0, 80, 32, 18, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_ASSAULT | L_RECON | L_LIGHT | L_SPY,
     },

    {{0,  20,   5,   0,   0,   0,   0,   0,  12,   0,   0,   0,   0,   0},
     "aau  aa unit",
     20, 10, 0, 0, 0, 70, 500,
     0.5, 1.0, 60, 18, 20, 1, 1, 0, 0, 0, 1, 2, 0, 0, 0, 0,
     L_FLAK | L_LIGHT,
     },

    {{0,  25,  40,  10,   0,   0,   0,   0,  24,   0,   0,   0,   0,   0},
     "art  artillery",
     20, 10, 0, 0, 0, 35, 800,
     0.1, 0.4, 70, 18, 20, 1, 0, 8, 50, 5, 2, 1, 0, 0, 0, 0,
     L_LIGHT,
     },

    {{0,  25,  20,   6,   0,   0,   0,   0,  12,   0,   0,   0,   0,   0},
     "lat  lt artillery",
     20, 10, 0, 0, 0, 70, 500,
     0.2, 0.6, 60, 30, 18, 1, 1, 5, 10, 3, 1, 1, 0, 0, 0, 0,
     L_LIGHT,
     },

    {{0,  25,  80,  12,   0,   0,   0,   0,  24,   0,   0,   0,   0,   0},
     "hat  hvy artillery",
     40, 20, 0, 0, 0, 100, 800,
     0.0, 0.2, 60, 12, 20, 1, 0, 11, 99, 8, 4, 1, 0, 0, 0, 0,
     0,
     },

    {{0,  25,  40,  10,   0,   0,   0,   0,  15,   0,   0,   0,   0,   0},
     "mat  mech artillery",
     20, 10, 0, 0, 0, 200, 1000,
     0.2, 0.6, 50, 35, 17, 1, 1, 8, 35, 6, 3, 3, 13, 1, 0, 0,
     L_LIGHT,
     },

    {{0,  20,   3,   0,   0,   0,   0,   0,  12,   0,   0,   0,   0,   0},
     "eng  engineer",
     10, 5, 0, 0, 0, 130, 3000,
     1.2, 2.4, 50, 25, 14, 2, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0,
     L_ENGINEER | L_ASSAULT | L_LIGHT,
     },

    {{0,  20,   4,   0,   0,   0,   0,   0,  15,   0,   0,   0,   0,   0},
     "meng mech engineer",
     10, 5, 0, 0, 0, 260, 4500,
     1.8, 3.5, 45, 33, 15, 3, 3, 0, 0, 0, 1, 5, 25, 2, 0, 0,
     L_ENGINEER | L_ASSAULT | L_LIGHT,
     },

    {{0,  50,   4,   0,   0,   0,   0,   0,  30,   0,   0,   0,   0,   0},
     "lar  lt armor",
     10, 5, 0, 0, 0, 150, 600,
     2.0, 1.0, 50, 42, 15, 4, 4, 0, 0, 0, 1, 2, 25, 1, 0, 0,
     L_RECON | L_LIGHT,
     },

    {{0, 100,   3,   0,   0,   0,   0,   0,  48,   0,   0,   0,   0,   0},
     "har  hvy armor",
     20, 10, 0, 0, 0, 120, 500,
     2.0, 0.8, 50, 18, 17, 1, 1, 0, 0, 0, 2, 1, 10, 2, 0, 0,
     0,
     },

    {{0,  50,   4,   0,   0,   0,   0,   0,  30,   0,   0,   0,   0,   0},
     "arm  armor",
     20, 10, 0, 0, 0, 170, 1000,
     3.0, 1.5, 40, 33, 16, 2, 2, 0, 0, 0, 1, 2, 13, 1, 0, 0,
     L_LIGHT,
     },

    {{0,  50,   4,   0,   0,   0,   0,   0,  30,   0,   0,   0,   0,   0},
     "sec  security",
     10, 5, 0, 0, 0, 170, 600,
     1.0, 2.0, 60, 25, 15, 2, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0,
     L_SECURITY | L_LIGHT,
     },

    {{0,  10,   0,   0,   0,   0,   0,   0,   7,   0,   0,   0,   0,   0},
     "rad  radar unit",
     10, 5, 0, 0, 0, 270, 1000,
     0.0, 0.0, 50, 33, 15, 3, 0, 0, 0, 0, 0, 2, 25, 2, 1, 0,
     L_XLIGHT | L_RADAR | L_LIGHT,
     },

/* name
   lcm, hcm, mil, gun, shell, tech,   $,
   att, def, vul, spd, vis, spy, rad, frg, acc, dam, amm, aaf, fc, fu, xpl,
   flags, nv
   cargo
*/

    {{0},
     NULL,
     0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0,
     }
};
