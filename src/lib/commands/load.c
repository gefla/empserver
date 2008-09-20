/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  load.c: load/unload goods from a sector onto a ship or land unit
 *
 *  Known contributors to this file:
 *     David Sharnoff, 1987
 *     Ken Stevens, 1995 (rewritten)
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2008
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "empobj.h"
#include "item.h"
#include "land.h"
#include "optlist.h"
#include "plague.h"
#include "plane.h"
#include "ship.h"
#include "unit.h"

/*
 * The values 1 and -1 are important below, don't change them.
 */
#define	LOAD	1
#define	UNLOAD	-1

static int load_plane_ship(struct sctstr *sectp, struct shpstr *sp,
			   int noisy, int load_unload, int *nshipsp);
static int load_land_ship(struct sctstr *sectp, struct shpstr *sp,
			  int noisy, int load_unload, int *nshipsp);
static int load_comm_ship(struct sctstr *sectp, struct shpstr *sp,
			  struct ichrstr *ich, int load_unload,
			  int *nshipsp);
static int load_plane_land(struct sctstr *sectp, struct lndstr *lp,
			   int noisy, int load_unload, int *nunitsp);
static int load_land_land(struct sctstr *sectp, struct lndstr *lp,
			  int noisy, int load_unload, int *nunitsp);
static int load_comm_land(struct sctstr *sectp, struct lndstr *lp,
			  struct ichrstr *ich, int load_unload,
			  int *nunitsp);

