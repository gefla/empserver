/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#include <config.h>

#include "file.h"
#include "nat.h"
#include "news.h"
#include "optlist.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"

#define NOISY	1

static int sb(natid, natid, struct sctstr *, coord, coord, int, int);

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
    int range;
    double eff;
    struct shpstr ship;
    struct nstr_item ni;
    int dam;

    if (own == 0)
	return 0;
    if (att == own)
	return 0;
    eff = 1.0;
    snxtitem_dist(&ni, EF_SHIP, x, y, 8);
    while (nxtitem(&ni, &ship) && eff > 0.30) {
	if (!feels_like_helping(ship.shp_own, own, att))
	    continue;

	if ((mchr[(int)ship.shp_type].m_flags & M_SUB) && !usesubs)
	    continue;
	range = roundrange(shp_fire_range(&ship));
	if (range < ni.curdist)
	    continue;
	dam = shp_fire(&ship);
	putship(ship.shp_uid, &ship);
	if (dam < 0)
	    continue;
	if (defending)
	    nreport(ship.shp_own, N_FIRE_BACK, att, 1);
	else
	    nreport(ship.shp_own, N_FIRE_S_ATTACK, att, 1);
	eff *= (1.0 - (0.01 * dam));
	if (noisy) {
	    pr_beep();
	    pr("Incoming shells do %d damage!\n", dam);
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
		       prship(&ship), cname(own), cname(att),
		       xyas(x, y, ship.shp_own), dam);
	    }
	}
    }
    return (int)(100 - eff * 100);
}

int
dd(natid att, natid def_own, coord ax, coord ay, int noisy, int defending)
{
    int dam;
    struct sctstr firing;
    struct nstr_sect ns;

    if (opt_NO_FORT_FIRE)	/* Forts can't fire! */
	return 0;
    if (def_own == 0)
	return 0;
    if (att == def_own)
	return 0;
    dam = 0;
    snxtsct_dist(&ns, ax, ay, 8);
    while (nxtsct(&ns, &firing) && dam < 80) {
	if (!feels_like_helping(firing.sct_own, def_own, att))
	    continue;
	/* XXX defdef damage is additive, but ship or land unit damage isn't */
	dam += sb(att, def_own, &firing, ax, ay, noisy, defending);
    }
    return dam;
}

/*
 * Shall CN attempt to help FRIEND against FOE?
 */
int
feels_like_helping(natid cn, natid friend, natid foe)
{
    if (cn == 0)
	return 0;		/* never helps anybody */
    if (cn == foe)
	return 0;		/* don't help anybody against self */
    if (cn == friend)
	return 1;		/* help self against anybody else */
    /* third party helps ally if at war with foe: */
    return getrel(getnatp(cn), friend) == ALLIED
	&& getrel(getnatp(cn), foe) == AT_WAR;
}

/* Shoot back
 *
 * See if the sector being fired at will defend itself.
 */
static int
sb(natid att, natid def, struct sctstr *sp, coord tx, coord ty, int noisy,
   int defending)
{
    int damage;
    natid own;
    int range, range2;

    own = sp->sct_own;
    if (own == 0)
	return 0;
    if (att == own)
	return 0;
    range = roundrange(fortrange(sp));
    range2 = mapdist(sp->sct_x, sp->sct_y, tx, ty);
    if (range < range2)
	return 0;
    damage = fort_fire(sp);
    putsect(sp);
    if (damage < 0)
	return 0;
    if (sp->sct_own != def)
	wu(0, sp->sct_own,
	   "%s fired on %s in %s in defense of %s, doing %d damage!\n",
	   xyas(sp->sct_x, sp->sct_y, sp->sct_own), cname(att),
	   xyas(tx, ty, sp->sct_own), cname(def), damage);
    if (defending)
	nreport(sp->sct_own, N_FIRE_BACK, att, 1);
    else
	nreport(sp->sct_own, N_FIRE_F_ATTACK, att, 1);
    if (noisy)
	pr("Incoming shell! %d damage done.\007\n", damage);
    return damage;
}
