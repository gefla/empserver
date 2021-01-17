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
 *  bomb.c: Fly bombing missions
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2021
 */

#include <config.h>

#include <ctype.h>
#include "chance.h"
#include "commands.h"
#include "damage.h"
#include "item.h"
#include "land.h"
#include "news.h"
#include "nuke.h"
#include "optlist.h"
#include "path.h"
#include "plane.h"
#include "retreat.h"
#include "ship.h"

static void pin_bomb(struct emp_qelem *, struct sctstr *);
static void eff_bomb(struct emp_qelem *, struct sctstr *);
static void comm_bomb(struct emp_qelem *, struct sctstr *);
static void ship_bomb(struct emp_qelem *, struct sctstr *);
static void plane_bomb(struct emp_qelem *, struct sctstr *);
static void land_bomb(struct emp_qelem *, struct sctstr *);
static void strat_bomb(struct emp_qelem *, struct sctstr *);
static int changed_plane_aborts(struct plist *);
static int pinflak_planedamage(struct plnstr *, struct plchrstr *,
			       natid, int);

int
bomb(void)
{
    char *p;
    coord tx, ty;
    coord ax, ay;
    int ap_to_target;
    char flightpath[MAX_PATH_LEN];
    struct nstr_item ni_bomb;
    struct nstr_item ni_esc;
    struct sctstr target;
    struct emp_qelem bomb_list;
    struct emp_qelem esc_list;
    struct sctstr ap_sect;
    char mission;
    struct plist *plp;
    struct emp_qelem *qp, *next;
    char buf[1024];

    if (get_planes(&ni_bomb, &ni_esc, player->argp[1], player->argp[2]) < 0)
	return RET_SYN;
    p = getstarg(player->argp[3], "pinpoint, or strategic? ", buf);
    if (!p || !*p)
	return RET_SYN;
    mission = *p;
    if (!strchr("ps", mission))
	return RET_SYN;
    if (!get_assembly_point(player->argp[4], &ap_sect, buf))
	return RET_SYN;
    ax = ap_sect.sct_x;
    ay = ap_sect.sct_y;
    if (!getpath(flightpath, player->argp[5], ax, ay, 0, 0, MOB_FLY))
	return RET_SYN;
    tx = ax;
    ty = ay;
    (void)pathtoxy(flightpath, &tx, &ty, fcost);
    pr("target sector is %s\n", xyas(tx, ty, player->cnum));
    getsect(tx, ty, &target);
    ap_to_target = strlen(flightpath);
    pr("range to target is %d\n", ap_to_target);
    /*
     * select planes within range
     */
    pln_sel(&ni_bomb, &bomb_list, &ap_sect, ap_to_target, 2,
	    P_B | P_T, P_M | P_O);
    pln_sel(&ni_esc, &esc_list, &ap_sect, ap_to_target, 2,
	    P_ESC | P_F, P_M | P_O);
    /*
     * now arm and equip the bombers, transports, whatever.
     */
    pln_arm(&bomb_list, 2 * ap_to_target, mission, NULL);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    pln_arm(&esc_list, 2 * ap_to_target, 'e', NULL);
    ac_encounter(&bomb_list, &esc_list, ax, ay, flightpath, 0);
    if (QEMPTY(&bomb_list)) {
	pr("No planes got through fighter defenses\n");
    } else if (target.sct_type == SCT_SANCT) {
	pr("You can't bomb that sector!\n");
    } else {
	switch (mission) {
	case 'p':
	    pin_bomb(&bomb_list, &target);
	    for (qp = bomb_list.q_forw; qp != &bomb_list; qp = next) {
		next = qp->q_forw;
		plp = (struct plist *)qp;
		changed_plane_aborts(plp);
	    }
	    for (qp = esc_list.q_forw; qp != &esc_list; qp = next) {
		next = qp->q_forw;
		plp = (struct plist *)qp;
		changed_plane_aborts(plp);
	    }
	    break;
	case 's':
	    nreport(player->cnum, N_SCT_BOMB, target.sct_own, 1);
	    strat_bomb(&bomb_list, &target);
	    break;
	default:
	    CANT_REACH();
	}
    }
    pln_put(&bomb_list);
    pln_put(&esc_list);
    return RET_OK;
}

