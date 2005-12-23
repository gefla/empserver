/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plnsub.c: Plane subroutine stuff
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#include <math.h>
#include "misc.h"
#include "player.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "item.h"
#include "plane.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "path.h"
#include "prototypes.h"
#include "optlist.h"

static int pln_equip(struct plist *, struct ichrstr *, int, s_char);

/*
 * Get assembly point argument.
 * If INPUT is not empty, use it, else prompt for more input using PROMPT.
 * If this yields a valid assembly point, read it into *AP_SECT and
 * return AP_SECT.
 * Else complain and return NULL.
 * *AP_SECT and BUF[1024] may be modified in either case.
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
    if (ap_sect->sct_own == player->cnum
	|| getrel(getnatp(ap_sect->sct_own), player->cnum) == ALLIED)
	return ap_sect;

    /* over own or allied ship is fine */
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, &ship)) {
	if (ship.shp_effic < SHIP_MINEFF || ship.shp_own == 0)
	    continue;
	if (ship.shp_own == player->cnum
	    || getrel(getnatp(ship.shp_own), player->cnum) == ALLIED)
	    return ap_sect;
    }

    pr("Assembly point not owned by you or an ally!\n");
    return NULL;
}

int
pln_onewaymission(struct sctstr *target, int *shipno, int *flagp)
{
    int nships;
    int cno;
    int flags;
    struct shpstr ship;
    char buf[1024];
    char *p;

    flags = *flagp;

    /* offer carriers */
    nships = carriersatxy(target->sct_x, target->sct_y,
			  M_FLY | M_CHOPPER, 0, player->cnum);
    if (nships) {
	for (;;) {
	    if (!(p = getstarg(0, "Carrier #? ", buf)) || !*p)
		break;
	    cno = atoi(p);
	    if (cno < 0
		|| !getship(cno, &ship)
		|| (!player->owner
		    && (getrel(getnatp(ship.shp_own), player->cnum)
			!= ALLIED))) {
		pr("Not yours\n");
		continue;
	    }
	    if (ship.shp_x != target->sct_x || ship.shp_y != target->sct_y) {
		pr("Ship #%d not in %s\n", cno,
		   xyas(target->sct_x, target->sct_y, player->cnum));
		continue;
	    }
	    if ((!(mchr[(int)ship.shp_type].m_flags & M_FLY)
		 && !(mchr[(int)ship.shp_type].m_flags & M_XLIGHT)
		 && !(mchr[(int)ship.shp_type].m_flags & M_CHOPPER))
		|| ship.shp_effic < SHP_AIROPS_EFF) {
		pr("Can't land on %s.\n", prship(&ship));
		continue;
	    }

	    /* clear to land on ship#CNO */
	    pr("landing on carrier %d\n", cno);
	    if (mchr[(int)ship.shp_type].m_flags & M_FLY)
		flags |= P_L;
	    if (mchr[(int)ship.shp_type].m_flags & M_CHOPPER)
		flags |= P_K;
	    if (mchr[(int)ship.shp_type].m_flags & M_XLIGHT)
		flags |= P_E;
	    *shipno = cno;
	    *flagp = flags;
	    return 0;
	}
    }

    /* try to land at sector */
    if (target->sct_own != player->cnum
	&& getrel(getnatp(target->sct_own), player->cnum) != ALLIED) {
	pr("Nowhere to land at sector %s!\n",
	   xyas(target->sct_x, target->sct_y, player->cnum));
	return -1;
    }
    if (target->sct_type == SCT_MOUNT) {
	pr("Nowhere to land at sector %s!\n",
	   xyas(target->sct_x, target->sct_y, player->cnum));
	return -1;
    }
    if (target->sct_type != SCT_AIRPT || target->sct_effic < 60)
	flags |= P_V;

    /* clear to land at sector */
    *shipno = -1;
    *flagp = flags;
    return 0;
}

int
pln_oneway_to_carrier_ok(struct emp_qelem *bomb_list,
			 struct emp_qelem *esc_list, int cno)
{
    struct emp_qelem *list, *qp;
    struct plist *plp;
    struct plchrstr *pcp;
    struct shpstr ship;
    struct mchrstr *mcp;
    int nchoppers, nxlight, nplane;

    if (cno < 0 || !getship(cno, &ship))
	return 0;

    count_planes(&ship);
    nchoppers = ship.shp_nchoppers;
    nxlight = ship.shp_nxlight;
    nplane = ship.shp_nplane;
    mcp = &mchr[(int)ship.shp_type];

