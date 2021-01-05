/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  game.h: The game file
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2007-2009
 */

#ifndef GAME_H
#define GAME_H

#include "file.h"

struct gamestr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned game_seqno: 12;
    unsigned game_generation: 12;
    int game_uid;
    time_t game_timestamp;
    /* end of part matching struct empobj */
    char game_upd_disable;	/* updates disabled? */
    char game_down;		/* playing disabled? */
    /*
     * The Empire clock.
     * Access it through game_tick_tick(), or else it'll be late.
     */
    short game_turn;		/* turn number */
    short game_tick;		/* elapsed ETUs in this turn */
    time_t game_rt;		/* when game_tick last ticked */
};

#define putgame() ef_write(EF_GAME, 0, ef_ptr(EF_GAME, 0))
#define getgamep() ((struct gamestr *)ef_ptr(EF_GAME, 0))

extern void game_ctrl_update(int);
extern int updates_disabled(void);
extern void game_ctrl_play(int);
extern int game_play_disabled(void);
extern void game_note_bsanct(void);
extern void game_record_update(time_t);
extern struct gamestr *game_tick_tick(void);
extern int game_tick_to_now(short *);
extern int game_step_a_tick(struct gamestr *, short *);
extern int game_reset_tick(short *);

#endif
