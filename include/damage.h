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
 *  damage.h: Damage formulas
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#ifndef _DAMAGE_H_
#define _DAMAGE_H_

#define PERCENT_DAMAGE(x) (100 * (x) / ((x) + 100))
#define DPERCENT_DAMAGE(x) ((double)(100.0 * (x) / ((x) + 100.0)))
#define DMINE_HITCHANCE(x) ((double) ( (x) / ((x)+20.0) ) )
#define DMINE_LHITCHANCE(x) ((double) ( (x) / ((x)+35.0) ) )
#define MINE_DAMAGE() (22 + random()%21)
#define MINE_LDAMAGE() (10 + random()%11)
#define DTORP_HITCHANCE(range, vis) ((double)(0.9/((range)+1)+(((vis)<6)?(5-(vis))*0.03:0)))
#define TORP_DAMAGE() (torpedo_damage + (random() % torpedo_damage) + \
		       (random() % torpedo_damage))

#endif /* _DAMAGE_H_ */