static void
pin_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct dchrstr *dcp;
    int nplanes;
    int nships;
    int type;
    int bad;
    char *p;
    int nsubs;
    int nunits;
    char buf[1024];
    int i;

    bad = 0;
    type = target->sct_type;
    dcp = &dchr[type];
    pr("Target sector is a %s constructed %s\n",
       effadv((int)target->sct_effic), dcp->d_name);
    nsubs = 0;
    nships = shipsatxy(target->sct_x, target->sct_y, 0, M_SUB, 0);
    if (pln_caps(list) & P_A) {
	nsubs = shipsatxy(target->sct_x, target->sct_y, M_SUB, 0, 1);
	if (nsubs > 0)
	    pr("Some subs are present in the sector.\n");
    }
    nplanes = planesatxy(target->sct_x, target->sct_y, 0, 0);
    nunits = unitsatxy(target->sct_x, target->sct_y, 0, L_SPY);
  retry:
    p = getstring("Bomb what? (ship, plane, land unit, efficiency, commodities) ",
		  buf);
    if (!p)
       return;
    if (!*p) {
	bad++;
	if (bad > 2)
	    return;
	goto retry;
    }
    switch (*p) {
    case 'l':
	if (nunits == 0) {
	    pr("no units there\n");
	    goto retry;
	}
	land_bomb(list, target);
	break;
    case 'p':
	if (nplanes == 0) {
	    pr("no planes there\n");
	    goto retry;
	}
	plane_bomb(list, target);
	break;
    case 's':
	if (nships == 0) {
	    if (pln_caps(list) & P_A) {
		if (nsubs == 0) {
		    pr("no ships there\n");
		    goto retry;
		}
	    } else {
		pr("no ships there\n");
		goto retry;
	    }
	}
	ship_bomb(list, target);
	break;
    case 'c':
	for (i = I_NONE + 1; i <= I_MAX; i++) {
	    if (target->sct_item[i])
		break;
	}
	if (i > I_MAX) {
	    pr("No bombable commodities in %s\n",
	       xyas(target->sct_x, target->sct_y, player->cnum));
	    goto retry;
	}
	comm_bomb(list, target);
	break;
    case 'e':
	eff_bomb(list, target);
	break;
    case 'q':
	pr("Aborting mission.\n");
	return;
    default:
	pr("Bad target type.\n");
	goto retry;
    }
}

static void
eff_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct plist *plp;
    struct emp_qelem *qp, *next;
    struct sctstr sect;
    int oldeff, dam = 0;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	if (changed_plane_aborts(plp))
	    continue;
	dam += pln_damage(&plp->plane, 'p', "");
    }
    getsect(target->sct_x, target->sct_y, &sect);
    target = &sect;
    oldeff = target->sct_effic;
    target->sct_effic = effdamage(target->sct_effic, dam);
    target->sct_avail = effdamage(target->sct_avail, dam);
    target->sct_road = effdamage(target->sct_road, dam);
    target->sct_rail = effdamage(target->sct_rail, dam);
    target->sct_defense = effdamage(target->sct_defense, dam);
    pr("did %d%% damage to efficiency in %s\n",
       oldeff - target->sct_effic,
       xyas(target->sct_x, target->sct_y, player->cnum));
    if (target->sct_own != player->cnum)
	mpr(target->sct_own,
	    "%s bombing raid did %d%% damage in %s\n",
	    cname(player->cnum), oldeff - target->sct_effic,
	    xyas(target->sct_x, target->sct_y, target->sct_own));
    bridge_damaged(target);
    putsect(&sect);
    collateral_damage(target->sct_x, target->sct_y, dam);
}

static void
comm_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct plist *plp;
    double b;
    int i;
    int amt, before;
    struct ichrstr *ip;
    struct emp_qelem *qp, *next;
    struct sctstr sect;
    int dam = 0;

    for (i = I_NONE + 1; i <= I_MAX; i++) {
	if (!target->sct_item[i])
	    continue;
	if (opt_SUPER_BARS && i == I_BAR)
	    continue;
	pr("some %s\n", ichr[i].i_name);
    }
    for (;;) {
	ip = whatitem(NULL, "commodity to bomb? ");
	if (player->aborted)
	    return;
	if (!ip)
	    continue;
	if (opt_SUPER_BARS && ip->i_uid == I_BAR) {
	    pr("You can't bomb %s!\n", ip->i_name);
	    continue;
	}
	break;
    }
    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	if (changed_plane_aborts(plp))
	    continue;
	dam += pln_damage(&plp->plane, 'p', "");
    }
    getsect(target->sct_x, target->sct_y, &sect);
    target = &sect;
    before = target->sct_item[ip->i_uid];
    target->sct_item[ip->i_uid] = amt = commdamage(before, dam, ip->i_uid);
    if (before > 0.0)
	b = 100.0 * (1.0 - (double)amt / (double)before);
    else
	b = 0.0;
    pr("did %.2f%% damage to %s in %s\n",
       b, ip->i_name, xyas(target->sct_x, target->sct_y, player->cnum));
    nreport(player->cnum, N_SCT_BOMB, target->sct_own, 1);
    if (target->sct_own != player->cnum)
	mpr(target->sct_own,
	    "%s precision bombing raid did %.2f%% damage to %s in %s\n",
	    cname(player->cnum), b, ip->i_name,
	    xyas(target->sct_x, target->sct_y, target->sct_own));
    putsect(&sect);
    collateral_damage(target->sct_x, target->sct_y, dam);
}

