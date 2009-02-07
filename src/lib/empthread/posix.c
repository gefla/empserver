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
 *  posix.c: Thread-related code common to POSIX systems
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006
 */

#include <config.h>

#include <signal.h>
#include <unistd.h>
#include "empthread.h"
#include "journal.h"
#include "prototypes.h"

static void panic(int sig);

void
empth_init_signals(void)
{
    struct sigaction act;

    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = panic;
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);
}

/* we're going down.  try to close the files at least */
static void
panic(int sig)
{
    struct sigaction act;

    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_DFL;
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGFPE, &act, NULL);

    /*
     * This code calls functions that are not safe to call from a
     * signal handler!  That could probably be rectified with some
     * effort.  However, we're already in a bad state.  Is it wise to
     * flush that state to disk, possibly overwriting good state?
     * FIXME make the code safe as far as practical
     */
    logerror("server received fatal signal %d", sig);
    log_last_commands();
    ef_fin_srv();
    journal_shutdown();
    /* End of unsafe code */

    if (CANT_HAPPEN(sig != SIGBUS && sig != SIGSEGV
		    && sig != SIGILL && sig != SIGFPE))
	_exit(1);
    if (raise(sig))
	_exit(1);
}
