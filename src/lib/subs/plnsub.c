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
 *  plnsub.c: Plane subroutine stuff
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2020
 */

#include <config.h>

#include "chance.h"
#include "empobj.h"
#include "item.h"
#include "land.h"
#include "map.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "nuke.h"
#include "optlist.h"
#include "plague.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

static int ship_can_carry(struct shpstr *, int, int, int, int);
static int inc_shp_nplane(struct plnstr *, int *, int *, int *);

/*
 * Get planes and escorts argument.
 * Read planes into *@ni_bomb, and (optional) escorts into *@ni_esc.
 * If @input_bomb is not empty, use it, else prompt for more input.
 * Same for @input_esc.
 * If we got a plane argument, initialize *@ni_bomb and *@ni_esc, and
 * return 0.
 * Else return -1 (*@ni_bomb and *@ni_esc may be modified).
 */
int
get_planes(struct nstr_item *ni_bomb, struct nstr_item *ni_esc,
	   char *input_bomb, char *input_esc)
{
    if (!snxtitem(ni_bomb, EF_PLANE, input_bomb, NULL))
	return -1;
    if (!snxtitem(ni_esc, EF_PLANE, input_esc, "escort(s)? ")) {
	if (player->aborted)
	    return -1;
	pr("No escorts...\n");
    }
    return 0;
}

/*
 * Get assembly point argument.
 * If @input is not empty, use it.
 * Else prompt for more input using @prompt.
 * If this yields a valid assembly point, read it into *@ap_sect and
 * return @ap_sect.
 * Else complain and return NULL.
 * *@ap_sect and @buf[1024] may be modified in either case.
 */
struct sctstr *
get_assembly_point(char *input, struct sctstr *ap_sect, char *buf)
{
    char *p;
    coord x, y;
    struct nstr_item ni;
    struct shpstr ship;

    p = getstarg(input, "assembly point? ", buf);
    if (!p || *p == 0)
	return NULL;
    if (!sarg_xy(p, &x, &y) || !getsect(x, y, ap_sect))
	return NULL;

    /* over own or allied sector is fine */
    if (relations_with(ap_sect->sct_own, player->cnum) == ALLIED)
	return ap_sect;

    /* over own or allied ship is fine */
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, &ship)) {
	if (!ship.shp_own)
	    continue;
	if (relations_with(ship.shp_own, player->cnum) == ALLIED)
	    return ap_sect;
    }

    pr("Assembly point not owned by you or an ally!\n");
    return NULL;
}

/*
 * Find out whether planes can fly one-way to @x,@y.
 * Offer the player any carriers there.  If he chooses one, read it
 * into @target->ship.  Else read the target sector into @target->sect.
 * If planes can land there, set required plane flags in *@flagsp, and
 * return 0.  Else return -1.
 */
int
pln_where_to_land(coord x, coord y,
		  union empobj_storage *target, int *flagsp)
{
    /* Keep conditions for landing consistent with pln_airbase_ok() */
    int nships;
    int cno;
    int fl;
    char buf[1024];
    char *p;

    /* offer carriers */
    nships = carriersatxy(x, y, player->cnum);
    if (nships) {
	for (;;) {
	    p = getstring("Carrier #? ", buf);
	    if (!p)
		return -1;
	    if (!*p)
		break;
	    cno = atoi(p);
	    if (!getship(cno, &target->ship)
		|| (!player->owner
		    && (relations_with(target->ship.shp_own, player->cnum)
			!= ALLIED))) {
		pr("Not yours\n");
		continue;
	    }
	    if (target->ship.shp_x != x || target->ship.shp_y != y) {
		pr("Ship #%d not in %s\n", cno, xyas(x, y, player->cnum));
		continue;
	    }
	    fl = carrier_planes(&target->ship, 0);
	    if (fl == 0) {
		pr("Can't land on %s.\n", prship(&target->ship));
		continue;
	    }
	    /* clear to land on ship#CNO */
	    pr("landing on carrier %d\n", cno);
	    *flagsp |= fl;
	    return 0;
	}
    }

    /* try to land at sector */
    getsect(x, y, &target->sect);
    if (relations_with(target->sect.sct_own, player->cnum) != ALLIED) {
	pr("Nowhere to land at sector %s!\n", xyas(x, y, player->cnum));
	return -1;
    }
    /* clear to land at sector */
    if (target->sect.sct_type == SCT_MOUNT) {
	*flagsp |= P_K;
    }
    if (target->sect.sct_type != SCT_AIRPT || target->sect.sct_effic < 60)
	*flagsp |= P_V;
    return 0;
}

