/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
/*  mnem  vtype   val sell lbs {in rg wh  ur bnk}  name */
    {'?', I_NONE,   0, 0,  0, {0,  0,  0,  0,  0}, "unused"},
    {'c', I_CIVIL,  1, 0,  1, {1, 10, 10, 10, 10}, "civilians"},
    {'m', I_MILIT,  0, 0,  1, {1,  1,  1,  1,  1}, "military"},
    {'s', I_SHELL,  5, 1,  1, {1,  1, 10,  1,  1}, "shells"},
    {'g', I_GUN,   60, 1, 10, {1,  1, 10,  1,  1}, "guns"},
    {'p', I_PETROL, 4, 1,  1, {1,  1, 10,  1,  1}, "petrol"},
    {'i', I_IRON,   2, 1,  1, {1,  1, 10,  1,  1}, "iron ore"},
    {'d', I_DUST,  20, 1,  5, {1,  1, 10,  1,  1}, "dust (gold)"},
    {'b', I_BAR,  280, 1, 50, {1,  1,  5,  1,  4}, "bars of gold"},
    {'f', I_FOOD,   0, 1,  1, {1,  1, 10,  1,  1}, "food"},
    {'o', I_OIL,    8, 1,  1, {1,  1, 10,  1,  1}, "oil"},
    {'l', I_LCM,    2, 1,  1, {1,  1, 10,  1,  1}, "light products"},
    {'h', I_HCM,    4, 1,  1, {1,  1, 10,  1,  1}, "heavy products"},
    {'u', I_UW,     1, 1,  2, {1,  1,  2,  1,  1}, "uncompensated workers"},
    {'r', I_RAD,  150, 1,  8, {1,  1, 10,  1,  1}, "radioactive materials"},
    {0,   I_NONE,   0, 0,  0, {0,  0,  0,  0,  0}, NULL}
};

int itm_maxno = (sizeof(ichr) / sizeof(struct ichrstr)) - 1;
