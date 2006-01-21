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
 *  genitem.h: Definition for generic items
 * 
 *  Known contributors to this file:
 * 
 */
/*
 * XXX cheap hack; this depends on the fact
 * that units are all the same starting from the top.
 * If you change the units, DON'T CHANGE the tops to
 * be non-identical. Also, if you change types of
 * parts of the tops, be sure to change this file!
 */

#ifndef GENITEM_H
#define GENITEM_H

struct genitem {
    short ef_type;
    natid own;
    short uid;
    coord x;
    coord y;
    s_char type;
    s_char effic;
    s_char mobil;
    short tech;
    s_char group;
    coord opx, opy;
    short mission;
    short radius;
};

#endif
