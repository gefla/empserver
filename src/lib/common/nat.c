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
 *  nat.c: Misc. accesses on the nation file
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include "misc.h"
#include "nat.h"
#include "file.h"
#include "optlist.h"

s_char *
cname(natid n)
{
	struct	natstr *np;

	if ((np = getnatp(n)) == 0)
		return 0;
	return np->nat_cnam;
}

s_char *
relatename(struct natstr *np, natid other)
{
	extern	s_char *relates[];

	return relates[getrel(np, other)];
}

s_char *
rejectname(struct natstr *np, natid other)
{
	extern s_char	*rejects[];

	return rejects[getrejects(other, np)];
}

s_char *
natstate(struct natstr *np)
{
	if ((np->nat_stat & STAT_INUSE) == 0)
		return "FREE";
	if (np->nat_stat & STAT_GOD)
		return "DEITY";
	if ((np->nat_stat & STAT_NORM) == 0)
		return "VISITOR";
	return "ACTIVE";
}

/* This returns the relations that np has with them */
int
getrel(struct natstr *np, natid them)
{
	return np->nat_relate[them];
}

int
getrejects(natid them, struct natstr *np)
{
	int	ind;
	int	shift;
	int	reject;

	ind = them / 4;
	shift = 12 - ((them - ((them / 4) << 2)) * 4);
	/* the 07 should *really* be 017 */
	reject = (np->nat_rejects[ind] >> shift) & 0x0f;
	return reject;
}

void
agecontact(struct natstr *np)
{
       int them;

       if (opt_LOSE_CONTACT) {
               for (them = 1;  them < MAXNOC;  ++them) {
                       if (them != np->nat_cnum && np->nat_contact[them]) {
                               --np->nat_contact[them];
                       }
               }
       }
}

int
getcontact(struct natstr *np, natid them)
{
	int	contact;

	if (opt_LOSE_CONTACT) {
	    contact = np->nat_contact[them];
	} else {
	    int ind = them/16;
	    int shift = (them % 16);

	    contact = (np->nat_contact[ind] >> shift) & 1;
	}
	return contact;
}	

void
putrel(struct natstr *np, natid them, int relate)
{
	np->nat_relate[them] = relate;
}

void
putreject(struct natstr *np, natid them, int how, int what)
{
	int	shift;
	int	newrej;
	int	ind;

	/* This 07 should be changed to 017 after the current game is over */
	what &= 0x0f;
	ind = them / 4;
	shift = 12 - ((them - ((them / 4) << 2)) * 4);
	newrej = np->nat_rejects[ind];
	if (how)
		newrej |= (what << shift);
	else
		newrej &= ~(what << shift);
	np->nat_rejects[ind] = newrej;
}

void
putcontact(struct natstr *np, natid them, int contact)
{
    if (opt_LOSE_CONTACT) {
	if (np->nat_contact[them] > contact)
		return;
	np->nat_contact[them] = contact;
    } else {
	int ind = them/16;
	int shift = them%16;
	int new = np->nat_contact[ind];
	if (contact)
		contact = 1;
	new &= ~(1 << shift);
	new |= (contact << shift);
	np->nat_contact[ind] = new;
    }
}
