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
 *  load.c: load/unload goods from a sector onto a ship or land unit
 *
 *  Known contributors to this file:
 *     David Sharnoff, 1987
 *     Ken Stevens, 1995 (rewritten)
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2021
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "item.h"
#include "land.h"
#include "optlist.h"
#include "plague.h"
#include "plane.h"
#include "ship.h"
#include "unit.h"

static int load_plane_ship(struct sctstr *sectp, struct shpstr *sp,
			   int noisy, int loading, int *nshipsp);
static int load_land_ship(struct sctstr *sectp, struct shpstr *sp,
			  int noisy, int loading, int *nshipsp);
static int load_comm_ship(struct sctstr *sectp, struct shpstr *sp,
			  struct ichrstr *ich, int loading,
			  int *nshipsp);
static int load_plane_land(struct sctstr *sectp, struct lndstr *lp,
			   int noisy, int loading, int *nunitsp);
static int load_land_land(struct sctstr *sectp, struct lndstr *lp,
			  int noisy, int loading, int *nunitsp);
static int load_comm_land(struct sctstr *sectp, struct lndstr *lp,
			  struct ichrstr *ich, int loading,
			  int *nunitsp);

int
c_load(void)
{
    int loading = **player->argp == 'l';
    int noisy;
    int type;
    struct nstr_item nbst;
    struct ichrstr *ich;
    int nships;
    struct sctstr sect;
    struct shpstr ship;
    int retval;
    char *p;
    char buf[1024];

    p = getstarg(player->argp[1],
		 "What commodity (or 'plane' or 'land')? ", buf);
    if (!p || !*p)
	return RET_SYN;
    ich = item_by_name(p);
    if (!strncmp(p, "plane", 5))
	type = EF_PLANE;
    else if (!strncmp(p, "land", 4))
	type = EF_LAND;
    else if (ich)
	type = EF_SECTOR;
    else {
	pr("Can't %sload '%s'\n", loading ? "" : "un", p);
	return RET_SYN;
    }

    p = getstarg(player->argp[2], "Ship(s): ", buf);
    if (!p || !*p)
	return RET_SYN;

    if (!snxtitem(&nbst, EF_SHIP, p, NULL))
	return RET_SYN;

    noisy = nbst.sel == NS_LIST;

    nships = 0;
    while (nxtitem(&nbst, &ship)) {
	if (!ship.shp_own)
	    continue;
	if (!player->owner) {
	    if (!loading || !noisy)
		continue;
	    if (relations_with(ship.shp_own, player->cnum) < FRIENDLY)
		continue;
	}

	if (!getsect(ship.shp_x, ship.shp_y, &sect))	/* XXX */
	    continue;
	if (!player->owner) {
	    if (ship.shp_own != player->cnum)
		continue;
	    if (!sect_has_dock(&sect))
		continue;
	    if (loading) {
		if (noisy)
		    pr("You don't own %s\n",
		       xyas(sect.sct_x, sect.sct_y, player->cnum));
		continue;
	    }
	}
	if (!sect_has_dock(&sect)) {
	    if (noisy)
		pr("Sector %s is not a harbor or canal.\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum));
	    continue;
	}
	if (!loading
	    && !player->owner
	    && relations_with(sect.sct_own, player->cnum) < FRIENDLY) {
	    if (noisy)
		pr("You can't unload into an unfriendly %s\n",
		   dchr[sect.sct_type].d_name);
	    continue;
	}
	if (sect.sct_effic < 2) {
	    if (noisy)
		pr("The %s at %s is not 2%% efficient yet.\n",
		   dchr[sect.sct_type].d_name,
		   xyas(sect.sct_x, sect.sct_y, player->cnum));
	    continue;
	}

	if (opt_MARKET) {
	    if (ontradingblock(EF_SHIP, &ship)) {
		if (noisy)
		    pr("%s is on the trading block\n", prship(&ship));
		continue;
	    }
	}

	switch (type) {
	case EF_PLANE:
	    retval = load_plane_ship(&sect, &ship, noisy, loading, &nships);
	    if (retval != 0)
		return retval;
	    break;
	case EF_LAND:
	    retval = load_land_ship(&sect, &ship, noisy, loading, &nships);
	    if (retval != 0)
		return retval;
	    break;
	case EF_SECTOR:
	    retval = load_comm_ship(&sect, &ship, ich, loading, &nships);
	    if (retval != 0)
		return retval;
	}
	/* load/unload plague */
	if (sect.sct_pstage == PLG_INFECT
	    && ship.shp_pstage == PLG_HEALTHY)
	    ship.shp_pstage = PLG_EXPOSED;
	if (ship.shp_pstage == PLG_INFECT
	    && sect.sct_pstage == PLG_HEALTHY)
	    sect.sct_pstage = PLG_EXPOSED;
	putsect(&sect);
	putship(ship.shp_uid, &ship);
    }
    if (!nships)
	pr("No ships affected\n");
    else
	pr("%d ship%s %sloaded\n", nships, splur(nships),
	   loading ? "" : "un");
    return RET_OK;
}

int
c_lload(void)
{
    int loading = player->argp[0][1] == 'l';
    int noisy;
    int type;
    struct nstr_item nbst;
    struct ichrstr *ich;
    int nunits;
    struct sctstr sect;
    struct lndstr land;
    int retval;
    char *p;
    char buf[1024];

    p = getstarg(player->argp[1],
		 "What commodity (or 'plane' or 'land')? ", buf);
    if (!p || !*p)
	return RET_SYN;
    ich = item_by_name(p);
    if (!strncmp(p, "plane", 5))
	type = EF_PLANE;
    else if (!strncmp(p, "land", 4))
	type = EF_LAND;
    else if (ich)
	type = EF_SECTOR;
    else {
	pr("Can't %sload '%s'\n", loading ? "" : "un", p);
	return RET_SYN;
    }

    p = getstarg(player->argp[2], "Unit(s): ", buf);
    if (!p || !*p)
	return RET_SYN;

    if (!snxtitem(&nbst, EF_LAND, p, NULL))
	return RET_SYN;

    noisy = nbst.sel == NS_LIST;

    nunits = 0;
    while (nxtitem(&nbst, &land)) {
	if (land.lnd_own == 0)
	    continue;
	if (!player->owner) {
	    if (!loading || !noisy)
		continue;
	    if (relations_with(land.lnd_own, player->cnum) != ALLIED)
		continue;
	}

	if (!getsect(land.lnd_x, land.lnd_y, &sect))	/* XXX */
	    continue;
	if (!player->owner) {
	    if (land.lnd_own != player->cnum)
		continue;
	    if (loading) {
		if (noisy)
		    pr("Sector %s is not yours.\n",
		       xyas(sect.sct_x, sect.sct_y, player->cnum));
		continue;
	    }
	    if (relations_with(sect.sct_own, player->cnum) != ALLIED) {
		if (noisy)
		    pr("Sector %s is not yours.\n",
		       xyas(sect.sct_x, sect.sct_y, player->cnum));
		continue;
	    }
	}

	if (opt_MARKET) {
	    if (ontradingblock(EF_LAND, &land)) {
		if (noisy)
		    pr("%s is on the trading block\n", prland(&land));
		continue;
	    }
	}

	switch (type) {
	case EF_LAND:
	    retval = load_land_land(&sect, &land, noisy, loading, &nunits);
	    if (retval != 0)
		return retval;
	    break;
	case EF_PLANE:
	    retval = load_plane_land(&sect, &land, noisy, loading, &nunits);
	    if (retval != 0)
		return retval;
	    break;
	case EF_SECTOR:
	    retval = load_comm_land(&sect, &land, ich, loading, &nunits);
	    if (retval != 0)
		return retval;
	}
	/* load/unload plague */
	if (sect.sct_pstage == PLG_INFECT
	    && land.lnd_pstage == PLG_HEALTHY)
	    land.lnd_pstage = PLG_EXPOSED;
	if (land.lnd_pstage == PLG_INFECT
	    && sect.sct_pstage == PLG_HEALTHY)
	    sect.sct_pstage = PLG_EXPOSED;

	putsect(&sect);
	putland(land.lnd_uid, &land);
    }
    if (nunits == 0)
	pr("No units affected\n");
    else
	pr("%d unit%s %sloaded\n", nunits, splur(nunits),
	   loading ? "" : "un");
    return RET_OK;
}

static int
move_amount(int sect_amt, int unit_amt, int unit_max,
	   int loading, int amount)
{
    int move_amt;

    if (amount < 0)
	move_amt = -amount - unit_amt;
    else
	move_amt = loading ? amount : -amount;
    move_amt = LIMIT_TO(move_amt, -unit_amt, unit_max - unit_amt);
    move_amt = LIMIT_TO(move_amt, sect_amt - ITEM_MAX, sect_amt);
    return move_amt;
}

int
load_comm_ok(struct sctstr *sectp, natid unit_own,
	     i_type item, int move_amt)
{
    if (!move_amt)
	return 0;
    if (move_amt < 0 && !player->god && unit_own != player->cnum)
	return 0;
    if (move_amt > 0 && !player->god && sectp->sct_own != player->cnum)
	return 0;
    if (sectp->sct_oldown != unit_own && item == I_CIVIL) {
	pr("%s civilians refuse to %s at %s!\n",
	   (move_amt < 0 ? unit_own : sectp->sct_oldown) == player->cnum
	   ? "Your" : "Foreign",
	   move_amt < 0 ? "disembark" : "board",
	   xyas(sectp->sct_x, sectp->sct_y, player->cnum));
	return 0;
    }
    return 1;
}

void
gift(natid givee, natid giver, void *ptr, char *mesg)
{
    if (giver != givee)
	wu(0, givee, "%s %s %s\n", cname(giver), unit_nameof(ptr), mesg);
    unit_give_away(ptr, givee, 0);
}

static int
still_ok_ship(struct sctstr *sectp, struct shpstr *shipp)
{
    if (!check_sect_ok(sectp))
	return 0;
    if (!check_ship_ok(shipp))
	return 0;
    return 1;
}

static int
still_ok_land(struct sctstr *sectp, struct lndstr *landp)
{
    if (!check_sect_ok(sectp))
	return 0;
    if (!check_land_ok(landp))
	return 0;
    return 1;
}

static int
plane_loadable(struct plnstr *pp, int noisy)
{
    if (pp->pln_ship >= 0) {
	if (noisy)
	    pr("%s is already on ship #%d!\n",
	       prplane(pp), pp->pln_ship);
	return 0;
    }
    if (pp->pln_land >= 0) {
	if (noisy)
	    pr("%s is already on land unit #%d!\n",
	       prplane(pp), pp->pln_land);
	return 0;
    }
    if (pp->pln_harden) {
	if (noisy)
	    pr("%s has been hardened and can't be loaded\n",
	       prplane(pp));
	return 0;
    }
    if (pln_is_in_orbit(pp)) {
	if (noisy)
	    pr("%s is in space\n", prplane(pp));
	return 0;
    }
    return 1;
}

static int
land_loadable(struct lndstr *lp, int noisy)
{
    if (lp->lnd_ship >= 0) {
	if (noisy)
	    pr("%s is already on ship #%d!\n",
	       prland(lp), lp->lnd_ship);
	return 0;
    }
    if (lp->lnd_land >= 0) {
	if (noisy)
	    pr("%s is already on land #%d!\n",
	       prland(lp), lp->lnd_land);
	return 0;
    }
    if (lnd_first_on_land(lp) >= 0) {
	/* Outlawed to prevent arbitrarily deep recursion */
	if (noisy)
	    pr("%s cannot be loaded since it is carrying units\n",
	       prland(lp));
	return 0;
    }
    if (lchr[lp->lnd_type].l_flags & L_HEAVY) {
	if (noisy)
	    pr("%s is too heavy to load.\n", prland(lp));
	return 0;
    }
    return 1;
}

static int
load_plane_ship(struct sctstr *sectp, struct shpstr *sp, int noisy,
		int loading, int *nshipsp)
{
    struct nstr_item ni;
    struct plnstr pln;
    int loaded = 0;
    char buf[1024];
    char *p;
    char prompt[512];
    struct mchrstr *mcp = mchr + sp->shp_type;

    if (mcp->m_nplanes + mcp->m_nchoppers + mcp->m_nxlight == 0) {
	if (noisy)
	    pr("%s cannot carry planes\n", prship(sp));
	return 0;
    }
    if (loading &&
	shp_nplane(sp, NULL, NULL, NULL)
		>= mcp->m_nchoppers + mcp->m_nxlight + mcp->m_nplanes) {
	pr("%s doesn't have room for any more planes\n", prship(sp));
	return 0;
    }
    sprintf(prompt, "Plane(s) to %s %s? ",
	    loading ? "load onto" : "unload from", prship(sp));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p)
	return RET_SYN;
    if (!snxtitem(&ni, EF_PLANE, p, NULL))
	return RET_SYN;

    if (!still_ok_ship(sectp, sp))
	return RET_SYN;

    noisy = ni.sel == NS_LIST;

    while (nxtitem(&ni, &pln)) {
	if (!player->owner)
	    continue;
	if (!(plchr[(int)pln.pln_type].pl_flags & P_L)
	    && !(plchr[(int)pln.pln_type].pl_flags & P_E)
	    && !(plchr[(int)pln.pln_type].pl_flags & P_K)
	    && !(plchr[(int)pln.pln_type].pl_flags & P_M)
	    ) {
	    if (noisy)
		pr("You can only load light planes, helos, xtra-light, or missiles onto ships.\n");
	    continue;
	}
	if (loading && !plane_loadable(&pln, noisy))
	    continue;

	if (!loading) {
	    if (pln.pln_ship != sp->shp_uid)
		continue;
	} else if (sp->shp_x != pln.pln_x || sp->shp_y != pln.pln_y)
	    continue;

	if (!could_be_on_ship(&pln, sp)) {
	    if (noisy) {
		if (plchr[(int)pln.pln_type].pl_flags & P_K)
		    p = "choppers";
		else if (plchr[(int)pln.pln_type].pl_flags & P_E)
		    p = "extra light planes";
		else if (plchr[(int)pln.pln_type].pl_flags & P_M)
		    p = "missiles";
		else
		    p = "planes";
		pr("%s cannot carry %s.\n", prship(sp), p);
	    }
	    continue;
	}
	/* Fit plane on ship */
	if (loading) {
	    if (!put_plane_on_ship(&pln, sp)) {
		pr("Can't put plane %d on this ship!\n", pln.pln_uid);
		continue;
	    }
	    sprintf(buf, "loaded on your %s at %s",
		    prship(sp), xyas(sp->shp_x, sp->shp_y, sp->shp_own));
	    gift(sp->shp_own, player->cnum, &pln, buf);
	    putplane(pln.pln_uid, &pln);
	} else {
	    pln.pln_ship = -1;
	    sprintf(buf, "unloaded in your %s at %s",
		    dchr[sectp->sct_type].d_name,
		    xyas(sectp->sct_x, sectp->sct_y, sectp->sct_own));
	    gift(sectp->sct_own, player->cnum, &pln, buf);
	    putplane(pln.pln_uid, &pln);
	}
	pr("%s %s %s at %s.\n",
	   prplane(&pln),
	   loading ? "loaded onto" : "unloaded from",
	   prship(sp), xyas(sp->shp_x, sp->shp_y, player->cnum));
	loaded = 1;
    }
    *nshipsp += loaded;
    return 0;
}

