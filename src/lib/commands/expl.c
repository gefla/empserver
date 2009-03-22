/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  expl.c: Take over unoccupied sectors
 *
 *  Known contributors to this file:
 *     Jeff Wallace, 1989
 */

#include <config.h>

#include "commands.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "optlist.h"
#include "plague.h"

static int explore_map(coord curx, coord cury, char *arg);

int
explore(void)
{
    int amount;
    struct sctstr sect;
    struct sctstr endsect;
    struct sctstr start;
    struct sctstr chksect;
    double weight;
    int mcost, dam;
    int infected;
    i_type vtype;
    int amt_src;
    int amt_dst;
    struct ichrstr *ip;
    int work;
    int loyal;
    int own, mob;
    int justtook;
    coord x, y;
    char *p;
    int n;
    int left;
    char buf[1024];
    char prompt[128];

    if (!(ip = whatitem(player->argp[1], "explore with what? (civ/mil) ")))
	return RET_SYN;
    vtype = ip->i_uid;
    if ((vtype != I_CIVIL) && (vtype != I_MILIT)) {
	pr("You can only explore with civs and mil.\n");
	return RET_FAIL;
    }
    if ((p = getstarg(player->argp[2], "from sector : ", buf)) == 0)
	return RET_SYN;
    if (!sarg_xy(p, &x, &y))
	return RET_SYN;
    if (!getsect(x, y, &sect) || !player->owner) {
	pr("Not yours\n");
	return RET_FAIL;
    }
    infected = sect.sct_pstage == PLG_INFECT;
    if ((amt_src = sect.sct_item[vtype]) <= 0) {
	pr("No %s in %s\n", ip->i_name,
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_SYN;
    }
    own = sect.sct_own;
    mob = sect.sct_mobil;
    if (vtype == I_CIVIL && sect.sct_oldown != own) {
	pr("You can't explore with conquered populace!\n");
	return RET_SYN;
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

    sprintf(prompt, "Number of %s to explore with? (max %d) ",
	    ip->i_name, amt_src);
    amount = onearg(player->argp[3], prompt);
    if (amount <= 0)
	return RET_SYN;
    if (!check_sect_ok(&sect))
	return RET_FAIL;
    if (amount > amt_src) {
	amount = amt_src;
	pr("Only exploring with %d.\n", amount);
    }

    if (!want_to_abandon(&sect, vtype, amount, NULL)) {
	pr("Explore cancelled.\n");
	return RET_FAIL;
    }

    if (!check_sect_ok(&sect))
	return RET_FAIL;

    weight = (double)amount * ip->i_lbs;
    /* remove commodities from source sector */
    getsect(x, y, &start);
    amt_src = start.sct_item[vtype];
    amt_src -= amount;
    if (amt_src < 0) {
	pr("%s in %s are gone!\n", ip->i_name,
	   xyas(start.sct_x, start.sct_y, player->cnum));
	return RET_OK;
    }
    start.sct_item[vtype] = amt_src;
    start.sct_flags |= MOVE_IN_PROGRESS;
    putsect(&start);
    /*
     * Now parse the path and return ending sector.
     */
    dam = 1;
    mcost = move_ground(&sect, &endsect, weight, player->argp[4],
			explore_map, 1, &dam);

    if (dam) {
	left = effdamage(amount, dam);
	if (left < amount) {
	    if (left) {
		pr("%d of the %s you were exploring with were destroyed!\n"
		   "Only %d %s made it to %s\n",
		   amount - left, ip->i_name, left, ip->i_name,
		   xyas(endsect.sct_x, endsect.sct_y, player->cnum));
	    } else {
		pr("All of the %s you were exploring with were destroyed!\n",
		   ip->i_name);
	    }
	    amount = left;
	}
    }
    if (mcost > 0)
	pr("Total movement cost = %d", mcost);
    else
	pr("No mobility used");

    if (mcost < 0) {
	pr("\nExplore aborted");
	getsect(start.sct_x, start.sct_y, &sect);
	sect.sct_mobil = mob;
    } else {
	/* Charge mobility */
	getsect(sect.sct_x, sect.sct_y, &sect);
	n = sect.sct_mobil - mcost;
	if (n < 0)
	    n = 0;
	sect.sct_mobil = n;
	pr(", %d mob left in %s", sect.sct_mobil,
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	putsect(&sect);
	getsect(endsect.sct_x, endsect.sct_y, &sect);
    }
    pr("\n");
    justtook = 0;

    /*
     *      Check for a multitude of problems
     */
    getsect(endsect.sct_x, endsect.sct_y, &chksect);
    if (amount <= 0) {
	getsect(start.sct_x, start.sct_y, &start);
	sect.sct_flags &= ~MOVE_IN_PROGRESS;
	putsect(&sect);
	return RET_FAIL;
    }
    if (chksect.sct_type == SCT_WATER) {
	pr("Bridge disappeared!\n");
	getsect(start.sct_x, start.sct_y, &start);
	start.sct_flags &= ~MOVE_IN_PROGRESS;
	putsect(&start);
	return RET_FAIL;
    }
    if (!player->god && chksect.sct_own && chksect.sct_own != player->cnum
	&& chksect.sct_type != SCT_SANCT) {
	pr("Somebody beat you there!\n");
	/* Send them back home */
	getsect(start.sct_x, start.sct_y, &sect);
	if (sect.sct_own != own) {
	    pr("Someone captured the sector you started from!\n");
	    pr("Your exploring people die of disappointment!\n");
	    sect.sct_flags &= ~MOVE_IN_PROGRESS;
	    putsect(&sect);
	    return RET_FAIL;
	}
    }

    if (!player->god && !player->owner && sect.sct_type != SCT_SANCT) {
	takeover(&sect, player->cnum);
	justtook = 1;
	sect.sct_oldown = own;
	sect.sct_work = 100;
	sect.sct_loyal = 0;
    }
    if (vtype == I_CIVIL && sect.sct_oldown != player->cnum) {
	pr("Your civilians don't want to stay!\n");
	getsect(start.sct_x, start.sct_y, &sect);
	if (sect.sct_own != own) {
	    pr("Someone captured the sector you started from!\n");
	    pr("Your exploring people die of disappointment!\n");
	    sect.sct_flags &= ~MOVE_IN_PROGRESS;
	    putsect(&sect);
	    return RET_FAIL;
	}
    }
    amt_dst = sect.sct_item[vtype];
    if (amount > ITEM_MAX - amt_dst) {
	amount = ITEM_MAX - amt_dst;
	pr("Only %d can be left there.\n", amount);
	if (amount <= 0)
	    getsect(start.sct_x, start.sct_y, &sect);
    }
    sect.sct_item[vtype] = amount + amt_dst;
    /*
     * Now add commodities to destination sector,
     * along with plague that came along for the ride.
     * Takeover unowned sectors if not deity.
     */
    if (!player->god && sect.sct_type != SCT_SANCT && justtook) {
	pr("Sector %s is now yours.\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	if (opt_MOB_ACCESS) {
	    game_tick_to_now(&sect.sct_access);
	    sect.sct_mobil = -(etu_per_update / sect_mob_neg_factor);
	} else {
	    sect.sct_mobil = 0;
	}
    }
    if (infected && sect.sct_pstage == PLG_HEALTHY)
	sect.sct_pstage = PLG_EXPOSED;
    if (vtype == I_CIVIL) {
	sect.sct_loyal
	    = (amt_dst * sect.sct_loyal + amount * loyal) / (amt_dst + amount);
	sect.sct_work
	    = (amt_dst * sect.sct_work + amount * work) / (amt_dst + amount);
    }
    putsect(&sect);
    getsect(start.sct_x, start.sct_y, &start);
    start.sct_flags &= ~MOVE_IN_PROGRESS;
    putsect(&start);
    return RET_OK;
}

/*ARGSUSED*/
static int
explore_map(coord curx, coord cury, char *arg)
{
    struct nstr_sect ns;
    struct sctstr sect;
    char view[7];
    int i;
    int changed = 0;

    snxtsct_dist(&ns, curx, cury, 1);
    i = 0;
    while (i < 7 && nxtsct(&ns, &sect)) {
	/* Nasty: this relies on the iteration order */
	view[i] = dchr[sect.sct_type].d_mnem;
	switch (sect.sct_type) {
	case SCT_WATER:
	case SCT_RURAL:
	case SCT_MOUNT:
	case SCT_WASTE:
	case SCT_PLAINS:
	    break;
	default:
	    if (sect.sct_own != player->cnum && !player->god)
		view[i] = '?';
	    break;
	}
	changed += map_set(player->cnum, ns.x, ns.y, view[i], 0);
	i++;
    }
    if (changed)
	writemap(player->cnum);
    if (!getsect(curx, cury, &sect))
	return RET_FAIL;
    pr("    %c %c         min gold fert  oil uran\n", view[0], view[1]);
    pr("   %c %c %c        %3d %4d %4d %4d %4d\n",
       view[2], view[3], view[4],
       sect.sct_min, sect.sct_gmin, sect.sct_fertil, sect.sct_oil,
       sect.sct_uran);
    pr("    %c %c\n", view[5], view[6]);
    return RET_OK;
}
