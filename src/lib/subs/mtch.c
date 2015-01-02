/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mtch.c: Matching operations on structures and commands.
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2009
 */

#include <config.h>

#include "com.h"
#include "match.h"
#include "prototypes.h"

/*
 * Search for COMMAND in COMS[], return its index.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several, M_IGNORE if the command should be ignored.
 * Ignore commands that require more permissions than COMSTAT.
 */
int
comtch(char *command, struct cmndstr *coms, int comstat)
{
    struct cmndstr *com;
    int status;

    if (!command || !*command)
	return M_IGNORE;
    status = M_NOTFOUND;
    for (com = coms; com->c_form; com++) {
	if ((com->c_permit & comstat) != com->c_permit)
	    continue;
	switch (mineq(command, com->c_form)) {
	case ME_MISMATCH:
	    break;
	case ME_PARTIAL:
	    if (status >= 0)
		return M_NOTUNIQUE;
	    status = com - coms;
	    break;
	case ME_EXACT:
	    return com - coms;
	}
    }

    return status;
}