int
load(void)
{
    int noisy;
    int load_unload;
    int type;
    struct nstr_item nbst;
    struct ichrstr *ich;
    int nships;
    struct sctstr sect;
    struct shpstr ship;
    int retval;
    char *p;
    char buf[1024];

    if (!(p = getstarg(player->argp[1],
		       "What commodity (or 'plane' or 'land')? ", buf))
	|| !*p)
	return RET_SYN;

    if (!strncmp(p, "plane", 5))
	type = EF_PLANE;
    else if (!strncmp(p, "land", 4))
	type = EF_LAND;
    else if (NULL != (ich = item_by_name(p)))
	type = EF_SECTOR;
    else {
	pr("Can't load '%s'\n", p);
	return RET_SYN;
    }

    if (!(p = getstarg(player->argp[2], "Ship(s): ", buf)) || !*p)
	return RET_SYN;

    noisy = isdigit(*p);

    if (!snxtitem(&nbst, EF_SHIP, p, NULL))
	return RET_SYN;

    load_unload = **player->argp == 'l' ? LOAD : UNLOAD;

    nships = 0;
    while (nxtitem(&nbst, &ship)) {
	if (!ship.shp_own)
	    continue;
	if (!player->owner && (load_unload == UNLOAD)) {
	    continue;
	}
	if (opt_MARKET) {
	    if (ontradingblock(EF_SHIP, &ship)) {
		pr("You cannot load/unload an item on the trading block!\n");
		continue;
	    }
	}

	if (ship.shp_own != player->cnum) {
	    if (!noisy)
		continue;
	    if (getrel(getnatp(player->cnum), ship.shp_own) < FRIENDLY)
		continue;
	}
	if (!getsect(ship.shp_x, ship.shp_y, &sect))	/* XXX */
	    continue;
	if ((sect.sct_own != player->cnum) &&
	    (ship.shp_own != player->cnum))
	    continue;
	if (!player->owner &&
	    !sect_has_dock(&sect))
	    continue;
	if (!sect.sct_own)
	    continue;
	if ((sect.sct_own != player->cnum) && (load_unload == LOAD)) {
	    if (noisy)
		pr("You don't own %s \n",
		   xyas(ship.shp_x, ship.shp_y, player->cnum));
	    continue;
	}
	if (!sect_has_dock(&sect)) {
	    if (noisy)
		pr("Sector %s is not a harbor or canal.\n",
		   xyas(ship.shp_x, ship.shp_y, player->cnum));
	    continue;
	}
	if (sect.sct_own != player->cnum && load_unload == UNLOAD
	    && getrel(getnatp(sect.sct_own), player->cnum) < FRIENDLY) {
	    if (noisy)
		pr("You can't unload into an unfriendly %s\n",
		   dchr[sect.sct_type].d_name);
	    continue;
	}
	if (sect.sct_effic < 2) {
	    if (noisy)
		pr("The %s at %s is not 2%% efficient yet.\n",
		   dchr[sect.sct_type].d_name,
		   xyas(ship.shp_x, ship.shp_y, player->cnum));
	    continue;
	}
	switch (type) {
	case EF_PLANE:
	    if (0 !=
		(retval =
		 load_plane_ship(&sect, &ship, noisy, load_unload,
				 &nships)))
		return retval;
	    break;
	case EF_LAND:
	    if (0 !=
		(retval =
		 load_land_ship(&sect, &ship, noisy, load_unload,
				&nships)))
		return retval;
	    break;
	case EF_SECTOR:
	    if (0 !=
		(retval =
		 load_comm_ship(&sect, &ship, ich, load_unload, &nships)))
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
	   load_unload == UNLOAD ? "un" : "");
    return RET_OK;
}

int
lload(void)
{
    int noisy;
    int load_unload;
    int type;
    struct nstr_item nbst;
    struct ichrstr *ich;
    int nunits;
    struct sctstr sect;
    struct lndstr land;
    int retval;
    char *p;
    char buf[1024];

    if (!(p = getstarg(player->argp[1],
		       "What commodity (or 'plane' or 'land')? ", buf))
	|| !*p)
	return RET_SYN;
    if (!strncmp(p, "plane", 5))
	type = EF_PLANE;
    else if (!strncmp(p, "land", 4))
	type = EF_LAND;
    else if (NULL != (ich = item_by_name(p)))
	type = EF_SECTOR;
    else {
	pr("Can't load '%s'\n", p);
	return RET_SYN;
    }

    if (!(p = getstarg(player->argp[2], "Unit(s): ", buf)) || !*p)
	return RET_SYN;

    noisy = isdigit(*p);

    if (!snxtitem(&nbst, EF_LAND, p, NULL))
	return RET_SYN;

    load_unload = player->argp[0][1] == 'l' ? LOAD : UNLOAD;

    nunits = 0;
    while (nxtitem(&nbst, &land)) {
	if (land.lnd_own == 0)
	    continue;

	if (player->cnum != land.lnd_own &&
	    getrel(getnatp(player->cnum), land.lnd_own) != ALLIED)
	    continue;

	if (!getsect(land.lnd_x, land.lnd_y, &sect))	/* XXX */
	    continue;

	if (sect.sct_own != player->cnum && land.lnd_own != player->cnum)
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(EF_LAND, &land)) {
		pr("You cannot load/unload an item on the trading block!\n");
		continue;
	    }
	}

	if (sect.sct_own != player->cnum &&
	    getrel(getnatp(sect.sct_own), land.lnd_own) != ALLIED) {
	    pr("Sector %s is not yours.\n",
	       xyas(land.lnd_x, land.lnd_y, player->cnum));
	    continue;
	}
	switch (type) {
	case EF_LAND:
	    if (0 !=
		(retval =
		 load_land_land(&sect, &land, noisy, load_unload,
				&nunits)))
		return retval;
	    break;
	case EF_PLANE:
	    if (0 !=
		(retval =
		 load_plane_land(&sect, &land, noisy, load_unload,
				 &nunits)))
		return retval;
	    break;
	case EF_SECTOR:
	    if (0 !=
		(retval =
		 load_comm_land(&sect, &land, ich, load_unload, &nunits)))
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
	   load_unload == UNLOAD ? "un" : "");
    return RET_OK;
}

