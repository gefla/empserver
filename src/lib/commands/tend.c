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
 *  tend.c: Transfer goodies from one ship to another.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1992
 *     Steve McClure, 2000
 *     Markus Armbruster, 2004-2021
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "land.h"
#include "plague.h"
#include "ship.h"

static int can_tend_to(struct shpstr *, int, struct shpstr *, int);
static int tend_comm_to(struct shpstr *, struct ichrstr *, int,
			struct shpstr *);
static void expose_ship(struct shpstr *s1, struct shpstr *s2);
static int tend_land(struct shpstr *tenderp, int, char *units);

int
c_tend(void)
{
    struct nstr_item targets;
    struct nstr_item tenders;
    struct shpstr tender;
    struct shpstr target;
    struct ichrstr *ip;
    struct mchrstr *vbase;
    int amt;
    int retval;
    int maxtender;
    int transfer;
    int total;
    int type;
    char *p;
    char prompt[512];
    char buf[1024];

    p = getstarg(player->argp[1], "Tend what commodity (or 'land')? ",
		 buf);
    if (!p || !*p)
	return RET_SYN;
    ip = item_by_name(p);
    if (!strncmp(p, "land", 4))
	type = EF_LAND;
    else if (ip)
	type = EF_SECTOR;
    else {
	pr("Can't tend '%s'\n", p);
	return RET_SYN;
    }

    if (!snxtitem(&tenders, EF_SHIP, player->argp[2], "Tender(s)? "))
	return RET_SYN;

    while (nxtitem(&tenders, &tender)) {
	if (!player->owner || !tender.shp_own) {
	    if (tenders.sel == NS_LIST)
		pr("You don't own ship #%d!\n", tender.shp_uid);
	    continue;
	}
	if (type == EF_LAND) {
	    sprintf(prompt, "Land unit(s) to tend from %s? ",
		    prship(&tender));
	    p = getstarg(player->argp[3], prompt, buf);
	    if (!p)
		return RET_FAIL;
	    if (!*p)
		continue;
	    if (!check_ship_ok(&tender))
		return RET_SYN;
	    retval = tend_land(&tender, tenders.sel == NS_LIST, p);
	    if (retval)
		return retval;
	    continue;
	}
	sprintf(prompt, "Number of %s to tend from %s? ",
		ip->i_name, prship(&tender));
	p = getstarg(player->argp[3], prompt, buf);
	if (!p)
	    return RET_FAIL;
	if (!*p)
	    continue;
	if (!check_ship_ok(&tender))
	    return RET_SYN;
	if (!(amt = atoi(p))) {
	    pr("Amount must be non-zero!\n");
	    return RET_SYN;
	}
	if (!tender.shp_item[ip->i_uid] && amt > 0) {
	    pr("No %s on %s\n", ip->i_name, prship(&tender));
	    continue;
	}
	vbase = &mchr[(int)tender.shp_type];
	maxtender = vbase->m_item[ip->i_uid];
	if (maxtender == 0) {
	    pr("%s cannot hold any %s\n", prship(&tender), ip->i_name);
	    continue;
	}
	if (!snxtitem(&targets, EF_SHIP,
		      player->argp[4], "Ships to be tended? "))
	    return RET_FAIL;
	if (!check_ship_ok(&tender))
	    return RET_SYN;
	total = 0;
	while (nxtitem(&targets, &target)) {
	    if (amt > 0) {
		if (!can_tend_to(&tender, tenders.sel == NS_LIST,
				 &target, targets.sel == NS_LIST))
		    continue;
	        transfer = tend_comm_to(&tender, ip, amt, &target);
	    } else {
		if (!player->owner) {
		    if (targets.sel == NS_LIST)
			pr("You don't own ship #%d!\n", target.shp_uid);
		    continue;
		}
		if (!can_tend_to(&target, targets.sel == NS_LIST,
				 &tender, tenders.sel == NS_LIST))
		    continue;
		transfer = tend_comm_to(&target, ip, -amt, &tender);
	    }
	    if (!transfer)
		continue;
	    total += transfer;
	    expose_ship(&tender, &target);
	    putship(target.shp_uid, &target);
	    if (amt > 0 && !tender.shp_item[ip->i_uid]) {
		pr("%s out of %s\n", prship(&tender), ip->i_name);
		break;
	    }
	    if (amt < 0 && tender.shp_item[ip->i_uid] == maxtender)
		break;
	}
	pr("%d total %s transferred %s %s\n",
	   total, ip->i_name, (amt > 0) ? "off of" : "to",
	   prship(&tender));
	tender.shp_mission = 0;
	putship(tender.shp_uid, &tender);
    }
    return RET_OK;
}

static int
can_tend_to(struct shpstr *from, int noisy_from,
	    struct shpstr *to, int noisy_to)
{
    /*
     * Careful: error messages must not disclose anything on foreign
     * @to the player doesn't already know, or could trivially learn.
     */
    if (!to->shp_own) {
	if (noisy_to)
	    pr("You don't own ship #%d!\n", to->shp_uid);
	return 0;
    }
    if (from->shp_uid == to->shp_uid) {
	if (noisy_from && noisy_to)
	    pr("%s won't tend to itself\n", prship(from));
	return 0;
    }
    if (from->shp_x != to->shp_x || from->shp_y != to->shp_y) {
	if (noisy_from && noisy_to) {
	    /* Don't disclose foreign @to exists elsewhere */
	    if (player->god || to->shp_own == player->cnum)
		pr("%s is not in the same sector as %s\n",
		   prship(to), prship(from));
	    else
		pr("You don't own ship #%d!\n", to->shp_uid);
	}
	return 0;
    }
    if (!player->god && to->shp_own != player->cnum
	&& relations_with(to->shp_own, player->cnum) < FRIENDLY) {
	if (noisy_to) {
	    /*
	     * Don't disclose unfriendly @to exists here unless
	     * lookout from @from would see it.
	     */
	    if ((mchr[from->shp_type].m_flags & M_SUB)
		|| (mchr[to->shp_type].m_flags & M_SUB))
		pr("You don't own ship #%d!\n", from->shp_uid);
	    else
		pr("You are not on friendly terms with"
		   " the owner of ship #%d!\n",
		   to->shp_uid);
	}
	return 0;
    }
    return 1;
}

