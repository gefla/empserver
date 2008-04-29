/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  recvclient.c: Receive input from the client
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include "empio.h"
#include "journal.h"
#include "player.h"
#include "prototypes.h"

/*
 * Receive a line of input from the current player.
 * If the player's eof flag is set, return -1 without receiving input.
 * If the player's aborted flag is set, return -2 without receiving
 * input.
 * Else receive one line and store it in CMD[SIZE].
 * This may block for input, yielding the processor.  Flush buffered
 * output when blocking, to make sure player sees the prompt.
 * If the player's connection has the I/O error indicator set, or the
 * line is "aborted", set the player's aborted flag and return -2.
 * If the player's connection has the EOF indicator set, or the line
 * is "ctld", set the player's eof flag and return -1.
 * Else return the length of the line.
 * Design bug: there is no way to indicate truncation of a long line.
 */
int
recvclient(char *cmd, int size)
{
    int count;

    count = -1;
    while (!player->aborted && !player->eof) {
	/* Try to get a line of input */
	count = io_gets(player->iop, cmd, size);
	if (count >= 0) {
	    /* got it */
	    if (strcmp(cmd, "ctld") == 0)
		player->eof = 1;
	    if (strcmp(cmd, "aborted") == 0)
		player->aborted = 1;
	    journal_input(cmd);
	    break;
	}

	/* Make sure player sees prompt */
	io_output_all(player->iop);

	/*
	 * If io_output_all() blocked and got unblocked by command
	 * abortion, we must return without blocking in io_input().
	 */
	if (player->aborted)
	    break;

	/* Await more input */
	io_input(player->iop, IO_WAIT);
	if (io_error(player->iop))
	    player->aborted = 1;
	else if (io_eof(player->iop))
	    player->eof = 1;
    }

    if (player->eof)
	return -1;
    if (player->aborted)
	return -2;

    return count;
}