static void
ship_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct plist *plp;
    struct mchrstr *mcp;
    int dam;
    char *q;
    int n;
    struct emp_qelem *qp, *next;
    int shipno;
    struct shpstr ship;
    int nships = 0;
    struct shiplist *head = NULL;
    char buf[1024];
    char prompt[128];
    int hitchance;
    int flak;
    int gun;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	free_shiplist(&head);
	plp = (struct plist *)qp;
	if (changed_plane_aborts(plp))
	    continue;
	if (plp->pcp->pl_flags & P_A)
	    nships = asw_shipsatxy(target->sct_x, target->sct_y, 0, 0,
				   &plp->plane, &head);
	else
	    nships = shipsatxy(target->sct_x, target->sct_y, 0, M_SUB, 0);
	if (nships == 0) {
	    pr("%s could not find any ships!\n", prplane(&plp->plane));
	    continue;
	}
	(void)sprintf(prompt, "%s, %d bombs.  Target ('~' to skip)? ",
		      prplane(&plp->plane), plp->load);
	shipno = -1;
	while (shipno < 0) {
	    if (!(q = getstring(prompt, buf)))
		goto out;
	    if (*q == 0)
		continue;
	    if (*q == '~')
		break;
	    if (*q == '?') {
		if (plp->pcp->pl_flags & P_A)
		    print_shiplist(head);
		else
		    shipsatxy(target->sct_x, target->sct_y, 0, M_SUB, 0);
		continue;
	    }
	    n = atoi(q);
	    if (n < 0)
		continue;
	    if ((!(plp->pcp->pl_flags & P_A) || on_shiplist(n, head)) &&
		getship(n, &ship) && ship.shp_own &&
		ship.shp_x == target->sct_x && ship.shp_y == target->sct_y)
		shipno = n;
	    else
		pr("Ship #%d not spotted\n", n);
	}
	if (shipno < 0)
	    continue;
	if ((plp->pcp->pl_flags & P_A) && !on_shiplist(shipno, head))
	    continue;
	if (changed_plane_aborts(plp))
	    continue;

	gun = shp_usable_guns(&ship);
	mcp = &mchr[(int)ship.shp_type];
	if (gun > 0 && !(mcp->m_flags & M_SUB)) {
	    flak = (int)(techfact(ship.shp_tech, gun) * 2.0);
	    mpr(ship.shp_own, "Flak! Firing %d guns from ship %s\n",
		flak, prship(&ship));
	    if (pinflak_planedamage(&plp->plane, plp->pcp, ship.shp_own, flak))
		continue;
	}

	if (nuk_on_plane(&plp->plane) >= 0)
	    hitchance = 100;
	else {
	    hitchance = pln_hitchance(&plp->plane,
				      shp_hardtarget(&ship), EF_SHIP);
	    pr("%d%% hit chance...", hitchance);
	}
	if (pct_chance(hitchance)) {
	    /* pin-bombing is more accurate than normal bombing */
	    dam = 2 * pln_damage(&plp->plane, 'p', "");
	} else {
	    pr("splash\n");
	    /* Bombs that miss have to land somewhere! */
	    dam = pln_damage(&plp->plane, 'p', NULL);
	    collateral_damage(target->sct_x, target->sct_y, dam);
	    continue;
	}
	if (mcp->m_flags & M_SUB)
	    nreport(player->cnum, N_SUB_BOMB, ship.shp_own, 1);
	else
	    nreport(player->cnum, N_SHP_BOMB, ship.shp_own, 1);
	if (ship.shp_own != player->cnum) {
	    mpr(ship.shp_own, "%s bombs did %d damage to %s at %s\n",
		cname(player->cnum), dam,
		prship(&ship),
		xyas(target->sct_x, target->sct_y, ship.shp_own));
	}
	shipdamage(&ship, dam);
	if (ship.shp_effic < SHIP_MINEFF)
	    pr("%s at %s sunk!\n",
	       prship(&ship),
	       xyas(target->sct_x, target->sct_y, player->cnum));
	if (dam && (ship.shp_rflags & RET_INJURED))
	    retreat_ship(&ship, ship.shp_own);
	else if (ship.shp_rflags & RET_BOMBED)
	    retreat_ship(&ship, ship.shp_own);
	putship(ship.shp_uid, &ship);
	collateral_damage(target->sct_x, target->sct_y, dam / 2);
    }