int
pln_can_land_on_carrier(struct emp_qelem *bomb_list,
			struct emp_qelem *esc_list,
			struct shpstr *sp)

{
    int n, nch, nxl, nmsl;
    struct emp_qelem *list, *qp;
    struct plist *plp;

    n = shp_nplane(sp, &nch, &nxl, &nmsl);

    /* for both lists */
    for (list = bomb_list;
	 list;
	 list = list == bomb_list ? esc_list : NULL) {
	for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	    plp = (struct plist *)qp;
	    if (plp->plane.pln_ship == sp->shp_uid)
		continue;
	    n++;
	    if (!inc_shp_nplane(&plp->plane, &nch, &nxl, &nmsl))
		return 0;
	}
    }
    return ship_can_carry(sp, n, nch, nxl, nmsl);
}

void
pln_newlanding(struct emp_qelem *list, coord tx, coord ty, int cno)
{
    struct emp_qelem *qp;
    struct plist *plp;
    struct shpstr ship;
    struct sctstr sect;

    if (cno >= 0)
	getship(cno, &ship);
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if (cno >= 0) {
	    if (!could_be_on_ship(&plp->plane, &ship))
		pr("\t%s cannot land on ship #%d! %s aborts!\n",
		   prplane(&plp->plane), cno, prplane(&plp->plane));
	    else if (!put_plane_on_ship(&plp->plane, &ship))
		pr("\tNo room on ship #%d! %s aborts!\n",
		   cno, prplane(&plp->plane));
	    else {
		if (plp->plane.pln_own != ship.shp_own) {
		    wu(0, ship.shp_own, "%s %s lands on your %s\n",
		       cname(player->cnum), prplane(&plp->plane),
		       prship(&ship));
		}
		if (plp->pstage == PLG_INFECT
		    && ship.shp_pstage == PLG_HEALTHY)
		    ship.shp_pstage = PLG_EXPOSED;
	    }
	} else {
	    plp->plane.pln_x = tx;
	    plp->plane.pln_y = ty;
	    getsect(tx, ty, &sect);
	    if (plp->plane.pln_own != sect.sct_own) {
		wu(0, sect.sct_own,
		   "%s %s lands at your sector %s\n",
		   cname(player->cnum),
		   prplane(&plp->plane), xyas(tx, ty, sect.sct_own));
	    }
	    if (plp->pstage == PLG_INFECT
		&& sect.sct_pstage == PLG_HEALTHY)
		sect.sct_pstage = PLG_EXPOSED;
	    plp->plane.pln_ship = cno;
	}
    }
}

void
pln_dropoff(struct emp_qelem *list, struct ichrstr *ip, coord tx, coord ty,
	    int cno)
{
    struct emp_qelem *qp;
    struct plist *plp;
    int amt;
    struct sctstr sect;
    struct shpstr ship;
    int there;
    int max;
    int pstage;

    if (!ip)
	return;
    if (cno < 0) {
	getsect(tx, ty, &sect);
	if (!sect.sct_own) {
	    if (sect.sct_type == SCT_WATER)
		pr("Your %s sink like a rock!\n", ip->i_name);
	    else
		pr("Your %s vanish without a trace.\n", ip->i_name);
	    return;
	}
	if (relations_with(sect.sct_own, player->cnum) != ALLIED) {
	    pr("You don't own %s!  Cargo jettisoned...\n",
	       xyas(tx, ty, player->cnum));
	    return;
	}
	if (ip->i_uid == I_CIVIL && sect.sct_own != sect.sct_oldown) {
	    pr("%s is occupied.  Your civilians suffer from identity crisis and die.\n",
	       xyas(tx, ty, player->cnum));
	    return;
	}
	there = sect.sct_item[ip->i_uid];
	max = ITEM_MAX;
	pstage = sect.sct_pstage;
    } else {
	getship(cno, &ship);
	there = ship.shp_item[ip->i_uid];
	max = mchr[ship.shp_type].m_item[ip->i_uid];
	pstage = ship.shp_pstage;
    }

    amt = 0;
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	amt += plp->load;
	if (plp->load
	    && plp->pstage == PLG_INFECT && pstage == PLG_HEALTHY)
	    pstage = PLG_EXPOSED;
    }

    there += amt;
    if (there > max) {
	pr("%d excess %s discarded\n", there - max, ip->i_name);
	amt -= there - max;
	there = max;
    }
    pr("%d %s landed safely", amt, ip->i_name);
    if (cno < 0) {
	sect.sct_item[ip->i_uid] = there;
	sect.sct_pstage = pstage;
	if (sect.sct_own != player->cnum)
	    wu(0, sect.sct_own, "%s planes drop %d %s in %s\n",
	       cname(player->cnum), amt, ip->i_name,
	       xyas(tx, ty, sect.sct_own));
	pr(" at %s\n", xyas(tx, ty, player->cnum));
	putsect(&sect);
    } else {
	ship.shp_item[ip->i_uid] = there;
	ship.shp_pstage = pstage;
	if (ship.shp_own != player->cnum)
	    wu(0, ship.shp_own, "%s planes land %d %s on carrier %d\n",
	       cname(player->cnum), amt, ip->i_name, ship.shp_uid);
	pr(" on carrier #%d\n", ship.shp_uid);
	putship(ship.shp_uid, &ship);
    }
}

