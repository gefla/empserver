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
 *  tfact.c: return tech fact given multiplier
 * 
 *  Known contributors to this file:
 *     Yannick Trembley
 */

#include "misc.h"
#include "nat.h"
#include "file.h"

extern float fire_range_factor;

double
tfact(natid cn, double mult)
{
	double	tlev;
	struct	natstr *np;

	np = getnatp(cn);
	tlev = np->nat_level[NAT_TLEV];
	tlev = (50.0 + tlev) / (200.0 + tlev);
	return mult * tlev;
}

double
tfactfire(natid cn, double mult)
{
	double	tlev;
	struct	natstr *np;

	np = getnatp(cn);
	tlev = np->nat_level[NAT_TLEV];
	tlev = (50.0 + tlev) / (200.0 + tlev);
	return mult * tlev * fire_range_factor;
}

double
techfact(int level, double mult)
{
	return mult * ((50.0 + level) / (200.0 + level));
}

/*
 * added so that firing range can be different to other ranges
 */ 
double
techfactfire(int level, double mult)
{
	return mult * ((50.0 + level) / (200.0 + level)) * fire_range_factor;
}

/*
 * figure out relative difference between two tech levels
 */
double
reltech(int level1, int level2, double mult)
{
	int	diff;

	diff = level1 - level2;
	return (1.0 + ((diff + 50.0) / (level1 + 50.0))) * mult;
}
