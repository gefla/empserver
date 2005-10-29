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
 *  fileinit.c: Initialize Empire tables for full server operations.
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 *  
 */

#include "file.h"
#include "prototypes.h"

struct fileinit {
    int ef_type;
    void (*init) (int, char *);
    int (*postread) (int, char *);
    int (*prewrite) (int, char *);
};

static struct fileinit fileinit[] = {
    {EF_SECTOR, NULL, sct_postread, sct_prewrite},
    {EF_SHIP, shp_init, shp_postread, shp_prewrite},
    {EF_PLANE, pln_init, pln_postread, pln_prewrite},
    {EF_LAND, lnd_init, lnd_postread, lnd_prewrite},
    {EF_NUKE, nuk_init, nuk_postread, nuk_prewrite}
};

/*
 * Initialize empfile for full server operations.
 * Like ef_init(), but additionally installs the server's callbacks.
 * This is separate from ef_init(), so that utility programs like
 * files can use empfile.
 */
void
ef_init_srv(void)
{
    unsigned i;

    for (i = 0; i < sizeof(fileinit) / sizeof(fileinit[0]); i++) {
	empfile[fileinit[i].ef_type].init = fileinit[i].init;
	empfile[fileinit[i].ef_type].postread = fileinit[i].postread;
	empfile[fileinit[i].ef_type].prewrite = fileinit[i].prewrite;
    }

    ef_init();
}
