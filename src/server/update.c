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
 *  update.c: Update scheduler
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1996
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2007-2020
 */

#include <config.h>

#include <errno.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif
#include <time.h>
#include "chance.h"
#include "empthread.h"
#include "game.h"
#include "misc.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "server.h"

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

    update_schedule_anchor = (time(NULL) + 59) / 60 * 60;
    if (update_get_schedule() < 0)
	exit(1);

    dp = player_new(-1);
    if (!dp)
	exit_nomem();
    update_thread = empth_create(update_sched, 512 * 1024, 0, "Update", dp);
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
    int n = ARRAY_SIZE(update_time);
    int i;

    ef_truncate(EF_UPDATES, 0);
    ef_extend(EF_UPDATES, n - 1);
    if (read_schedule(schedulefil, update_time, n,
		      now + 30, update_schedule_anchor) < 0) {
	logerror("No update schedule!");
	ef_truncate(EF_UPDATES, 0);
	return -1;
    }
    logerror("Update schedule read");
    for (i = 0; update_time[i]; i++) ;
    ef_truncate(EF_UPDATES, i);
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
		next_update += roll0(update_window);
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

    for (p = player_next(NULL); p; p = player_next(p)) {
	if (p->state != PS_PLAYING)
	    continue;
	if (p->command) {
	    pr_flash(p, "Update aborting command\n");
	    p->may_sleep = PLAYER_SLEEP_NEVER;
	    p->aborted = 1;
	    empth_wakeup(p->proc);
	}
    }
    empth_rwlock_wrlock(update_lock);
    if (*pre_update_hook) {
	if (run_hook(pre_update_hook, "pre-update")) {
	    empth_rwlock_unlock(update_lock);
	    return;
	}
    }
    update_main();
    empth_rwlock_unlock(update_lock);
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