void
pln_mine(struct emp_qelem *list, coord tx, coord ty)
{
    struct emp_qelem *qp;
    struct plist *plp;
    int amt;
    struct sctstr sect;

    amt = 0;
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	amt += plp->load;

    }
    if (amt > 0) {
	getsect(tx, ty, &sect);
	if (sect.sct_type != SCT_WATER) {
	    pr("Your sea mines have no effect here.\n");
	    return;
	}
	sect.sct_mines = MIN(sect.sct_mines + amt, MINES_MAX);
	pr("%d mines laid in %s.\n", amt, xyas(tx, ty, player->cnum));
	if (map_set(player->cnum, tx, ty, 'X', 0))
	    writemap(player->cnum);
	putsect(&sect);
    }
}

/*
 * Has @pp's type capabilities satisfying @wantflags and @nowantflags?
 * A plane type is capable unless
 * - it lacks all of the P_B, P_T in @wantflags, or
 * - it lacks all of the P_F, P_ESC in @wantflags, or
 * - it lacks all of the P_E, P_L, P_K in @wantflags, or
 * - it lacks any of the other capabilities in @wantflags, or
 * - it has any of the capabilities in @nowantflags.
 */
int
pln_capable(struct plnstr *pp, int wantflags, int nowantflags)
{
    int flags = plchr[(int)pp->pln_type].pl_flags;

    if (wantflags & (P_B | P_T)) {
	if ((flags & wantflags & (P_B | P_T)) == 0)
	    return 0;
	wantflags &= ~(P_B | P_T);
    }

    if (wantflags & (P_F | P_ESC)) {
	if ((flags & wantflags & (P_F | P_ESC)) == 0)
	    return 0;
	wantflags &= ~(P_F | P_ESC);
    }

    if (wantflags & (P_E | P_L | P_K)) {
	if ((flags & wantflags & (P_E | P_L | P_K)) == 0)
	    return 0;
	wantflags &= ~(P_E | P_L | P_K);
    }

    if ((flags & wantflags) != wantflags)
	return 0;

    if (flags & nowantflags)
	return 0;

    return 1;
}

/*
 * Return union of capabilities of planes in @list.
 */
int
pln_caps(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct plist *plp;
    int fl;

    fl = 0;
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	fl |= plp->pcp->pl_flags;
    }

    return fl;
}

/*
 * Find plane types that can operate from carrier @sp.
 * If @msl find missile types, else non-missile types.
 * Return a combination of P_L, P_K, P_E.
 * It's zero if @sp can't support air operations due to its type or
 * state (low efficiency).
 */
int
carrier_planes(struct shpstr *sp, int msl)
{
    struct mchrstr *mcp = mchr + sp->shp_type;
    int res;

    if (sp->shp_effic < SHP_AIROPS_EFF)
	return 0;

    res = 0;
    if (mcp->m_flags & M_FLY)
	res |= P_L;
    if ((mcp->m_flags & M_MSL) && msl)
	res |= P_L;
    if (mcp->m_nchoppers && !msl)
	res |= P_K;
    if (mcp->m_nxlight)
	res |= P_E;
    return res;
}

