/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  torp.c: Fire torpedoes at enemy ships
 * 
 *  Known contributors to this file:
 *     Dave Pare
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 2000
 */

#include <stdio.h>
#include "misc.h"
#include "player.h"
#include "var.h"
#include "ship.h"
#include "file.h"
#include "xy.h"
#include "nat.h"
#include "nsc.h"
#include "news.h"
#include "retreat.h"
#include "damage.h"
#include "commands.h"
#include "optlist.h"

static void anti_torp(int f, int ntorping, int vshipown);
static int candchrg(struct shpstr *, struct shpstr *);
static int canshoot(struct shpstr *, struct shpstr *);
static int cantorp(struct shpstr *, struct shpstr *);
static void fire_dchrg(struct shpstr *sp, struct shpstr *targ, int ntargets);
static int fire_torp(struct shpstr *, struct shpstr *, int, int);

int
torp(void)
{
    natid vshipown;
    int range;
    int dam;
    int shells;
    int subno;
    int victno;
    double erange;
    double hitchance;
    struct shpstr vship;
    struct shpstr sub;
    s_char *ptr;
    double mobcost;
    struct mchrstr *mcp;
    struct nstr_item nbst;
    s_char buf[1024];
    s_char *sav;
    int ntorping = 0;
    s_char prompt[128];

    if (!(sav = getstarg(player->argp[1], "From ship(s)? ", buf)))
	return RET_SYN;
    if (!snxtitem(&nbst, EF_SHIP, sav))
	return RET_SYN;
    while (nxtitem(&nbst, (s_char *)&sub)) {
	if (sub.shp_own != player->cnum)
	    continue;
	if ((mchr[(int)sub.shp_type].m_flags & M_TORP) == 0)
	    continue;
	shells = sub.shp_item[I_SHELL];
	if (shells < SHP_TORP_SHELLS)
	    shells += supply_commod(sub.shp_own, sub.shp_x, sub.shp_y,
				    I_SHELL, SHP_TORP_SHELLS - shells);
	if (sub.shp_item[I_GUN] == 0 || shells < SHP_TORP_SHELLS)
	    continue;
	if (sub.shp_item[I_MILIT] < 1)
	    continue;
	if (sub.shp_effic < 60)
	    continue;
	if (sub.shp_mobil <= 0)
	    continue;
	ntorping++;
    }
    pr("%d ships are eligible to torp\n", ntorping);
    snxtitem(&nbst, EF_SHIP, sav);
    while (nxtitem(&nbst, (s_char *)&sub)) {
	if (!sub.shp_own)
	    continue;
	if (sub.shp_own != player->cnum) {
	    continue;
	}
	if ((mchr[(int)sub.shp_type].m_flags & M_TORP) == 0) {
	    pr("Ship # %d: A %s can't fire torpedoes!\n", sub.shp_uid,
	       mchr[(int)sub.shp_type].m_name);
	    continue;
	}
	shells = sub.shp_item[I_SHELL];
	if (shells < SHP_TORP_SHELLS)
	    shells += supply_commod(sub.shp_own, sub.shp_x, sub.shp_y,
				    I_SHELL, SHP_TORP_SHELLS - shells);
	if (sub.shp_item[I_GUN] == 0 || shells < SHP_TORP_SHELLS) {
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
	if ((ptr = getstarg(player->argp[2], prompt, buf)) == 0)
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
	    if (!(mchr[(int)sub.shp_type].m_flags & M_SUBT)) {
		pr("You can't torpedo a submarine!\n");
		continue;
	    }
	}
	if ((mchr[(int)sub.shp_type].m_flags & M_SUB) == 0)
	    anti_torp(sub.shp_uid, ntorping, vshipown);
	getship(sub.shp_uid, &sub);
	if (sub.shp_own == 0) {
	    continue;
	}
	erange = ((double)sub.shp_effic / 100.0) *
	    techfact(sub.shp_tech, ((double)sub.shp_frnge));
	erange = (double)roundrange(erange);
	pr("Effective torpedo range is %.1f\n", erange);
	shells -= SHP_TORP_SHELLS;
	sub.shp_item[I_SHELL] = shells;
	putship(sub.shp_uid, &sub);
	mcp = &mchr[(int)sub.shp_type];
	mobcost = sub.shp_effic * 0.01 * sub.shp_speed;
	mobcost = (480.0 / (mobcost + techfact(sub.shp_tech, mobcost)));

	/* Mob cost for a torp is equal to the cost of 1/2 sector of movement */
	mobcost /= 2.0;
	sub.shp_mobil -= mobcost;
	pr("Whooosh... ");
	getship(victno, &vship);
	vshipown = vship.shp_own;
	range = mapdist(sub.shp_x, sub.shp_y, vship.shp_x, vship.shp_y);
	hitchance = DTORP_HITCHANCE(range, sub.shp_visib);
	if (range <= erange) {
	    pr("Hitchance = %d%%\n", (int)(hitchance * 100));
	}
	/* Now, can the torpedo even get there? */
	if (!line_of_sight((s_char **)0, sub.shp_x, sub.shp_y,
			   vship.shp_x, vship.shp_y)) {
	    pr("BOOM!... Torpedo slams into land before reaching target.\n");
	    /* We only tell the victim if we were within range. */
	    if (range <= erange) {
		if (vshipown != 0)
		    wu(0, vshipown, "Torpedo sighted @ %s by %s\n",
		       xyas(sub.shp_x, sub.shp_y, vshipown),
		       prship(&vship));
	    }
	} else if (range > erange) {
	    pr("Out of range\n");
	} else if (hitchance >= 1.0 || chance(hitchance)) {
	    pr("BOOM!...\n");
	    dam = TORP_DAMAGE();
	    if (vshipown != 0)
		wu(0, vshipown, "%s in %s torpedoed %s for %d damage.\n",
		   prsub(&sub), xyas(sub.shp_x, sub.shp_y, vshipown),
		   prship(&vship), dam);
	    if (vship.shp_rflags & RET_TORPED) {
		retreat_ship(&vship, 't');
		shipdamage(&vship, dam);
	    } else
		shipdamage(&vship, dam);
	    pr("Torpedo hit %s for %d damage.\n", prship(&vship), dam);

	    if (vship.shp_effic < SHIP_MINEFF)
		pr("%s sunk!\n", prship(&vship));
	    putship(vship.shp_uid, &vship);
	    if (mchr[(int)sub.shp_type].m_flags & M_SUB)
		nreport(vshipown, N_TORP_SHIP, 0, 1);
	    else
		nreport(vshipown, N_SHIP_TORP, player->cnum, 1);
	} else {
	    pr("Missed\n");
	    if (vshipown != 0)
		wu(0, vshipown, "Torpedo sighted @ %s by %s\n",
		   xyas(sub.shp_x, sub.shp_y, vshipown), prship(&vship));
	}
	sub.shp_mission = 0;
	putship(sub.shp_uid, &sub);
	if (mchr[(int)sub.shp_type].m_flags & M_SUB)
	    anti_torp(sub.shp_uid, ntorping, vshipown);
    }
    return RET_OK;
}