    /* for both lists */
    for (list = bomb_list;
	 list;
	 list = list == bomb_list ? esc_list : NULL) {
	for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	    /* FIXME duplicates put_plane_on_ship() logic; refactor */
	    plp = (struct plist *)qp;
	    pcp = &plchr[(int)plp->plane.pln_type];
	    if (plp->plane.pln_ship == ship.shp_uid)
		continue;
	    /* try chopper space */
	    if ((pcp->pl_flags & P_K) && (mcp->m_flags & M_CHOPPER)
		&& nchoppers < mcp->m_nchoppers)
		++nchoppers;
	    /* try xlight space */
	    else if ((pcp->pl_flags & P_E) && (mcp->m_flags & M_XLIGHT)
		     && nxlight < mcp->m_nxlight)
		++nxlight;
	    /* try plane space */
	    else if ((((pcp->pl_flags & P_L) && (mcp->m_flags & M_FLY))
		      || ((pcp->pl_flags & P_M) && (pcp->pl_flags & P_L)
			  && (mcp->m_flags & M_MSL)))
		     && nplane < mcp->m_nplanes)
		++nplane;
	    else
		return 0;		/* won't be able to land */
	}
    }
    return 1;
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
	    count_planes(&ship);
	    if (!can_be_on_ship(plp->plane.pln_uid, ship.shp_uid))
		pr("\t%s cannot land on ship #%d! %s aborts!\n",
		   prplane(&plp->plane), cno, prplane(&plp->plane));
	    else if (!put_plane_on_ship(&plp->plane, &ship))
		pr("\tNo room on ship #%d! %s aborts!\n", cno,
		   prplane(&plp->plane));
	    else {
		if (plp->plane.pln_own != ship.shp_own) {
/*					plp->plane.pln_own = ship.shp_own;*/
		    wu(0, ship.shp_own,
		       "%s %s lands on your %s\n",
		       cname(player->cnum),
		       prplane(&plp->plane), prship(&ship));
		}
	    }
	} else {
	    plp->plane.pln_x = tx;
	    plp->plane.pln_y = ty;
	    getsect(tx, ty, &sect);
	    if (plp->plane.pln_own != sect.sct_own) {
/*				plp->plane.pln_own = sect.sct_own;*/
		wu(0, sect.sct_own,
		   "%s %s lands at your sector %s\n",
		   cname(player->cnum),
		   prplane(&plp->plane), xyas(tx, ty, sect.sct_own));
	    }
	    plp->plane.pln_ship = cno;
	}
    }
    if (cno >= 0)
	putship(ship.shp_uid, &ship);
}

void
pln_dropoff(struct emp_qelem *list, struct ichrstr *ip, coord tx, coord ty,
	    void *ptr, int type)
{
    struct emp_qelem *qp;
    struct plist *plp;
    int amt;
    struct sctstr *sectp;
    struct shpstr *sp;
    int there;
    int max;
    struct mchrstr *mp;

    if (ip == 0)
	return;
    amt = 0;
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	amt += plp->misc;
    }
    if (type == EF_SECTOR) {
	sectp = ptr;
	if (!sectp->sct_own) {
	    if (sectp->sct_type == SCT_WATER)
		pr("Your %s sink like a rock!\n", ip->i_name);
	    else
		pr("Your %s vanish without a trace.\n", ip->i_name);
	    return;
	}
	if (sectp->sct_own != player->cnum
	    && getrel(getnatp(sectp->sct_own), player->cnum) != ALLIED) {
	    pr("You don't own %s!  Cargo jettisoned...\n",
	       xyas(tx, ty, player->cnum));
	    return;
	}
	if (ip->i_vtype == I_CIVIL && sectp->sct_own != sectp->sct_oldown) {
	    pr("%s is occupied.  Your civilians suffer from identity crisis and die.\n",
	       xyas(tx, ty, player->cnum));
	    return;
	}
	there = sectp->sct_item[ip->i_vtype];
	max = ITEM_MAX;
    } else {
	sp = ptr;
	there = sp->shp_item[ip->i_vtype];
	mp = &mchr[(int)sp->shp_type];
	max = mp->m_item[ip->i_vtype];
    }
    there += amt;
    if (there > max) {
	pr("%d excess %s discarded\n", max - there, ip->i_name);
	amt = max - there;
	there = max;
    }
    pr("%d %s landed safely", amt, ip->i_name);
    if (type == EF_SECTOR) {
	sectp = ptr;
	sectp->sct_item[ip->i_vtype] = there;
	if (sectp->sct_own != player->cnum)
	    wu(0, sectp->sct_own, "%s planes drop %d %s in %s\n",
	       cname(player->cnum), amt, ip->i_name,
	       xyas(sectp->sct_x, sectp->sct_y, sectp->sct_own));
	pr(" at %s\n", xyas(tx, ty, player->cnum));
	putsect((struct sctstr *)ptr);
    } else {
	struct shpstr *sp = (struct shpstr *)ptr;
	sp->shp_item[ip->i_vtype] = there;
	if (sp->shp_own != player->cnum)
	    wu(0, sp->shp_own, "%s planes land %d %s on carrier %d\n",
	       cname(player->cnum), amt, ip->i_name, sp->shp_uid);
	pr(" on carrier #%d\n", sp->shp_uid);
	putship(sp->shp_uid, sp);
    }
}

