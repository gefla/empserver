/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  recvclient.c: Receive input from the client
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Markus Armbruster, 2006-2012
 *     Ron Koenderink, 2009
 */

#include <config.h>

#include "empio.h"
#include "journal.h"
#include "player.h"
#include "prototypes.h"

/*
 * Receive a line of input from the current player.
 * If the player's aborted flag is set, return -1 without receiving
 * input.
 * Else receive one line and store it in CMD[SIZE].
 * This may block for input, yielding the processor.  Flush buffered
 * output when blocking, to make sure player sees the prompt.
 * If the player's connection has the I/O error or EOF indicator set,
 * or the line is "aborted", set the player's aborted flag and return
 * -1.
 * If we block and time out, set the EOF indicator on the player's
 * connection, set the player's aborted flag, and return -1.
 * If the line is "ctld", set the player's eof and aborted flag and
 * return -1.
 * Else return the length of the line.
 * Design bug: there is no way to indicate truncation of a long line.
 */
int
recvclient(char *cmd, int size)
{
    int count, res;
    time_t deadline;

    count = -1;
    while (!player->aborted) {
	/* Try to get a line of input */
	count = io_gets(player->iop, cmd, size);
	if (count >= 0) {
	    /* got it */
	    if (strcmp(cmd, "ctld") == 0)
		player->aborted = player->got_ctld = 1;
	    if (strcmp(cmd, "aborted") == 0)
		player->aborted = 1;
	    journal_input(cmd);
	    break;
	}

	/*
	 * Flush all queued output before potentially sleeping in
	 * io_input(), to make sure player sees the prompt.
	 */
	deadline = player_io_deadline(player, 0);
	while (io_output(player->iop, deadline) > 0)
	    ;

	/*
	 * Try to receive some input.  Need to recompute deadline;
	 * command abortion during io_output() might have changed it.
	 */
	deadline = player_io_deadline(player, 0);
	res = io_input(player->iop, deadline);
	if (res > 0)
	    ;
	else if (res < 0)
	    player->aborted = 1;
	else if (io_eof(player->iop))
	    player->aborted = 1;
	else if (!player->aborted) {
	    pr_flash(player, "idle connection terminated\n");
	    io_set_eof(player->iop);
	    player->aborted = 1;
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
	return -1;
    }

    player->recvfail = 0;
    return count;
}