void
gift(natid givee, natid giver, void *ptr, char *mesg)
{
    if (giver != givee)
	wu(0, givee, "%s %s %s\n", cname(giver), obj_nameof(ptr), mesg);
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
load_plane_ship(struct sctstr *sectp, struct shpstr *sp, int noisy,
		int load_unload, int *nshipsp)
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
    if (load_unload == LOAD &&
	shp_nplane(sp, NULL, NULL, NULL)
		>= mcp->m_nchoppers + mcp->m_nxlight + mcp->m_nplanes) {
	if (noisy)
	    pr("%s doesn't have room for any more planes\n", prship(sp));
	return 0;
    }
    sprintf(prompt, "Plane(s) to %s %s? ",
	    load_unload == LOAD ? "load onto" : "unload from", prship(sp));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p)
	return RET_SYN;
    if (!snxtitem(&ni, EF_PLANE, p, NULL))
	return RET_SYN;

    if (!still_ok_ship(sectp, sp))
	return RET_SYN;

    if (noisy && p && *p)
	noisy = isdigit(*p);

    while (nxtitem(&ni, &pln)) {
	if (pln.pln_own != player->cnum)
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
	if (load_unload == LOAD && pln.pln_ship > -1) {
	    if (noisy)
		pr("%s is already on ship #%d!\n",
		   prplane(&pln), pln.pln_ship);
	    continue;
	}
	if (load_unload == LOAD && pln.pln_land > -1) {
	    if (noisy)
		pr("%s is already on land unit #%d!\n",
		   prplane(&pln), pln.pln_land);
	    continue;
	}
	if (pln.pln_harden != 0) {
	    if (noisy)
		pr("%s has been hardened and can't be loaded\n",
		   prplane(&pln));
	    continue;
	}

	/* Plane sanity done */
	/* Find the right ship */
	if (load_unload == UNLOAD) {
	    if (pln.pln_ship != sp->shp_uid)
		continue;
	} else if (sp->shp_x != pln.pln_x || sp->shp_y != pln.pln_y)
	    continue;

	/* ship to (plane or missle) sanity */
	if (!could_be_on_ship(&pln, sp, 0, 0, 0, 0)) {
	    if (plchr[(int)pln.pln_type].pl_flags & P_L) {
		strcpy(buf, "planes");
	    } else if (plchr[(int)pln.pln_type].pl_flags & P_K) {
		strcpy(buf, "choppers");
	    } else if (plchr[(int)pln.pln_type].pl_flags & P_M) {
		strcpy(buf, "missiles");
	    } else if (plchr[(int)pln.pln_type].pl_flags & P_E) {
		strcpy(buf, "extra light planes");
	    }			/* else impossible */
	    if (noisy)
		pr("%s cannot carry %s.\n", prship(sp), buf);
	    continue;
	}
	/* Fit plane on ship */
	if (load_unload == LOAD) {
	    if (!put_plane_on_ship(&pln, sp)) {
		if (noisy)
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
	   (load_unload == UNLOAD) ?
	   "unloaded from" : "loaded onto",
	   prship(sp), xyas(sp->shp_x, sp->shp_y, player->cnum));
	loaded = 1;
    }
    *nshipsp += loaded;
    return 0;
}