void
pln_mine(struct emp_qelem *list, struct sctstr *sectp)
{
    struct emp_qelem *qp;
    struct plist *plp;
    int amt;

    amt = 0;
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	amt += plp->misc;

    }
    if (amt > 0) {
	if (sectp->sct_type != SCT_WATER) {
	    pr("Your seamines have no effect here.\n");
	    return;
	}
	sectp->sct_mines = min(sectp->sct_mines + amt, MINES_MAX);
	pr("%d mines laid in %s.\n", amt,
	   xyas(sectp->sct_x, sectp->sct_y, player->cnum));
	if (map_set(player->cnum, sectp->sct_x, sectp->sct_y, 'X', 0))
	    writemap(player->cnum);
	putsect(sectp);
    }
}

void
pln_sel(struct nstr_item *ni, struct emp_qelem *list, struct sctstr *ap,
	int ap_to_target, int rangemult, int wantflags, int nowantflags)
{
    struct plnstr plane;
    struct shpstr ship;
    struct lndstr land;
    struct sctstr sect;
    int range;
    struct plchrstr *pcp;
    struct plist *plp;
    int y;
    int bad, bad1;
    unsigned int x;

    emp_initque(list);
    while (nxtitem(ni, &plane)) {
	if (!player->owner)
	    continue;
	if (plane.pln_mobil <= 0)
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, (int *)&plane)) {
		pr("plane #%d inelligible - it's for sale.\n",
		   plane.pln_uid);
		continue;
	    }
	}

	range = mapdist(plane.pln_x, plane.pln_y, ap->sct_x, ap->sct_y);
	if (range > 4) {
	    pr("%s too far from assembly point\n", prplane(&plane));
	    continue;
	}
	if (plane.pln_effic < 40) {
	    pr("%s not efficient enough (must be 40%%)\n",
	       prplane(&plane));
	    continue;
	}
	range += ap_to_target;
	pcp = &plchr[(int)plane.pln_type];
	bad = 0;
	bad1 = 0;
	if (wantflags) {
	    for (x = 0; x < sizeof(wantflags) * 8; x++) {
		y = (1 << x);
		if ((wantflags & y) == y)
		    if ((pcp->pl_flags & y) != y) {
			switch (y) {
			case P_F:
			case P_ESC:
			    bad1 = 2;
			    break;
			case P_E:
			case P_L:
			case P_K:
			    bad1 = 1;
			    break;
			default:
			    bad = 1;
			}
		    }
	    }
	    if (bad)
		continue;
	    if (bad1 == 2) {
		if ((pcp->pl_flags & P_ESC) || (pcp->pl_flags & P_F))
		    bad1 = 0;
	    }
	    if (bad1 == 1) {
		if ((wantflags & P_L) && (pcp->pl_flags & P_L))
		    bad1 = 0;
		if ((wantflags & P_K) && (pcp->pl_flags & P_K))
		    bad1 = 0;
		if ((wantflags & P_E) && (pcp->pl_flags & P_E))
		    bad1 = 0;
	    }
	    if (bad1)
		continue;
	}
	bad = 0;
	bad1 = 0;
	if (nowantflags) {
	    for (x = 0; x < sizeof(nowantflags) * 8; x++) {
		y = (1 << x);
		if ((nowantflags & y) == y)
		    if ((pcp->pl_flags & y) == y)
			bad = 1;
	    }
	    if (bad)
		continue;
	}
	range *= rangemult;
	if (plane.pln_range < range) {
	    pr("%s out of range (%d:%d)\n", prplane(&plane),
	       plane.pln_range, range);
	    continue;
	}
	if (plane.pln_ship >= 0) {
	    if (!getship(plane.pln_ship, &ship) ||
		plane.pln_own != player->cnum) {
	      shipsunk:
		plane.pln_effic = 0;
		pr("(note) ship not valid for %s\n", prplane(&plane));
		putplane(plane.pln_uid, &plane);
		continue;
	    }
	    if (!can_be_on_ship(plane.pln_uid, ship.shp_uid))
		goto shipsunk;
	    if (ship.shp_effic < SHIP_MINEFF)
		goto shipsunk;
	    if (ship.shp_effic < SHP_AIROPS_EFF)
		continue;
	    /* Can't fly off non-owned ships or non-allied ship */
	    if ((ship.shp_own != player->cnum) &&
		(getrel(getnatp(ship.shp_own), player->cnum) != ALLIED)) {
		pr("(note) An ally does not own the ship %s is on\n",
		   prplane(&plane));
		continue;
	    }
	}
	if (plane.pln_land >= 0) {
	    if (!getland(plane.pln_land, &land) ||
		(plane.pln_own != player->cnum)) {
	      landdead:
		plane.pln_effic = 0;
		pr("(note) land unit not valid for %s\n", prplane(&plane));
		putplane(plane.pln_uid, &plane);
		continue;
	    }
	    if (!(plchr[(int)plane.pln_type].pl_flags & P_E))
		goto landdead;
	    if (land.lnd_effic < LAND_MINEFF)
		goto landdead;
	    if (land.lnd_effic < LND_AIROPS_EFF)
		continue;
	    /* Can't fly off units in ships or other units */
	    if ((land.lnd_ship >= 0) || (land.lnd_land >= 0))
		continue;
	    /* Can't fly off non-owned units or non-allied unit */
	    if ((land.lnd_own != player->cnum) &&
		(getrel(getnatp(land.lnd_own), player->cnum) != ALLIED)) {
		pr("(note) An ally does not own the unit %s is on\n",
		   prplane(&plane));
		continue;
	    }
	}
	/* Now, check the sector status if not on a plane or unit */
	if ((plane.pln_ship < 0) && (plane.pln_land < 0)) {
	    if (!getsect(plane.pln_x, plane.pln_y, &sect))
		continue;
	    /* First, check allied status */
	    /* Can't fly from non-owned sectors or non-allied sectors */
	    if ((sect.sct_own != player->cnum) &&
		(getrel(getnatp(sect.sct_own), player->cnum) != ALLIED)) {
		pr("(note) An ally does not own the sector %s is in\n",
		   prplane(&plane));
		continue;
	    }
	    /* non-vtol plane */
	    if ((pcp->pl_flags & P_V) == 0) {
		if (sect.sct_type != SCT_AIRPT) {
		    pr("%s not at airport\n", prplane(&plane));
		    continue;
		}
		if (sect.sct_effic < 40) {
		    pr("%s is not 40%% efficient, %s can't take off from there.\n", xyas(sect.sct_x, sect.sct_y, plane.pln_own), prplane(&plane));
		    continue;
		}
		if (rangemult == 2 && sect.sct_effic < 60) {
		    pr("%s is not 60%% efficient, %s can't land there.\n",
		       xyas(sect.sct_x, sect.sct_y, plane.pln_own),
		       prplane(&plane));
		    continue;
		}
	    }
	}
	pr("%s standing by\n", prplane(&plane));
	plane.pln_mission = 0;
	putplane(plane.pln_uid, &plane);
	plp = malloc(sizeof(struct plist));
	plp->misc = 0;
	plp->bombs = 0;
	plp->pcp = pcp;
	plp->plane = plane;
	emp_insque(&plp->queue, list);
    }
}

