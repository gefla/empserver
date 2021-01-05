/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  cargo.c: Cargo lists
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2009
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "unit.h"

struct clink {
    int next;
    int head[EF_NUKE - EF_PLANE + 1];
};

/*
 * Cargo lists
 *
 * Persistent game state encodes "who carries what" by storing the
 * carrier UID in the cargo.  Cargo lists augment that: they store
 * lists of cargo for each carrier.
 *
 * clink[TYPE] points to an array of cargo list links.  The array has
 * nclink[TYPE] elements.  nclink[TYPE] tracks ef_nelem(TYPE).
 * clink[TYPE][UID] is the cargo list link for unit UID of type TYPE.
 * TYPE must be a unit file type: EF_SHIP, EF_PLANE, EF_LAND or
 * EF_NUKE.  Other slots of clink[] and nclink[] are unused and remain
 * zero.
 *
 * clink[TYPE][UID].next is the UID of the next unit of the same type
 * in the same carrier, -1 if none.
 *
 * clink[TYPE][UID].head[CARGO-EF_PLANE] is the UID of the first unit
 * of type CARGO carried by this unit, -1 if none.  The next unit, if
 * any, is clink[CARGO][clink[TYPE][UID].head[CARGO-EF_PLANE]].next,
 * and so forth.
 *
 * Each type of carrier can only carry certain types of cargo, but
 * cargo lists know nothing about that.
 */
static struct clink *clink[EF_NUKE + 1];
static int nclink[EF_NUKE + 1];

/*
 * Return pointer to @cl's cargo list head for file type @type.
 */
static int *
clink_headp(struct clink *cl, int type)
{
    static int dummy;

    if (CANT_HAPPEN(type < EF_PLANE || type > EF_NUKE)) {
	dummy = -1;
	return &dummy;
    }
    return &cl->head[type - EF_PLANE];
}

/*
 * Initialize cargo list link @cl to empty.
 */
static void
clink_init(struct clink *cl)
{
    unsigned i;

    cl->next = -1;
    for (i = 0; i < sizeof(cl->head) / sizeof(*cl->head); i++)
	cl->head[i] = -1;
}

/*
 * Check whether *@uidp is a valid UID for file type @type.
 */
static void
clink_check1(int *uidp, int type)
{
    if (CANT_HAPPEN(*uidp >= nclink[type]))
	*uidp = -1;
}

/*
 * Check validity of cargo lists for file type @type.
 */
static void
clink_check(int type)
{
    int carr_type, i;

    /* check the heads for all carriers */
    if (type != EF_SHIP) {
	for (carr_type = EF_PLANE; carr_type <= EF_NUKE; carr_type++) {
	    for (i = 0; i < nclink[carr_type]; i++)
		clink_check1(clink_headp(&clink[carr_type][i], type),
			     type);
	}
    }
    /* check the nexts */
    for (i = 0; i < nclink[type]; i++)
	clink_check1(&clink[type][i].next, type);
}

/*
 * Add to @cl's cargo list for type @type the UID @uid.
 * @uid must not be on any cargo list already.
 */
static void
clink_add(struct clink *cl, int type, int uid)
{
    int *head = clink_headp(cl, type);

    if (CANT_HAPPEN(type < 0 || type > EF_NUKE
		    || uid < 0 || uid >= nclink[type]))
	return;
    if (CANT_HAPPEN(*head >= nclink[type]))
	*head = -1;
    CANT_HAPPEN(clink[type][uid].next >= 0);
    clink[type][uid].next = *head;
    *head = uid;
}

/*
 * Remove from @cl's cargo list for type @type the UID @uid.
 * @uid must be on that cargo list.
 */
static void
clink_rem(struct clink *cl, int type, int uid)
{
    int *head = clink_headp(cl, type);
    struct clink *linkv;
    int n;
    int *p;

    if (CANT_HAPPEN(type < 0 || type > EF_NUKE))
	return;
    linkv = clink[type];
    n = nclink[type];

    for (p = head; *p != uid; p = &linkv[*p].next) {
	if (CANT_HAPPEN(*p < 0 || *p >= n))
	    return;
    }

    *p = linkv[uid].next;
    linkv[uid].next = -1;
}

/*
 * Update cargo lists for a change of @cargo's carrier.
 * Carrier is of type @type, and changes from UID @old to @new.
 * Negative UIDs mean no carrier.
 */
