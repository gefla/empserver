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
 *  item.c: Item characteristics
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "item.h"

struct ichrstr ichr[] = {
/*  mnem  vtype   val sell lbs {in rg wh  ur bnk} melt  name */
    {'c', I_CIVIL,  1, 0,  1, {1, 10, 10, 10, 10},   4, "civilians"},
    {'m', I_MILIT,  0, 0,  1, {1,  1,  1,  1,  1},  20, "military"},
    {'s', I_SHELL,  5, 1,  1, {1,  1, 10,  1,  1},  80, "shells"},
    {'g', I_GUN,   60, 1, 10, {1,  1, 10,  1,  1}, 100, "guns"},
    {'p', I_PETROL, 4, 1,  1, {1,  1, 10,  1,  1},  50, "petrol"},
    {'i', I_IRON,   2, 1,  1, {1,  1, 10,  1,  1}, 100, "iron ore"},
    {'d', I_DUST,  20, 1,  5, {1,  1, 10,  1,  1}, 100, "dust (gold)"},
    {'b', I_BAR,  280, 1, 50, {1,  1,  5,  1,  4}, 200, "bars of gold"},
    {'f', I_FOOD,   0, 1,  1, {1,  1, 10,  1,  1},   2, "food"},
    {'o', I_OIL,    8, 1,  1, {1,  1, 10,  1,  1},  50, "oil"},
    {'l', I_LCM,    2, 1,  1, {1,  1, 10,  1,  1}, 100, "light products"},
    {'h', I_HCM,    4, 1,  1, {1,  1, 10,  1,  1}, 100, "heavy products"},
    {'u', I_UW,     1, 1,  2, {1,  1,  2,  1,  1},   2, "uncompensated workers"},
    {'r', I_RAD,  150, 1,  8, {1,  1, 10,  1,  1},1000, "radioactive materials"},
    {0,   I_NONE,   0, 0,  0, {0,  0,  0,  0,  0},   0, NULL}
};