int
pln_arm(struct emp_qelem *list, int dist, int mission, struct ichrstr *ip,
	int flags, int mission_flags)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct plist *plp;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	if (pln_equip(plp, ip, flags, mission) < 0) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}
	if (flags & (P_S | P_I)) {
	    if (plp->pcp->pl_flags & P_S)
		mission_flags |= P_S;
	    if (plp->pcp->pl_flags & P_I)
		mission_flags |= P_I;
	}
	if (!(plp->pcp->pl_flags & P_H))
	    /* no stealth on this mission */
	    mission_flags &= ~P_H;
	if (!(plp->pcp->pl_flags & P_X))
	    /* no stealth on this mission */
	    mission_flags &= ~P_X;
	if (!(plp->pcp->pl_flags & P_A)) {
	    /* no asw on this mission */
	    mission_flags &= ~P_A;
	}
	if (!(plp->pcp->pl_flags & P_MINE)) {
	    /* no asw on this mission */
	    mission_flags &= ~P_MINE;
	}
	plp->plane.pln_mobil -= pln_mobcost(dist, &plp->plane, flags);
	pr("%s equipped\n", prplane(&plp->plane));
    }
    return mission_flags;
}

static int
pln_equip(struct plist *plp, struct ichrstr *ip, int flags, s_char mission)
{
    struct plchrstr *pcp;
    struct plnstr *pp;
    int needed;
    struct lndstr land;
    struct shpstr ship;
    struct sctstr sect;
    i_type itype;
    int rval;
    short *item;
    int own;
    int abandon_needed;

    pp = &plp->plane;
    pcp = plp->pcp;
    if (pp->pln_ship >= 0) {
	getship(pp->pln_ship, &ship);
	item = ship.shp_item;
	own = ship.shp_own;
    } else if (pp->pln_land >= 0) {
	getland(pp->pln_land, &land);
	item = land.lnd_item;
	own = land.lnd_own;
    } else {
	getsect(pp->pln_x, pp->pln_y, &sect);
	item = sect.sct_item;
	own = sect.sct_oldown;
    }
    if (ip) {
	if (ip->i_vtype == I_CIVIL) {
	    if (pp->pln_own != own) {
		pr("You don't control those civilians!\n");
		return -1;
	    }
	}
    }
    if (pcp->pl_fuel > item[I_PETROL]) {
	pr("%s not enough petrol there!\n", prplane(pp));
	return -1;
    }
    item[I_PETROL] -= pcp->pl_fuel;
    rval = 0;
    if ((flags & P_F) == 0) {
	itype = I_NONE;
	needed = 0;
	switch (mission) {
	case 's':
	case 'p':
	    if (pp->pln_nuketype == -1) {
		itype = I_SHELL;
		needed = pp->pln_load;
	    }
	    break;
	case 't':
	case 'd':
	    if ((pcp->pl_flags & P_C) == 0 || ip == 0)
		break;
	    itype = ip->i_vtype;
	    needed = (pp->pln_load * 2) / ip->i_lbs;
	    break;
	case 'm':
	    if ((pcp->pl_flags & P_MINE) == 0)
		break;
	    itype = I_SHELL;
	    needed = (pp->pln_load * 2) / ip->i_lbs;
	    break;
	case 'a':
	    if ((pcp->pl_flags & (P_V | P_C)) == 0)
		break;
	    itype = I_MILIT;
	    needed = pp->pln_load / ip->i_lbs;
	    break;
	case 'n':
	    if (pp->pln_nuketype == -1)
		rval = -1;
	    break;
	default:
	    break;
	}
	if (rval < 0 || (itype != I_NONE && needed <= 0)) {
	    pr("%s can't contribute to mission\n", prplane(pp));
	    return -1;
	}
	if (itype != I_NONE) {
#if 0
	    /* Supply is broken somewhere, so don't use it for now */
	    if (itype == I_SHELL && item[itype] < needed)
		item[itype] += supply_commod(plp->plane.pln_own,
					     plp->plane.pln_x,
					     plp->plane.pln_y,
					     I_SHELL, needed);
#endif
	    abandon_needed = !!would_abandon(&sect, itype, needed, NULL);
	    if (item[itype] < needed + abandon_needed) {
		pr("Not enough %s for %s\n", ichr[itype].i_name, prplane(pp));
		return -1;
	    }
	    item[itype] -= needed;
	}
	if (itype == I_SHELL && (mission == 's' || mission == 'p'))
	    plp->bombs = needed;
	else
	    plp->misc = needed;
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
	       cname(pp->pln_own), prplane(pp), xyas(sect.sct_x,
						     sect.sct_y,
						     sect.sct_own));
	}
	putsect(&sect);
    }
    return rval;
}