static int
load_land_ship(struct sctstr *sectp, struct shpstr *sp, int noisy,
	       int load_unload, int *nshipsp)
{
    struct nstr_item ni;
    struct lndstr land;
    int loaded = 0;
    char *p;
    char prompt[512];
    char buf[1024];
    int load_spy = 0;

    if (load_unload == LOAD) {
	if (opt_LANDSPIES) {
	    if ((mchr[(int)sp->shp_type].m_flags & M_SUB) &&
		(mchr[(int)sp->shp_type].m_nland == 0)) {
		if (shp_nland(sp) >= 2) {
		    pr("Non-land unit carrying subs can only carry up to two spy units.\n");
		    return 0;
		}
		/* Eh, let 'em load a spy only */
		load_spy = 1;
	    }
	}
	if (!load_spy && shp_nland(sp) >= mchr[sp->shp_type].m_nland) {
	    if (noisy) {
		if (mchr[(int)sp->shp_type].m_nland)
		    pr("%s doesn't have room for any more land units!\n",
		       prship(sp));
		else
		    pr("%s cannot carry land units!\n", prship(sp));
	    }
	    return 0;
	}
    }
    sprintf(prompt, "Land unit(s) to %s %s? ",
	    load_unload == LOAD ? "load onto" : "unload from", prship(sp));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p)
	return RET_SYN;
    if (!snxtitem(&ni, EF_LAND, p, NULL))
	return RET_SYN;

    if (!still_ok_ship(sectp, sp))
	return RET_SYN;

    if (noisy && p && *p)
	noisy = isdigit(*p);

    while (nxtitem(&ni, &land)) {
	if (land.lnd_own != player->cnum)
	    continue;

	if (load_unload == LOAD) {
	    if (land.lnd_ship > -1) {
		if (noisy)
		    pr("%s is already on ship #%d!\n",
		       prland(&land), land.lnd_ship);
		continue;
	    }
	    if (land.lnd_land > -1) {
		if (noisy)
		    pr("%s is already on land #%d!\n",
		       prland(&land), land.lnd_land);
		continue;
	    }
	    if (lnd_first_on_land(&land) >= 0) {
		if (noisy)
		    pr("%s cannot be loaded since it is carrying units\n",
		       prland(&land));
		continue;
	    }
	    if (lchr[(int)land.lnd_type].l_flags & L_HEAVY) {
		if (noisy)
		    pr("%s is too heavy to load.\n", prland(&land));
		continue;
	    }
	    if (load_spy && !(lchr[(int)land.lnd_type].l_flags & L_SPY)) {
		if (noisy)
		    pr("Subs can only carry spy units.\n");
		continue;
	    }
	}

	/* Unit sanity done */
	/* Find the right ship */
	if (load_unload == UNLOAD) {
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
	if (load_unload == LOAD) {
	    /* We have to check again, since it may have changed */
	    if (opt_LANDSPIES) {
		if ((mchr[(int)sp->shp_type].m_flags & M_SUB) &&
		    (mchr[(int)sp->shp_type].m_nland == 0)) {
		    if (shp_nland(sp) >= 2) {
			pr("Non-land unit carrying subs can only carry up to two spy units.\n");
			return 0;
		    }
		    /* Eh, let 'em load a spy only */
		    load_spy = 1;
		}
	    }
	    if (!load_spy && shp_nland(sp) >= mchr[sp->shp_type].m_nland) {
		if (noisy) {
		    if (mchr[(int)sp->shp_type].m_nland)
			pr("%s doesn't have room for any more land units!\n",
			   prship(sp));
		    else
			pr("%s cannot carry land units!\n", prship(sp));
		}
		return 0;
	    }
	    sprintf(buf, "loaded on your %s at %s",
		    prship(sp), xyas(sp->shp_x, sp->shp_y, sp->shp_own));
	    gift(sp->shp_own, player->cnum, &land, buf);
	    land.lnd_ship = sp->shp_uid;
	    land.lnd_harden = 0;
#if 0
           /*
            * FIXME if this supplies from the sector, the putsect in
            * load() / lload() duplicates those supplies, causing a
            * seqno mismatch
            */
	    resupply_all(&land);
#endif
	    putland(land.lnd_uid, &land);
	    if (!has_supply(&land))
		pr("WARNING: %s is out of supply!\n", prland(&land));
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
	   (load_unload == UNLOAD) ?
	   "unloaded from" : "loaded onto",
	   prship(sp), xyas(sp->shp_x, sp->shp_y, player->cnum));
	loaded = 1;
    }
    *nshipsp += loaded;
    return 0;
}

static int
load_comm_ship(struct sctstr *sectp, struct shpstr *sp,
	       struct ichrstr *ich, int load_unload, int *nshipsp)
{
    i_type item = ich->i_uid;
    struct mchrstr *mcp = &mchr[(int)sp->shp_type];
    int ship_amt, ship_max, sect_amt, move_amt;
    int amount;
    char prompt[512];
    char *p;
    char buf[1024];

    sprintf(prompt, "Number of %s to %s %s at %s? ",
	    ich->i_name,
	    (load_unload == UNLOAD) ?
	    "unload from" : "load onto",
	    prship(sp), xyas(sp->shp_x, sp->shp_y, player->cnum));
    if (!(p = getstarg(player->argp[3], prompt, buf)) || !*p)
	return RET_SYN;

    if (!still_ok_ship(sectp, sp))
	return RET_SYN;

    ship_amt = sp->shp_item[item];
    ship_max = mcp->m_item[item];
    sect_amt = sectp->sct_item[item];
    amount = atoi(p);
    if (amount < 0)
	move_amt = -amount - ship_amt;
    else
	move_amt = load_unload == LOAD ? amount : -amount;
    if (move_amt > ship_max - ship_amt)
	move_amt = ship_max - ship_amt;
    if (move_amt < -ship_amt)
	move_amt = -ship_amt;
    if (move_amt > sect_amt)
	move_amt = sect_amt;
    if (move_amt < sect_amt - ITEM_MAX)
	move_amt = sect_amt - ITEM_MAX;
    if (!move_amt)
	return RET_OK;
    if (sectp->sct_oldown != player->cnum && item == I_CIVIL) {
	pr("%s civilians refuse to %s at %s!\n",
	   move_amt < 0 ? "Your" : "Foreign",
	   move_amt < 0 ? "disembark" : "board",
	   xyas(sectp->sct_x, sectp->sct_y, player->cnum));
	return RET_FAIL;
    }

