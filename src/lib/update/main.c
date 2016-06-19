/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  main.c: World update main function
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1996
 *     Doug Hay, 1998
 *     Markus Armbruster, 2006-2016
 */

#include <config.h>

#include <sys/resource.h>
#include "empthread.h"
#include "file.h"
#include "game.h"
#include "journal.h"
#include "optlist.h"
#include "prototypes.h"
#include "server.h"
#include "unit.h"
#include "update.h"

struct budget nat_budget[MAXNOC];
int pops[MAXNOC];
int tpops[MAXNOC];

void
update_main(void)
{
    int etu = etu_per_update;
    struct rusage rus1, rus2;
    int n;
    int i;
    struct natstr *np;

    logerror("production update (%d etus)", etu);
    getrusage(RUSAGE_SELF, &rus1);
    game_record_update(time(NULL));
    journal_update(etu);
    /* Ensure back-to-back production reports are separate: */
    for (n = 0; n < MAXNOC; n++)
	clear_telegram_is_new(n);

    /* First, make sure all mobility is updated correctly. */
    if (opt_MOB_ACCESS) {
	mob_ship();
	mob_sect();
	mob_plane();
	mob_land();
    }

    if (opt_AUTO_POWER)
	update_power();

    /*
     * set up all the variables which get used in the
     * sector production routine (for producing education,
     * happiness, and printing out the state of the nation)
     */
    memset(nat_budget, 0, sizeof(nat_budget));
    memset(pops, 0, sizeof(pops));
    for (n = 0; n < MAXNOC; n++) {
	if (!(np = getnatp(n)))
	    continue;
	nat_budget[n].start_money = nat_budget[n].money = np->nat_money;
	tpops[n] = count_pop(n);
    }

    logerror("preparing sectors...");
    prepare_sects(etu);
    logerror("done preparing sectors.");
    logerror("producing for countries...");
    for (i = 0; i < MAXNOC; i++) {
	if (!(np = getnatp(i)))
	    continue;
	if (np->nat_stat == STAT_SANCT) {
	    continue;
	}

	/* maintain units */
	prod_ship(etu, i, NULL, 0);
	prod_plane(etu, i, NULL, 0);
	prod_land(etu, i, NULL, 0);

	/* produce all sects */
	produce_sect(np, etu, NULL);

	/* build units */
	prod_ship(etu, i, NULL, 1);
	prod_plane(etu, i, NULL, 1);
	prod_land(etu, i, NULL, 1);
    }
    logerror("done producing for countries.");

    finish_sects(etu);
    prod_nat(etu);
    age_levels(etu);

    /* Only update mobility for non-MOB_ACCESS here, since it doesn't
       get done for MOB_ACCESS anyway during the update */
    if (!opt_MOB_ACCESS) {
	mob_ship();
	mob_sect();
	mob_plane();
	mob_land();
    }
    if (update_demand == UPD_DEMAND_SCHED
	|| update_demand == UPD_DEMAND_ASYNC)
	update_removewants();
    /* flush all mem file objects to disk */
    ef_flush(EF_NATION);
    ef_flush(EF_SECTOR);
    ef_flush(EF_SHIP);
    ef_flush(EF_PLANE);
    ef_flush(EF_LAND);
    unit_cargo_init();
    delete_old_announcements();
    delete_old_news();
    delete_old_lostitems();
    getrusage(RUSAGE_SELF, &rus2);
    logerror("End update %g user %g system",
	     rus2.ru_utime.tv_sec + rus2.ru_utime.tv_usec / 1e6
	     - (rus1.ru_utime.tv_sec + rus1.ru_utime.tv_usec / 1e6),
	     rus2.ru_stime.tv_sec + rus2.ru_stime.tv_usec / 1e6
	     - (rus1.ru_stime.tv_sec + rus1.ru_stime.tv_usec / 1e6));
}
