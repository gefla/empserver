/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  torp.c: Fire torpedoes at enemy ships
 *
 *  Known contributors to this file:
 *     Dave Pare
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 2000
 *     Markus Armbruster, 2004-2015
 */

#include <config.h>

#include "chance.h"
#include "commands.h"
#include "news.h"
#include "retreat.h"
#include "ship.h"

static void anti_torp(int f, int ntorping, int vshipown);
static void fire_dchrg(struct shpstr *, struct shpstr *, int);
static int fire_torp(struct shpstr *, struct shpstr *, int);

int
torp(void)
{
    natid vshipown;
    int range;
    int dam;
    int subno;
    int victno;
    int erange;
    double hitchance;
    struct shpstr vship;
    struct shpstr sub;
    struct mchrstr *sub_mcp;
    char *ptr;
    struct nstr_item nbst;
    char buf[1024];
    int ntorping = 0;
    char prompt[128];

    if (!snxtitem(&nbst, EF_SHIP, player->argp[1], "From ship(s)? "))
	return RET_SYN;
    while (nxtitem(&nbst, &sub)) {
	if (sub.shp_own != player->cnum)
	    continue;
	if ((mchr[(int)sub.shp_type].m_flags & M_TORP) == 0)
	    continue;
	if (sub.shp_item[I_GUN] == 0
	    || sub.shp_item[I_SHELL] < SHP_TORP_SHELLS)
	    continue;
	if (sub.shp_item[I_MILIT] < 1)
	    continue;
	if (sub.shp_effic < 60)
	    continue;
	if (sub.shp_mobil <= 0)
	    continue;
	ntorping++;
    }
    pr("%d ships are eligible to torpedo\n", ntorping);
    snxtitem_rewind(&nbst);
    while (nxtitem(&nbst, &sub)) {
	if (!sub.shp_own)
	    continue;
	if (sub.shp_own != player->cnum) {
	    continue;
	}
	sub_mcp = &mchr[sub.shp_type];
	if (!(sub_mcp->m_flags & M_TORP)) {
	    pr("Ship # %d: A %s can't fire torpedoes!\n",
	       sub.shp_uid, sub_mcp->m_name);
	    continue;
	}
	if (sub.shp_item[I_GUN] == 0
	    || sub.shp_item[I_SHELL] < SHP_TORP_SHELLS) {
	    pr("Ship #%d has insufficient armament\n", sub.shp_uid);
	    continue;
	}
	if (sub.shp_item[I_MILIT] < 1) {
	    pr("Ship #%d has insufficient crew\n", sub.shp_uid);
	    continue;
	}
	if (sub.shp_effic < 60) {
	    pr("Ship #%d torpedo tubes inoperative.\n", sub.shp_uid);
	    continue;
	}
	if (sub.shp_mobil <= 0) {
	    pr("Ship #%d has insufficient mobility\n", sub.shp_uid);
	    continue;
	}
	subno = sub.shp_uid;
	sprintf(prompt, "Ship %d, target? ", sub.shp_uid);
	if (!(ptr = getstarg(player->argp[2], prompt, buf)))
	    return RET_SYN;
	if (!check_ship_ok(&sub))
	    return RET_FAIL;
	if ((victno = atoi(ptr)) < 0)
	    return RET_SYN;
	if (!getship(victno, &vship))
	    return RET_FAIL;
	if (!vship.shp_own)
	    return RET_FAIL;
	vshipown = vship.shp_own;
	if (victno == subno) {
	    pr("Shooting yourself, eh?  How strange...\n");
	    continue;
	}
	if (mchr[(int)vship.shp_type].m_flags & M_SUB) {
	    if (!(sub_mcp->m_flags & M_SUBT)) {
		pr("You can't torpedo a submarine!\n");
		continue;
	    }
	}
	dam = shp_torp(&sub, 1);
	sub.shp_mission = 0;
	putship(sub.shp_uid, &sub);
	if (CANT_HAPPEN(dam < 0)) {
	    pr("Ship #%d has insufficient armament\n", sub.shp_uid);
	    continue;
	}

	if (!(sub_mcp->m_flags & M_SUB)) {
	    pr("Starting our attack run...\n");
	    anti_torp(sub.shp_uid, ntorping, vshipown);
	}
	getship(sub.shp_uid, &sub);
	if (sub.shp_own == 0)
	    continue;

	erange = roundrange(torprange(&sub));
	pr("Effective torpedo range is %d.0\n", erange);
	pr("Whooosh... ");
	getship(victno, &vship);
	vshipown = vship.shp_own;
	range = mapdist(sub.shp_x, sub.shp_y, vship.shp_x, vship.shp_y);
	hitchance = shp_torp_hitchance(&sub, range);
	if (range <= erange) {
	    pr("Hitchance = %d%%\n", (int)(hitchance * 100));
	}
	if (range > erange)
	    pr("Out of range\n");
	else if (!line_of_sight(NULL, sub.shp_x, sub.shp_y,
				vship.shp_x, vship.shp_y)) {
	    pr("BOOM!... Torpedo slams into land before reaching target.\n");
	    /* We only tell the victim if we were within range. */
	    if (vshipown != 0 && vshipown != player->cnum)
		wu(0, vshipown, "Torpedo sighted @ %s by %s\n",
		   xyas(sub.shp_x, sub.shp_y, vshipown),
		   prship(&vship));
	} else if (chance(hitchance)) {
	    pr("BOOM!...\n");
	    if (vshipown != 0 && vshipown != player->cnum)
		wu(0, vshipown, "%s in %s torpedoed %s for %d damage.\n",
		   sub_mcp->m_flags & M_SUB ? "sub" : prship(&sub),
		   xyas(sub.shp_x, sub.shp_y, vshipown),
		   prship(&vship), dam);
	    pr("Torpedo hit %s for %d damage.\n", prsub(&vship), dam);
	    if (!(mchr[vship.shp_type].m_flags & M_SUB)) {
		if (mchr[sub.shp_type].m_flags & M_SUB)
		    nreport(vshipown, N_TORP_SHIP, 0, 1);
		else
		    nreport(vshipown, N_SHIP_TORP, player->cnum, 1);
	    }
	    shipdamage(&vship, dam);
	    if (vship.shp_effic < SHIP_MINEFF)
		pr("%s sunk!\n", prsub(&vship));
	    putship(vship.shp_uid, &vship);
	    if (vship.shp_rflags & RET_TORPED)
		retreat_ship(&vship, vshipown, 't');
	} else {
	    pr("Missed\n");
	    if (vshipown != 0 && vshipown != player->cnum)
		wu(0, vshipown, "Torpedo sighted @ %s by %s\n",
		   xyas(sub.shp_x, sub.shp_y, vshipown), prship(&vship));
	}

	if (sub_mcp->m_flags & M_SUB)
	    anti_torp(sub.shp_uid, ntorping, vshipown);
    }
    return RET_OK;
}