out:
    free_shiplist(&head);
}

static void
plane_bomb(struct emp_qelem *list, struct sctstr *target)
{
    int dam;
    char *q;
    int n;
    natid own;
    struct plnstr plane;
    struct emp_qelem *qp, *next;
    int planeno;
    struct plist *plp;
    char prompt[128];
    char buf[1024];
    int hitchance;
    int nplanes;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	if (changed_plane_aborts(plp))
	    continue;
	nplanes = planesatxy(target->sct_x, target->sct_y, 0, 0);
	if (nplanes == 0) {
	    pr("%s could not find any planes!\n", prplane(&plp->plane));
	    continue;
	}
	(void)sprintf(prompt, "%s, %d bombs.  Target ('~' to skip)? ",
		      prplane(&plp->plane), plp->load);
	planeno = -1;
	while (planeno < 0) {
	    if (!(q = getstring(prompt, buf)))
		return;
	    if (*q == 0)
		continue;
	    if (*q == '~')
		break;
	    if (*q == '?') {
		planesatxy(target->sct_x, target->sct_y, 0, 0);
		continue;
	    }
	    n = atoi(q);
	    if (n < 0)
		continue;
	    if (getplane(n, &plane) && plane.pln_own &&
		plane.pln_x == target->sct_x &&
		plane.pln_y == target->sct_y &&
		plane.pln_ship < 0 && plane.pln_land < 0 &&
		!(plane.pln_flags & PLN_LAUNCHED))
		planeno = n;
	    else
		pr("Plane #%d not spotted\n", n);
	}
	if (planeno < 0)
	    continue;
	if (changed_plane_aborts(plp))
	    continue;
	if (nuk_on_plane(&plp->plane) >= 0)
	    hitchance = 100;
	else {
	    hitchance = pln_hitchance(&plp->plane, 0, EF_PLANE);
	    pr("%d%% hit chance...", hitchance);
	}
	if (pct_chance(hitchance)) {
	    /* pin-bombing is more accurate than normal bombing */
	    dam = 2 * pln_damage(&plp->plane, 'p', "");
	} else {
	    pr("thud\n");
	    /* Bombs that miss have to land somewhere! */
	    dam = pln_damage(&plp->plane, 'p', NULL);
	    collateral_damage(target->sct_x, target->sct_y, dam);
	    continue;
	}
	if (dam > 100)
	    dam = 100;
	own = plane.pln_own;
	if (dam > plane.pln_effic)
	    plane.pln_effic = 0;
	else
	    plane.pln_effic -= dam;
	plane.pln_mobil = damage(plane.pln_mobil, dam);
	mpr(own, "%s bombs did %d%% damage to %s at %s\n",
	       cname(player->cnum), dam, prplane(&plane),
	       xyas(target->sct_x, target->sct_y, own));
	nreport(player->cnum, N_DOWN_PLANE, own, 1);
	putplane(plane.pln_uid, &plane);
	collateral_damage(target->sct_x, target->sct_y, dam);
    }
}

