/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2006-2009
 *     Ron Koenderink, 2009
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
 * If the player's connection has the I/O error or EOF indicator set,
 * or the line is "ctld", set the player's eof and aborted flag and
 * return -1.
 * If the line is "aborted", set the player's aborted flag and return
 * -2.
 * Else return the length of the line.
 * Design bug: there is no way to indicate truncation of a long line.
 */
int
recvclient(char *cmd, int size)
{
    int count, res;

    count = -1;
    while (!player->aborted) {
	/* Try to get a line of input */
	count = io_gets(player->iop, cmd, size);
	if (count >= 0) {
	    /* got it */
	    if (strcmp(cmd, "ctld") == 0)
		player->aborted = player->eof = 1;
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

	res = io_input(player->iop, IO_WAIT);
	if (res > 0)
	    ;
	else if (res < 0)
	    player->aborted = player->eof = 1;
	else if (io_eof(player->iop))
	    player->aborted = player->eof = 1;
	else if (!player->aborted) {
	    pr_flash(player, "idle connection terminated\n");
	    player->aborted = player->eof = 1;
	}
    }

    if (player->aborted) {
	player->recvfail++;
	if (player->recvfail > 255) {
	    /*
	     * Looks like the thread is stuck in a loop that fails to
	     * check errors; oops once, then slow it down drastically.
	     */
	    CANT_HAPPEN(player->recvfail == 256);
	    empth_sleep(time(NULL) + 60);
	}
	return player->eof ? -1 : -2;
    }

    player->recvfail = 0;
    return count;
}
