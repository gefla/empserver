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
 *  item.c: Item characteristics
 * 
 *  Known contributors to this file:
 *     
 */

/*
 * must be in same order as defines in var.h
 */

#include "misc.h"
#include "var.h"
#include "item.h"

struct ichrstr ichr[I_MAX + 2] = {
/*        mnem vtype      val  sell lbs rg,wh,ur,bnk  name */
    {'?', 0, 0, 0, 0, {0, 0, 0, 0}, "unused"},
/* STM CHANGED HERE */
    {'c', V_CIVIL, 1, 0, 1, {10, 10, 10, 10}, "civilians"},
    {'m', V_MILIT, 0, 0, 1, {1, 1, 1, 1}, "military"},
    {'s', V_SHELL, 5, 1, 1, {1, 10, 1, 1}, "shells"},
    {'g', V_GUN, 60, 1, 10, {1, 10, 1, 1}, "guns"},
    {'p', V_PETROL, 4, 1, 1, {1, 10, 1, 1}, "petrol"},
    {'i', V_IRON, 2, 1, 1, {1, 10, 1, 1}, "iron ore"},
    {'d', V_DUST, 20, 1, 5, {1, 10, 1, 1}, "dust (gold)"},
    {'b', V_BAR, 280, 1, 50, {1, 5, 1, 4}, "bars of gold"},
    {'f', V_FOOD, 0, 1, 1, {1, 10, 1, 1}, "food"},
    {'o', V_OIL, 8, 1, 1, {1, 10, 1, 1}, "oil"},
    {'l', V_LCM, 2, 1, 1, {1, 10, 1, 1}, "light products"},
    {'h', V_HCM, 4, 1, 1, {1, 10, 1, 1}, "heavy products"},
    {'u', V_UW, 1, 1, 2, {1, 2, 1, 1}, "uncompensated workers"},
    {'r', V_RAD, 150, 1, 8, {1, 10, 1, 1}, "radioactive materials"},
    {0, 0, 0, 0, 0, {0, 0, 0, 0}, 0}
};

int itm_maxno = (sizeof(ichr) / sizeof(struct ichrstr)) - 1;