static int
load_land_ship(struct sctstr *sectp, struct shpstr *sp, int noisy,
	       int loading, int *nshipsp)
{
    struct nstr_item ni;
    struct lndstr land;
    int loaded = 0;
    char *p;
    char prompt[512];
    char buf[1024];
    int load_spy = 0;

    if (!mchr[(int)sp->shp_type].m_nland
	&& !(mchr[sp->shp_type].m_flags & M_SUB)) {
	if (noisy)
	    pr("%s cannot carry land units!\n", prship(sp));
	return 0;
    }
    if (loading) {
	if ((mchr[(int)sp->shp_type].m_flags & M_SUB) &&
	    (mchr[(int)sp->shp_type].m_nland == 0)) {
	    if (shp_nland(sp) >= 2) {
		pr("Non-land unit carrying subs can only carry up to two spy units.\n");
		return 0;
	    }
	    /* Eh, let 'em load a spy only */
	    load_spy = 1;
	}
	if (!load_spy && shp_nland(sp) >= mchr[sp->shp_type].m_nland) {
	    pr("%s doesn't have room for any more land units!\n",
	       prship(sp));
	    return 0;
	}
    }
    sprintf(prompt, "Land unit(s) to %s %s? ",
	    loading ? "load onto" : "unload from", prship(sp));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p)
	return RET_SYN;
    if (!snxtitem(&ni, EF_LAND, p, NULL))
	return RET_SYN;

    if (!still_ok_ship(sectp, sp))
	return RET_SYN;

    noisy = ni.sel == NS_LIST;

    while (nxtitem(&ni, &land)) {
	if (!player->owner)
	    continue;

	if (loading) {
	    if (!land_loadable(&land, noisy))
		continue;
	    if (load_spy && !(lchr[(int)land.lnd_type].l_flags & L_SPY)) {
		if (noisy)
		    pr("Subs can only carry spy units.\n");
		continue;
	    }
	}

	/* Unit sanity done */
	/* Find the right ship */
	if (!loading) {
	    if (land.lnd_ship != sp->shp_uid)
		continue;
	    if (land.lnd_land > -1)
		continue;
	} else if (sp->shp_x != land.lnd_x || sp->shp_y != land.lnd_y)
	    continue;

	if ((!(lchr[(int)land.lnd_type].l_flags & L_LIGHT)) &&
	    (!((mchr[(int)sp->shp_type].m_flags & M_SUPPLY) &&
	       (!(mchr[(int)sp->shp_type].m_flags & M_SUB))))) {
	    if (noisy) {
		pr("You can only load light units onto ships,\n");
		pr("unless the ship is a non-sub supply ship\n");
		pr("%s not loaded\n", prland(&land));
	    }
	    continue;
	}
	/* Fit unit on ship */
	if (loading) {
	    if (load_spy) {
		if (shp_nland(sp) >= 2) {
		    pr("Non-land unit carrying subs can only carry up to two spy units.\n");
		    return 0;
		}
	    } else {
		if (shp_nland(sp) >= mchr[sp->shp_type].m_nland) {
		    pr("%s doesn't have room for any more land units!\n",
		       prship(sp));
		    return 0;
		}
	    }
	    sprintf(buf, "loaded on your %s at %s",
		    prship(sp), xyas(sp->shp_x, sp->shp_y, sp->shp_own));
	    gift(sp->shp_own, player->cnum, &land, buf);
	    land.lnd_ship = sp->shp_uid;
	    land.lnd_harden = 0;
	    putland(land.lnd_uid, &land);
#if 0
	   /*
	    * FIXME if this supplies from the sector, the putsect in
	    * load() / lload() duplicates those supplies, causing a
	    * seqno mismatch
	    */
	    if (!lnd_supply_all(&land))
		pr("WARNING: %s is out of supply!\n", prland(&land));
#else
	    if (!lnd_in_supply(&land))
		pr("WARNING: %s is out of supply!\n", prland(&land));
#endif
	} else {
	    sprintf(buf, "unloaded in your %s at %s",
		    dchr[sectp->sct_type].d_name,
		    xyas(sectp->sct_x, sectp->sct_y, sectp->sct_own));

	    /* Spies are unloaded quietly, others aren't */
	    if (!(lchr[(int)land.lnd_type].l_flags & L_SPY))
		gift(sectp->sct_own, player->cnum, &land, buf);
	    land.lnd_ship = -1;
	    putland(land.lnd_uid, &land);
	}
	pr("%s %s %s at %s.\n",
	   prland(&land),
	   loading ? "loaded onto" : "unloaded from",
	   prship(sp), xyas(sp->shp_x, sp->shp_y, player->cnum));
	loaded = 1;
    }
    *nshipsp += loaded;
    return 0;
}