/*
 * Can @pp operate out its sector?
 * If @oneway, consider only takeoff, else takeoff and landing.
 * If @noisy, report to current player when it can't.
 */
int
pln_airbase_ok(struct plnstr *pp, int oneway, int noisy)
{
    /* Keep conditions for landing consistent with pln_where_to_land() */
    struct shpstr ship;
    struct lndstr land;
    struct sctstr sect;
    struct plchrstr *pcp = plchr + pp->pln_type;

    if (CANT_HAPPEN(noisy && pp->pln_own != player->cnum))
	noisy = 0;

    if (pp->pln_ship >= 0) {
	/* ship: needs to be own or allied, efficient */
	if (!getship(pp->pln_ship, &ship)) {
	    CANT_REACH();
	    return 0;
	}
	if (relations_with(ship.shp_own, pp->pln_own) != ALLIED) {
	    if (noisy)
		pr("(note) An ally does not own the ship %s is on\n",
		   prplane(pp));
	    return 0;
	}
	if (!(carrier_planes(&ship, pcp->pl_flags & P_M) & pcp->pl_flags))
	    return 0;

    } else if (pp->pln_land >= 0) {
	/* land: needs to be own or allied, efficient, not embarked */
	if (!getland(pp->pln_land, &land)) {
	    CANT_REACH();
	    return 0;
	}
	if (relations_with(land.lnd_own, pp->pln_own) != ALLIED) {
	    if (noisy)
		pr("(note) An ally does not own the unit %s is on\n",
		   prplane(pp));
	    return 0;
	}
	if (land.lnd_effic < LND_AIROPS_EFF || !(pcp->pl_flags & P_E))
	    return 0;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    return 0;

    } else {
	/* sector: needs to be own or allied, efficient, suitable type */
	if (!getsect(pp->pln_x, pp->pln_y, &sect)) {
	    CANT_REACH();
	    return 0;
	}
	/* mountain requires helo or missile */
	if (sect.sct_type == SCT_MOUNT && !(pcp->pl_flags & (P_K | P_M))) {
	    if (noisy)
		pr("(note) %s is in a mountain and can't take off\n",
		   prplane(pp));
	    return 0;
	}

	if (relations_with(sect.sct_own, pp->pln_own) != ALLIED) {
	    if (noisy)
		pr("(note) An ally does not own the sector %s is in\n",
		   prplane(pp));
	    return 0;
	}
	/* need airfield unless VTOL */
	if ((pcp->pl_flags & P_V) == 0) {
	    if (sect.sct_type != SCT_AIRPT) {
		if (noisy)
		    pr("%s not at airport\n", prplane(pp));
		return 0;
	    }
	    if (sect.sct_effic < 40) {
		if (noisy)
		    pr("%s is not 40%% efficient, %s can't take off from there.\n",
		       xyas(sect.sct_x, sect.sct_y, player->cnum),
		       prplane(pp));
		return 0;
	    }
	    if (!oneway && sect.sct_effic < 60) {
		if (noisy)
		    pr("%s is not 60%% efficient, %s can't land there.\n",
		       xyas(sect.sct_x, sect.sct_y, player->cnum),
		       prplane(pp));
		return 0;
	    }
	}
    }

    return 1;
}

void
pln_sel(struct nstr_item *ni, struct emp_qelem *list, struct sctstr *ap,
	int ap_to_target, int rangemult, int wantflags, int nowantflags)
{
    struct plnstr plane;
    int range;
    struct plchrstr *pcp;
    struct plist *plp;

    emp_initque(list);
    while (nxtitem(ni, &plane)) {
	/*
	 * It would be nice to let deities fly foreign planes, but
	 * much of the code assumes that only the plane's owner can
	 * fly it.
	 */
	if (!plane.pln_own || plane.pln_own != player->cnum)
	    continue;
	if (plane.pln_mobil <= 0)
	    continue;
	if (plane.pln_effic < 40) {
	    pr("%s not efficient enough (must be 40%%)\n",
	       prplane(&plane));
	    continue;
	}
	if (!pln_capable(&plane, wantflags, nowantflags))
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, &plane)) {
		pr("plane #%d ineligible - it's for sale.\n",
		   plane.pln_uid);
		continue;
	    }
	}

	range = mapdist(plane.pln_x, plane.pln_y, ap->sct_x, ap->sct_y);
	if (range > 4) {
	    pr("%s too far from assembly point\n", prplane(&plane));
	    continue;
	}
	range += ap_to_target;
	range *= rangemult;
	pcp = &plchr[(int)plane.pln_type];
	if (plane.pln_range < range) {
	    pr("%s out of range (%d:%d)\n",
	       prplane(&plane), plane.pln_range, range);
	    continue;
	}
	if (!pln_airbase_ok(&plane, rangemult != 2, 1))
	    continue;
	pr("%s standing by\n", prplane(&plane));
	plane.pln_mission = 0;
	putplane(plane.pln_uid, &plane);
	plp = malloc(sizeof(struct plist));
	plp->load = 0;
	plp->pstage = PLG_HEALTHY;
	plp->pcp = pcp;
	plp->plane = plane;
	emp_insque(&plp->queue, list);
    }
}

