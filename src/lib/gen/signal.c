/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sig.c: block and unblock signals for critical sections
 * 
 *  Known contributors to this file:
 *     Doug Hay, 1998
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "gen.h"
#include <signal.h>

u_int mask;

void
blocksig(void)
{
#if !defined(_WIN32)
    mask = sigsetmask(0xffffffff);
#endif
}

void
unblocksig(void)
{
#if !defined(_WIN32)
    sigsetmask(mask);
#endif
}

#ifdef sys5
const s_char *sys_siglist[] = {
    "no signal",
    "interrupt",
    "quit",
    "illegal instruction",
    "trace trap",
    "IOT instruction",
    "system crash imminent",
    "floating point exception",
    "kill",
    "bus error",
    "segmentation violation",
    "bad argument to system call",
    "write on a pipe with no one to read it",
    "alarm clock",
    "software termination",
    "user defined signal 1",
    "user defined signal 2",
    "death of a child",
    "power-fail restart",
    "asychronous i/o",
    "PTY read/write",
    "I/O intervention required",
    "monitor mode granted",
    "monitor mode retracted",
    "sound ack",
    "data pending",
};
#else
#if (!defined __ppc__) && (!defined linux) && (!defined FBSD) && (!defined __linux__)
/* linux and osx declare sys_siglist in signal.h */
extern s_char *sys_siglist[];
#endif /* linux */
#endif /* sys5 */

const s_char *
signame(int sig)
{
#ifdef POSIX_SIGNALS
    if (sig <= 0 || sig > _sys_nsig)
	return "bad signal";
    return _sys_siglist[sig];
#else  /* POSIX_SIGNALS */
    if (sig <= 0 || sig > NSIG)
	return "bad signal";
    return sys_siglist[sig];
#endif /* POSIX_SIGNALS */
}
