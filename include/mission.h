/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mission.h: Definitions for things having to do with missions.
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#ifndef MISSION_H
#define MISSION_H

#define MI_NONE		0
#define MI_INTERDICT	1
#define MI_SUPPORT	2
#define MI_RESERVE	3
#define MI_ESCORT	4
#define MI_SINTERDICT	5	/* sub interdiction */
#define MI_AIR_DEFENSE	6
#define MI_DSUPPORT	7
#define MI_OSUPPORT	8

#define SECT_HARDTARGET (-92917)	/* a very low number that no-one might pick */

#endif
