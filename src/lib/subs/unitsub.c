/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  unitsub.c: Common subroutines for multiple type of units
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 *     Markus Armbruster, 2009-2015
 */

#include <config.h>

#include <math.h>
#include "file.h"
#include "map.h"
#include "optlist.h"
#include "path.h"
#include "player.h"
#include "prototypes.h"
#include "unit.h"

char *
unit_nameof(struct empobj *gp)
{
    switch (gp->ef_type) {
    case EF_SHIP:
	return prship((struct shpstr *)gp);
    case EF_PLANE:
	return prplane((struct plnstr *)gp);
    case EF_LAND:
	return prland((struct lndstr *)gp);
    case EF_NUKE:
	return prnuke((struct nukstr *)gp);
    }
    CANT_REACH();
    return "The Beast #666";
}

static void
unit_list(struct emp_qelem *unit_list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *ulp;
    int type, npln, nch, nxl;
    struct empobj *unit;
    struct lndstr *lnd;
    struct shpstr *shp;

    if (CANT_HAPPEN(QEMPTY(unit_list)))
	return;
    qp = unit_list->q_back;
    ulp = (struct ulist *)qp;
    type = ulp->unit.gen.ef_type;
    if (CANT_HAPPEN(type != EF_LAND && type != EF_SHIP))
	return;

    if (type == EF_LAND)
	pr("lnd#     land type       x,y    a  eff mil  sh gun xl ln  mu tech retr\n");
    else
	pr("shp#     ship type       x,y   fl  eff mil  sh gun pn he xl ln mob tech\n");

    for (; qp != unit_list; qp = next) {
	next = qp->q_back;
	ulp = (struct ulist *)qp;
	lnd = &ulp->unit.land;
	shp = &ulp->unit.ship;
	unit = &ulp->unit.gen;
	if (CANT_HAPPEN(type != unit->ef_type))
	    continue;
	pr("%4d ", unit->uid);
	pr("%-16.16s ", empobj_chr_name(unit));
	prxy("%4d,%-4d ", unit->x, unit->y);
	pr("%1.1s", &unit->group);
	pr("%4d%%", unit->effic);
	if (type == EF_LAND) {
	    pr("%4d", lnd->lnd_item[I_MILIT]);
	    pr("%4d", lnd->lnd_item[I_SHELL]);
	    pr("%4d", lnd->lnd_item[I_GUN]);
	    pr("%3d%3d", lnd_nxlight(lnd), lnd_nland(lnd));
	} else {
	    pr("%4d", shp->shp_item[I_MILIT]);
	    pr("%4d", shp->shp_item[I_SHELL]);
	    pr("%4d", shp->shp_item[I_GUN]);
	    npln = shp_nplane(shp, &nch, &nxl, NULL);
	    pr("%3d%3d%3d", npln - nch - nxl, nch, nxl);
	    pr("%3d", shp_nland(shp));
	}
	pr("%4d", unit->mobil);
	pr("%4d", unit->tech);
	if (type == EF_LAND) {
	    pr("%4d%%", lnd->lnd_retreat);
	}
	pr("\n");
    }
}

static void
unit_view(struct emp_qelem *list)
{
    struct sctstr sect;
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *ulp;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	ulp = (struct ulist *)qp;
	if (CANT_HAPPEN(!(ef_flags(ulp->unit.gen.ef_type) & EFF_XY)))
	    continue;
	getsect(ulp->unit.gen.x, ulp->unit.gen.y, &sect);
	if (ulp->unit.gen.ef_type == EF_SHIP) {
	    if (mchr[ulp->unit.ship.shp_type].m_flags & M_FOOD)
		pr("[fert:%d] ", sect.sct_fertil);
	    if (mchr[ulp->unit.ship.shp_type].m_flags & M_OIL)
		pr("[oil:%d] ", sect.sct_oil);
	}
	pr("%s @ %s %d%% %s\n", unit_nameof(&ulp->unit.gen),
	   xyas(ulp->unit.gen.x, ulp->unit.gen.y, player->cnum),
	   sect.sct_effic, dchr[sect.sct_type].d_name);
    }
}

void
unit_rad_map_set(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct empobj *unit;

    for (qp = list->q_back; qp != list; qp = qp->q_back) {
	unit = &((struct ulist *)qp)->unit.gen;
	rad_map_set(unit->own, unit->x, unit->y, unit->effic, unit->tech,
		    unit->ef_type == EF_SHIP
		    ? mchr[unit->type].m_vrnge : lchr[unit->type].l_spy);
    }
}

static struct empobj *
get_leader(struct emp_qelem *list)
{
    return &((struct ulist *)(list->q_back))->unit.gen;
}