static int
tend_comm_to(struct shpstr *from, struct ichrstr *ip, int amt,
	     struct shpstr *to)
{
    int can_give = from->shp_item[ip->i_uid];
    int to_max = mchr[to->shp_type].m_item[ip->i_uid];
    int can_take = to_max - to->shp_item[ip->i_uid];
    int transfer;

    if (!to_max) {
	pr("%s cannot hold any %s\n", prship(to), ip->i_name);
	return 0;
    }
    if (ip->i_uid == I_CIVIL && from->shp_own != to->shp_own) {
	pr("%s civilians refuse to board %s!\n",
	   from->shp_own == player->cnum ? "Your" : "Foreign",
	   prship(to));
	return 0;
    }
    if (!can_give) {
	pr("No %s on %s\n", ip->i_name, prship(from));
	return 0;
    }
    if (!can_take) {
	pr("%s can't hold more %s\n", prship(to), ip->i_name);
	return 0;
    }

    transfer = MIN(can_give, amt);
    transfer = MIN(can_take, transfer);
    from->shp_item[ip->i_uid] -= transfer;
    to->shp_item[ip->i_uid] += transfer;
    if (to->shp_own != player->cnum) {
	wu(0, to->shp_own, "%s tended %d %s to %s\n",
	   cname(player->cnum), transfer, ip->i_name, prship(to));
    }

    return transfer;
}

static void
expose_ship(struct shpstr *s1, struct shpstr *s2)
{
    if (s1->shp_pstage == PLG_INFECT && s2->shp_pstage == PLG_HEALTHY)
	s2->shp_pstage = PLG_EXPOSED;
    if (s2->shp_pstage == PLG_INFECT && s1->shp_pstage == PLG_HEALTHY)
	s1->shp_pstage = PLG_EXPOSED;
}

static int
tend_land(struct shpstr *tenderp, int noisy, char *units)
{
    struct nstr_item lni;
    struct nstr_item targets;
    struct shpstr target;
    struct lndstr land;
    char buf[1024];

    if (!snxtitem(&lni, EF_LAND, units, NULL))
	return RET_SYN;

    while (nxtitem(&lni, &land)) {
	if (!player->owner || !land.lnd_own) {
	    if (lni.sel == NS_LIST)
		pr("You don't own land unit #%d!\n", land.lnd_uid);
	    continue;
	}
	if (land.lnd_ship != tenderp->shp_uid) {
	    if (lni.sel == NS_LIST)
		pr("%s is not on %s!\n", prland(&land), prship(tenderp));
	    continue;
	}
	if (!(lchr[(int)land.lnd_type].l_flags & L_ASSAULT)) {
	    pr("%s does not have \"assault\" capability and can't be tended\n",
	       prland(&land));
	    continue;
	}
	if (!snxtitem(&targets, EF_SHIP,
		      player->argp[4], "Ship to be tended? "))
	    return RET_FAIL;
	if (!check_ship_ok(tenderp) || !check_land_ok(&land))
	    return RET_SYN;
	while (nxtitem(&targets, &target)) {
	    if (!can_tend_to(tenderp, noisy,
			     &target, targets.sel == NS_LIST))
		continue;

	    /* Fit unit on ship */
	    getship(target.shp_uid, &target);

	    if ((!(lchr[(int)land.lnd_type].l_flags & L_LIGHT)) &&
		(!((mchr[(int)target.shp_type].m_flags & M_SUPPLY) &&
		   (!(mchr[(int)target.shp_type].m_flags & M_SUB))))) {
		pr("You can only load light units onto ships,\n"
		   "unless the ship is a non-sub supply ship\n"
		   "%s not tended\n", prland(&land));
		continue;
	    }

	    if ((mchr[(int)target.shp_type].m_flags & M_SUB) &&
		(lchr[(int)land.lnd_type].l_flags & L_SPY) &&
		!mchr[(int)target.shp_type].m_nland) {
		if (shp_nland(&target) > 1) {
		    pr("%s doesn't have room for more than two spy units!\n",
		       prship(&target));
		    continue;
		}
	    } else if (shp_nland(&target) >= mchr[target.shp_type].m_nland) {
		if (mchr[(int)target.shp_type].m_nland)
		    pr("%s doesn't have room for any more land units!\n",
		       prship(&target));
		else
		    pr("%s doesn't carry land units!\n", prship(&target));
		continue;
	    }
	    pr("%s transferred from %s to %s\n",
	       prland(&land), prship(tenderp), prship(&target));
	    sprintf(buf, "loaded on your %s at %s",
		    prship(&target), xyas(target.shp_x, target.shp_y,
					  target.shp_own));
	    gift(target.shp_own, player->cnum, &land, buf);
	    land.lnd_ship = target.shp_uid;
	    land.lnd_harden = 0;
	    putland(land.lnd_uid, &land);
	    expose_ship(tenderp, &target);
	    putship(target.shp_uid, &target);
	    putship(tenderp->shp_uid, tenderp);
	    break;
	}
    }
    return 0;
}
