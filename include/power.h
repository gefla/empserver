/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  power.h: Definitions for things having to do with the power report.
 *
 *  Known contributors to this file:
 *
 */

#ifndef POWER_H
#define POWER_H

#include "file.h"
#include "types.h"

struct powstr {
    natid p_nation;
    float p_sects;
    float p_effic;
    float p_civil;
    float p_milit;
    float p_shell;
    float p_guns;
    float p_petrol;
    float p_iron;
    float p_dust;
    float p_food;
    float p_oil;
    float p_bars;
    float p_planes;
    float p_ships;
    float p_units;
    float p_money;
    float p_power;
};

#define getpower(n, p) ef_read(EF_POWER, (n), (p))
#define putpower(n, p) ef_write(EF_POWER, (n), (p))
#define getpowerp(n) ((struct powstr *)ef_ptr(EF_POWER, (n)))

#endif
