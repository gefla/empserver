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
 *  expl.c: Take over unoccupied sectors
 * 
 *  Known contributors to this file:
 *     Jeff Wallace, 1989
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "item.h"
#include "file.h"
#include "nat.h"
#include "optlist.h"
#include "commands.h"

static int explore_map(s_char *what, coord curx, coord cury, s_char *arg);

int
explore(void)
{
    register int amount;
    struct sctstr sect;
    struct sctstr endsect;
    struct sctstr start;
    struct sctstr chksect;
    double weight;
    int mcost, dam;
    int infected;
    int vtype;
    int amt_src;
    int amt_dst;
    struct ichrstr *ip;
    int work;
    int loyal;
    int own, mob;
    int justtook;
    coord x, y;
    s_char *p;
    int n;
    int left;
    s_char buf[1024];
    s_char prompt[128];

    if ((ip =
	 whatitem(player->argp[1], "explore with what? (civ/mil) ")) == 0)
	return RET_SYN;
    vtype = ip->i_vtype;
    if ((vtype != V_CIVIL) && (vtype != V_MILIT)) {
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
    infected = getvar(V_PSTAGE, (s_char *)&sect, EF_SECTOR) == PLG_INFECT;
    if ((amt_src = getvar(vtype, (s_char *)&sect, EF_SECTOR)) <= 0) {
	pr("No %s in %s\n", ip->i_name,
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_SYN;
    }
    own = sect.sct_own;
    mob = (int)sect.sct_mobil;
    if (vtype == V_CIVIL && sect.sct_oldown != own) {
	pr("You can't explore with conquered populace!\n");
	return RET_SYN;
    }
    if (mob <= 0) {
	pr("No mobility in %s\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_SYN;
    }
    if (vtype == V_CIVIL) {
	work = sect.sct_work;
	if (work != 100)
	    pr("Warning: civil unrest\n");
	loyal = sect.sct_loyal;
    } else if (vtype == V_MILIT) {
	work = 100;
	loyal = 0;
    }
    sprintf(prompt, "Number of %s to explore with? (max %d) ",
	    ip->i_name, amt_src);
    amount = onearg(player->argp[3], prompt);
    if (!check_sect_ok(&sect))
	return RET_FAIL;
    if (amount > amt_src) {
	amount = amt_src;
	pr("Only exploring with %d.\n", amount);
    }

    if (!want_to_abandon(&sect, vtype, amount, 0)) {
	pr("Explore cancelled.\n");
	return RET_FAIL;
    }

    if (!check_sect_ok(&sect))
	return RET_FAIL;

    if (amount <= 0)
	return RET_SYN;
    weight = amount * ip->i_lbs;
    /* remove commodities from source sector */
    getsect(x, y, &start);
    amt_src = getvar(vtype, (s_char *)&start, EF_SECTOR);
    amt_src -= amount;
    if (amt_src < 0) {
	pr("%s in %s are gone!\n", ip->i_name,
	   xyas(start.sct_x, start.sct_y, player->cnum));
	return RET_OK;
    }
    putvar(vtype, amt_src, (s_char *)&start, EF_SECTOR);
    start.sct_flags |= MOVE_IN_PROGRESS;
    putsect(&start);
    /*
     * Now parse the path and return ending sector.
     */
    dam = 1;
    mcost = move_ground((s_char *)ip, &sect, &endsect,
			weight, player->argp[4],
			explore_map, 1, &dam);

    if (dam) {
	left = effdamage(amount, dam);
	if (left < amount) {
	    if (left) {
		pr("%d of the %s you were exploring with were destroyed!\nOnly %d %s made it to %s\n", amount - left, ip->i_name, left, ip->i_name, xyas(endsect.sct_x, endsect.sct_y, player->cnum));
	    } else {
		pr("All of the %s you were exploring with were destroyed!\n", ip->i_name);
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
	sect.sct_mobil = (u_char)mob;
    } else {
	/* Charge mobility */
	getsect(sect.sct_x, sect.sct_y, &sect);
	n = sect.sct_mobil - mcost;
	if (n < 0)
	    n = 0;
	sect.sct_mobil = (u_char)n;
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
    if (chksect.sct_type == '.') {
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
	(void)takeover(&sect, player->cnum);
	justtook = 1;
	sect.sct_oldown = own;
	sect.sct_work = work;
	sect.sct_loyal = loyal;
    }
    if (vtype == V_CIVIL && sect.sct_oldown != player->cnum) {
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
    getsect(start.sct_x, start.sct_y, &start);
    start.sct_flags &= ~MOVE_IN_PROGRESS;
    putsect(&start);
    amt_dst = getvar(vtype, (s_char *)&sect, EF_SECTOR);
    if (32767 - amt_dst < amount) {
	amount = 32767 - amt_dst;
	pr("Only %d can be left there.\n", amount);
	if (amount <= 0)
	    getsect(start.sct_x, start.sct_y, &sect);
    }
    if (putvar(vtype, amount + amt_dst, (s_char *)&sect, EF_SECTOR) < 0) {
	pr("No more room in %s.\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_OK;
    }
    /*
     * Now add commodities to destination sector,
     * along with plague that came along for the ride.
     * Takeover unowned sectors if not deity.
     */
    if (!player->god && sect.sct_type != SCT_SANCT && justtook) {
	pr("Sector %s is now yours.\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	if (opt_MOB_ACCESS) {
	    time(&sect.sct_access);
	    sect.sct_mobil = -(etu_per_update / sect_mob_neg_factor);
	} else {
	    sect.sct_mobil = 0;
	}
    }
    if (infected && getvar(V_PSTAGE, (s_char *)&sect, EF_SECTOR) == 0)
	putvar(V_PSTAGE, PLG_EXPOSED, (s_char *)&sect, EF_SECTOR);
    if (vtype == V_CIVIL) {
	if (opt_NEW_WORK) {
	    sect.sct_loyal = ((amt_dst * sect.sct_loyal) +
			      (amount * loyal)) / (amt_dst + amount);
	    sect.sct_work = ((amt_dst * sect.sct_work) +
			     (amount * work)) / (amt_dst + amount);
	} else {		/* ! NEW_WORK */

	    /* It only takes one bad apple... */
	    if (sect.sct_loyal < loyal)
		sect.sct_loyal = loyal;
	    if (sect.sct_work > work)
		sect.sct_work = work;
	}			/* end NEW_WORK */
    }
    putsect(&sect);
    return RET_OK;
}

/*ARGSUSED*/
static int
explore_map(s_char *what, coord curx, coord cury, s_char *arg)
{
    struct nstr_sect ns;
    struct natstr *np;
    struct sctstr sect;
    coord rel_x, rel_y;
    s_char range[128];
    s_char view[7];
    int i;
    int changed = 0;

    np = getnatp(player->cnum);
    rel_x = xrel(np, curx);
    rel_y = yrel(np, cury);
    sprintf(range, "%d:%d,%d:%d", rel_x - 2, rel_x + 2, rel_y - 1,
	    rel_y + 1);
    if (!snxtsct(&ns, range))
	return RET_FAIL;
    i = 0;
    while (i < 7 && nxtsct(&ns, &sect)) {
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
