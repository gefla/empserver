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
 *  mtch.c: Matching operations on structures and commands.
 * 
 *  Known contributors to this file:
 *     (List known contributors to this file)
 */
/*
 * XXX These routines gamble that structures are all longword-aligned.
 * If this is not true, they will BREAK!
 */

#include "misc.h"
#include "player.h"
#include "com.h"
#include "match.h"
#include "prototypes.h"

/*
 * find a matching integer from a member of a structure.
 * Inspired by stmtch above.
 */
int
intmatch(register int value, register int *ptr, int size)
{
    register int i;

    size /= sizeof(*ptr);
    for (i = 0; *ptr; i++, ptr += size)
	if (value == *ptr)
	    return i;
    return -1;
}

/*
 * Search for COMMAND in COMS[], return its index.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several, M_IGNORE if the command should be ignored.
 * Ignore commands that require more permissions than COMSTAT.
 */
int
comtch(register s_char *command, struct cmndstr *coms, int comstat,
       int god)
{
    register struct cmndstr *com;
    register int status;

    if (command == 0 || *command == 0)
	return M_IGNORE;
    status = M_NOTFOUND;
    for (com = coms; com->c_form != 0; com++) {
	if ((com->c_permit & comstat) != com->c_permit && !god)
	    continue;
	switch (mineq(command, com->c_form)) {
	case ME_MISMATCH:
	    break;
	case ME_PARTIAL:
	    if (status >= 0)
		return M_NOTUNIQUE;
	    status = com - coms;
	case ME_EXACT:
	    return com - coms;
	}
    }

    return status;
}