void
unit_carrier_change(struct empobj *cargo, int type, int old, int new)
{
    if (CANT_HAPPEN(type < 0 || type > EF_NUKE))
	return;
    if (old >= 0 && !CANT_HAPPEN(old >= nclink[type]))
	clink_rem(&clink[type][old], cargo->ef_type, cargo->uid);
    if (new >= 0 && !CANT_HAPPEN(new >= nclink[type]))
	clink_add(&clink[type][new], cargo->ef_type, cargo->uid);
}

/*
 * Update cargo lists for a change of @pp's carrier.
 * Carrier is of type @type, and changes from UID @old to @new.
 * Negative UIDs mean no carrier.
 */
void
pln_carrier_change(struct plnstr *pp, int type, int old, int new)
{
    unit_carrier_change((struct empobj *)pp, type, old, new);
}

/*
 * Update cargo lists for a change of @lp's carrier.
 * Carrier is of type @type, and changes from UID @old to @new.
 * Negative UIDs mean no carrier.
 */
void
lnd_carrier_change(struct lndstr *lp, int type, int old, int new)
{
    unit_carrier_change((struct empobj *)lp, type, old, new);
}

/*
 * Update cargo lists for a change of @np's carrier.
 * Carrier is of type @type, and changes from UID @old to @new.
 * Negative UIDs mean no carrier.
 */
void
nuk_carrier_change(struct nukstr *np, int type, int old, int new)
{
    unit_carrier_change((struct empobj *)np, type, old, new);
}

/*
 * Initialize cargo lists from game state.
 */
void
unit_cargo_init(void)
{
    int i;
    struct plnstr *pp;
    struct lndstr *lp;
    struct nukstr *np;

    memset(nclink, 0, sizeof(nclink));
    for (i = EF_SHIP; i <= EF_NUKE; i++)
	unit_onresize(i);

    for (i = 0; (pp = getplanep(i)); i++) {
	if (!pp->pln_own) {
	    if (CANT_HAPPEN(pp->pln_ship >= 0 || pp->pln_land >= 0))
		pp->pln_ship = pp->pln_land = -1;
	    continue;
	}
	if (CANT_HAPPEN(pp->pln_ship >= 0 && pp->pln_land >= 0))
	    pp->pln_land = -1;
	pln_carrier_change(pp, EF_SHIP, -1, pp->pln_ship);
	pln_carrier_change(pp, EF_LAND, -1, pp->pln_land);
    }
    for (i = 0; (lp = getlandp(i)); i++) {
	if (!lp->lnd_own) {
	    if (CANT_HAPPEN(lp->lnd_ship >= 0 || lp->lnd_land >= 0))
		lp->lnd_ship = lp->lnd_land = -1;
	    continue;
	}
	if (CANT_HAPPEN(lp->lnd_ship >= 0 && lp->lnd_land >= 0))
	    lp->lnd_land = -1;
	lnd_carrier_change(lp, EF_SHIP, -1, lp->lnd_ship);
	lnd_carrier_change(lp, EF_LAND, -1, lp->lnd_land);
    }
    for (i = 0; (np = getnukep(i)); i++) {
	if (!np->nuk_own) {
	    if (CANT_HAPPEN(np->nuk_plane >= 0))
		np->nuk_plane = -1;
	    continue;
	}
	nuk_carrier_change(np, EF_PLANE, -1, np->nuk_plane);
    }
}

/*
 * Resize clink[@type] to match ef_nelem(@type).
 * Return 0 on success, -1 on error.
 * This is the struct empfile onresize callback for units.
 */
void
unit_onresize(int type)
{
    int n, i;
    struct clink *cl;

    if (CANT_HAPPEN(type < EF_SHIP || type > EF_NUKE))
	return;

    n = ef_nelem(type);
    cl = realloc(clink[type], n * sizeof(*clink[type]));
    if (!cl && n)
	exit_nomem();
    for (i = nclink[type]; i < n; i++)
	clink_init(&cl[i]);
    clink[type] = cl;
    nclink[type] = n;
    if (ef_flags(type) & EFF_MEM)
	clink_check(type);
}

/*
 * Find first unit on a carrier's cargo list for file type @cargo_type.
 * Search carrier @uid of type @type.
 * Return first unit's UID, or -1 if the carrier isn't carrying such
 * units.
 */
int
unit_cargo_first(int type, int uid, int cargo_type)
{
    int *headp;

    if (CANT_HAPPEN(type < EF_SHIP || type > EF_NUKE))
	return -1;
    if (CANT_HAPPEN(uid < 0 || uid >= nclink[type]))
	return -1;
    headp = clink_headp(&clink[type][uid], cargo_type);
    if (CANT_HAPPEN(!headp))
	return -1;
    return *headp;
}