static void
switch_leader(struct emp_qelem *list, char *arg)
{
    int uid = arg ? atoi(arg) : -1;

    struct emp_qelem *qp, *save;
    struct ulist *ulp;

    if (QEMPTY(list))
	return;

    save = qp = list->q_back;
    do {
	emp_remque(qp);
	emp_insque(qp, list);
	qp = list->q_back;
	ulp = (struct ulist *)qp;
	if (ulp->unit.gen.uid == uid || uid == -1)
	    break;
    } while (list->q_back != save);
}

static char *
unit_move_parse(char *cp, char *arg1_default)
{
    int ac;

    ac = parse(cp, player->argbuf, player->argp, NULL, NULL, NULL);
    if (CANT_HAPPEN(ac <= 0)) {
	player->argp[0] = "";
	return "";
    }
    if (ac == 1) {
	player->argp[1] = arg1_default;
	return cp + 1;
    }
    return "";
}

static char *
unit_move_non_dir(struct emp_qelem *list, char *cp, int *map_shown)
{
    struct empobj *leader = get_leader(list);
    int bmap = 0, stopping;
    char leader_str[32];

    *map_shown = 0;
    sprintf(leader_str, "%d", leader->uid);

    switch (*cp) {
    case 'B':
	bmap = 'b';
	/* fall through */
    case 'M':
	cp = unit_move_parse(cp, leader_str);
	display_region_map(bmap, leader->ef_type, leader->x, leader->y,
			   player->argp[1], player->argp[2]);
	*map_shown = 1;
	break;
    case 'f':
	cp = unit_move_parse(cp, NULL);
	switch_leader(list, player->argp[1]);
	break;
    case 'i':
	cp++;
	unit_list(list);
	break;
    case 'm':
	cp++;
	if (leader->ef_type == EF_SHIP)
	    stopping = shp_sweep(list, 1, 1, player->cnum);
	else
	    stopping = lnd_sweep(list, 1, 1, player->cnum);
	if (stopping)
	    cp = "";
	break;
    case 'r':
	cp = unit_move_parse(cp, leader_str);
	radar(leader->ef_type);
	player->btused++;	/* FIXME use player_coms[].c_cost */
	*map_shown = 1;
	break;
    case 'l':
	cp = unit_move_parse(cp, leader_str);
	do_look(leader->ef_type);
	player->btused++;	/* FIXME likewise */
	break;
    case 's':
	if (leader->ef_type != EF_SHIP)
	    return NULL;
	cp = unit_move_parse(cp, leader_str);
	sona();
	player->btused++;	/* FIXME likewise */
	*map_shown = 1;
	break;
    case 'd':
	cp = unit_move_parse(cp, NULL);
	if (!player->argp[1]) {
	    player->argp[1] = leader_str;
	    player->argp[2] = "1";
	} else if (!player->argp[2]) {
	    player->argp[2] = player->argp[1];
	    player->argp[1] = leader_str;
	}
	if (leader->ef_type == EF_SHIP)
	    mine();
	else
	    landmine();
	player->btused++;	/* FIXME likewise */
	*map_shown = 1;
	break;
    case 'v':
	cp++;
	unit_view(list);
	break;
    default:
	return NULL;
    }

    return cp;
}

static char *
unit_move_getpath(struct emp_qelem *list, int suppress_map, char *path)
{
    struct empobj *leader = get_leader(list);
    double minmob, maxmob;
    struct emp_qelem *qp;
    struct ulist *ulp;
    char prompt[64];

    minmob = HUGE_VAL;
    maxmob = -HUGE_VAL;
    for (qp = list->q_back; qp != list; qp = qp->q_back) {
	ulp = (struct ulist *)qp;
	if (ulp->mobil < minmob)
	    minmob = ulp->mobil;
	if (ulp->mobil > maxmob)
	    maxmob = ulp->mobil;
    }
    if (!suppress_map)
	nav_map(leader->x, leader->y,
		leader->ef_type == EF_SHIP
		? !(mchr[leader->type].m_flags & M_SUB) : 1);
    snprintf(prompt, sizeof(prompt), "<%.1f:%.1f: %s> ",
	     maxmob, minmob,
	     xyas(leader->x, leader->y, player->cnum));
    return getstring(prompt, path);
}