void
pln_arm(struct emp_qelem *list, int dist, char mission, struct ichrstr *ip)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct plist *plp;
    struct plnstr *pp;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	pp = &plp->plane;
	getplane(pp->pln_uid, pp);
	if ((pp->pln_flags & PLN_LAUNCHED)
	    || pln_equip(plp, ip, mission) < 0) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}
	pp->pln_flags |= PLN_LAUNCHED;
	pp->pln_mobil -= pln_mobcost(dist, pp, mission);
	putplane(pp->pln_uid, pp);
	pr("%s equipped\n", prplane(pp));
    }
}

int
pln_equip(struct plist *plp, struct ichrstr *ip, char mission)
{
    struct plchrstr *pcp;
    struct plnstr *pp;
    int load, needed;
    struct lndstr land;
    struct shpstr ship;
    struct sctstr sect;
    i_type itype;
    short *item;
    int own;
    int abandon_needed;

    pp = &plp->plane;
    pcp = plp->pcp;
    if (pp->pln_ship >= 0) {
	getship(pp->pln_ship, &ship);
	plp->pstage = ship.shp_pstage;
	item = ship.shp_item;
	own = ship.shp_own;
    } else if (pp->pln_land >= 0) {
	getland(pp->pln_land, &land);
	plp->pstage = land.lnd_pstage;
	item = land.lnd_item;
	own = land.lnd_own;
    } else {
	getsect(pp->pln_x, pp->pln_y, &sect);
	plp->pstage = sect.sct_pstage;
	item = sect.sct_item;
	own = sect.sct_oldown;
    }
    if (pcp->pl_fuel > item[I_PETROL]) {
	pr("%s not enough petrol there!\n", prplane(pp));
	return -1;
    }
    item[I_PETROL] -= pcp->pl_fuel;
    load = pln_load(pp);
    itype = I_NONE;
    switch (mission) {
    case 's':		/* strategic bomb */
    case 'p':		/* pinpoint bomb */
	itype = I_SHELL;
	break;
    case 't':		/* transport */
	if (!(pcp->pl_flags & P_C) || !ip)
	    break;
	itype = ip->i_uid;
	load *= 2;
	break;
    case 'm':		/* mine */
	if ((pcp->pl_flags & P_MINE) == 0)
	    break;
	itype = I_SHELL;
	load *= 2;
	break;
    case 'd':		/* drop */
	if (!(pcp->pl_flags & P_C) || CANT_HAPPEN(!ip))
	    break;
	itype = ip->i_uid;
	if (pcp->pl_flags & P_V)
	    load *= 2;
	break;
    case 'a':		/* paradrop */
	if (!(pcp->pl_flags & P_P))
	    break;
	itype = I_MILIT;
	if (pcp->pl_flags & P_V)
	    load *= 2;
	break;
    case 'r':		/* reconnaissance */
    case 'e':		/* escort */
	load = 0;
	break;
    case 'i':		/* missile interception */
	if (CANT_HAPPEN(!(pcp->pl_flags & P_M)
			|| !(pcp->pl_flags & (P_N | P_O))))
	    break;
	if (load)
	    itype = I_SHELL;
	break;
    default:
	CANT_REACH();
	load = 0;
    }

    if (itype != I_NONE) {
	needed = load / ichr[itype].i_lbs;
	if (needed <= 0) {
	    pr("%s can't contribute to mission\n", prplane(pp));
	    return -1;
	}
	if (nuk_on_plane(pp) >= 0) {
	    if (mission == 's' || mission == 't')
		needed = 0;
	    else {
		pr("%s can't fly this mission"
		   " while it is carrying a nuclear weapon\n",
		   prplane(pp));
		return -1;
	    }
	}
	if (itype == I_CIVIL && pp->pln_own != own) {
	    pr("You don't control those civilians!\n");
	    return -1;
	}
#if 0
	/* Supply is broken somewhere, so don't use it for now */
	if (itype == I_SHELL && item[itype] < needed)
	    item[itype] += supply_commod(plp->plane.pln_own,
					 plp->plane.pln_x,
					 plp->plane.pln_y,
					 I_SHELL, needed);
#endif
	if (pp->pln_ship >= 0 || pp->pln_land >= 0)
	    abandon_needed = 0;
	else
	    abandon_needed = !!would_abandon(&sect, itype, needed, NULL);
	if (item[itype] < needed + abandon_needed) {
	    pr("Not enough %s for %s\n", ichr[itype].i_name, prplane(pp));
	    return -1;
	}
	item[itype] -= needed;
	plp->load = needed;
    }

    if (pp->pln_ship >= 0) {
	if (pp->pln_own != ship.shp_own) {
	    wu(0, ship.shp_own,
	       "%s %s prepares for takeoff from ship %s\n",
	       cname(pp->pln_own), prplane(pp), prship(&ship));
	}
	putship(ship.shp_uid, &ship);
    } else if (pp->pln_land >= 0) {
	if (pp->pln_own != land.lnd_own) {
	    wu(0, land.lnd_own,
	       "%s %s prepares for takeoff from unit %s\n",
	       cname(pp->pln_own), prplane(pp), prland(&land));
	}
	putland(land.lnd_uid, &land);
    } else {
	if (pp->pln_own != sect.sct_own) {
	    wu(0, sect.sct_own, "%s %s prepares for takeoff from %s\n",
	       cname(pp->pln_own), prplane(pp),
	       xyas(sect.sct_x, sect.sct_y, sect.sct_own));
	}
	putsect(&sect);
    }
    return 0;
}