static int
load_comm_ship(struct sctstr *sectp, struct shpstr *sp,
	       struct ichrstr *ich, int loading, int *nshipsp)
{
    i_type item = ich->i_uid;
    struct mchrstr *mcp = &mchr[(int)sp->shp_type];
    int ship_amt, sect_amt, move_amt;
    char prompt[512];
    char *p;
    char buf[1024];

    sprintf(prompt, "Number of %s to %s %s at %s? ",
	    ich->i_name,
	    loading ? "load onto" : "unload from",
	    prship(sp), xyas(sp->shp_x, sp->shp_y, player->cnum));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p || !*p)
	return RET_SYN;

    if (!still_ok_ship(sectp, sp))
	return RET_SYN;

    ship_amt = sp->shp_item[item];
    sect_amt = sectp->sct_item[item];
    move_amt = move_amount(sect_amt, ship_amt, mcp->m_item[item],
			   loading, atoi(p));
    if (!load_comm_ok(sectp, sp->shp_own, item, move_amt))
	return RET_OK;
    if (!abandon_askyn(sectp, item, move_amt, NULL))
	return RET_FAIL;
    if (!still_ok_ship(sectp, sp))
	return RET_SYN;
    sectp->sct_item[item] = sect_amt - move_amt;
    sp->shp_item[item] = ship_amt + move_amt;

    if (move_amt >= 0) {
	pr("%d %s loaded onto %s at %s\n",
	   move_amt, ich->i_name,
	   prship(sp), xyas(sp->shp_x, sp->shp_y, player->cnum));
	if (sp->shp_own != player->cnum) {
	    wu(0, sp->shp_own, "%s loaded %d %s onto %s at %s\n",
	       cname(player->cnum), move_amt, ich->i_name,
	       prship(sp), xyas(sp->shp_x, sp->shp_y, sp->shp_own));
	}
    } else {
	pr("%d %s unloaded from %s at %s\n",
	   -move_amt, ich->i_name,
	   prship(sp), xyas(sp->shp_x, sp->shp_y, player->cnum));
	if (sectp->sct_own != player->cnum) {
	    wu(0, sectp->sct_own, "%s unloaded %d %s from %s at %s\n",
	       cname(player->cnum), -move_amt, ich->i_name,
	       prship(sp), xyas(sp->shp_x, sp->shp_y, sectp->sct_own));
	}
    }
    ++*nshipsp;
    return 0;
}

