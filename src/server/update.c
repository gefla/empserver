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
#include "misc.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "server.h"

empth_sem_t *update_sem;
empth_rwlock_t *update_lock;
int update_pending;
time_t update_time;

static void update_sched(void *);
static void update_force(void *);
static void update_wait(void *unused);
static int run_hook(char *cmd, char *name);

void
update_init(void)
{
    struct player *dp;
    int stacksize;

    update_sem = empth_sem_create("Update", 0);
    update_lock = empth_rwlock_create("Update");
    if (!update_sem || !update_lock)
	exit_nomem();

    dp = player_new(-1);
    if (!dp)
	exit_nomem();
    /* FIXME ancient black magic; figure out true stack need */
    stacksize = 100000 +
/* finish_sects */ WORLD_X * WORLD_Y * (2 * sizeof(double) +
					sizeof(char *));
    if (!empth_create(PP_UPDATE, update_wait, stacksize, 0,
		      "Update", "Updates the world", dp))
	exit_nomem();

    if (!empth_create(PP_SCHED, update_sched, 50 * 1024, 0,
		      "UpdateSched", "Schedules updates to occur", NULL))
	exit_nomem();
}

/*ARGSUSED*/
static void
update_sched(void *unused)
{
    int wind;
    time_t now, delta;

    if (s_p_etu <= 0) {
	logerror("bad value for s_p_etu (%d)", s_p_etu);
	s_p_etu = 2 * 60;
	logerror("setting s_p_etu to %d", s_p_etu);
    }
    while (1) {
	time(&now);
	next_update_time(&now, &update_time, &delta);
	if (update_window > 0) {
	    wind = (random() % update_window);
	    update_time += wind;
	    delta += wind;
	}
	logerror("Next update at %s", ctime(&update_time));
	logerror("Next update in %ld seconds", (long)delta);
	/* sleep until update is scheduled to go off */
	empth_sleep(update_time);
	time(&now);
	now += adj_update;
	if (!gamehours(now)) {
	    logerror("No update permitted (hours restriction)");
	    continue;
	}
	if (!updatetime(&now)) {
	    logerror("No update wanted");
	    continue;
	}
	if (updates_disabled()) {
	    logerror("Updates disabled...skipping update");
	    continue;
	}
	empth_sem_signal(update_sem);
    }
    /*NOTREACHED*/
}

/*
 * Trigger an update SECS_FROM_NOW seconds from now.
 * Return 0 on success, -1 on failure.
 */
int
update_trigger(time_t secs_from_now)
{
    static time_t *secp;

    if (secs_from_now < 0)
	return -1;

    if (secs_from_now == 0) {
	empth_sem_signal(update_sem);
	return 0;
    }

    /* FIXME make triggers overwrite, not accumulate */
    secp = malloc(sizeof(time_t));
    if (!secp)
	return -1;
    *secp = secs_from_now;
    if (!empth_create(PP_SCHED, update_force, 50 * 1024, 0, "forceUpdate",
		      "Schedules an update", secp))
	return -1;
    return 0;
}

static void
update_force(void *seconds)
{
    time_t now;

    time(&now);
    empth_sleep(now + *(time_t *)seconds);
    empth_sem_signal(update_sem);
    free(seconds);
    empth_exit();
}

/*ARGSUSED*/
static void
update_wait(void *unused)
{
    struct player *p;

    player->proc = empth_self();
    player->cnum = 0;
    player->god = 1;

    while (1) {
	empth_sem_wait(update_sem);
	update_pending = 1;
	for (p = player_next(0); p != 0; p = player_next(p)) {
	    if (p->state != PS_PLAYING)
		continue;
	    if (p->command) {
		pr_flash(p, "Update aborting command\n");
		p->aborted = 1;
		empth_wakeup(p->proc);
	    }
	}
	empth_rwlock_wrlock(update_lock);
	if (*pre_update_hook) {
	    if (run_hook(pre_update_hook, "pre-update")) {
		update_pending = 0;
		empth_rwlock_unlock(update_lock);
		continue;
	    }
	}
	update_main();
	update_pending = 0;
	empth_rwlock_unlock(update_lock);
    }
    /*NOTREACHED*/
}

static int
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
