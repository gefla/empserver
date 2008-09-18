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
 *  update.c: Update scheduler
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1996
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include <errno.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif
#include <time.h>
#include "empthread.h"
#include "game.h"
#include "misc.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "server.h"

/*
 * Update is running.
 * Can be used to suppress messages, or direct them to bulletins.
 */
int update_running;

static time_t update_schedule_anchor;
static int update_wanted;

static empth_t *update_thread;

static int update_get_schedule(void);
static void update_sched(void *);
static void update_run(void);

void
update_init(void)
{
    struct player *dp;
    int stacksize;

    update_schedule_anchor = (time(NULL) + 59) / 60 * 60;
    if (update_get_schedule() < 0)
	exit(1);

    play_lock = empth_rwlock_create("Update");
    if (!play_lock)
	exit_nomem();

    dp = player_new(-1);
    if (!dp)
	exit_nomem();
    /* FIXME ancient black magic; figure out true stack need */
    stacksize = 100000 +
/* finish_sects */ WORLD_X * WORLD_Y * (2 * sizeof(double) +
					sizeof(char *));
    update_thread = empth_create(update_sched, stacksize, 0,
				 "Update", dp);
    if (!update_thread)
	exit_nomem();
}

/*
 * Get the schedule for future updates into update_time[].
 * Return 0 on success, -1 on failure.
 */
static int
update_get_schedule(void)
{
    time_t now = time(NULL);

    if (read_schedule(schedulefil, update_time,
		      sizeof(update_time) / sizeof(*update_time),
		      now + 30, update_schedule_anchor) < 0) {
	logerror("No update schedule!");
	update_time[0] = 0;
	return -1;
    }
    logerror("Update schedule read");
    return 0;
}

/*ARGSUSED*/
static void
update_sched(void *unused)
{
    time_t next_update, now;

    player->proc = empth_self();
    player->cnum = 0;
    player->god = 1;

    for (;;) {
	/*
	 * Sleep until the next scheduled update or an unscheduled
	 * wakeup.
	 */
	next_update = update_time[0];
	if (next_update) {
	    if (update_window > 0)
		next_update += random() % update_window;
	    logerror("Next update at %s", ctime(&next_update));
	    /* sleep until update is scheduled to go off */
	    empth_sleep(next_update);
	} else {
	    logerror("No update scheduled");
	    /* want to sleep forever, but empthread doesn't provide that */
	    while (empth_sleep(time(NULL) + (60 * 60 * 24)) >= 0) ;
	}

	now = time(NULL);
	if (next_update != 0 && now >= next_update) {
	    /* scheduled update time reached */
	    if (now >= next_update + 60)
		logerror("Missed the update!");
	    else if (update_demand == UPD_DEMAND_SCHED && !demand_check())
		;
	    else if (updates_disabled())
		logerror("Updates disabled...skipping update");
	    else
		update_wanted = 1;
	    update_schedule_anchor = update_time[0];
	}
	/* else unscheduled update if update_wanted is set */

	if (update_wanted) {
	    update_wanted = 0;
	    update_run();
	}

	update_get_schedule();
    }
    /*NOTREACHED*/
}

/*
 * Trigger an update.
 * Return 0 on success, -1 on failure.
 */
int
update_trigger(void)
{
    logerror("Triggering unscheduled update");
    update_wanted = 1;
    empth_wakeup(update_thread);
    return 0;
}

/*
 * Reload the update schedule.
 * Return 0 on success, -1 on failure.
 */
int
update_reschedule(void)
{
    empth_wakeup(update_thread);
    return 0;
}

static void
update_run(void)
{
    struct player *p;

    play_wrlock_wanted = 1;
    for (p = player_next(0); p != 0; p = player_next(p)) {
	if (p->state != PS_PLAYING)
	    continue;
	if (p->command) {
	    pr_flash(p, "Update aborting command\n");
	    p->aborted = 1;
	    empth_wakeup(p->proc);
	}
    }
    empth_rwlock_wrlock(play_lock);
    if (*pre_update_hook) {
	if (run_hook(pre_update_hook, "pre-update")) {
	    play_wrlock_wanted = 0;
	    empth_rwlock_unlock(play_lock);
	    return;
	}
    }
    update_running = 1;
    update_main();
    play_wrlock_wanted = update_running = 0;
    empth_rwlock_unlock(play_lock);
}

int
run_hook(char *cmd, char *name)
{
    int status;

    fflush(NULL);

    status = system(cmd);
    if (status == 0)
	;			/* successful exit */
    else if (status == -1)
	logerror("couldn't execute command processor for %s hook (%s)",
		 name, strerror(errno));
#ifndef _WIN32
    else if (WIFEXITED(status))
	logerror("%s hook terminated unsuccessfully (exit status %d)",
		 name, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
	logerror("%s hook terminated abnormally (signal %d)",
		 name, WTERMSIG(status));
#endif
    else if (status)
	logerror("%s hook terminated strangely (status %d)",
		 name, status);
    return status;
}
