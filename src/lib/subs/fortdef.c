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
 *  fortdef.c: Fort defends an area.
 * 
 *  Known contributors to this file:
 *    
 */
/*
 * The base routines can also be used for general purposes.
 * Noisy tells whther to send teles, print things, etc.
 * Defending tells whether they are being defensive, or offensive.
 */

#include "misc.h"
#include "var.h"
#include "xy.h"
#include "nat.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "news.h"
#include "nsc.h"
#include "file.h"
#include "options.h"
#include "optlist.h"
#include "prototypes.h"

#define	NOISY	1

/*
 * See if any nearby ships will open up on the attacker
 * Return damage done to attacker, if any.
 * Subtracts shells used for firing.  Responding ships
 * require military, shells, and guns.
 */
int
shipdef(natid att, natid own, coord x, coord y)
{
    return sd(att, own, x, y, NOISY, 1, 1);
}

int
sd(natid att, natid own, coord x, coord y, int noisy, int defending,
   int usesubs)
{
    int nshot;
    double range;
    double eff;
    struct shpstr ship;
    struct nstr_item ni;
    int vec[I_MAX + 1];
    int dam, rel, rel2;

    if (own == 0)
	return 0;
    if (att == own)
	return 0;
    eff = 1.0;
    snxtitem_dist(&ni, EF_SHIP, x, y, 8);
    while (nxtitem(&ni, (caddr_t)&ship) && eff > 0.30) {
	if (ship.shp_own == att)
	    continue;
	if (ship.shp_own == 0)
	    continue;

	rel = getrel(getnatp(ship.shp_own), own);
	rel2 = getrel(getnatp(ship.shp_own), att);
	if ((ship.shp_own != own) && ((rel != ALLIED) || (rel2 != AT_WAR)))
	    continue;
	if (ship.shp_effic < 60)
	    continue;
	if ((mchr[(int)ship.shp_type].m_flags & M_SUB) && (!usesubs))
	    continue;
	range = techfact(ship.shp_tech,
			 ship.shp_frnge * ship.shp_effic / 200.0);
	range = (double)roundrange(range);
	if (range < ni.curdist)
	    continue;
	/* must have gun, shell, and milit to fire */
	if (getvec(VT_ITEM, vec, (caddr_t)&ship, EF_SHIP) < 3)
	    continue;
	if (vec[I_SHELL] < ship.shp_glim)
	    vec[I_SHELL] += supply_commod(ship.shp_own, ship.shp_x,
					  ship.shp_y, I_SHELL,
					  vec[I_SHELL] - ship.shp_glim);
	nshot = min(min(vec[I_GUN], vec[I_SHELL]), vec[I_MILIT]);
	nshot = min(nshot, ship.shp_glim);
	if (nshot <= 0)
	    continue;
	ship.shp_item[I_SHELL] = vec[I_SHELL] - nshot;
	putship(ship.shp_uid, &ship);
	if (defending)
	    nreport(ship.shp_own, N_FIRE_BACK, att, 1);
	else
	    nreport(ship.shp_own, N_FIRE_S_ATTACK, att, 1);
	dam = seagun(ship.shp_effic, nshot);
	eff *= (1.0 - (0.01 * dam));
	if (noisy) {
	    pr_beep();
	    pr("Incoming shell%s %d damage!\n",
	       nshot == 1 ? " does" : "s do", dam);
	}
	if (noisy || (ship.shp_own != own)) {
	    if (ship.shp_own == own)
		wu(0, own, "%s fired on %s at %s doing %d damage.\n",
		   prship(&ship), cname(att), xyas(x, y, own), dam);
	    else {
		if (defending)
		    wu(0, ship.shp_own,
		       "%s fired on %s at %s in defense of %s, doing %d damage.\n",
		       prship(&ship), cname(att), xyas(x, y, ship.shp_own),
		       cname(own), dam);
		else
		    wu(0, ship.shp_own,
		       "%s supported %s attacks against %s at %s, doing %d damage.\n",
		       prship(&ship), cname(own), cname(att), xyas(x, y,
								   ship.
								   shp_own),
		       dam);
	    }
	}
    }
    return (int)100 - (eff * 100);
}