static char *
unit_move_route(struct empobj *unit, char *buf, size_t bufsz)
{
    coord destx;
    coord desty;
    struct sctstr sect;
    size_t len;
    double c;
    int mtype;

    if (CANT_HAPPEN(unit->ef_type != EF_LAND && unit->ef_type != EF_SHIP))
	return NULL;

    if (!sarg_xy(buf, &destx, &desty))
	return buf;
    if (unit->ef_type == EF_SHIP) {
	c = path_find(unit->x, unit->y, destx, desty,
		      player->cnum, MOB_SAIL);
	if (c < 0 || unit->mobil <= 0) {
	    pr("Can't get to '%s' right now.\n",
	       xyas(destx, desty, player->cnum));
	    return NULL;
	}
    } else {
	getsect(unit->x, unit->y, &sect);
	mtype = lnd_mobtype((struct lndstr *)unit);
	/*
	 * Note: passing sect.sct_own for actor is funny, but works:
	 * its only effect is to confine the search to that nation's
	 * land.  It doesn't affect mobility costs.  The real actor is
	 * different for marching in allied land, and passing it would
	 * break path finding there.
	 */
	c = path_find(unit->x, unit->y, destx, desty, sect.sct_own, mtype);
	if (c < 0) {
	    pr("No owned %s from %s to %s!\n",
	       mtype == MOB_RAIL ? "railway" : "path",
	       xyas(unit->x, unit->y, player->cnum),
	       xyas(destx, desty, player->cnum));
	    return NULL;
	}
    }
    len = path_find_route(buf, bufsz, unit->x, unit->y, destx, desty);
    if (len == 0 || unit->ef_type == EF_LAND) {
	if (len + 1 < bufsz)
	    strcpy(buf + len, "h");
	len++;
    }
    if (len >= bufsz) {
	pr("Can't handle path to %s, it's too long, sorry\n",
	   xyas(destx, desty, player->cnum));
	return NULL;
    }
    pr("Using path '%s'\n", buf);
    return buf;
}

int
unit_move(struct emp_qelem *list)
{
    struct empobj *leader = get_leader(list);
    int leader_uid = leader->uid;
    int type = leader->ef_type;
    int moved, suppress_map, dir, stopping;
    char *cp;
    char path[1024];

    unit_rad_map_set(list);

    pr("%s is %s\n",
	type == EF_SHIP ? "Flagship" : "Leader",
	unit_nameof(leader));

    cp = "";
    if (player->argp[2]) {
	strcpy(path, player->argp[2]);
	cp = unit_move_route(leader, path, sizeof(path));
	if (!cp)
	    cp = "";
    }

    moved = suppress_map = 0;
    for (;;) {
	/*
	 * Invariants:
	 * - shp_may_nav() true for all ships
	 * - lnd_may_mar() true for all land units
	 * - leader is up-to-date
	 * Implies all are in the same sector
	 */
	if (!*cp) {
	    cp = unit_move_getpath(list, suppress_map, path);
	    if (!cp)
		return RET_FAIL;
	    cp = unit_move_route(leader, path, sizeof(path));
	    if (!cp || !*cp)
		cp = "h";
	    suppress_map = 0;
	} else if ((dir = chkdir(*cp, DIR_STOP, DIR_LAST)) >= 0) {
	    cp++;
	    if (type == EF_SHIP)
		stopping = shp_nav_dir(list, dir, player->cnum)
		    || shp_nav_gauntlet(list, 1, player->cnum);
	    else {
		if (!moved && !lnd_abandon_askyn(list))
		    return RET_FAIL;
		stopping = lnd_mar_dir(list, dir, player->cnum)
		    || lnd_mar_gauntlet(list, 1, player->cnum);
	    }
	    if (dir == DIR_STOP)
		return RET_OK;
	    moved = 1;
	    if (stopping)
		cp = "";
	} else {
	    cp = unit_move_non_dir(list, cp, &suppress_map);
	    if (!cp) {
		direrr("`%c' to stop", ", `%c' to view", NULL);
		pr(", `i' to list %s, `f' to change %s,\n",
		   type == EF_SHIP ? "ships" : "units",
		   type == EF_SHIP ? "flagship" : "leader");
		pr("`r' to radar, %s`l' to look, `M' to map, `B' to bmap,\n",
		   type == EF_SHIP ? "`s' to sonar, " : "");
		pr("`d' to drop mines, and `m' to minesweep\n");
		cp = "";
	    }
	}

	if (type == EF_SHIP)
	    shp_nav_stay_behind(list, player->cnum);
	else
	    lnd_mar_stay_behind(list, player->cnum);

	if (QEMPTY(list)) {
	    pr("No %s left\n", type == EF_SHIP ? "ships" : "lands");
	    return RET_OK;
	}

	leader = get_leader(list);
	if (leader->uid != leader_uid) {
	    leader_uid = leader->uid;
	    pr("Changing %s to %s\n",
	       leader->ef_type == EF_SHIP ? "flagship" : "leader",
	       unit_nameof(leader));
	}
	unit_rad_map_set(list);
    }
}