void
pln_put(struct emp_qelem *list)
{
    struct emp_qelem *qp, *next;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	pln_put1((struct plist *)qp);
    }
}

void
pln_put1(struct plist *plp)
{
    struct plnstr *pp;
    struct shpstr ship;
    struct sctstr sect;

    pp = &plp->plane;

    if (CANT_HAPPEN((pp->pln_flags & PLN_LAUNCHED)
		    && (plchr[pp->pln_type].pl_flags & P_M)
		    && pp->pln_effic >= PLANE_MINEFF))
	pp->pln_effic = 0;   /* bug: missile launched but not used up */

    if (!(pp->pln_flags & PLN_LAUNCHED))
	;			/* never took off */
    else if (pp->pln_effic < PLANE_MINEFF) {
	;			/* destroyed */
    } else if (pp->pln_ship >= 0) {
	/* It is landing on a carrier */
	getship(pp->pln_ship, &ship);
	/* We should do more, like make sure it's really
	   a carrier, etc. but for now just make sure it's
	   not sunk. */
	if (!ship.shp_own) {
	    mpr(pp->pln_own,
		"Ship #%d has been sunk, plane #%d has nowhere to land, and\n"
		"splashes into the sea.\n",
		pp->pln_ship, pp->pln_uid);
	    pp->pln_effic = 0;
	}
    } else {
	/* Presume we are landing back in a sector. */
	getsect(pp->pln_x, pp->pln_y, &sect);
	if (sect.sct_type == SCT_WATER || sect.sct_type == SCT_WASTE) {
	    mpr(pp->pln_own,
		"Nowhere to land at %s, plane #%d crashes and burns...\n",
		xyas(pp->pln_x, pp->pln_y, pp->pln_own), pp->pln_uid);
	    pp->pln_effic = 0;
	}
    }
    pp->pln_flags &= ~PLN_LAUNCHED;
    putplane(pp->pln_uid, pp);
    emp_remque(&plp->queue);
    free(plp);
}

/*
 * Can a carrier of @sp's type carry this load of planes?
 * The load consists of @n planes, of which @nch are choppers, @nxl
 * xlight, @nmsl light missiles, and the rest are light fixed-wing
 * planes.
 */