static void
land_bomb(struct emp_qelem *list, struct sctstr *target)
{
    int dam;
    char *q;
    int n;
    natid own;
    char prompt[128];
    char buf[1024];
    struct lndstr land;
    struct emp_qelem *qp, *next;
    int unitno;
    int aaf, flak, hitchance;
    struct plist *plp;
    int nunits;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	if (changed_plane_aborts(plp))
	    continue;
	nunits = unitsatxy(target->sct_x, target->sct_y, 0, L_SPY);
	if (nunits == 0) {
	    pr("%s could not find any units!\n", prplane(&plp->plane));
	    continue;
	}
	(void)sprintf(prompt, "%s, %d bombs.  Target ('~' to skip)? ",
		      prplane(&plp->plane), plp->load);
	unitno = -1;
	while (unitno < 0) {
	    if (!(q = getstring(prompt, buf)))
		return;
	    if (*q == 0)
		continue;
	    if (*q == '~')
		break;
	    if (*q == '?') {
		unitsatxy(target->sct_x, target->sct_y, 0, L_SPY);
		continue;
	    }
	    n = atoi(q);
	    if (n < 0)
		continue;
	    if (getland(n, &land) && land.lnd_own &&
		land.lnd_ship < 0 && land.lnd_land < 0 &&
		!(lchr[land.lnd_type].l_flags & L_SPY) &&
		land.lnd_x == target->sct_x && land.lnd_y == target->sct_y)
		unitno = n;
	    else
		pr("Unit #%d not spotted\n", n);
	}
	if (unitno < 0)
	    continue;
	if (changed_plane_aborts(plp))
	    continue;

	aaf = lnd_aaf(&land);
	if (aaf) {
	    flak = roundavg(techfact(land.lnd_tech,
				     aaf * 3.0 * land.lnd_effic / 100.0));
	    mpr(land.lnd_own,
		"Flak! Firing flak guns from unit %s (AA rating %d)\n",
		prland(&land), aaf);
	    if (pinflak_planedamage(&plp->plane, plp->pcp, land.lnd_own, flak))
		continue;
	}

	if (nuk_on_plane(&plp->plane) >= 0)
	    hitchance = 100;
	else {
	    hitchance = pln_hitchance(&plp->plane,
				      lnd_hardtarget(&land), EF_LAND);
	    pr("%d%% hit chance...", hitchance);
	}
	if (pct_chance(hitchance)) {
	    dam = 2 * pln_damage(&plp->plane, 'p', "");
	} else {
	    pr("thud\n");
	    /* Bombs that miss have to land somewhere! */
	    dam = pln_damage(&plp->plane, 'p', NULL);
	    collateral_damage(target->sct_x, target->sct_y, dam);
	    continue;
	}
	if (dam > 100)
	    dam = 100;
	own = land.lnd_own;
	if (own != player->cnum)
	    mpr(own, "%s bombs did %d%% damage to %s at %s\n",
		cname(player->cnum), dam, prland(&land),
		xyas(target->sct_x, target->sct_y, own));
	landdamage(&land, dam);
	if (dam && (land.lnd_rflags & RET_INJURED))
	    retreat_land(&land, own);
	else if (land.lnd_rflags & RET_BOMBED)
	    retreat_land(&land, own);
	nreport(player->cnum, N_UNIT_BOMB, own, 1);
	putland(land.lnd_uid, &land);
	collateral_damage(target->sct_x, target->sct_y, dam);
    }
}

static void
strat_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct plist *plp;
    int dam = 0;
    struct emp_qelem *qp;
    struct sctstr sect;
    struct nukstr nuke;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if (getnuke(nuk_on_plane(&plp->plane), &nuke))
	    detonate(&nuke, target->sct_x, target->sct_y,
		     plp->plane.pln_flags & PLN_AIRBURST);
	else
	    dam += pln_damage(&plp->plane, 's', "");
    }
    if (dam <= 0)
	return;
    getsect(target->sct_x, target->sct_y, &sect);
    target = &sect;
    if (target->sct_own != player->cnum)
	mpr(target->sct_own, "%s bombing raid did %d damage in %s\n",
	    cname(player->cnum), PERCENT_DAMAGE(dam),
	    xyas(target->sct_x, target->sct_y, target->sct_own));

    sectdamage(target, dam);

    pr("did %d damage in %s\n", PERCENT_DAMAGE(dam),
       xyas(target->sct_x, target->sct_y, player->cnum));
    putsect(&sect);
}

static int
changed_plane_aborts(struct plist *plp)
{
    if (check_plane_ok(&plp->plane))
	return 0;
    getplane(plp->plane.pln_uid, &plp->plane);
    pln_put1(plp);
    return 1;
}

static int
pinflak_planedamage(struct plnstr *pp, struct plchrstr *pcp, natid from,
		    int flak)
{
    int disp;
    char dmess[14];
    int dam;

    dam = ac_flak_dam(flak, pln_def(pp), pcp->pl_flags);
    if (dam <= 0)
	return 0;
    disp = ac_damage_plane(pp, from, dam, 1, dmess);
    mpr(pp->pln_own, "    Flak! %s takes %d%s%s.\n",
	prplane(pp), dam, *dmess ? " --" : "", dmess);

    putplane(pp->pln_uid, pp);
    return disp > 0;
}