static void
anti_torp(int f, int ntorping, int vshipown)
{
    int range;
    double erange;
    struct shpstr sub;
    struct shpstr dd;
    int x;

    getship(f, &sub);

    if (sub.shp_own == vshipown)
	return;

    if ((mchr[(int)sub.shp_type].m_flags & M_SUB) == 0)
	pr("Starting our attack run...\n");

    x = 0;
    while (getship(x++, &dd) && sub.shp_effic >= SHIP_MINEFF) {
	if (dd.shp_own == 0)
	    continue;
	if (dd.shp_own != vshipown)
	    continue;
	if (dd.shp_effic < 60)
	    continue;

	if (!canshoot(&dd, &sub))
	    continue;

	erange = techfact(dd.shp_tech, ((double)dd.shp_frnge)) / 2.0;

	erange = (double)roundrange(erange);

	range = mapdist(sub.shp_x, sub.shp_y, dd.shp_x, dd.shp_y);

	if (range > erange)
	    continue;

	if (!line_of_sight((s_char **)0, sub.shp_x, sub.shp_y,
			   dd.shp_x, dd.shp_y))
	    continue;

	if (cantorp(&dd, &sub)) {
	    /* Try torping.. if we can, maybe we can fire */
	    if (!fire_torp(&dd, &sub, range, ntorping))
		if (candchrg(&dd, &sub))
		    fire_dchrg(&dd, &sub, ntorping);
	} else
	    fire_dchrg(&dd, &sub, ntorping);
    }
}

/* Can ship A shoot at ship B? */
static int
canshoot(struct shpstr *a, struct shpstr *b)
{
    /* Anyone can shoot a normal ship */
    if ((mchr[(int)b->shp_type].m_flags & M_SUB) == 0)
	return 1;

    /* You can depth-charge a sub */
    if (mchr[(int)a->shp_type].m_flags & M_DCH)
	return 1;

    /* If you have SUBT flag, you can torp a sub */
    if (mchr[(int)a->shp_type].m_flags & M_SUBT)
	return 1;

    return 0;
}

/* Can ship A torp ship B? */
static int
cantorp(struct shpstr *a, struct shpstr *b)
{
    if ((mchr[(int)a->shp_type].m_flags & M_TORP) == 0)
	return 0;

    /* Anyone with TORP flag can torp a normal ship */
    if ((mchr[(int)b->shp_type].m_flags & M_SUB) == 0)
	return 1;

    /* Ship b is a sub, so we need to have the SUBT flag */
    if (mchr[(int)a->shp_type].m_flags & M_SUBT)
	return 1;

    return 0;
}