static int
load_plane_land(struct sctstr *sectp, struct lndstr *lp, int noisy,
		int loading, int *nunitsp)
{
    struct nstr_item ni;
    struct plnstr pln;
    int loaded = 0;
    char *p;
    char prompt[512];
    char buf[1024];
    struct lchrstr *lcp = lchr + lp->lnd_type;

    if (!lcp->l_nxlight) {
	if (noisy)
	    pr("%s cannot carry extra-light planes.\n", prland(lp));
	return 0;
    }
    if (loading && lnd_nxlight(lp) >= lcp->l_nxlight) {
	pr("%s doesn't have room for any more extra-light planes\n",
	   prland(lp));
	return 0;
    }
    sprintf(prompt, "Plane(s) to %s %s? ",
	    loading ? "load onto" : "unload from", prland(lp));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p)
	return RET_SYN;
    if (!snxtitem(&ni, EF_PLANE, p, NULL))
	return RET_SYN;

    if (!still_ok_land(sectp, lp))
	return RET_SYN;

    noisy = ni.sel == NS_LIST;

    while (nxtitem(&ni, &pln)) {
	if (!player->owner)
	    continue;

	if (!(plchr[(int)pln.pln_type].pl_flags & P_E)) {
	    if (noisy)
		pr("You can only load xlight planes onto units.\n");
	    continue;
	}
	if (loading && !plane_loadable(&pln, noisy))
	    continue;

	/* Plane sanity done */
	/* Find the right unit */
	if (!loading) {
	    if (pln.pln_land != lp->lnd_uid)
		continue;
	} else if (lp->lnd_x != pln.pln_x || lp->lnd_y != pln.pln_y)
	    continue;

	/* Fit plane on unit */
	if (loading) {
	    if (!put_plane_on_land(&pln, lp)) {
		pr("Can't put plane %d on this unit!\n", pln.pln_uid);
		continue;
	    }
	    sprintf(buf, "loaded on %s at %s",
		    prland(lp), xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));
	    gift(lp->lnd_own, player->cnum, &pln, buf);
	    putplane(pln.pln_uid, &pln);
	} else {
	    pln.pln_land = -1;
	    sprintf(buf, "unloaded at your sector at %s",
		    xyas(sectp->sct_x, sectp->sct_y, sectp->sct_own));
	    gift(sectp->sct_own, player->cnum, &pln, buf);
	    putplane(pln.pln_uid, &pln);
	}
	pr("%s %s %s at %s.\n",
	   prplane(&pln),
	   loading ? "loaded onto" : "unloaded from",
	   prland(lp), xyas(lp->lnd_x, lp->lnd_y, player->cnum));
	loaded = 1;
    }
    *nunitsp += loaded;
    return 0;
}

