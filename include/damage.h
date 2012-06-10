/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  damage.h: Damage formulas
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#ifndef DAMAGE_H
#define DAMAGE_H

#define PERCENT_DAMAGE(x) (100 * (x) / ((x) + 100))
#define DPERCENT_DAMAGE(x) (100.0 * (x) / ((x) + 100.0))
#define DMINE_HITCHANCE(x) ((x) / ((x) + 20.0))
#define DMINE_LHITCHANCE(x) ((x) / ((x) + 35.0))
#define MINE_DAMAGE() (22 + random()%21)
#define MINE_LDAMAGE() (11 + random()%20)
#define DTORP_HITCHANCE(range, vis) \
    (0.9 / ((range)+1) + (((vis) < 6) ? (5-(vis)) * 0.03 : 0.0))
#define TORP_DAMAGE() (torpedo_damage + (random() % torpedo_damage) + \
		       (random() % torpedo_damage))

#endif
