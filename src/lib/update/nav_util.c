/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nav_util.c: Utilities for autonav and sail
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"

#include <ctype.h>
#include "plague.h"
#include "ship.h"
#include "plane.h"
#include "land.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "path.h"
#include "file.h"
#include "item.h"
#include "optlist.h"
#include "player.h"
#include "update.h"
#include "subs.h"
#include "common.h"
#include "gen.h"

/* load a specific ship given its 
 * location and what field to modify.
 * new autonav code
 * Chad Zabel 6/1/94 
 */
int
load_it(struct shpstr *sp, struct sctstr *psect, int i)
{
    int shipown, amount, ship_amt, sect_amt;
    int abs_max, max_amt, transfer;
    i_type comm;
    struct mchrstr *vship;

    amount = sp->shp_lend[i];
    shipown = sp->shp_own;
    comm = sp->shp_tend[i];
    if (CANT_HAPPEN(comm <= I_NONE || comm > I_MAX))
	return 0;

    ship_amt = sp->shp_item[comm];
    sect_amt = psect->sct_item[comm];

    /* check for disloyal civilians */
    if (psect->sct_oldown != shipown && comm == I_CIVIL) {
	wu(0, shipown,
	   "Ship #%d - unable to load disloyal civilians at %s.",
	   sp->shp_uid, xyas(psect->sct_x, psect->sct_y, psect->sct_own));
	return 0;
    }
    if (comm == I_CIVIL || comm == I_MILIT)
	sect_amt--;		/* leave 1 civ or mil to hold the sector. */
    vship = &mchr[(int)sp->shp_type];
    abs_max = max_amt = vship->m_item[comm];

    if (!abs_max)
	return 0;		/* can't load the ship, skip to the end. */

    max_amt = MIN(sect_amt, max_amt - ship_amt);
    if (max_amt <= 0 && (ship_amt != abs_max)) {
	sp->shp_autonav |= AN_LOADING;
	return 0;
    }


    transfer = amount - ship_amt;
    if (transfer > sect_amt) {	/* not enough in the   */
	transfer = sect_amt;	/* sector to fill the  */
	sp->shp_autonav |= AN_LOADING;	/* ship, set load flag */
    }
    if (ship_amt + transfer > abs_max)	/* Do not load more    */
	transfer = abs_max - ship_amt;	/* then the max alowed */
    /* on the ship.        */

    if (transfer <= 0)
	return 0;		/* nothing to move */


    sp->shp_item[comm] = ship_amt + transfer;
    if (comm == I_CIVIL || comm == I_MILIT)
	sect_amt++;		/*adjustment */
    psect->sct_item[comm] = sect_amt - transfer;

    /* deal with the plague */
    if (psect->sct_pstage == PLG_INFECT && sp->shp_pstage == PLG_HEALTHY)
	sp->shp_pstage = PLG_EXPOSED;
    if (sp->shp_pstage == PLG_INFECT && psect->sct_pstage == PLG_HEALTHY)
	psect->sct_pstage = PLG_EXPOSED;

    return 1;			/* we did someloading return 1 to keep */
    /* our loop happy in nav_ship()        */

}

/* unload_it 
 * A guess alot of this looks like load_it but because of its location
 * in the autonav code I had to split the 2 procedures up.
 * unload_it dumps all the goods from the ship to the harbor.
 * ONLY goods in the trade fields will be unloaded.
 * new autonav code
 * Chad Zabel 6/1/94  
 */
void
unload_it(struct shpstr *sp)
{
    struct sctstr *sectp;
    int i;
    int landowner;
    int shipown;
    i_type comm;
    int sect_amt;
    int ship_amt;
    int max_amt;

    sectp = getsectp(sp->shp_x, sp->shp_y);

    landowner = sectp->sct_own;
    shipown = sp->shp_own;

    for (i = 0; i < TMAX; ++i) {
	if (sp->shp_tend[i] == I_NONE || sp->shp_lend[i] == 0)
	    continue;
	if (landowner == 0)
	    continue;
	if (sectp->sct_type != SCT_HARBR)
	    continue;

	comm = sp->shp_tend[i];
	if (CANT_HAPPEN(comm <= I_NONE || comm > I_MAX))
	    continue;
	ship_amt = sp->shp_item[comm];
	sect_amt = sectp->sct_item[comm];

	/* check for disloyal civilians */
	if (sectp->sct_oldown != shipown && comm == I_CIVIL) {
	    wu(0, sp->shp_own,
	       "Ship #%d - unable to unload civilians into a disloyal sector at %s.",
	       sp->shp_uid, xyas(sectp->sct_x, sectp->sct_y,
				 sectp->sct_own));
	    continue;
	}
	if (comm == I_CIVIL)
	    ship_amt--;		/* This leaves 1 civs on board the ship */

	max_amt = MIN(ship_amt, ITEM_MAX - sect_amt);
	if (max_amt <= 0)
	    continue;

	sp->shp_item[comm] = ship_amt - max_amt;
	sectp->sct_item[comm] = sect_amt + max_amt;

	if (sectp->sct_pstage == PLG_INFECT && sp->shp_pstage == PLG_HEALTHY)
	    sp->shp_pstage = PLG_EXPOSED;
	if (sp->shp_pstage == PLG_INFECT && sectp->sct_pstage == PLG_HEALTHY)
	    sectp->sct_pstage = PLG_EXPOSED;
    }
}

/* auto_fuel_ship 
 * Assume a check for fuel=0 has already been made and passed.  
 * Try to fill a ship using petro. and then oil.            
 * new autonav code.
 * This should be merged with the fuel command someday. 
 * Chad Zabel 6/1/94
 */

void
auto_fuel_ship(struct shpstr *sp)
{
    double d;
    int totalfuel = 0;
    int need;
    int maxfuel;
    int newfuel = 0;
    int add_fuel = 0;

    if (opt_FUEL == 0)
	return;
    getship(sp->shp_uid, sp);	/* refresh */
    /* fill with petro */
    maxfuel = mchr[(int)sp->shp_type].m_fuelc;
    d = (double)maxfuel / 5.0;
    if ((d - (int)d > 0.0))
	d++;
    need = (int)d;

    newfuel = supply_commod(sp->shp_own, sp->shp_x,
			    sp->shp_y, I_PETROL, need);
    add_fuel += newfuel * 5;
    if (add_fuel > maxfuel)
	add_fuel = maxfuel;
    sp->shp_fuel += add_fuel;
    totalfuel += add_fuel;

    if (totalfuel == maxfuel) {
	putship(sp->shp_uid, sp);
	return;			/* the ship is full */
    }
    add_fuel = 0;
    /* fill with oil */
    d = (double)(maxfuel - totalfuel) / 50.0;
    if ((d - (int)d > 0.0))
	d++;
    need = (int)d;

    newfuel = supply_commod(sp->shp_own, sp->shp_x,
			    sp->shp_y, I_OIL, need);
    add_fuel = newfuel * 50;
    if (add_fuel > maxfuel)
	add_fuel = maxfuel;
    sp->shp_fuel += add_fuel;
    putship(sp->shp_uid, sp);
}