static int
ship_can_carry(struct shpstr *sp, int n, int nch, int nxl, int nmsl)
{
    struct mchrstr *mcp = &mchr[sp->shp_type];
    int nfw = n - nch - nxl - nmsl;

    if (nch > mcp->m_nchoppers) /* overflow into fixed-wing slots */
	nfw += nch - mcp->m_nchoppers;
    if (nxl > mcp->m_nxlight)	/* overflow into missile slots */
	nmsl += nxl - mcp->m_nxlight;
    if (nmsl && !(mcp->m_flags & (M_MSL | M_FLY)))
	return 0;		/* missile slots wanted */
    if (nfw && !(mcp->m_flags & M_FLY))
	return 0;		/* fixed-wing slots wanted */
    return nfw + nmsl <= mcp->m_nplanes;
}

/*
 * Increment carrier plane counters for @pp.
 * If it's a chopper, increment *@nch.
 * Else, if it's x-light, increment *@nxl.
 * Else, if it's a light missile, increment *@msl.
 * Return non-zero if it's a chopper, x-light or light.
 */
static int
inc_shp_nplane(struct plnstr *pp, int *nch, int *nxl, int *nmsl)
{
    struct plchrstr *pcp = &plchr[pp->pln_type];

    if (pcp->pl_flags & P_K)
	(*nch)++;
    else if (pcp->pl_flags & P_E)
	(*nxl)++;
    else if (!(pcp->pl_flags & P_L))
	return 0;
    else if (pcp->pl_flags & P_M)
	(*nmsl)++;
    return 1;
}

/*
 * Can @pp be loaded on a ship of @sp's type?
 */
int
could_be_on_ship(struct plnstr *pp, struct shpstr *sp)
{
    int nch = 0, nxl = 0, nmsl = 0;

    if (!inc_shp_nplane(pp, &nch, &nxl, &nmsl))
	return 0;
    return ship_can_carry(sp, 1, nch, nxl, nmsl);
}

int
put_plane_on_ship(struct plnstr *plane, struct shpstr *ship)
{
    int n, nch, nxl, nmsl;

    if (plane->pln_ship == ship->shp_uid)
	return 1;		/* Already on ship */

    n = shp_nplane(ship, &nch, &nxl, &nmsl);
    if (!inc_shp_nplane(plane, &nch, &nxl, &nmsl))
	return 0;		/* not a carrier plane */
    if (!ship_can_carry(ship, n + 1, nch, nxl, nmsl))
	return 0;		/* no space */

    plane->pln_x = ship->shp_x;
    plane->pln_y = ship->shp_y;
    plane->pln_ship = ship->shp_uid;
    putplane(plane->pln_uid, plane);
    return 1;
}

int
put_plane_on_land(struct plnstr *plane, struct lndstr *land)
{
    if (plane->pln_land == land->lnd_uid)
	return 1;		/* Already on unit */
    if (!(plchr[plane->pln_type].pl_flags & P_E))
	return 0;
    if (lnd_nxlight(land) >= lchr[land->lnd_type].l_nxlight)
	return 0;

    plane->pln_x = land->lnd_x;
    plane->pln_y = land->lnd_y;
    plane->pln_land = land->lnd_uid;
    putplane(plane->pln_uid, plane);
    return 1;
}