/* Can ship A depth-charge (or fire guns at) ship B? */
static int
candchrg(struct shpstr *a, struct shpstr *b)
{
    if ((mchr[(int)b->shp_type].m_flags & M_SUB) == 0) {
	if ((mchr[(int)a->shp_type].m_flags & M_SUB) == 0)
	    return 1;

	return 0;
    }

    if ((mchr[(int)a->shp_type].m_flags & M_DCH) == 0)
	return 0;

    return 1;
}

static void
fire_dchrg(struct shpstr *sp, struct shpstr *targ, int ntargets)
{
    int dam;
    int shells;
    int gun;
    double guneff;

    shells = sp->shp_item[I_SHELL];
    gun = sp->shp_item[I_GUN];
    gun = min(gun, sp->shp_glim);
    gun = min(gun, sp->shp_item[I_MILIT] / 2);

    shells +=
	supply_commod(sp->shp_own, sp->shp_x, sp->shp_y, I_SHELL,
		      (gun + 1) / 2 - shells);

    gun = min(gun, shells * 2);
    if (gun == 0)
	return;

    /* ok, all set.. now, we shoot */
    shells -= ldround(((double)gun) / 2.0, 1);
    sp->shp_item[I_SHELL] = shells;
    putship(sp->shp_uid, sp);

    guneff = seagun(sp->shp_effic, gun);
    dam = (int)guneff;

    if ((mchr[(int)targ->shp_type].m_flags & M_SUB) == 0) {
	pr_beep();
	pr("Kaboom!!! Incoming shells!\n");
	if (sp->shp_own != 0)
	    wu(0, sp->shp_own,
	       "%s fired at %s\n", prship(sp), prship(targ));

	if (ntargets > 2)
	    dam /= ((float)ntargets / 2.0);
	pr_beep();
	pr("BLAM! %d damage!\n", dam);
	shipdamage(targ, dam);
	putship(targ->shp_uid, targ);
    } else {
	pr("\nCAPTAIN!  !!Depth charges!!...\n");
	if (sp->shp_own != 0)
	    wu(0, sp->shp_own,
	       "%s depth charged %s\n", prship(sp), prsub(targ));

	if (ntargets > 2)
	    dam /= ((float)ntargets / 2.0);

	pr("click...WHAM!  %d damage!\n", dam);
	shipdamage(targ, dam);
	putship(targ->shp_uid, targ);
    }
}

static int
fire_torp(struct shpstr *sp, struct shpstr *targ, int range, int ntargets)
{
    int dam;
    int shells;
    double hitchance;
    double mobcost;
    struct mchrstr *mcp;

    shells = sp->shp_item[I_SHELL];

    if (shells < SHP_TORP_SHELLS)
	shells += supply_commod(sp->shp_own, sp->shp_x, sp->shp_y, I_SHELL,
				SHP_TORP_SHELLS - shells);

    if (sp->shp_item[I_GUN] == 0 || shells < SHP_TORP_SHELLS)
	return 0;

    if (sp->shp_item[I_MILIT] < 1)
	return 0;

    if (sp->shp_effic < 60)
	return 0;

    if (sp->shp_mobil <= 0)
	return 0;

    /* All set.. fire! */
    shells -= SHP_TORP_SHELLS;
    sp->shp_item[I_SHELL] = shells;
    putship(sp->shp_uid, sp);

    mcp = &mchr[(int)sp->shp_type];
    mobcost = sp->shp_effic * 0.01 * sp->shp_speed;
    mobcost = (480.0 / (mobcost + techfact(sp->shp_tech, mobcost)));

    /* Mob cost for a torp is equal to the cost of 1/2 sector of movement */
    mobcost /= 2.0;
    sp->shp_mobil -= mobcost;

    hitchance = DTORP_HITCHANCE(range, sp->shp_visib);

    pr("Captain! Torpedoes sighted!\n");

    if (chance(hitchance)) {
	pr("BOOM!...\n");
	if (sp->shp_own != 0)
	    wu(0, sp->shp_own, "%s @ %s torpedoed %s\n",
	       prship(sp),
	       xyas(sp->shp_x, sp->shp_y, sp->shp_own), prsub(targ));
	dam = TORP_DAMAGE();

	if (ntargets > 2)
	    dam /= ((float)ntargets / 2.0);

	shipdamage(targ, dam);
	putship(targ->shp_uid, targ);

	if (mchr[(int)sp->shp_type].m_flags & M_SUB)
	    nreport(targ->shp_own, N_TORP_SHIP, 0, 1);
	else
	    nreport(targ->shp_own, N_SHIP_TORP, player->cnum, 1);
    } else {
	pr("Missed!\n");
	if (sp->shp_own != 0)
	    wu(0, sp->shp_own,
	       "%s missed %s with a torp at %s\n",
	       prship(sp), prsub(targ),
	       xyas(sp->shp_x, sp->shp_y, sp->shp_own));
    }

    return 1;
}

s_char *
prsub(struct shpstr *sp)
{
    if (mchr[(int)sp->shp_type].m_flags & M_SUB)
	return "sub";
    else
	return prship(sp);
}
