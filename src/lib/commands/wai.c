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
 *  wai.c: Wait for telegrams and announcements
 * 
 *  Known contributors to this file:
 *   
 */

#include "misc.h"
#include "player.h"
#include "nat.h"
#include "file.h"
#include "empio.h"
#include "commands.h"

int
wai(void)
{
    struct natstr *np;

    np = getnatp(player->cnum);
    io_output(player->iop, IO_WAIT);
    if (np->nat_ann == 0 && np->nat_tgms == 0) {
	player->waiting++;
	io_input(player->iop, IO_WAIT);
	player->waiting--;
    }
    if (player->aborted || io_eof(player->iop))
	return RET_OK;
    if (np->nat_ann != 0)
	player->argp[0] = "wire";
    if (np->nat_tgms != 0)
	player->argp[0] = "read";
    return rea();
}
