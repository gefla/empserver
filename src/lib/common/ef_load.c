/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ef_load.c: Load custom game configuration files
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 *  
 */

#include <config.h>

#include <stdio.h>
#include <time.h>

#include "prototypes.h"
#include "file.h"

int
ef_load()
{
    struct empfile *ep;
    FILE *fp;
    int retval = 0;
    
    for (ep = empfile; ep->name; ep++) {
	if (!EF_IS_GAME_STATE(ep->uid) && ep->file) {
	    if ((fp = fopen(ep->file, "r")) == NULL) {
		continue;
	    }
	    if (xundump(fp, ep->name, ep->uid) < 0)
		retval = -1;
	    else {
		int ch = getc(fp);
		if (ch != EOF) {
		    fprintf(stderr, "%s: Junk after the table\n",
			ep->file);
		    retval = -1;
		}
	    }
	    fclose(fp);
	}
    }
    return retval;
}
