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
 *  natarg.c: Return countr # given country name or country #
 * 
 *  Known contributors to this file:
 *     (List known contributors to this file)
 */

#include <ctype.h>
#include "misc.h"
#include "nat.h"
#include "player.h"
#include "file.h"
#include "prototypes.h"
#include "optlist.h"

int
natarg(s_char *arg, s_char *prompt)
{
	s_char	buf[1024];
	int	n;
        struct  natstr *np;
        int     byname = 0;

	arg = getstarg(arg, prompt, buf);
	if (arg == 0 || *arg == 0)
		return -1;
	if (isdigit(*arg))
		n = atoi(arg);
	else {
		n = cnumb(arg);
		if (opt_HIDDEN) {
		    byname = 1;
		}
	}
	if (n < 0 || n >= MAXNOC) {
		pr("No such country exists.\n");
		n = -1;
	}
	if (opt_HIDDEN) {
	    if (!player->god && !getcontact(getnatp(player->cnum), n)) {
                if ((np = getnatp(n)) == 0)
                        return -1;
                if ((np->nat_stat & STAT_GOD) == 0) {
                        if (byname) {
                                pr("No such country exists.\n");
                                n = -1;
                        } else {
                                pr("Country has not been contacted.\n");
                                n = -2;
                        }
                }
	    }
	}
	return n;
}