/*
 * Determine if any nearby gun-equipped sectors are within
 * range and able to fire at an attacker.  Firing sectors
 * need to have guns, shells, and military.  Sector being
 * attacked is x,y -- attacker is at ax,ay.
 */
#if 0
/* defdef isn't called anywhere, and uses wrong
 * number of arguments for dd */
int
defdef(att, def_own, defval, ax, ay)
natid att;
natid def_own;
int defval;
coord ax;
coord ay;
{
    return dd(att, def_own, defval, ax, ay, NOISY, 1);
}
#endif

int
dd(natid att, natid def_own, coord ax, coord ay, int noisy, int defending)
{
    int dam, rel, rel2;
    double tech;
    double range;
    struct sctstr firing;
    struct nstr_sect ns;
    int vec[I_MAX + 1];

    if (opt_NO_FORT_FIRE)	/* Forts can't fire! */
	return 0;
    if (def_own == 0)
	return 0;
    if (att == def_own)
	return 0;
    tech = tfactfire(def_own, 1.0);
    dam = 0;
    snxtsct_dist(&ns, ax, ay, 8);
    while (nxtsct(&ns, &firing) && dam < 80) {
	if (firing.sct_own == att)
	    continue;
	if (firing.sct_own == 0)
	    continue;
	if (firing.sct_effic < (u_char)FORTEFF)
	    continue;
	rel = getrel(getnatp(firing.sct_own), def_own);
	rel2 = getrel(getnatp(firing.sct_own), att);
	if ((firing.sct_own != def_own) &&
	    ((rel != ALLIED) || (rel2 != AT_WAR)))
	    continue;
	if (getvec(VT_ITEM, vec, (caddr_t)&firing, EF_SECTOR) < 0)
	    continue;

	range = tfactfire(def_own, 7.0);
	if (firing.sct_effic > 59)
	    range++;
	/* Here we round down the range, and then add 1 to it
	   to determine if we could possibly hit the sector.  If
	   we do, we call sb where the range is re-calculated and
	   the percentages are checked. */
	range = (double)((int)(range) + 1);
	if (range < ns.curdist)
	    continue;
	/* XXX defdef damage is additive, but ship or land unit damage isn't */
	dam += sb(att, def_own, &firing, ax, ay, noisy, defending);
    }
    return dam;
}

/* Shoot back
 *
 * See if the sector being fired at will defend itself.
 */
int
sb(natid att, natid def, struct sctstr *sp, coord tx, coord ty, int noisy,
   int defending)
{
    register int damage;
    natid own;
    int shell;
    double range;
    int range2, gun;

    if (sp->sct_type != SCT_FORTR) {
	/* XXX I don't like this restriction */
	return 0;
    }

    if (sp->sct_effic < (u_char)FORTEFF)
	return 0;

    own = sp->sct_own;
    if (own == 0)
	return 0;
    if (att == own)
	return 0;
    range = tfactfire(own, 7.0);
    if (sp->sct_effic > 59)
	range++;
    range = (double)roundrange(range);
    range2 = mapdist((int)sp->sct_x, (int)sp->sct_y, tx, ty);
    if (range < range2)
	return 0;
    gun = sp->sct_item[I_GUN];
    if (gun == 0)
	return 0;
    shell = sp->sct_item[I_SHELL];
    if (shell <= 0)
	shell += supply_commod(sp->sct_own, sp->sct_x, sp->sct_y, I_SHELL,
			       1);
    if (shell <= 0)
	return 0;
    sp->sct_item[I_SHELL] = shell - 1;
    putsect(sp);
    damage = landgun((int)sp->sct_effic, gun);
    if (sp->sct_own != def)
	wu(0, sp->sct_own,
	   "%s fired on %s in %s in defense of %s, doing %d damage!\n",
	   xyas(sp->sct_x, sp->sct_y, sp->sct_own), cname(att), xyas(tx,
								     ty,
								     sp->
								     sct_own),
	   cname(def), damage);
    if (defending)
	nreport(sp->sct_own, N_FIRE_BACK, att, 1);
    else
	nreport(sp->sct_own, N_FIRE_F_ATTACK, att, 1);
    if (noisy)
	pr("Incoming shell! %d damage done.\007\n", damage);
    return damage;
}