static int
load_comm_land(struct sctstr *sectp, struct lndstr *lp,
	       struct ichrstr *ich, int loading, int *nunitsp)
{
    i_type item = ich->i_uid;
    struct lchrstr *lcp = &lchr[(int)lp->lnd_type];
    int land_amt, sect_amt, move_amt;
    char prompt[512];
    char *p;
    char buf[1024];

    sprintf(prompt, "Number of %s to %s %s at %s? ",
	    ich->i_name,
	    loading ? "load onto" : "unload from",
	    prland(lp), xyas(lp->lnd_x, lp->lnd_y, player->cnum));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p || !*p)
	return RET_SYN;

    if (!still_ok_land(sectp, lp))
	return RET_SYN;

    land_amt = lp->lnd_item[item];
    sect_amt = sectp->sct_item[item];
    move_amt = move_amount(sect_amt, land_amt, lcp->l_item[item],
			   loading, atoi(p));
    if (!load_comm_ok(sectp, lp->lnd_own, item, move_amt))
	return RET_OK;
    sectp->sct_item[item] = sect_amt - move_amt;
    lp->lnd_item[item] = land_amt + move_amt;

    /* Did we put mils onto this unit? If so, reset the fortification */
    if (item == I_MILIT && move_amt > 0)
	lp->lnd_harden = 0;

    if (move_amt >= 0) {
	pr("%d %s loaded onto %s at %s\n",
	   move_amt, ich->i_name,
	   prland(lp), xyas(lp->lnd_x, lp->lnd_y, player->cnum));
	if (lp->lnd_own != player->cnum) {
	    wu(0, lp->lnd_own, "%s loaded %d %s onto %s at %s\n",
	       cname(player->cnum), move_amt, ich->i_name,
	       prland(lp), xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));
	}
    } else {
	pr("%d %s unloaded from %s at %s\n",
	   -move_amt, ich->i_name,
	   prland(lp), xyas(lp->lnd_x, lp->lnd_y, player->cnum));
	if (sectp->sct_own != player->cnum) {
	    wu(0, sectp->sct_own, "%s unloaded %d %s from %s at %s\n",
	       cname(player->cnum), -move_amt, ich->i_name,
	       prland(lp), xyas(lp->lnd_x, lp->lnd_y, sectp->sct_own));
	}
    }
    ++*nunitsp;
    return 0;
}

