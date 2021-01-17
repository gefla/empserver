/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  control.c: Military control functions
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Markus Armbruster, 2014-2016
 */

#include <config.h>

#include "land.h"
#include "nsc.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "unit.h"

/*
 * Return strength of security detail in @sp.
 * Store sum of efficiency of land units with security capability in
 * @seceffp unless it is null.
 */
double
security_strength(struct sctstr *sp, int *seceffp)
{
    double strength;
    int seceff;
    struct nstr_item ni;
    struct lndstr land;

    strength = sp->sct_item[I_MILIT];
    seceff = 0;
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own != sp->sct_own)
	    continue;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    continue;
	strength += land.lnd_item[I_MILIT];
	if (lchr[land.lnd_type].l_flags & L_SECURITY) {
	    strength += land.lnd_item[I_MILIT] * land.lnd_effic / 100.0;
	    seceff += land.lnd_effic;
	}
    }

    if (seceffp)
	*seceffp = seceff;
    return strength;
}

/*
 * Does the player->owner have military control of this sector?
 */
int
military_control(struct sctstr *sp)
{
    int tot_mil;

    if (sp->sct_oldown != sp->sct_own) {
	tot_mil = sp->sct_item[I_MILIT] + security_strength(sp, NULL);
	if (tot_mil * 10 < sp->sct_item[I_CIVIL])
	    return 0;
    }

    return 1;
}

/*
 * Ask user to confirm abandonment of sector @sp, if any.
 * If removing @amnt commodities of type @vtype and the land units in
 * @list would abandon the sector, ask the user to confirm.
 * All land units in @land_list must be in this sector, owned by the
 * player, and not loaded onto anything.  @land_list may be null.
 * Return zero when abandonment was declined, else non-zero.
 */
int
abandon_askyn(struct sctstr *sp, i_type vtype, int amnt,
	      struct ulist *land_list)
{
    char prompt[80];

    /*
     * First, would we be abandoning it?  If not, just return that
     * it's OK to move out.
     */
    if (!would_abandon(sp, vtype, amnt, land_list))
	return 1;

    sprintf(prompt, "Do you really want to abandon %s [yn]? ",
	    xyas(sp->sct_x, sp->sct_y, player->cnum));

    return askyn(prompt);
}

/*
 * Would removing this stuff from @sp abandon it?
 * Consider removal of @amnt commodities of type @vtype and the land
 * units in @land_list.
 * All land units in @land_list must be in this sector, owned by the
 * player, and not loaded onto anything.  @land_list may be null.
 */
int
would_abandon(struct sctstr *sp, i_type vtype, int amnt,
	      struct ulist *land_list)
{
    int mil, civs, nland;
    struct nstr_item ni;
    struct lndstr land;

    /*
     * sct_prewrite() abandons when there are no civilians, military
     * and own units left.
     */

    if (vtype != I_CIVIL && vtype != I_MILIT)
	return 0;

    mil = sp->sct_item[I_MILIT];
    civs = sp->sct_item[I_CIVIL];

    if (vtype == I_MILIT)
	mil -= amnt;
    if (vtype == I_CIVIL)
	civs -= amnt;

    if (!sp->sct_own || civs > 0 || mil > 0)
	return 0;

    /*
     * Okay, no civilians and no military would be left.  Any own land
     * units left?  Land units on ships stay, so count them.  Land
     * units not on anything stay unless in @land_list, so count them,
     * then subtract length of @land_list.  Land units on land units
     * stay if their carrier stays, and therefore won't change the
     * outcome; don't count them.
     */

    nland = 0;
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == player->cnum && land.lnd_land < 0)
	    ++nland;
    }

    if (land_list)
	nland -= emp_quelen(&land_list->queue);
    return nland <= 0;
}
