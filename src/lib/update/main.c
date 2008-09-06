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
 *  main.c: World update main function
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1996
 *     Doug Hay, 1998
 *     Markus Armbruster, 2006-2007
 */

#include <config.h>

#include "budg.h"
#include "empthread.h"
#include "game.h"
#include "journal.h"
#include "player.h"
#include "server.h"
#include "unit.h"
#include "update.h"

long money[MAXNOC];
long pops[MAXNOC];
long sea_money[MAXNOC];
long lnd_money[MAXNOC];
long air_money[MAXNOC];
long tpops[MAXNOC];

void
update_main(void)
{
    int etu = etu_per_update;
    int n;
    int x;
    struct bp *bp;
    int cn, cn2, rel;
    struct natstr *cnp;
    struct natstr *np;

    logerror("production update (%d etus)", etu);
    game_record_update(time(NULL));
    journal_update(etu);

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
    memset(pops, 0, sizeof(pops));
    memset(air_money, 0, sizeof(air_money));
    memset(sea_money, 0, sizeof(sea_money));
    memset(lnd_money, 0, sizeof(lnd_money));
    bp = bp_alloc();
    for (n = 0; n < MAXNOC; n++) {
	money[n] = 0;
	if (!(np = getnatp(n)))
	    continue;
	money[n] = np->nat_money;
	tpops[n] = count_pop(n);
    }

    logerror("preparing sectors...");
    prepare_sects(etu, bp);
    logerror("done preparing sectors.");
    logerror("producing for countries...");
    for (x = 0; x < MAXNOC; x++) {
	long p_sect[SCT_TYPE_MAX+2][2];

	memset(p_sect, 0, sizeof(p_sect));
	if (!(np = getnatp(x)))
	    continue;
	if (np->nat_stat == STAT_SANCT) {
#ifdef DEBUG
	    logerror("Country %i is in sanctuary and did not update", x);
#endif
	    continue;
	}
	np->nat_money += (int)(np->nat_reserve * money_res * etu);

	/* maintain units */
	prod_ship(etu, x, bp, 0);
	prod_plane(etu, x, bp, 0);
	prod_land(etu, x, bp, 0);

	/* produce all sects */
	produce_sect(x, etu, bp, p_sect);

	/* build units */
	prod_ship(etu, x, bp, 1);
	prod_plane(etu, x, bp, 1);
	prod_land(etu, x, bp, 1);
    }
    logerror("done producing for countries.");

    finish_sects(etu);
    prod_nat(etu);
    age_levels(etu);
    free(bp);
    if (opt_SLOW_WAR) {
	/* Update war declarations */
	/* MOBILIZATION->SITZKRIEG->AT_WAR */
	for (cn = 1; cn < MAXNOC; cn++) {
	    if ((cnp = getnatp(cn)) == 0)
		break;
	    for (cn2 = 1; cn2 < MAXNOC; cn2++) {
		if (cn2 == cn)
		    continue;
		rel = getrel(cnp, cn2);
		if (rel == MOBILIZATION) {
		    rel = SITZKRIEG;
		    setrel(cn, cn2, rel);
		} else if (rel == SITZKRIEG) {
		    rel = AT_WAR;
		    setrel(cn, cn2, rel);
		}
	    }
	}
    }
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
    /* Clear all the telegram flags */
    for (cn = 0; cn < MAXNOC; cn++)
	clear_telegram_is_new(cn);
    logerror("End update");
}