void
pln_put(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct emp_qelem *newqp;
    struct plist *plp;
    struct plnstr *pp;
    struct shpstr ship;
    struct sctstr sect;

    /* Here is where planes return home from bombing runs.
       We need to make sure they still have somewhere to return
       home to! */
    qp = list->q_forw;
    while (qp != list) {
	plp = (struct plist *)qp;
	pp = &plp->plane;
	/* Ok, check out where it wants to land */
	if (pp->pln_ship >= 0) {
	    /* It is landing on a carrier */
	    getship(pp->pln_ship, &ship);
	    /* We should do more, like make sure it's really
	       a carrier, etc. but for now just make sure it's
	       not sunk. */
	    if (ship.shp_effic < SHIP_MINEFF) {
		mpr(pp->pln_own,
		    "Ship #%d has been sunk, plane #%d has nowhere to land, and\nsplashes into the sea.\n",
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
	putplane(pp->pln_uid, pp);
	newqp = qp->q_forw;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }
}

void
pln_removedupes(struct emp_qelem *bomb_list, struct emp_qelem *esc_list)
{
    struct emp_qelem *bomb;
    struct emp_qelem *esc;
    struct plist *bombp;
    struct plist *escp;

    if (QEMPTY(bomb_list) || QEMPTY(esc_list))
	return;
    bomb = bomb_list->q_forw;
    while (bomb != bomb_list) {
	if (QEMPTY(esc_list)) {
	    bomb = bomb_list;
	    continue;
	}
	esc = esc_list->q_forw;
	bombp = (struct plist *)bomb;
	while (esc != esc_list) {
	    escp = (struct plist *)esc;
	    if (escp->plane.pln_uid == bombp->plane.pln_uid) {
		emp_remque(esc);
		free(esc);
		esc = esc_list;
	    } else
		esc = esc->q_forw;
	}
	bomb = bomb->q_forw;
    }
}

int
put_plane_on_ship(struct plnstr *plane, struct shpstr *ship)
{
    struct plchrstr *pcp;
    struct mchrstr *mcp;

    pcp = &plchr[(int)plane->pln_type];
    mcp = &mchr[(int)ship->shp_type];

    if (((int)plane->pln_ship) == ((int)ship->shp_uid))
	return 1;		/* Already on ship */

    /* Try to put on ship as a chopper plane */
    if ((pcp->pl_flags & P_K) &&
	(mcp->m_flags & M_CHOPPER) &&
	(ship->shp_nchoppers < mcp->m_nchoppers)) {

	ship->shp_nchoppers++;
	plane->pln_x = ship->shp_x;
	plane->pln_y = ship->shp_y;
	plane->pln_ship = ship->shp_uid;
	putship(ship->shp_uid, ship);
	putplane(plane->pln_uid, plane);
	return 1;
    }

    /* Try to put on ship as an xlight plane */
    if ((pcp->pl_flags & P_E) &&
	(mcp->m_flags & M_XLIGHT) &&
	(ship->shp_nxlight < mcp->m_nxlight)) {

	ship->shp_nxlight++;
	plane->pln_x = ship->shp_x;
	plane->pln_y = ship->shp_y;
	plane->pln_ship = ship->shp_uid;
	putship(ship->shp_uid, ship);
	putplane(plane->pln_uid, plane);
	return 1;
    }

    /* Try to put on ship as a normal plane */
    if ((((pcp->pl_flags & P_L) && (mcp->m_flags & M_FLY)) ||
	 ((pcp->pl_flags & P_M) && (pcp->pl_flags & P_L) &&
	  (mcp->m_flags & M_MSL))) &&
	(ship->shp_nplane < mcp->m_nplanes)) {

	ship->shp_nplane++;
	plane->pln_x = ship->shp_x;
	plane->pln_y = ship->shp_y;
	plane->pln_ship = ship->shp_uid;
	putship(ship->shp_uid, ship);
	putplane(plane->pln_uid, plane);
	return 1;
    }

    /* We have failed */
    return 0;
}

int
take_plane_off_ship(struct plnstr *plane, struct shpstr *ship)
{
    struct plchrstr *pcp;
    struct mchrstr *mcp;

    pcp = &plchr[(int)plane->pln_type];
    mcp = &mchr[(int)ship->shp_type];

    /* Try to take off ship as a chopper plane */
    if ((pcp->pl_flags & P_K) &&
	(mcp->m_flags & M_CHOPPER) && (ship->shp_nchoppers)) {

	ship->shp_nchoppers--;
	plane->pln_ship = -1;
	putship(ship->shp_uid, ship);
	putplane(plane->pln_uid, plane);
	return 1;
    }

    /* Try to take off ship as an xlight plane */
    if ((pcp->pl_flags & P_E) &&
	(mcp->m_flags & M_XLIGHT) && (ship->shp_nxlight)) {

	ship->shp_nxlight--;
	plane->pln_ship = -1;
	putship(ship->shp_uid, ship);
	putplane(plane->pln_uid, plane);
	return 1;
    }

    /* Try to take off ship as a normal plane */
    if ((((pcp->pl_flags & P_L) && (mcp->m_flags & M_FLY)) ||
	 ((pcp->pl_flags & P_M) && (pcp->pl_flags & P_L) &&
	  (mcp->m_flags & M_MSL))) && (ship->shp_nplane)) {

	ship->shp_nplane--;
	plane->pln_ship = -1;
	putship(ship->shp_uid, ship);
	putplane(plane->pln_uid, plane);
	return 1;
    }

    /* We have failed */
    return 0;
}

int
take_plane_off_land(struct plnstr *plane, struct lndstr *land)
{
    struct plchrstr *pcp;
    struct lchrstr *lcp;

    pcp = &plchr[(int)plane->pln_type];
    lcp = &lchr[(int)land->lnd_type];

    /* Try to take off ship as an xlight plane */
    if ((pcp->pl_flags & P_E) &&
	(lcp->l_flags & L_XLIGHT) && (land->lnd_nxlight)) {

	land->lnd_nxlight--;
	plane->pln_land = -1;
	putland(land->lnd_uid, land);
	putplane(plane->pln_uid, plane);
	return 1;
    }

    /* We have failed */
    return 0;
}

int
can_be_on_ship(int p, int s)
{
    struct plnstr plane;
    struct shpstr ship;
    struct plchrstr *pcp;
    struct mchrstr *mcp;

    getplane(p, &plane);
    getship(s, &ship);

    pcp = &plchr[(int)plane.pln_type];
    mcp = &mchr[(int)ship.shp_type];

    if (pcp->pl_flags & P_L)
	if (mcp->m_flags & M_FLY)
	    return 1;

    if (pcp->pl_flags & P_K)
	if (mcp->m_flags & M_CHOPPER)
	    return 1;

    if (pcp->pl_flags & P_M)
	if (mcp->m_flags & M_MSL)
	    return 1;

    if (pcp->pl_flags & P_E)
	if (mcp->m_flags & M_XLIGHT)
	    return 1;

    return 0;
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

    if ((sect.sct_type != SCT_WATER) && (sect.sct_type != SCT_HARBR))
	return;

    for (qp = plane_list->q_forw; ((qp != plane_list) && (mines_there));
	 qp = next) {
	next = qp->q_forw;
	ip = (struct plist *)qp;
	pp = &ip->plane;
	pcp = ip->pcp;
	if (!(pcp->pl_flags & P_SWEEP))	/* if it isn't an sweep plane */
	    continue;

	if (chance(((double)(100 - pp->pln_acc)) / 100.0)) {
	    pr("Sweep! in %s\n",
	       xyas(sect.sct_x, sect.sct_y, pp->pln_own));
	    mines_there--;
	    found = 1;
	}
    }

    if (found && map_set(player->cnum, sect.sct_x, sect.sct_y, 'X', 0))
	writemap(player->cnum);
    sect.sct_mines = mines_there;
    putsect(&sect);
}

void
count_planes(struct shpstr *sp)
{
    struct nstr_item ni;
    struct plnstr plane;
    struct plchrstr *pcp;
    struct mchrstr *mcp;
    int nplane = 0;
    int nchoppers = 0;
    int nxlight = 0;

    if (sp->shp_effic < SHIP_MINEFF)
	return;

    mcp = &mchr[(int)sp->shp_type];
    snxtitem_xy(&ni, EF_PLANE, sp->shp_x, sp->shp_y);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own == 0)
	    continue;
	if (plane.pln_ship == sp->shp_uid) {
	    pcp = &plchr[(int)plane.pln_type];
	    if ((pcp->pl_flags & P_K) && (nchoppers < mcp->m_nchoppers))
		nchoppers++;
	    else if ((pcp->pl_flags & P_E) && (nxlight < mcp->m_nxlight))
		nxlight++;
	    else if ((pcp->pl_flags & P_L) || (pcp->pl_flags & P_M))
		nplane++;
	}
    }

    if (nplane != sp->shp_nplane ||
	nxlight != sp->shp_nxlight || nchoppers != sp->shp_nchoppers) {
	sp->shp_nplane = nplane;
	sp->shp_nxlight = nxlight;
	sp->shp_nchoppers = nchoppers;
	putship(sp->shp_uid, sp);
    }
}

void
count_land_planes(struct lndstr *lp)
{
    struct nstr_item ni;
    struct plnstr plane;
    int nplane = 0;

    if (lp->lnd_effic < LAND_MINEFF)
	return;

    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own == 0)
	    continue;
	if (plane.pln_land == lp->lnd_uid)
	    nplane++;
    }

    if (lp->lnd_nxlight != nplane) {
	lp->lnd_nxlight = nplane;
	putland(lp->lnd_uid, lp);
    }
}