/*
 * Find the next unit on a cargo list for file type @cargo_type.
 * Get the unit after @cargo_uid.
 * Return its UID, or -1 if there are no more on this list.
 */
int
unit_cargo_next(int cargo_type, int cargo_uid)
{
    if (CANT_HAPPEN(cargo_type < EF_SHIP || cargo_type > EF_NUKE))
	return -1;
    if (CANT_HAPPEN(cargo_uid < 0 || cargo_uid >= nclink[cargo_type]))
	return -1;
    return clink[cargo_type][cargo_uid].next;
}

/*
 * If @sp carries planes, return the UID of the first one, else -1.
 */
int
pln_first_on_ship(struct shpstr *sp)
{
    return unit_cargo_first(EF_SHIP, sp->shp_uid, EF_PLANE);
}

/*
 * If @lp carries planes, return the UID of the first one, else -1.
 */
int
pln_first_on_land(struct lndstr *lp)
{
    return unit_cargo_first(EF_LAND, lp->lnd_uid, EF_PLANE);
}

/*
 * Find the next plane on the same carrier as plane#@uid.
 * Return its UID, or -1 if there are no more.
 */
int
pln_next_on_unit(int uid)
{
    return unit_cargo_next(EF_PLANE, uid);
}

/*
 * If @sp carries land units, return the UID of the first one, else -1.
 */
int
lnd_first_on_ship(struct shpstr *sp)
{
    return unit_cargo_first(EF_SHIP, sp->shp_uid, EF_LAND);
}

/*
 * If @lp carries land units, return the UID of the first one, else -1.
 */
int
lnd_first_on_land(struct lndstr *lp)
{
    return unit_cargo_first(EF_LAND, lp->lnd_uid, EF_LAND);
}

/*
 * Find the next land unit on the same carrier as land#@uid.
 * Return its UID, or -1 if there are no more.
 */
int
lnd_next_on_unit(int uid)
{
    return unit_cargo_next(EF_LAND, uid);
}

/*
 * If @pp carries a nuke, return its UID, else -1.
 */
int
nuk_on_plane(struct plnstr *pp)
{
    return unit_cargo_first(EF_PLANE, pp->pln_uid, EF_NUKE);
}

/*
 * Return length of a carrier's cargo list for file type @cargo_type.
 * Use carrier @uid of type @type.
 */
int
unit_cargo_count(int type, int uid, int cargo_type)
{
    int n, cargo;

    n = 0;
    for (cargo = unit_cargo_first(type, uid, cargo_type);
	 cargo >= 0;
	 cargo = unit_cargo_next(cargo_type, cargo))
	n++;

    return n;
}

/*
 * Return number of land units loaded on @sp.
 */
int
shp_nland(struct shpstr *sp)
{
    return unit_cargo_count(EF_SHIP, sp->shp_uid, EF_LAND);
}

/*
 * Return number of land units loaded on @lp.
 */
int
lnd_nland(struct lndstr *lp)
{
    return unit_cargo_count(EF_LAND, lp->lnd_uid, EF_LAND);
}

int
unit_nplane(int type, int uid, int *nchopper, int *nxlight, int *nmsl)
{
    int n, nch, nxl, nms, cargo;
    struct plnstr *pp;
    struct plchrstr *pcp;

    n = nxl = nch = nms = 0;
    for (cargo = unit_cargo_first(type, uid, EF_PLANE);
	 (pp = getplanep(cargo));
	 cargo = pln_next_on_unit(cargo)) {
	pcp = &plchr[pp->pln_type];
	if (pcp->pl_flags & P_K)
	    nch++;
	else if (pcp->pl_flags & P_E)
	    nxl++;
	else if (pcp->pl_flags & P_M)
	    nms++;
	n++;
    }

    if (nchopper)
	*nchopper = nch;
    if (nxlight)
	*nxlight = nxl;
    if (nmsl)
	*nmsl = nms;
    return n;
}

int
shp_nplane(struct shpstr *sp, int *nchopper, int *nxlight, int *nmsl)
{
    return unit_nplane(EF_SHIP, sp->shp_uid, nchopper, nxlight, nmsl);
}

int
lnd_nxlight(struct lndstr *lp)
{
    int n;

    unit_nplane(EF_LAND, lp->lnd_uid, NULL, &n, NULL);
    return n;
}
