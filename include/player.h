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
 *  player.h: Definitions for player information (threads)
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Doug Hay, 1998
 *     Markus Armbruster, 2005-2010
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <time.h>
#include "empthread.h"
#include "misc.h"
#include "queue.h"
#include "types.h"

	/* nstat values */
#define VIS		bit(0)
#define SANCT		(bit(1) | VIS)
#define NORM		(bit(2) | VIS)
#define GOD		(bit(3) | NORM | VIS)
#define EXEC		bit(5)
#define CAP		bit(6)
#define MONEY		bit(7)

enum player_sleep {
    PLAYER_SLEEP_NEVER, PLAYER_SLEEP_ON_INPUT, PLAYER_SLEEP_FREELY
};

struct player {
    struct emp_qelem queue;
    empth_t *proc;
    char hostaddr[46];
    char hostname[512];		/* may be empty */
    char client[128];		/* may be empty */
    char userid[32];		/* may be empty */
    int authenticated;
    natid cnum;
    int state;
    int flags;
    struct cmndstr *command;	/* currently executing command */
    struct iop *iop;
    char combuf[1024];		/* command input buffer, UTF-8 */
    char argbuf[1024];		/* argument buffer, ASCII */
    char *argp[128];		/* arguments, ASCII, valid if command */
    char *condarg;		/* conditional, ASCII, valid if command */
    char *comtail[128];		/* start of args in combuf[] */
    time_t lasttime;		/* when nat_timeused was last updated */
    int btused;
    int god;
    int owner;
    int nstat;			/* command capabilities */
    int simulation;		/* e.g. budget command */
    double dolcost;
    time_t curup;		/* when last input was received */
    enum player_sleep may_sleep; /* when may thread sleep? */
    int aborted;		/* interrupt cookie or EOF received? */
    int eof;			/* EOF (cookie or real) received? */
    int recvfail;		/* #recvclient() failures */
    int curid;			/* for pr, cur. line's id, -1 none */
    char *map;			/* pointer to in-mem map */
    char *bmap;			/* pointer to in-mem bmap */
};

#define PS_INIT		0
#define PS_PLAYING	1
#define PS_SHUTDOWN	2

/* player flags */
enum {
    PF_UTF8 = bit(0),		/* client wants UTF-8 */
    PF_DOWN = bit(1),		/* told player game is down */
    PF_HOURS = bit(2)		/* told player hours restriction is on */
};

#endif