    if (!want_to_abandon(sectp, item, move_amt, 0))
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
		int load_unload, int *nunitsp)
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
    if (load_unload == LOAD && lnd_nxlight(lp) >= lcp->l_nxlight) {
	if (noisy)
	    pr("%s doesn't have room for any more extra-light planes\n",
	       prland(lp));
	return 0;
    }
    sprintf(prompt, "Plane(s) to %s %s? ",
	    load_unload == LOAD ? "load onto" : "unload from", prland(lp));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p)
	return RET_SYN;
    if (!snxtitem(&ni, EF_PLANE, p, NULL))
	return RET_SYN;

    if (!still_ok_land(sectp, lp))
	return RET_SYN;

    if (noisy && p && *p)
	noisy = isdigit(*p);

    if (sectp->sct_own != player->cnum && load_unload == LOAD) {
	pr("Sector %s is not yours.\n",
	   xyas(lp->lnd_x, lp->lnd_y, player->cnum));
	return 0;
    }

    while (nxtitem(&ni, &pln)) {
	if (pln.pln_own != player->cnum)
	    continue;

	if (!(plchr[(int)pln.pln_type].pl_flags & P_E)) {
	    if (noisy)
		pr("You can only load xlight planes onto units.\n");
	    continue;
	}

	if (load_unload == LOAD && pln.pln_ship > -1) {
	    if (noisy)
		pr("%s is already on ship #%d!\n",
		   prplane(&pln), pln.pln_ship);
	    continue;
	}
	if (load_unload == LOAD && pln.pln_land > -1) {
	    if (noisy)
		pr("%s is already on unit #%d!\n",
		   prplane(&pln), pln.pln_land);
	    continue;
	}
	if (pln.pln_harden != 0) {
	    if (noisy)
		pr("%s has been hardened and can't be loaded\n",
		   prplane(&pln));
	    continue;
	}

	/* Plane sanity done */
	/* Find the right unit */
	if (load_unload == UNLOAD) {
	    if (pln.pln_land != lp->lnd_uid)
		continue;
	} else if (lp->lnd_x != pln.pln_x || lp->lnd_y != pln.pln_y)
	    continue;

	/* Fit plane on unit */
	if (load_unload == LOAD) {
	    if (!put_plane_on_land(&pln, lp)) {
		if (noisy)
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
	   (load_unload == UNLOAD) ?
	   "unloaded from" : "loaded onto",
	   prland(lp), xyas(lp->lnd_x, lp->lnd_y, player->cnum));
	loaded = 1;
    }
    *nunitsp += loaded;
    return 0;
}

static int
load_comm_land(struct sctstr *sectp, struct lndstr *lp,
	       struct ichrstr *ich, int load_unload, int *nunitsp)
{
    i_type item = ich->i_uid;
    struct lchrstr *lcp = &lchr[(int)lp->lnd_type];
    int land_amt, land_max, sect_amt, move_amt;
    int amount;
    char prompt[512];
    char *p;
    char buf[1024];

    sprintf(prompt, "Number of %s to %s %s at %s? ",
	    ich->i_name,
	    (load_unload == UNLOAD) ?
	    "unload from" : "load onto",
	    prland(lp), xyas(lp->lnd_x, lp->lnd_y, player->cnum));
    if (!(p = getstarg(player->argp[3], prompt, buf)) || !*p)
	return RET_SYN;

    if (!still_ok_land(sectp, lp))
	return RET_SYN;