int
count_sect_planes(struct sctstr *sp)
{
    int count = 0;
    struct nstr_item ni;
    struct plnstr plane;

    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &plane)) {
	if (!plane.pln_own)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	if (plane.pln_x == sp->sct_x && plane.pln_y == sp->sct_y)
	    ++count;
    }

    return count;
}

int
put_plane_on_land(struct plnstr *plane, struct lndstr *land)
{
    struct plchrstr *pcp;
    struct lchrstr *lcp;

    pcp = &plchr[(int)plane->pln_type];
    lcp = &lchr[(int)land->lnd_type];

    if (((int)plane->pln_land) == ((int)land->lnd_uid))
	return 1;		/* Already on unit */

    /* Try to put on unit as an xlight plane */
    if ((pcp->pl_flags & P_E) &&
	(lcp->l_flags & L_XLIGHT) &&
	(land->lnd_nxlight < lcp->l_nxlight)) {

	land->lnd_nxlight++;
	plane->pln_x = land->lnd_x;
	plane->pln_y = land->lnd_y;
	plane->pln_land = land->lnd_uid;
	putland(land->lnd_uid, land);
	putplane(plane->pln_uid, plane);
	return 1;
    }

    /* We have failed */
    return 0;
}

int
pln_hitchance(struct plnstr *pp, int hardtarget, int type)
{
    struct plchrstr *pcp = plchr + pp->pln_type;
    float tfact = (float)(pp->pln_tech - pcp->pl_tech) /
	(pp->pln_tech - pcp->pl_tech / 2);
    int acc = pp->pln_acc;
    int hitchance;

    if (type == EF_SHIP) {
	if (pcp->pl_flags & P_A)
	    acc -= 20;
	if (!(pcp->pl_flags & P_T))
	    acc += 35;
    }
    hitchance = (int)(pp->pln_effic * (1.0 - 0.1 * tfact) *
		      (1.0 - (float)acc / 100.0)) - hardtarget;

    /* smooth out the bottom of the graph with asymtote at 5 -KHS */
    if (hitchance < 20)
	hitchance = 5 + ldround(300.0 / (40.0 - hitchance), 1);
    if (hitchance > 100)
	hitchance = 100;
    return hitchance;
}

