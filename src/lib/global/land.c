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
 *  land.c: Land unit characteristics
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#include "misc.h"
#include "var.h"
#include "land.h"

struct lchrstr lchr[] = {

/* name
   lcm, hcm, mil, gun, shell, tech,   $,
   att, def, vul, spd, vis, spy, rad, frg, acc, dam, amm, aaf, fc, fu, xpl, mxl
   flags, nv
   cargo
*/
/* ((int)((ETUS*4)/(mil/1000)) is enough food for 3 updates */

    {2,
     {V_FOOD, V_MILIT},
     {((int)((ETUS * 4) / 20)), 20},
     "cav  cavalry",
     10, 5, 0, 0, 0, 30, 500,
     1.2, 0.5, 80, 32, 18, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_RECON | L_LIGHT,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS) / 4)), 1, 25},
     "linf light infantry",
     8, 4, 0, 0, 0, 40, 300,
     1.0, 1.5, 60, 28, 15, 2, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0,
     L_ASSAULT | L_LIGHT,
     },

    {2,
     {V_FOOD, V_MILIT},
     {((int)((ETUS * 4) / 10)), 100},
     "inf  infantry",
     10, 5, 0, 0, 0, 50, 500,
     1.0, 1.5, 60, 25, 15, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_ASSAULT | L_LIGHT,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS * 5) / 5)), 8, 100},
     "mtif motor inf",
     15, 10, 0, 0, 0, 190, 400,
     1.2, 2.2, 60, 33, 17, 1, 3, 0, 0, 0, 2, 3, 13, 1, 0, 0,
     L_LIGHT,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS * 5) / 5)), 8, 100},
     "mif  mech inf",
     15, 10, 0, 0, 0, 190, 800,
     1.5, 2.5, 50, 33, 17, 1, 3, 0, 0, 0, 2, 3, 13, 1, 0, 0,
     L_LIGHT,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS * 5) / 5)), 4, 100},
     "mar  marines",
     10, 5, 0, 0, 0, 140, 1000,
     1.4, 2.4, 60, 25, 14, 2, 1, 0, 0, 0, 1, 2, 0, 0, 0, 0,
     L_MARINE | L_ASSAULT | L_LIGHT,
     },

    {10,
     {V_SHELL, V_GUN, V_FOOD, V_LCM, V_HCM, V_DUST, V_BAR, V_IRON,
      V_PETROL, V_MILIT},
     {200, 10, 300, 200, 100, 100, 10, 100, 300, 25},
     "sup  supply",
     10, 5, 0, 0, 0, 50, 500,
     0.1, 0.2, 80, 25, 20, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_LIGHT | L_SUPPLY,
     },

    {12,
     {V_SHELL, V_GUN, V_FOOD, V_LCM, V_HCM, V_DUST, V_BAR, V_IRON,
      V_PETROL, V_MILIT, V_OIL, V_RAD},
     {990, 200, 990, 990, 990, 500, 100, 500, 990, 990, 990, 150},
     "tra  train",
     100, 50, 0, 0, 0, 40, 3500,
     0.0, 0.0, 120, 10, 25, 3, 0, 0, 0, 0, 0, 0, 0, 0, 5, 12,
     L_XLIGHT | L_TRAIN | L_HEAVY | L_SUPPLY,
     },

    {0,
     {V_FOOD},
     {0,},
     "spy  infiltrator",
     10, 5, 0, 0, 0, 40, 750,
     0.0, 0.0, 80, 32, 18, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_ASSAULT | L_RECON | L_LIGHT | L_SPY,
     },

    {1,
     {V_SHELL},
     {3,},
     "com  commando",
     10, 5, 0, 0, 0, 55, 1500,
     0.0, 0.0, 80, 32, 18, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     L_ASSAULT | L_RECON | L_LIGHT | L_SPY,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS) / 5)), 5, 20},
     "aau  aa unit",
     20, 10, 0, 0, 0, 70, 500,
     0.5, 1.0, 60, 18, 20, 1, 1, 0, 0, 0, 1, 2, 0, 0, 0, 0,
     L_FLAK | L_LIGHT,
     },

    {4,
     {V_FOOD, V_SHELL, V_MILIT, V_GUN},
     {((int)((ETUS * 4) / 10)), 40, 25, 10},
     "art  artillery",
     20, 10, 0, 0, 0, 35, 800,
     0.1, 0.4, 70, 18, 20, 1, 0, 8, 50, 5, 2, 1, 0, 0, 0, 0,
     L_LIGHT,
     },

    {4,
     {V_FOOD, V_SHELL, V_MILIT, V_GUN},
     {((int)((ETUS * 4) / 20)), 20, 25, 6},
     "lat  lt artillery",
     20, 10, 0, 0, 0, 70, 500,
     0.2, 0.6, 60, 30, 18, 1, 1, 5, 10, 3, 1, 1, 0, 0, 0, 0,
     L_LIGHT,
     },

    {4,
     {V_FOOD, V_SHELL, V_MILIT, V_GUN},
     {((int)((ETUS * 4) / 10)), 80, 25, 12},
     "hat  hvy artillery",
     40, 20, 0, 0, 0, 100, 800,
     0.0, 0.2, 60, 12, 20, 1, 0, 11, 99, 8, 4, 1, 0, 0, 0, 0,
     0,
     },

    {4,
     {V_FOOD, V_SHELL, V_MILIT, V_GUN},
     {((int)((ETUS * 5) / 20)), 40, 25, 10},
     "mat  mech artillery",
     20, 10, 0, 0, 0, 200, 1000,
     0.2, 0.6, 50, 35, 17, 1, 1, 8, 35, 6, 3, 3, 13, 1, 0, 0,
     L_LIGHT,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS * 4) / 20)), 3, 20},
     "eng  engineer",
     10, 5, 0, 0, 0, 130, 3000,
     1.2, 2.4, 50, 25, 14, 2, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0,
     L_ENGINEER | L_ASSAULT | L_LIGHT,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS * 5) / 20)), 4, 20},
     "meng mech engineer",
     10, 5, 0, 0, 0, 260, 4500,
     1.8, 3.5, 45, 33, 15, 3, 3, 0, 0, 0, 1, 5, 25, 2, 0, 0,
     L_ENGINEER | L_ASSAULT | L_LIGHT,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS * 5) / 10)), 4, 50},
     "lar  lt armor",
     10, 5, 0, 0, 0, 150, 600,
     2.0, 1.0, 50, 42, 15, 4, 4, 0, 0, 0, 1, 2, 25, 1, 0, 0,
     L_RECON | L_LIGHT,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS * 4) / 5)), 3, 100},
     "har  hvy armor",
     20, 10, 0, 0, 0, 120, 500,
     2.0, 0.8, 50, 18, 17, 1, 1, 0, 0, 0, 2, 1, 10, 2, 0, 0,
     0,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS * 5) / 10)), 4, 50},
     "arm  armor",
     20, 10, 0, 0, 0, 170, 1000,
     3.0, 1.5, 40, 33, 16, 2, 2, 0, 0, 0, 1, 2, 13, 1, 0, 0,
     L_LIGHT,
     },

    {3,
     {V_FOOD, V_SHELL, V_MILIT},
     {((int)((ETUS * 5) / 10)), 4, 50},
     "sec  security",
     10, 5, 0, 0, 0, 170, 600,
     1.0, 2.0, 60, 25, 15, 2, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0,
     L_SECURITY | L_LIGHT,
     },

    {2,
     {V_FOOD, V_MILIT},
     {((int)((ETUS * 5) / 40)), 10},
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

    {0,
     {0},
     {0},
     "",
     0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0,
     }
};

int lnd_maxno = (sizeof(lchr) / sizeof(struct lchrstr)) - 1;
