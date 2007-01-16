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
 *  server.h: Server startup, control and shutdown
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2004
 */

#ifndef SERVER_H
#define SERVER_H

#include "empthread.h"

extern int shutdown_pending;
extern int update_pending;
extern empth_sem_t *update_sem;
extern empth_rwlock_t *update_lock;
extern time_t update_time;
extern int updating_mob;

void mobility_init(void);

/* thread entry points */
void delete_lostitems(void *);
void market_update(void *);
void mobility_check(void *);
void player_kill_idle(void *);
void update_main(void);
void update_init(void);
void shutdown_sequence(void *);
void update_force(void *);

#endif
