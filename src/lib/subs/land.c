/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  land.c: Land unit post-read and pre-write
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1996
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "file.h"
#include "land.h"
#include "ship.h"
#include "xy.h"
#include "nat.h"
#include "deity.h"
#include "nsc.h"
#include "plane.h"
#include "prototypes.h"
#include "optlist.h"

int
lnd_postread(int n, s_char *ptr)
{
    struct	lndstr *llp = (struct lndstr *) ptr;
    struct	shpstr theship;
    struct  lndstr theland;
    
	if (llp->lnd_uid != n) {
		logerror("lnd_postread: Error - %d != %d, zeroing.\n", llp->lnd_uid, n);
		bzero(ptr, sizeof(struct lndstr));
	}
    if (llp->lnd_ship >= 0 && llp->lnd_own && llp->lnd_effic >= LAND_MINEFF) {
		if (getship(llp->lnd_ship, &theship) &&
			(theship.shp_effic >= SHIP_MINEFF)) {
			/* wooof!  Carriers are a pain */
			if (llp->lnd_mission){
				/*
				 *  If the unit is on a mission centered
				 *  on it's loc, the op-area travels with
				 *  the unit.
				 */
				if ((llp->lnd_opx == llp->lnd_x) &&
					(llp->lnd_opy == llp->lnd_y)) {
					llp->lnd_opx = theship.shp_x;
					llp->lnd_opy = theship.shp_y;
				}
			}
			if (llp->lnd_x != theship.shp_x || llp->lnd_y != theship.shp_y)
				time(&llp->lnd_timestamp);
			llp->lnd_x = theship.shp_x;
			llp->lnd_y = theship.shp_y;
		}
    }
    if (llp->lnd_land >= 0 && llp->lnd_own && llp->lnd_effic >= LAND_MINEFF) {
		if (getland(llp->lnd_land, &theland) &&
			(theland.lnd_effic >= LAND_MINEFF)) {
			/* wooof!  Carriers are a pain */
			if (llp->lnd_mission){
				/*
				 *  If the unit is on a mission centered
				 *  on it's loc, the op-area travels with
				 *  the unit.
				 */
				if ((llp->lnd_opx == llp->lnd_x) &&
					(llp->lnd_opy == llp->lnd_y)){
					llp->lnd_opx = theland.lnd_x;
					llp->lnd_opy = theland.lnd_y;
				}
			}
			if (llp->lnd_x != theland.lnd_x || llp->lnd_y != theland.lnd_y)
				time(&llp->lnd_timestamp);
			llp->lnd_x = theland.lnd_x;
			llp->lnd_y = theland.lnd_y;
		}
    }
    if (opt_MOB_ACCESS)
		lnd_do_upd_mob(llp);
    
    player->owner = (player->god || llp->lnd_own == player->cnum);
    return 1;
}

/*ARGSUSED*/
int
lnd_prewrite(int n, s_char *ptr)
{
    struct	lndstr *llp = (struct lndstr *) ptr;
    struct	lndstr land;
    struct lndstr *lp;
    struct plnstr *pp;
    int i;
    
    llp->ef_type = EF_LAND;
    llp->lnd_uid = n;
    
    time(&llp->lnd_timestamp);

    if (llp->lnd_own && llp->lnd_effic < LAND_MINEFF) {
		makelost(EF_LAND, llp->lnd_own, llp->lnd_uid, llp->lnd_x, llp->lnd_y);
		llp->lnd_own = 0;
		
		getland(n, &land);
		
		for (i = 0; NULL != (lp = getlandp(i)); i++) {
			if (lp->lnd_own && lp->lnd_land == n) {
				mpr(lp->lnd_own, "%s MIA!\n", prland(lp));
				makelost(EF_LAND, lp->lnd_own, lp->lnd_uid, lp->lnd_x, lp->lnd_y);
				lp->lnd_own = 0;
				lp->lnd_effic = 0;
				lp->lnd_ship = -1;
				lp->lnd_land = -1;
				putland(lp->lnd_uid, lp);
			}
		}
		for (i = 0; NULL != (pp = getplanep(i)); i++) {
			if (pp->pln_own && pp->pln_land == n) {
				mpr(pp->pln_own, "%s MIA!\n", prplane(pp));
				makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x, pp->pln_y);
				pp->pln_own = 0;
				pp->pln_effic = 0;
				pp->pln_ship = -1;
				pp->pln_land = -1;
				putplane(pp->pln_uid, pp);
			}
		}
    } else
		getland(n, &land);
    
    return 1;
}

void
lnd_init(int n, s_char *ptr)
{
	struct	lndstr *lp = (struct lndstr *) ptr;

	lp->ef_type = EF_LAND;
	lp->lnd_uid = n;
	lp->lnd_own = 0;
}

s_char *
prland(struct lndstr *lp)
{
	return prbuf("%s #%d", lchr[(int)lp->lnd_type].l_name, lp->lnd_uid);
}