/* return 0 if there was a nuclear detonation */

int
pln_damage(struct plnstr *pp, coord x, coord y, s_char type, int *nukedamp,
	   int noisy)
{
    struct plchrstr *pcp = plchr + pp->pln_type;
    int i;
    int hitroll;
    int dam = 0;
    int aim;
    int effective = 1;
    int pinbomber = 0;

    if (pp->pln_nuketype != -1) {
	mpr(pp->pln_own, "Releasing RV's for %s detonation...\n",
	    pp->pln_flags & PLN_AIRBURST ? "airburst" : "groundburst");
	*nukedamp = detonate(pp, x, y);
	return 0;
    } else
	*nukedamp = 0;

    if (!pp->pln_load)		/* e.g. ab, blowing up on launch pad */
	return 0;

    i = roll(pp->pln_load) + 1;
    if (i > pp->pln_load)
	i = pp->pln_load;

    if (pcp->pl_flags & P_M) {
	if (pcp->pl_flags & P_MAR)
	    pinbomber = 1;
    } else if (pcp->pl_flags & P_T)
	pinbomber = 1;

    aim = 100 - pp->pln_acc;
    if (type == 's') {
	if (pinbomber) {
	    aim = pp->pln_acc;
	    effective = 0;
	}
	aim += 30;
    } else {
	if (!pinbomber) {
	    effective = 0;
	}
    }
    while (i--) {
	dam += roll(6);
	hitroll = roll(100);
	if (hitroll >= 90) {
	    dam += 8;
	    if (noisy)
		mpr(pp->pln_own, "BLAM");
	} else if (hitroll < aim) {
	    dam += 5;
	    if (noisy)
		mpr(pp->pln_own, "Blam");
	} else {
	    dam += 1;
	    if (noisy)
		mpr(pp->pln_own, "blam");
	}
	if (i && noisy)
	    mpr(pp->pln_own, "-");
    }
    if (noisy)
	mpr(pp->pln_own, "\n");
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
pln_mobcost(int dist, struct plnstr *pp, int flags)
{
    int cost;

    if ((flags & P_F) || (flags & P_ESC))
	cost = 10 * 100 / pp->pln_effic;
    else
	cost = 20 * 100 / pp->pln_effic;

    cost = ldround((double)cost * dist / pp->pln_range_max, 1);

    return min(32 + pp->pln_mobil, cost + 5);
}

/*
 * Set PP's tech to TLEV along with everything else that depends on it.
 */
void
pln_set_tech(struct plnstr *pp, int tlev)
{
    struct plchrstr *pcp = plchr + pp->pln_type;
    int tech_diff = tlev - pcp->pl_tech;
    int limited_range = pp->pln_range < pp->pln_range_max;

    if (CANT_HAPPEN(tech_diff < 0)) {
      tlev -= tech_diff;
      tech_diff = 0;
    }

    pp->pln_tech = tlev;
    pp->pln_att = PLN_ATTDEF(pcp->pl_att, tech_diff);
    pp->pln_def = PLN_ATTDEF(pcp->pl_def, tech_diff);
    pp->pln_acc = PLN_ACC(pcp->pl_acc, tech_diff);
    pp->pln_range_max = PLN_RAN(pcp->pl_range, tech_diff);
    pp->pln_load = PLN_LOAD(pcp->pl_load, tech_diff);

    if (!limited_range || pp->pln_range > pp->pln_range_max)
	pp->pln_range = pp->pln_range_max;
}
