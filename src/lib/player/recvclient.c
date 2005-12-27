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
 *  recvclient.c: Receive input from the client
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include "prototypes.h"
#include <stdio.h>
#include "misc.h"
#include "empio.h"
#include "player.h"
#include "empthread.h"

/*
 * Receive a line of input from the current player.
 * If the player's aborted flag is set, return -2 without receiving
 * input.
 * Else receive one line and store it in CMD[SIZE].
 * This may block for input, yielding the processor.  Flush buffered
 * output when blocking, to make sure player sees the prompt.
 * If the player's connection has the I/O error indicator set, or the
 * line is "aborted", set the player's aborted flag and return -2.
 * If the player's connection has the EOF indicator set, or the line
 * is "ctld", return -1.
 * Else return the length of the line.
 * Design bug: there is no way to indicate truncation of a long line.
 */
int
recvclient(char *cmd, int size)
{
    int count;

    if (player->aborted)
	return -2;
    count = io_gets(player->iop, cmd, size);
    while (!player->aborted && count < 0) {
	io_output_all(player->iop);
	io_input(player->iop, IO_WAIT);
	if (io_error(player->iop))
	    player->aborted++;
	else if (io_eof(player->iop))
	    return -1;
	else
	    count = io_gets(player->iop, cmd, size);
    }
    if (count > 0) {
	if (strcmp(cmd, "ctld") == 0)
	    return -1;
	if (strcmp(cmd, "aborted") == 0)
	    player->aborted = 1;
    }
    if (player->aborted)
	return -2;
    return count;
}