    land_amt = lp->lnd_item[item];
    land_max = lcp->l_item[item];
    sect_amt = sectp->sct_item[item];
    amount = atoi(p);
    if (amount < 0)
	move_amt = -amount - land_amt;
    else
	move_amt = load_unload == LOAD ? amount : -amount;
    if (move_amt > land_max - land_amt)
	move_amt = land_max - land_amt;
    if (move_amt < -land_amt)
	move_amt = -land_amt;
    if (move_amt > sect_amt)
	move_amt = sect_amt;
    if (move_amt < sect_amt - ITEM_MAX)
	move_amt = sect_amt - ITEM_MAX;
    if (!move_amt)
	return RET_OK;
    if (sectp->sct_own != player->cnum && move_amt > 0) {
	pr("Sector %s is not yours.\n",
	   xyas(lp->lnd_x, lp->lnd_y, player->cnum));
	return RET_FAIL;
    }
    if (sectp->sct_oldown != player->cnum && item == I_CIVIL) {
	pr("%s civilians refuse to %s at %s!\n",
	   move_amt < 0 ? "Your" : "Foreign",
	   move_amt < 0 ? "disembark" : "board",
	   xyas(sectp->sct_x, sectp->sct_y, player->cnum));
	return RET_FAIL;
    }
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
	       int load_unload, int *nunitsp)
{
    struct nstr_item ni;
    struct lndstr land;
    int loaded = 0;
    char *p;
    char prompt[512];
    char buf[1024];

    if (load_unload == LOAD
	&& lnd_nland(lp) >= lchr[lp->lnd_type].l_nland) {
	if (noisy) {
	    if (lchr[lp->lnd_type].l_nland)
		pr("%s doesn't have room for any more land units!\n",
		   prland(lp));
	    else
		pr("%s cannot carry land units!\n", prland(lp));
	}
	return 0;
    }
    sprintf(prompt, "Land unit(s) to %s %s? ",
	    load_unload == LOAD ? "load onto" : "unload from", prland(lp));
    p = getstarg(player->argp[3], prompt, buf);
    if (!p)
	return RET_SYN;
    if (!snxtitem(&ni, EF_LAND, p, NULL))
	return RET_SYN;

    if (!still_ok_land(sectp, lp))
	return RET_SYN;

    if (noisy && p && *p)
	noisy = isdigit(*p);

    while (nxtitem(&ni, &land)) {

	if (land.lnd_own != player->cnum)
	    continue;

	if (load_unload == LOAD) {
	    if (land.lnd_ship > -1) {
		if (noisy)
		    pr("%s is already on ship #%d!\n",
		       prland(&land), land.lnd_ship);
		continue;
	    }
	    if (land.lnd_land > -1) {
		if (noisy)
		    pr("%s is already on land #%d!\n",
		       prland(&land), land.lnd_land);
		continue;
	    }
	    if (lnd_first_on_land(&land) >= 0) {
		if (noisy)
		    pr("%s cannot be loaded since it is carrying units\n",
		       prland(&land));
		continue;
	    }
	    if (land.lnd_uid == lp->lnd_uid) {
		if (noisy)
		    pr("%s can't be loaded onto itself!\n", prland(&land));
		continue;
	    }
	    if (lchr[(int)land.lnd_type].l_flags & (L_HEAVY | L_TRAIN)) {
		if (noisy)
		    pr("%s is too heavy to load.\n", prland(&land));
		continue;
	    }
	}

	/* Unit sanity done */
	/* Find the right ship */
	if (load_unload == UNLOAD) {
	    if (land.lnd_land != lp->lnd_uid)
		continue;
	    if (land.lnd_ship > -1)
		continue;
	} else if (lp->lnd_x != land.lnd_x || lp->lnd_y != land.lnd_y)
	    continue;

	/* Fit unit on ship */
	if (load_unload == LOAD) {
	    if (lnd_nland(lp) >= lchr[lp->lnd_type].l_nland) {
		if (noisy) {
		    if (lchr[lp->lnd_type].l_nland)
			pr("%s doesn't have room for any more land units!\n",
			   prland(lp));
		    else
			pr("%s cannot carry land units!\n", prland(lp));
		}
		break;
	    }
	    sprintf(buf, "loaded on your %s at %s",
		    prland(lp), xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));
	    gift(lp->lnd_own, player->cnum, &land, buf);
	    land.lnd_land = lp->lnd_uid;
	    land.lnd_harden = 0;
#if 0
           /* FIXME same issue as in load_land_ship() */
	    resupply_all(&land);
#endif
	    putland(land.lnd_uid, &land);
	    if (!has_supply(&land))
		pr("WARNING: %s is out of supply!\n", prland(&land));
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
	   (load_unload == UNLOAD) ?
	   "unloaded from" : "loaded onto",
	   prland(lp), xyas(lp->lnd_x, lp->lnd_y, player->cnum));
	loaded = 1;
    }
    *nunitsp += loaded;
    return 0;
}
