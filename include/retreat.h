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
 *  retreat.h: Things having to do with retreating
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#ifndef RETREAT_H
#define RETREAT_H

#define MAX_RETREAT	2	/* Max number of sectors you can retreat */
#define RET_LEN		10

/* Retreat conditions */
#define RET_GROUP	1	/* Whole group retreats */
#define RET_INJURED	2	/* Retreat when damaged at all */
#define RET_TORPED	4	/* Retreat when torped */
#define RET_SONARED	8	/* Retreat when sonared */
#define RET_HELPLESS	16	/* Retreat when fired upon from beyond range */
#define RET_BOMBED	32	/* Retreat when bombed */
#define RET_DCHRGED	64	/* Retreat when depth-charged */
#define RET_BOARDED	128	/* Retreat when unsuccessfully boarded */

#endif