/*
 * Teleport UNIT to X,Y.
 * If UNIT's mission op-area is centered on it, keep it centered.
 */
void
unit_teleport(struct empobj *unit, coord x, coord y)
{
    if (unit->opx == unit->x && unit->opy == unit->y) {
	unit->opx = x;
	unit->opy = y;
    }
    unit->x = x;
    unit->y = y;
}

/*
 * Update cargo of CARRIER for movement or destruction.
 * If the carrier is destroyed, destroy its cargo (planes, land units,
 * nukes).
 * Else update their location to the carrier's.  Any op sectors equal
 * to location get updated, too.
 * Return number of units updated.
 */
int
unit_update_cargo(struct empobj *carrier)
{
    int cargo_type;
    struct nstr_item ni;
    union empobj_storage obj;
    int n = 0;

    for (cargo_type = EF_PLANE; cargo_type <= EF_NUKE; cargo_type++) {
	snxtitem_cargo(&ni, cargo_type, carrier->ef_type, carrier->uid);
	while (nxtitem(&ni, &obj)) {
	    if (carrier->own)
		unit_teleport(&obj.gen, carrier->x, carrier->y);
	    else {
		mpr(obj.gen.own, "%s lost!\n", unit_nameof(&obj.gen));
		obj.gen.effic = 0;
	    }
	    put_empobj(cargo_type, obj.gen.uid, &obj);
	    n++;
	}
    }
    return n;
}

/*
 * Drop cargo of UNIT.
 * Give it to NEWOWN, unless it's zero.
 */
void
unit_drop_cargo(struct empobj *unit, natid newown)
{
    int type;
    struct nstr_item ni;
    union empobj_storage cargo;

    for (type = EF_PLANE; type <= EF_NUKE; type++) {
	snxtitem_cargo(&ni, type, unit->ef_type, unit->uid);
	while (nxtitem(&ni, &cargo)) {
	    switch (type) {
	    case EF_PLANE:
		cargo.plane.pln_ship = cargo.plane.pln_land = -1;
		break;
	    case EF_LAND:
		cargo.land.lnd_ship = cargo.land.lnd_land = -1;
		break;
	    case EF_NUKE:
		cargo.nuke.nuk_plane = -1;
		break;
	    }
	    mpr(cargo.gen.own, "%s transferred off %s %d to %s\n",
		unit_nameof(&cargo.gen),
		ef_nameof(unit->ef_type), unit->uid,
		xyas(cargo.gen.x, cargo.gen.y, cargo.gen.own));
	    if (newown)
		unit_give_away(&cargo.gen, newown, cargo.gen.own);
	    put_empobj(type, cargo.gen.uid, &cargo.gen);
	}
    }
}

/*
 * Give UNIT and its cargo to RECIPIENT.
 * No action if RECIPIENT already owns UNIT.
 * If GIVER is non-zero, inform RECIPIENT and GIVER of the transaction.
 * Clears mission and group on the units given away.
 */
void
unit_give_away(struct empobj *unit, natid recipient, natid giver)
{
    int type;
    struct nstr_item ni;
    union empobj_storage cargo;

    if (unit->own == recipient)
	return;

    if (giver) {
	mpr(unit->own, "%s given to %s\n",
	    unit_nameof(unit), cname(recipient));
	mpr(recipient, "%s given to you by %s\n",
	    unit_nameof(unit), cname(giver));
    }

    unit->own = recipient;
    unit_wipe_orders(unit);
    put_empobj(unit->ef_type, unit->uid, unit);

    for (type = EF_PLANE; type <= EF_NUKE; type++) {
	snxtitem_cargo(&ni, type, unit->ef_type, unit->uid);
	while (nxtitem(&ni, &cargo))
	    unit_give_away(&cargo.gen, recipient, giver);
    }
}

/*
 * Wipe orders and such from UNIT.
 */
void
unit_wipe_orders(struct empobj *unit)
{
    struct shpstr *sp;
    struct plnstr *pp;
    struct lndstr *lp;

    unit->group = 0;
    unit->opx = unit->opy = 0;
    unit->mission = 0;
    unit->radius = 0;

    switch (unit->ef_type) {
    case EF_SHIP:
	sp = (struct shpstr *)unit;
	sp->shp_rflags = 0;
	sp->shp_rpath[0] = 0;
	break;
    case EF_PLANE:
	pp = (struct plnstr *)unit;
	pp->pln_range = pln_range_max(pp);
	break;
    case EF_LAND:
	lp = (struct lndstr *)unit;
	lp->lnd_retreat = morale_base;
	lp->lnd_rflags = 0;
	lp->lnd_rpath[0] = 0;
	break;
    case EF_NUKE:
	break;
    default:
	CANT_REACH();
    }
}
