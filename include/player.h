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
 *  player.h: Definitions for player information (threads)
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Doug Hay, 1998
 */

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <sys/types.h>
#if !defined(_WIN32)
#include <netinet/in.h>
#endif
#include "queue.h"
#include "misc.h"
#include "com.h"
#include "empthread.h"

struct player {
    struct emp_qelem queue;
    empth_t *proc;
    char hostaddr[32];
    char hostname[512];		/* may be empty */
    char client[128];		/* may be empty */
    char userid[32];		/* may be empty */
    int validated;
    natid cnum;
    int state;
    int flags;
    struct cmndstr *command;
    struct iop *iop;
    char combuf[1024];		/* command input buffer, UTF-8 */
    char *argp[128];		/* arguments, ASCII */
    char *condarg;		/* conditional, ASCII */
    time_t lasttime;
    int ncomstat;
    int minleft;
    int btused;
    int visitor;
    int god;
    int owner;
    int nstat;
    int waiting;
    int simulation;		/* e.g. budget command */
    double dolcost;
    int broke;
    time_t curup;		/* used in calc of minutes used */
    int aborted;
    int curid;			/* for pr, cur. line's id, -1 none */
    int blitz_time;
    char *map;			/* pointer to in-mem map */
    char *bmap;			/* pointer to in-mem bmap */
};

#define PS_INIT		0
#define PS_LOGIN	1
#define PS_PLAYING	2
#define PS_SHUTDOWN	3
#define PS_KILL		4

/* player flags */
enum {
    PF_UTF8 = bit(0)			/* client wants UTF-8 */
};

#endif /* _PLAYER_H_ */
