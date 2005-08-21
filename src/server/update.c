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
 *  update.c: Update scheduler
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1996
 */

#include <errno.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif
#include "misc.h"
#include "player.h"
#include "empthread.h"
#include "prototypes.h"
#include "optlist.h"
#include "server.h"

empth_sem_t *update_sem;
time_t update_time;

static void update_wait(void *unused);
static int run_hook(char *cmd, char *name);

/*ARGSUSED*/
void
update_sched(void *unused)
{
    int wind;
    time_t now, delta;

    update_sem = empth_sem_create("Update", 0);
    empth_create(PP_SCHED, update_wait, (50 * 1024), 0, "UpdateWait",
		 "Waits until players idle", 0);
    time(&now);
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

void
update_force(void *seconds)
{
    time_t now;

    time(&now);
    empth_sleep(now + *(int *)seconds);
    empth_sem_signal(update_sem);
    empth_exit();
}

/*ARGSUSED*/
static void
update_wait(void *unused)
{
    struct player *p;
    int running;
    time_t now;
    int stacksize;
    struct player *dp;

    while (1) {
	empth_sem_wait(update_sem);
	update_pending = 1;
	running = 0;
	for (p = player_next(0); p != 0; p = player_next(p)) {
	    if (p->state != PS_PLAYING)
		continue;
	    if (p->command) {
		pr_flash(p, "Update aborting command\n");
		p->aborted = 1;
		empth_wakeup(p->proc);
		running++;
	    }
	}
	time(&now);
	if (running) {
	    /* sleep a few, wait for aborts to take effect */
	    empth_sleep(now + 2);
	}
	if (*pre_update_hook) {
	    if (run_hook(pre_update_hook, "pre-update")) {
		update_pending = 0;
		continue;
	    }
	}
	/* 
	 * we rely on the fact that update's priority is the highest
	 * in the land so it can finish before it yields.
	 */
	dp = player_new(0, 0);
	if (!dp) {
	    logerror("can't create dummy player for update");
	    update_pending = 0;
	    continue;
	}
	stacksize = 100000 +
/* finish_sects */ WORLD_X * WORLD_Y * (2 * sizeof(double) +
					sizeof(s_char *));

	empth_create(PP_UPDATE, update_main, stacksize, 0,
		     "UpdateRun", "Updates the world", dp);
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