static void
anti_torp(int f, int ntorping, int vshipown)
{
    struct shpstr sub;
    struct shpstr dd;
    int x;

    getship(f, &sub);

    if (sub.shp_own == vshipown)
	return;

    x = 0;
    while (getship(x++, &dd) && sub.shp_effic >= SHIP_MINEFF) {
	if (dd.shp_own == 0)
	    continue;
	if (dd.shp_own != vshipown)
	    continue;

	if (!fire_torp(&dd, &sub, ntorping))
	    fire_dchrg(&dd, &sub, ntorping);
    }
}

static void
fire_dchrg(struct shpstr *sp, struct shpstr *targ, int ntargets)
{
    int range, erange, dam;

    erange = roundrange(shp_fire_range(sp));
    range = mapdist(sp->shp_x, sp->shp_y, targ->shp_x, targ->shp_y);
    if (range > erange)
	return;

    if ((mchr[(int)targ->shp_type].m_flags & M_SUB) == 0) {
	/* Return fire to a torpedo boat */
	if (mchr[sp->shp_type].m_flags & M_SUB)
	    return;		/* sub deck gun can't return fire */
	dam = shp_fire(sp);
	putship(sp->shp_uid, sp);
	if (dam < 0)
	    return;
	if (ntargets > 2)
	    dam /= ntargets / 2;

	pr_beep();
	pr("Kaboom!!! Incoming shells!\n");
	if (sp->shp_own != 0)
	    wu(0, sp->shp_own,
	       "%s fired at %s\n", prship(sp), prship(targ));
	pr_beep();
	pr("BLAM! %d damage!\n", dam);
    } else {
	/* Return fire to a submarine */
	dam = shp_dchrg(sp);
	putship(sp->shp_uid, sp);
	if (dam < 0)
	    return;
	if (ntargets > 2)
	    dam /= ntargets / 2;

	pr("\nCAPTAIN!  !!Depth charges!!...\n");
	if (sp->shp_own != 0)
	    wu(0, sp->shp_own,
	       "%s depth charged %s\n", prship(sp), prsub(targ));
	pr("click...WHAM!  %d damage!\n", dam);
    }
    shipdamage(targ, dam);
    putship(targ->shp_uid, targ);
}

static int
fire_torp(struct shpstr *sp, struct shpstr *targ, int ntargets)
{
    int range, erange, dam;

    if ((mchr[targ->shp_type].m_flags & M_SUB)
	&& (mchr[sp->shp_type].m_flags & M_SUBT) == 0)
	return 0;		/* need sub-torp to torpedo a sub */

    erange = roundrange(torprange(sp));
    range = mapdist(sp->shp_x, sp->shp_y, targ->shp_x, targ->shp_y);
    if (range > erange)
	return 0;

    if (!line_of_sight(NULL, sp->shp_x, sp->shp_y,
		       targ->shp_x, targ->shp_y))
	return 0;
    dam = shp_torp(sp, 1);
    putship(sp->shp_uid, sp);
    if (dam < 0)
	return 0;

    pr("Captain! Torpedoes sighted!\n");

    if (chance(shp_torp_hitchance(sp, range))) {
	pr("BOOM!...\n");
	if (!(mchr[targ->shp_type].m_flags & M_SUB)) {
	    if (mchr[sp->shp_type].m_flags & M_SUB)
		nreport(targ->shp_own, N_TORP_SHIP, 0, 1);
	    else
		nreport(targ->shp_own, N_SHIP_TORP, sp->shp_own, 1);
	}
	if (sp->shp_own != 0)
	    wu(0, sp->shp_own, "%s @ %s torpedoed %s\n",
	       prship(sp),
	       xyas(sp->shp_x, sp->shp_y, sp->shp_own), prsub(targ));
	if (ntargets > 2)
	    dam /= ntargets / 2;

	shipdamage(targ, dam);
	putship(targ->shp_uid, targ);

    } else {
	pr("Missed!\n");
	if (sp->shp_own != 0)
	    wu(0, sp->shp_own,
	       "%s missed %s with a torpedo at %s\n",
	       prship(sp), prsub(targ),
	       xyas(sp->shp_x, sp->shp_y, sp->shp_own));
    }

    return 1;
}

char *
prsub(struct shpstr *sp)
{
    if (mchr[(int)sp->shp_type].m_flags & M_SUB)
	return prbuf("sub #%d", sp->shp_uid);
    else
	return prship(sp);
}
