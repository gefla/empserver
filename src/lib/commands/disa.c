/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  disa.c: Disable updates (deity)
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#if defined(_WIN32)
#include <io.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "commands.h"
#include "optlist.h"

int
disa(void)
{
    int fd;

    if ((fd = open(disablefil, O_RDWR | O_CREAT | O_TRUNC, S_IRWUG)) < 0)
	return RET_FAIL;
    close(fd);
    pr("Updates are disabled\n");
    return RET_OK;
}
