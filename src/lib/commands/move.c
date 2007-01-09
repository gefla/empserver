/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  move.c: Move commodities around
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "land.h"
#include "optlist.h"
#include "path.h"
#include "plague.h"


static int cmd_move_map(coord curx, coord cury, char *arg);

int
move(void)
{
    int amount;
    struct sctstr sect;
    struct sctstr endsect;
    struct sctstr start;
    struct sctstr tsct;
    int packing;
    double weight;
    int left;
    int mcost, dam;
    int infected;
    i_type vtype;
    int amt_src;
    int amt_dst;
    struct ichrstr *ip;
    int work;
    int loyal;
    int own, mob;
    int istest = 0;
    int n;
    coord x, y;
    char *p;
    char prompt[1024];
    char buf[1024];


    istest = *player->argp[0] == 't';
    if ((ip = whatitem(player->argp[1], "move what? ")) == 0)
	return RET_SYN;
    vtype = ip->i_uid;
    if (!(p = getstarg(player->argp[2], "from sector : ", buf)))
	return RET_SYN;
    if (!sarg_xy(p, &x, &y))
	return RET_SYN;
    if (!getsect(x, y, &sect) || !player->owner) {
	pr("Not yours\n");
	return RET_FAIL;
    }
    /*
     * military control necessary to move
     * goodies in occupied territory.
     */
    if (!istest && sect.sct_oldown != player->cnum && vtype != I_MILIT) {
	if (!military_control(&sect)) {
	    pr("Military control required to move goods.\n");
	    return RET_FAIL;
	}
    }
    infected = sect.sct_pstage == PLG_INFECT;
    amt_src = sect.sct_item[vtype];
    if (!istest && amt_src <= 0) {
	pr("No %s in %s\n", ip->i_name,
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_FAIL;
    }
    own = sect.sct_own;
    mob = sect.sct_mobil;
    if (!istest && vtype == I_CIVIL && sect.sct_oldown != own) {
	pr("You can't move conquered populace!\n");
	return RET_FAIL;
    }
    if (mob <= 0) {
	pr("No mobility in %s\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_SYN;
    }

    /* only used when moving civs; but prevent spurious compiler warnings */
    work = sect.sct_work;
    loyal = sect.sct_loyal;
    if (vtype == I_CIVIL && work != 100)
	pr("Warning: civil unrest\n");

    if (istest)
	sprintf(prompt, "Number of %s to test move? ", ip->i_name);
    else
	sprintf(prompt, "Number of %s to move? (max %d) ",
		ip->i_name, amt_src);
    if ((amount = onearg(player->argp[3], prompt)) < 0)
	return RET_FAIL;
    if (!check_sect_ok(&sect))
	return RET_FAIL;
    if (amount > amt_src) {
	if (istest) {
	    pr("Note: there are actually only %d %s in %s,\n"
	       "but the test will be made for %d %s as you requested.\n",
	       amt_src, ip->i_name,
	       xyas(sect.sct_x, sect.sct_y, player->cnum),
	       amount, ip->i_name);
	} else {
	    amount = amt_src;
	    pr("Only moving %d.\n", amount);
	}
    }

    if (!istest && !want_to_abandon(&sect, vtype, amount, 0)) {
	pr("Move cancelled.\n");
	return RET_FAIL;
    }

    if (!check_sect_ok(&sect))
	return RET_FAIL;

    if (amount <= 0)
	return RET_SYN;
    packing = sect.sct_effic >= 60 ? dchr[sect.sct_type].d_pkg : IPKG;
    weight = (double)amount * ip->i_lbs / ip->i_pkg[packing];

    /*
     * First remove stuff from source sector
     */
    if (!istest) {
	getsect(x, y, &start);
	if (start.sct_own != player->cnum) {
	    pr("Somebody has captured that sector!\n");
	    return RET_FAIL;
	}
	amt_src = start.sct_item[vtype];
	if (amt_src < amount) {
	    pr("Only %d %s left in %s!\n",
	       amt_src, ip->i_name,
	       xyas(start.sct_x, start.sct_y, player->cnum));
	    amount = amt_src;
	    amt_src = 0;
	} else
	    amt_src -= amount;

	start.sct_item[vtype] = amt_src;
	start.sct_flags |= MOVE_IN_PROGRESS;
	putsect(&start);
    }

    /*
     * Now parse the path and return ending sector.
     */
    dam = !istest && !(opt_SUPER_BARS && vtype == I_BAR);
    if (dam && !chance(weight / 200.0))
	dam = 0;
    mcost = move_ground(&sect, &endsect, weight, player->argp[4],
			cmd_move_map, 0, &dam);

    if (dam) {
	left = commdamage(amount, dam, ip->i_uid);
	if (left < amount) {
	    if (left) {
		pr("%d of the %s you were moving were destroyed!\n"
		   "Only %d %s made it to %s\n",
		   amount - left, ip->i_name, left, ip->i_name,
		   xyas(endsect.sct_x, endsect.sct_y, player->cnum));
	    } else {
		pr("All of the %s you were moving were destroyed!\n",
		   ip->i_name);
	    }
	    amount = left;
	}
    }

    if (mcost > 0)
	pr("Total movement cost = %d\n", mcost);
    else
	pr("No mobility used\n");

    left = 0;
    if (mcost < 0) {
	pr("Move aborted\n");
	getsect(x, y, &sect);
	sect.sct_mobil = mob;
	left = mob;
    } else {
	if (!istest) {
	    /*
	     * Decrement mobility appropriately.
	     */
	    getsect(x, y, &start);
	    mob = start.sct_mobil;
	    if (mob < mcost) {
		if (mob > 0)
		    mob = 0;
	    } else
		mob -= mcost;
	    start.sct_mobil = mob;
	    left = start.sct_mobil;
	    putsect(&start);
	}
	getsect(endsect.sct_x, endsect.sct_y, &sect);
    }

    /*
     * Check for lotsa stuff
     */
    if (sect.sct_own != player->cnum) {
	if (sect.sct_own != 0)
	    pr("Somebody has captured that sector!\n");
	getsect(x, y, &sect);
    }
    if (vtype == I_CIVIL && sect.sct_item[I_CIVIL]
	&& sect.sct_oldown != player->cnum) {
	pr("Your civilians don't want to stay!\n");
	getsect(x, y, &sect);
    }

    amt_dst = sect.sct_item[vtype];
    if (amount > ITEM_MAX - amt_dst) {
	pr("Only enough room for %d in %s.  The goods will be returned.\n",
	   ITEM_MAX - amt_dst, xyas(sect.sct_x, sect.sct_y, player->cnum));
	/* FIXME Not nice.  Move what we can and return the rest.  */
	getsect(x, y, &sect);
    }

    if (istest)
	return RET_OK;

    pr("%d mob left in %s\n", left,
       xyas(start.sct_x, start.sct_y, player->cnum));

    if (amount <= 0) {
	getsect(x, y, &start);
	start.sct_flags &= ~MOVE_IN_PROGRESS;
	putsect(&start);
	return RET_OK;
    }

    /*
     * If the sector that things are going to is no longer owned by
     * the player, and it was the starting sector, try to find
     * somewhere to dump the stuff.  If nowhere to dump it, it
     * disappears.
     */
    if (sect.sct_own != player->cnum && sect.sct_x == x && sect.sct_y == y) {
	pr("Can't return the goods, since the starting point is no longer\n");
	pr("owned by you.\n");
	/* First lets see if there is one with room */
	for (n = DIR_FIRST; n <= DIR_LAST; n++) {
	    getsect(x + diroff[n][0], y + diroff[n][1], &tsct);
	    if (tsct.sct_own != player->cnum)
		continue;
	    amt_dst = tsct.sct_item[vtype];
	    if (amount > ITEM_MAX - amt_dst)
		continue;
	    n = -1;
	    break;
	}
	if (n > -1) {
	    /* Find any sector if none with room */
	    for (n = DIR_FIRST; n <= DIR_LAST; n++) {
		getsect(x + diroff[n][0], y + diroff[n][1], &tsct);
		if (tsct.sct_own != player->cnum)
		    continue;
		n = -1;
		break;
	    }
	    if (n > -1) {
		pr("The goods had nowhere to go, and were destroyed.\n");
		sect.sct_flags &= ~MOVE_IN_PROGRESS;
		putsect(&sect);
		return RET_OK;
	    }
	}
	pr("The goods were dumped into %s.\n",
	   xyas(tsct.sct_x, tsct.sct_y, player->cnum));
	getsect(tsct.sct_x, tsct.sct_y, &sect);
    }

    amt_dst = sect.sct_item[vtype];
    if (amount > ITEM_MAX - amt_dst) {
	amount = ITEM_MAX - amt_dst;
	pr("Only room for %d, the rest were lost.\n", amount);
    }
    sect.sct_item[vtype] = amount + amt_dst;
    /*
     * Now add commodities to destination sector,
     * along with plague that came along for the ride.
     * Takeover unowned sectors if not deity.
     */
    if (infected && sect.sct_pstage == PLG_HEALTHY)
	sect.sct_pstage = PLG_EXPOSED;
    if (vtype == I_CIVIL) {
	sect.sct_loyal
	    = (amt_dst * sect.sct_loyal + amount * loyal) / (amt_dst + amount);
	sect.sct_work
	    = (amt_dst * sect.sct_work + amount * work) / (amt_dst + amount);
    }
    putsect(&sect);
    getsect(x, y, &start);
    start.sct_flags &= ~MOVE_IN_PROGRESS;
    putsect(&start);
    return RET_OK;
}

/*
 * Pretty tacky, but it works.
 * If more commands start doing this, then
 * rewrite map to do the right thing.
 */
/*ARGSUSED*/
static int
cmd_move_map(coord curx, coord cury, char *arg)
{
    return display_region_map(0, EF_SHIP, curx, cury, arg);
}

int
want_to_abandon(struct sctstr *sp, i_type vtype, int amnt, struct lndstr *lp)
{
    char prompt[80];

    /*
     * First, would we be abandoning it?  If not, just return that
     * it's ok to move out.
     */
    if (!would_abandon(sp, vtype, amnt, lp))
	return 1;

    sprintf(prompt, "Do you really want to abandon %s [yn]? ",
	    xyas(sp->sct_x, sp->sct_y, player->cnum));

    return askyn(prompt);
}

int
would_abandon(struct sctstr *sp, i_type vtype, int amnt, struct lndstr *lp)
{
    int mil, civs, loyalcivs;

    if ((vtype != I_CIVIL) && (vtype != I_MILIT))
	return 0;

    mil = sp->sct_item[I_MILIT];
    civs = sp->sct_item[I_CIVIL];

    if (vtype == I_MILIT)
	mil -= amnt;

    if (vtype == I_CIVIL)
	civs -= amnt;

    if (sp->sct_own == sp->sct_oldown)
	loyalcivs = civs;
    else
	loyalcivs = 0;

    /* If they have a military unit there, they still own it */
    if (sp->sct_own != 0
	&& ((loyalcivs == 0) && (mil == 0)
	    && (has_units(sp->sct_x, sp->sct_y, sp->sct_own, lp) == 0)))
	return 1;

    return 0;
}