void
plane_sweep(struct emp_qelem *plane_list, coord x, coord y)
{
    struct plnstr *pp;
    struct plchrstr *pcp;
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct plist *ip;
    struct sctstr sect;
    int mines_there;
    int found = 0;

    getsect(x, y, &sect);
    mines_there = sect.sct_mines;

    if (mines_there == 0)
	return;

    if (sect.sct_type != SCT_WATER)
	return;

    for (qp = plane_list->q_forw; ((qp != plane_list) && (mines_there));
	 qp = next) {
	next = qp->q_forw;
	ip = (struct plist *)qp;
	pp = &ip->plane;
	pcp = ip->pcp;
	if (!(pcp->pl_flags & P_SWEEP))	/* if it isn't an sweep plane */
	    continue;

	if (chance((100.0 - pln_acc(pp)) / 100.0)) {
	    pr("Sweep! in %s\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	    mines_there--;
	    found = 1;
	}
    }

    if (found && map_set(player->cnum, sect.sct_x, sect.sct_y, 'X', 0))
	writemap(player->cnum);
    sect.sct_mines = mines_there;
    putsect(&sect);
}

int
pln_hitchance(struct plnstr *pp, int hardtarget, int type)
{
    struct plchrstr *pcp = plchr + pp->pln_type;
    double tfact = (double)(pp->pln_tech - pcp->pl_tech) /
	(pp->pln_tech - pcp->pl_tech / 2);
    int acc = pln_acc(pp);
    int hitchance;

    if (type == EF_SHIP) {
	if (pcp->pl_flags & P_A)
	    acc -= 20;
	if (!(pcp->pl_flags & P_T))
	    acc += 35;
    }
    hitchance = (int)(pp->pln_effic * (1.0 - 0.1 * tfact) *
		      (1.0 - acc / 100.0)) - hardtarget;

    /* smooth out the bottom of the graph with asymtote at 5 -KHS */
    if (hitchance < 20)
	hitchance = 5 + ldround(300.0 / (40.0 - hitchance), 1);
    if (hitchance > 100)
	hitchance = 100;
    return hitchance;
}

int
pln_damage(struct plnstr *pp, char type, char *noisy)
{
    struct plchrstr *pcp = plchr + pp->pln_type;
    int load, i, hitroll, aim, len;
    int dam = 0;
    int effective = 1;
    int pinbomber = 0;
    char buf[80];

    if (CANT_HAPPEN(nuk_on_plane(pp) >= 0))
	return 0;

    load = pln_load(pp);
    if (!load)		       /* e.g. ab, blowing up on launch pad */
	return 0;

    i = roll(load) + 1;
    if (i > load)
	i = load;

    if (pcp->pl_flags & P_M) {
	if (pcp->pl_flags & P_MAR)
	    pinbomber = 1;
    } else if (pcp->pl_flags & P_T)
	pinbomber = 1;

    aim = pln_acc(pp);
    if (type == 's') {
	effective = !pinbomber;
	aim = 30 + (pinbomber ? aim : 100 - aim);
    } else {
	effective = pinbomber;
	aim = 100 - aim;
    }

    len = noisy ? snprintf(buf, sizeof(buf), "%s", noisy) : 0;
    while (i--) {
	if (noisy) {
	    if (len > 75) {
		mpr(pp->pln_own, "%s\n", buf);
		len = 0;
	    }
	}
	dam += roll(6);
	hitroll = roll(100);
	if (hitroll >= 90) {
	    dam += 8;
	    if (noisy)
		len += sprintf(buf + len, "BLAM");
	} else if (hitroll < aim) {
	    dam += 5;
	    if (noisy)
		len += sprintf(buf + len, "Blam");
	} else {
	    dam += 1;
	    if (noisy)
		len += sprintf(buf + len, "blam");
	}
	if (noisy) {
	    if (i)
		len += sprintf(buf + len, "-");
	}
    }
    if (noisy && len)
	mpr(pp->pln_own, "%s\n", buf);
    if (effective)
	dam *= 2;
    return dam;
}

int
pln_identchance(struct plnstr *pp, int hardtarget, int type)
{
    double misschance =
	(100.0 - pln_hitchance(pp, hardtarget, type)) / 100.0;
    return (int)(100 - 100 * misschance * misschance);
}

int
pln_mobcost(int dist, struct plnstr *pp, char mission)
{
    double cost;

    cost = 20.0 / (pp->pln_effic / 100.0);
    if (mission == 'e' || mission == 0)
	cost /= 2;		/* escort or intercept */

    return ldround(cost * dist / pln_range_max(pp) + 5, 1);
}

int
pln_is_in_orbit(struct plnstr *pp)
{
    return (plchr[pp->pln_type].pl_flags & (P_M | P_O)) == P_O
	&& (pp->pln_flags & PLN_LAUNCHED);
}

/*
 * Set @pp's tech to @tlev along with everything else that depends on it.
 */
void
pln_set_tech(struct plnstr *pp, int tlev)
{
    struct plchrstr *pcp = plchr + pp->pln_type;
    int limited_range = pp->pln_range < pln_range_max(pp);
    int range_max;

    if (CANT_HAPPEN(tlev < pcp->pl_tech))
	tlev = pcp->pl_tech;
    pp->pln_tech = tlev;

    range_max = pln_range_max(pp);
    if (!limited_range || pp->pln_range > range_max)
	pp->pln_range = range_max;
}
