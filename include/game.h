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
 *  game.h: The game file
 * 
 *  Known contributors to this file:
 *       Markus Armbruster, 2007
 */

#ifndef GAME_H
#define GAME_H

#include <time.h>

struct gamestr {
    /* initial part must match struct empobj */
    short ef_type;
    /* end of part matching struct empobj */
    char game_upd_disable;	/* updates disabled? */
    /*
     * The Empire clock.
     * Access it through game_tick_tick(), or else it'll be late.
     */
    short game_turn;		/* turn number */
    short game_tick;		/* elapsed etus in this turn */
    time_t game_rt;		/* when game_tick last ticked */
};

#define putgame() ef_write(EF_GAME, 0, ef_ptr(EF_GAME, 0))
#define getgamep() ((struct gamestr *)ef_ptr(EF_GAME, 0))

extern void game_ctrl_update(int);
extern int updates_disabled(void);
extern void game_record_update(time_t);
extern struct gamestr *game_tick_tick(void);
extern int game_tick_to_now(short *);
extern int game_step_a_tick(struct gamestr *, short *);
extern int game_reset_tick(short *);

#endif
