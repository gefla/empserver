/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  types.h: Empire types
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2007
 */

#ifndef TYPES_H
#define TYPES_H

typedef unsigned char natid;	/* NSC_NATID must match this */
typedef short coord;

struct bp;
struct emp_qelem;
struct empobj;
struct gamestr;
struct lndstr;
struct lndstr;
struct lonstr;
struct natstr;
struct nchrstr;
struct nstr_item;
struct nstr_sect;
struct nukstr;
struct player;
struct plist;
struct plnstr;
struct range;
struct sctstr;
struct shiplist;
struct shpstr;
struct trdstr;
struct trtstr;
struct comstr;
struct cmndstr;
struct ulist;

union empobj_storage;

#endif
