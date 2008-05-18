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
 *  game.c: Game file access
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2007-2008
 */

/*
 * On Empire Time:
 *
 * An Empire turn is terminated by an update.  The Empire clock counts
 * turns and ETUs, i.e. it ticks etu_per_update times per turn.  When
 * updates move around in real time (schedule change, downtime, etc.),
 * the Empire clock automatically adapts the duration of an ETU
 * accordingly.
 */

#include <config.h>

#include <math.h>
#include "file.h"
#include "game.h"
#include "optlist.h"
#include "prototypes.h"
#include "server.h"

/*
 * Disable updates 
 */
void
game_ctrl_update(int enable)
{
    struct gamestr *game = getgamep();

    game->game_upd_disable = !enable;
    putgame();
}

/*
 * Are updates disabled?
 */
int
updates_disabled(void)
{
    return getgamep()->game_upd_disable;
}

/*
 * Notice that a player broke sanctuary.
 * This starts the Empire clock if it hasn't been started yet.
 */
void
game_note_bsanct(void)
{
    struct gamestr *game = getgamep();

    if (game->game_rt == 0) {
	game->game_rt = time(NULL);
	putgame();
    }
}

/*
 * Record an update in the game file, the current time is NOW.
 * This starts the Empire clock if it hasn't been started yet.
 */
void
game_record_update(time_t now)
{
    struct gamestr *game = getgamep();

    game->game_turn++;
    game->game_tick = 0;
    game->game_rt = now;
    putgame();
}

/*
 * Return current duration of an ETU in seconds.
 * Note: may return HUGE_VAL when the Empire clock is not ticking.
 */
static double
secs_per_etu(struct gamestr *game)
{
    double secs;

    if (!game->game_rt || !update_time[0])
	return HUGE_VAL;	/* not started or no update scheduled */

    secs = update_time[0] - game->game_rt;
    if (secs < 0)
	return HUGE_VAL;	/* update seems to be late */
    return secs / (etu_per_update - game->game_tick);
}

/*
 * Update the Empire clock according to the current real time.
 * Return the game struct.
 */
struct gamestr *
game_tick_tick(void)
{
    struct gamestr *game = getgamep();
    double dsecs, s_p_etu;
    int detu;

    dsecs = time(NULL) - game->game_rt;
    if (CANT_HAPPEN(dsecs < 0))
	dsecs = 0;
    s_p_etu = secs_per_etu(game);
    detu = (int)(dsecs / s_p_etu);
    if (detu > 0) {
	if (CANT_HAPPEN(game->game_tick + detu > etu_per_update))
	    detu = etu_per_update - game->game_tick;
	game->game_tick += detu;
	game->game_rt += detu * s_p_etu;
	putgame();
    }

    return game;
}

/*
 * Set ETU timestamp *TICK to the current ETU time.
 * Return by how many ETUs it was increased.
 */
int
game_tick_to_now(short *tick)
{
    return game_step_a_tick(game_tick_tick(), tick);
}

/*
 * Set ETU timestamp *TICK to the ETU time recorded in the game struct.
 * The Empire clock is not updated.
 * Return by how many ETUs it was increased.
 */
int
game_step_a_tick(struct gamestr *game, short *tick)
{
    int etu;

    etu = game->game_tick - *tick;
    if (CANT_HAPPEN(etu < 0))
	etu = 0;
    *tick = game->game_tick;
    return etu;
}

/*
 * Reset ETU timestamp *TICK to zero.
 * Return how many ETUs it had left until etu_per_update.
 */
int
game_reset_tick(short *tick)
{
    int etu;

    etu = etu_per_update - *tick;
    if (CANT_HAPPEN(etu < 0))
	etu = 0;
    *tick = 0;
    return etu;
}