static int
load_land_land(struct sctstr *sectp, struct lndstr *lp, int noisy,
	       int loading, int *nunitsp)
{
    struct nstr_item ni;
    struct lndstr land;
    int loaded = 0;
    char *p;
    char prompt[512];
    char buf[1024];

    if (!lchr[lp->lnd_type].l_nland) {
	if (noisy)
	    pr("%s cannot carry land units!\n", prland(lp));
	return 0;
    }
    if (loading && lnd_nland(lp) >= lchr[lp->lnd_type].l_nland) {
	pr("%s doesn't have room for any more land units!\n",
	   prland(lp));
	return 0;
    }
    sprintf(prompt, "Land unit(s) to %s %s? ",
	    loading ? "load onto" : "unload from", prland(lp));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p)
	return RET_SYN;
    if (!snxtitem(&ni, EF_LAND, p, NULL))
	return RET_SYN;

    if (!still_ok_land(sectp, lp))
	return RET_SYN;

    noisy = ni.sel == NS_LIST;

    while (nxtitem(&ni, &land)) {
	if (!player->owner)
	    continue;

	if (loading) {
	    if (land.lnd_uid == lp->lnd_uid) {
		if (noisy)
		    pr("%s can't be loaded onto itself!\n", prland(&land));
		continue;
	    }
	    if (!land_loadable(&land, noisy))
		continue;
	}

	/* Unit sanity done */
	/* Find the right ship */
	if (!loading) {
	    if (land.lnd_land != lp->lnd_uid)
		continue;
	    if (land.lnd_ship > -1)
		continue;
	} else if (lp->lnd_x != land.lnd_x || lp->lnd_y != land.lnd_y)
	    continue;

	/* Fit unit on ship */
	if (loading) {
	    if (lnd_nland(lp) >= lchr[lp->lnd_type].l_nland) {
		pr("%s doesn't have room for any more land units!\n",
		   prland(lp));
		break;
	    }
	    sprintf(buf, "loaded on your %s at %s",
		    prland(lp), xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));
	    gift(lp->lnd_own, player->cnum, &land, buf);
	    land.lnd_land = lp->lnd_uid;
	    land.lnd_harden = 0;
	    putland(land.lnd_uid, &land);
#if 0
	   /* FIXME same issue as in load_land_ship() */
	    if (!lnd_supply_all(&land))
		pr("WARNING: %s is out of supply!\n", prland(&land));
#else
	    if (!lnd_in_supply(&land))
		pr("WARNING: %s is out of supply!\n", prland(&land));
#endif
	} else {
	    sprintf(buf, "unloaded in your %s at %s",
		    dchr[sectp->sct_type].d_name,
		    xyas(sectp->sct_x, sectp->sct_y, sectp->sct_own));
	    gift(sectp->sct_own, player->cnum, &land, buf);
	    land.lnd_land = -1;
	    putland(land.lnd_uid, &land);
	}
	pr("%s %s %s at %s.\n",
	   prland(&land),
	   loading ? "loaded onto" : "unloaded from",
	   prland(lp), xyas(lp->lnd_x, lp->lnd_y, player->cnum));
	loaded = 1;
    }
    *nunitsp += loaded;
    return 0;
}
