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
 *  aircombat.c: Deal with air to air combat
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1992
 *     Steve McClure, 1996
 *     Markus Armbruster, 2006-2020
 */

#include <config.h>

#include "chance.h"
#include "land.h"
#include "map.h"
#include "misc.h"
#include "mission.h"
#include "nat.h"
#include "news.h"
#include "nsc.h"
#include "optlist.h"
#include "path.h"
#include "plague.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

#define FLAK_GUN_MAX 14

static void ac_intercept(struct emp_qelem *, struct emp_qelem *,
			 struct emp_qelem *, natid, coord, coord, int);
static void ac_combat_headers(natid, natid);
static void ac_airtoair(struct emp_qelem *, struct emp_qelem *);
static void ac_dog(struct plist *, struct plist *);
static void ac_putplane(struct plist *, int);
static void ac_doflak(struct emp_qelem *, struct sctstr *);
static void ac_landflak(struct emp_qelem *, coord, coord);
static void ac_shipflak(struct emp_qelem *, coord, coord);
static void ac_fireflak(struct emp_qelem *, natid, int);
static void getilists(struct emp_qelem *, enum relations[], natid);
static int do_evade(struct emp_qelem *, struct emp_qelem *);

void
ac_encounter(struct emp_qelem *bomb_list, struct emp_qelem *esc_list,
	     coord x, coord y, char *path, int mission_flags)
{
    int val;
    int dir;
    unsigned char gotships[MAXNOC];
    unsigned char gotlands[MAXNOC];
    enum relations rel[MAXNOC];
    int overfly[MAXNOC];
    int flags;
    struct emp_qelem ilist[MAXNOC];
    natid plane_owner;
    struct sctstr sect;
    struct shpstr ship;
    struct lndstr land;
    struct nstr_item ni;
    natid cn;
    struct plist *plp;
    int evaded;
    struct shiplist *head = NULL;
    int changed = 0;
/* We want to only intercept once per sector per owner.  So, if we overfly
   a sector, and then overfly some land units or ships, we don't want to
   potentially intercept 3 times. */

    plp = (struct plist *)bomb_list->q_forw;
    plane_owner = plp->plane.pln_own;

    memset(overfly, 0, sizeof(overfly));
    getilists(ilist, rel, plane_owner);

    if (CANT_HAPPEN(mission_flags && plane_owner != player->cnum))
	mission_flags = 0;

    if (mission_flags & PM_R) {
	flags = pln_caps(bomb_list);
	if (flags & P_S) {
	    pr("\nSPY Plane report\n");
	    prdate();
	    sathead();
	} else if (flags & P_A) {
	    pr("\nAnti-Sub Patrol report\n");
	} else {
	    pr("\nReconnaissance report\n");
	    prdate();
	}
    }

    for (;;) {
	getsect(x, y, &sect);
	memset(gotships, 0, sizeof(gotships));
	snxtitem_xy(&ni, EF_SHIP, x, y);
	while (nxtitem(&ni, &ship)) {
	    if (mchr[(int)ship.shp_type].m_flags & M_SUB)
		continue;
	    gotships[ship.shp_own] = 1;
	}
	memset(gotlands, 0, sizeof(gotlands));
	snxtitem_xy(&ni, EF_LAND, x, y);
	while (nxtitem(&ni, &land)) {
	    if (land.lnd_ship >= 0 || land.lnd_land >= 0)
		continue;
	    gotlands[land.lnd_own] = 1;
	}

	if (mission_flags & PM_R) {
	    flags = pln_caps(bomb_list);
	    if (opt_HIDDEN)
		setcont(player->cnum, sect.sct_own, FOUND_FLY);
	    if (sect.sct_type == SCT_WATER)
		pr("flying over %s at %s\n",
		   dchr[sect.sct_type].d_name, xyas(x, y, player->cnum));
	    else if (flags & P_S)
		satdisp_sect(&sect, flags & P_I ? 10 : 50);
	    else
		look_at_sect(&sect, 25);

	    if (flags & P_S)
		satdisp_units(sect.sct_x, sect.sct_y);
	    else {
		for (cn = 1; cn < MAXNOC; cn++) {
		    if (cn == player->cnum)
			continue;
		    if (gotships[cn])
			pr("Flying over %s ships in %s\n",
			   cname(cn), xyas(x, y, player->cnum));
		    if (gotlands[cn])
			pr("Flying over %s land units in %s\n",
			   cname(cn), xyas(x, y, player->cnum));
		}
	    }
	} else
	    mpr(plane_owner, "flying over %s at %s\n",
		dchr[sect.sct_type].d_name, xyas(x, y, plane_owner));
	changed += map_set(plane_owner, sect.sct_x, sect.sct_y,
			   dchr[sect.sct_type].d_mnem, 0);

	evaded = do_evade(bomb_list, esc_list);
	if (!evaded) {
	    overfly[sect.sct_own]++;
	    for (cn = 1; cn < MAXNOC; cn++) {
		if (rel[cn] == ALLIED)
		    continue;
		if (cn != sect.sct_own && !gotships[cn] && !gotlands[cn])
		    continue;
		mpr(cn, "%s planes spotted over %s\n",
		    cname(plane_owner), xyas(x, y, cn));
		if (opt_HIDDEN)
		    setcont(cn, plane_owner, FOUND_FLY);
	    }

	    /* Fire flak */
	    if (rel[sect.sct_own] <= HOSTILE)
		ac_doflak(bomb_list, &sect);
	    /* If bombers left, fire flak from units and ships */
	    if (!QEMPTY(bomb_list))
		ac_landflak(bomb_list, x, y);
	    if (!QEMPTY(bomb_list))
		ac_shipflak(bomb_list, x, y);
	    /* mission planes aborted due to flak -- don't send escorts */
	    if (QEMPTY(bomb_list))
		break;

	    for (cn = 1; cn < MAXNOC && !QEMPTY(bomb_list); cn++) {
		if (rel[cn] > HOSTILE)
		    continue;
		ac_intercept(bomb_list, esc_list, &ilist[cn], cn, x, y,
			     !(cn == sect.sct_own
			       || gotships[cn] || gotlands[cn]));
	    }
	}

	if (mission_flags & PM_R) {
	    flags = pln_caps(bomb_list);
	    if (sect.sct_type == SCT_WATER && mission_flags & PM_S)
		plane_sweep(bomb_list, x, y);
	    if (sect.sct_type == SCT_WATER && flags & P_A)
		plane_sona(bomb_list, x, y, &head);
	}

	dir = *path++;
	if (!dir || QEMPTY(bomb_list) || (val = diridx(dir)) == DIR_STOP)
	    break;
	x = xnorm(x + diroff[val][0]);
	y = ynorm(y + diroff[val][1]);
    }

    /* Let's report all of the overflights even if aborted */
    for (cn = 1; cn < MAXNOC; cn++) {
	if (overfly[cn] > 0 && rel[cn] != ALLIED)
	    nreport(plane_owner, N_OVFLY_SECT, cn, overfly[cn]);
    }
    /* If the map changed, update it */
    if (changed)
	writemap(plane_owner);

    free_shiplist(&head);
    for (cn = 1; cn < MAXNOC; cn++)
	pln_put(&ilist[cn]);
}

static void
sam_intercept(struct emp_qelem *att_list, struct emp_qelem *def_list,
	      natid def_own, natid plane_owner, coord x, coord y,
	      int only_mission)
{
    struct emp_qelem *aqp;
    struct emp_qelem *anext;
    struct emp_qelem *dqp;
    struct emp_qelem *dnext;
    struct plist *aplp;
    struct plist *dplp;
    struct plnstr *pp;
    int first = 1;

    for (aqp = att_list->q_forw,
	 dqp = def_list->q_forw;
	 aqp != att_list && dqp != def_list; aqp = anext) {
	anext = aqp->q_forw;
	aplp = (struct plist *)aqp;
	if (aplp->pcp->pl_cost < 1000)
	    continue;
	for (; dqp != def_list; dqp = dnext) {
	    dnext = dqp->q_forw;
	    dplp = (struct plist *)dqp;
	    pp = &dplp->plane;
	    if (!(dplp->pcp->pl_flags & P_M))
		continue;
	    if (only_mission && !pp->pln_mission)
		continue;
	    if (pp->pln_range < mapdist(x, y, pp->pln_x, pp->pln_y))
		continue;
	    if (pp->pln_mission
		&& pp->pln_radius < mapdist(x, y, pp->pln_opx, pp->pln_opy))
		continue;
	    if (CANT_HAPPEN(pp->pln_flags & PLN_LAUNCHED)
		|| mission_pln_equip(dplp, NULL, 0) < 0) {
		emp_remque(dqp);
		free(dqp);
		continue;
	    }
	    pp->pln_flags |= PLN_LAUNCHED;
	    putplane(pp->pln_uid, pp);
	    if (first) {
		first = 0;
		mpr(plane_owner, "%s launches SAMs!\n", cname(def_own));
		mpr(def_own, "Launching SAMs at %s planes over %s!\n",
		    cname(plane_owner), xyas(x, y, def_own));
		ac_combat_headers(plane_owner, def_own);
	    }
	    ac_dog(aplp, dplp);
	    dqp = dnext;
	    break;
	}
    }
    if (!first) {
	mpr(plane_owner, "\n");
	mpr(def_own, "\n");
    }
}

static void
ac_intercept(struct emp_qelem *bomb_list, struct emp_qelem *esc_list,
	     struct emp_qelem *def_list, natid def_own, coord x, coord y,
	     int only_mission)
{
    struct plnstr *pp;
    struct plist *plp;
    int icount;
    struct emp_qelem *next;
    struct emp_qelem *qp;
    struct emp_qelem int_list;
    int att_count;
    natid plane_owner;
    int dist;

    plp = (struct plist *)bomb_list->q_forw;
    plane_owner = plp->plane.pln_own;

    sam_intercept(bomb_list, def_list, def_own, plane_owner, x, y,
		  only_mission);
    sam_intercept(esc_list, def_list, def_own, plane_owner, x, y,
		  only_mission);

    att_count = emp_quelen(bomb_list) + emp_quelen(esc_list);
    if (!att_count)
	return;

    emp_initque(&int_list);
    icount = 0;
    for (qp = def_list->q_forw; qp != def_list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	pp = &plp->plane;
	/* SAMs interdict separately */
	if (plp->pcp->pl_flags & P_M)
	    continue;
	if (only_mission && !pp->pln_mission)
	    continue;
	dist = mapdist(x, y, pp->pln_x, pp->pln_y) * 2;
	if (pp->pln_range < dist)
	    continue;
	if (pp->pln_mission
	    && pp->pln_radius < mapdist(x, y, pp->pln_opx, pp->pln_opy))
	    continue;
	if (CANT_HAPPEN(pp->pln_flags & PLN_LAUNCHED)
	    || mission_pln_equip(plp, NULL, 0) < 0) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}
	/* got one; delete from def_list, add to int_list */
	emp_remque(qp);
	emp_insque(qp, &int_list);
	pp->pln_flags |= PLN_LAUNCHED;
	pp->pln_mobil -= pln_mobcost(dist, pp, 0);
	putplane(pp->pln_uid, pp);
	icount++;
	if (icount > att_count)
	    break;
    }
    if (icount == 0)
	return;
    mpr(plane_owner, "%d %s fighter%s rising to intercept!\n",
	icount, cname(def_own), icount == 1 ? " is" : "s are");
    mpr(def_own, "%d fighter%s intercepting %s planes over %s!\n",
	icount, icount == 1 ? " is" : "s are", cname(plane_owner),
	xyas(x, y, def_own));
    ac_combat_headers(plane_owner, def_own);
    ac_airtoair(esc_list, &int_list);
    ac_airtoair(bomb_list, &int_list);
    mpr(plane_owner, "\n");
    mpr(def_own, "\n");
    pln_put(&int_list);
}

static void
ac_combat_headers(natid plane_owner, natid def_own)
{
    mpr(plane_owner,
	" %-10.10s %-10.10s  strength int odds  damage           results\n",
	cname(plane_owner), cname(def_own));
    mpr(def_own,
	" %-10.10s %-10.10s  strength int odds  damage           results\n",
	cname(def_own), cname(plane_owner));
}

/*
 * air-to-air combat.
 */
static void
ac_airtoair(struct emp_qelem *att_list, struct emp_qelem *int_list)
{
    struct plist *attacker;
    struct plist *interceptor;
    struct emp_qelem *att;
    struct emp_qelem *in;
    int more_att;
    int more_int;
    struct emp_qelem *att_next;
    struct emp_qelem *in_next;

    att = att_list->q_forw;
    in = int_list->q_forw;
    more_att = 1;
    more_int = 1;
    if (QEMPTY(att_list) || QEMPTY(int_list)) {
	more_att = 0;
	more_int = 0;
    }
    while (more_att || more_int) {
	in_next = in->q_forw;
	att_next = att->q_forw;
	attacker = (struct plist *)att;
	interceptor = (struct plist *)in;
	ac_dog(attacker, interceptor);
	in = in_next;
	att = att_next;
	if (att == att_list) {
	    more_att = 0;
	    if (QEMPTY(att_list))
		more_int = 0;
	    else
		att = att->q_forw;
	}
	if (in == int_list) {
	    more_int = 0;
	    if (QEMPTY(int_list))
		more_att = 0;
	    else
		in = in->q_forw;
	}
    }
}

static void
ac_dog_report(natid to, int intensity, double odds,
	      struct plist *p1, int val1, int dam1, char *dam_mesg1,
	      struct plist *p2, int val2, int dam2, char *dam_mesg2)
{
    mpr(to, " %3.3s #%-4d  %3.3s #%-4d   %3d/%-3d %3d  %3.2f  %3d/%-3d"
	"%-13.13s %-13.13s\n",
	p1->pcp->pl_name, p1->plane.pln_uid,
	p2->pcp->pl_name, p2->plane.pln_uid,
	val1, val2, intensity, odds, dam1, dam2,
	dam_mesg1, dam_mesg2);
}

static void
ac_dog(struct plist *ap, struct plist *dp)
{
    int att, def;
    double odds;
    int intensity, i;
    natid att_own, def_own;
    int adam, ddam, adisp, ddisp;
    char adam_mesg[14], ddam_mesg[14];

    att_own = ap->plane.pln_own;
    def_own = dp->plane.pln_own;

    att = pln_att(&ap->plane);
    if (att == 0)
	att = pln_def(&ap->plane);
    att = att * ap->plane.pln_effic / 100;
    att = MAX(att, ap->pcp->pl_def / 2);

    def = pln_def(&dp->plane) * dp->plane.pln_effic / 100;
    def = MAX(def, dp->pcp->pl_def / 2);

    if ((ap->pcp->pl_flags & P_F) && ap->load != 0)
	att -= 2;
    if ((dp->pcp->pl_flags & P_F) && dp->load != 0)
	def -= 2;
    att += ap->pcp->pl_stealth / 25.0;
    def += dp->pcp->pl_stealth / 25.0;
    if (att < 1) {
	def += 1 - att;
	att = 1;
    }
    if (def < 1) {
	att += 1 - def;
	def = 1;
    }
    odds = ((double)att / ((double)def + (double)att));
    if (odds <= 0.05)
	odds = 0.05;
    intensity = roll(20) + roll(20) + roll(20) + roll(20) + 1;

    adam = 0;
    ddam = 0;
    for (i = 0; i < intensity; i++) {
	if (chance(odds)) {
	    ddam++;
	    if (dp->plane.pln_effic - ddam < PLANE_MINEFF)
		break;
	} else {
	    adam++;
	    if (ap->plane.pln_effic - adam < PLANE_MINEFF)
		break;
	}
    }

    if (dp->pcp->pl_flags & P_M)
	ddam = 100;

    adisp = ac_damage_plane(&ap->plane, def_own, adam, 0, adam_mesg);
    ddisp = ac_damage_plane(&dp->plane, att_own, ddam, 0, ddam_mesg);
    ac_dog_report(att_own, intensity, odds,
		  ap, att, adam, adam_mesg,
		  dp, def, ddam, ddam_mesg);
    ac_dog_report(def_own, intensity, odds,
		  dp, def, ddam, ddam_mesg,
		  ap, att, adam, adam_mesg);
    ac_putplane(ap, adisp);
    ac_putplane(dp, ddisp);

    if (opt_HIDDEN) {
	setcont(att_own, def_own, FOUND_FLY);
	setcont(def_own, att_own, FOUND_FLY);
    }
}

int
ac_damage_plane(struct plnstr *pp, natid from, int dam, int flak,
		char *mesg)
{
    int eff, disp;

    *mesg = 0;
    if (dam <= 0) {
	if (!flak)
	    snprintf(mesg, 14, " no damage");
	return 0;
    }

    eff = pp->pln_effic - dam;
    if (eff < 0)
	eff = 0;

    disp = 0;
    if (eff < PLANE_MINEFF) {
	snprintf(mesg, 14, " shot down");
	disp = 1;
    } else if (eff < 80 && chance((80 - eff) / 100.0)) {
	snprintf(mesg, 14, " aborted @%2d%%", eff);
	disp = 2;
    } else if (!flak)
	snprintf(mesg, 14, " cleared");

    pp->pln_effic = eff;
    pp->pln_mobil -= MIN(32 + pp->pln_mobil, dam / 2);

    if (disp == 1 && from != 0 && !(plchr[pp->pln_type].pl_flags & P_M))
	nreport(from, N_DOWN_PLANE, pp->pln_own, 1);
    return disp;
}

/*
 * NOTE: This routine may remove the appropriate plane element from the
 * queue if it gets destroyed.  That means that the caller must assume
 * that the current queue pointer is invalid on return from the
 * call.  (this has caused bugs in the past)
 */
static void
ac_putplane(struct plist *plp, int disp)
{
    if (disp)
	pln_put1(plp);
    else
	putplane(plp->plane.pln_uid, &plp->plane);
}

static void
ac_doflak(struct emp_qelem *list, struct sctstr *from)
{
    int gun;
    natid plane_owner;
    struct plist *plp;

    plp = (struct plist *)list->q_forw;
    plane_owner = plp->plane.pln_own;

    gun = MIN(FLAK_GUN_MAX, from->sct_item[I_GUN]);
    gun = roundavg(tfact(from->sct_own, 2.0 * gun));
    if (gun > 0) {
	mpr(plane_owner, "firing %d flak guns in %s...\n",
	    gun, xyas(from->sct_x, from->sct_y, plane_owner));
	mpr(from->sct_own, "firing %d flak guns in %s...\n",
	    gun, xyas(from->sct_x, from->sct_y, from->sct_own));
	ac_fireflak(list, from->sct_own, gun);
    }
}

static void
ac_shipflak(struct emp_qelem *list, coord x, coord y)
{
    struct nstr_item ni;
    struct shpstr ship;
    struct mchrstr *mcp;
    double flak, total, ngun;
    int gun;
    struct plist *plp;
    natid plane_owner;
    natid from;

    plp = (struct plist *)list->q_forw;
    plane_owner = plp->plane.pln_own;

    total = ngun = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (!QEMPTY(list) && nxtitem(&ni, &ship)) {
	if (ship.shp_own == 0 || ship.shp_own == plane_owner)
	    continue;
	mcp = &mchr[(int)ship.shp_type];
	if (mcp->m_flags & M_SUB)
	    continue;
	if (relations_with(ship.shp_own, plane_owner) > HOSTILE)
	    continue;
	gun = shp_usable_guns(&ship);
	if (gun == 0)
	    continue;
	flak = gun * (ship.shp_effic / 100.0);
	ngun += flak;
	total += techfact(ship.shp_tech, flak * 2.0);

	mpr(ship.shp_own, "firing %.0f flak guns from %s...\n",
	    flak, prship(&ship));
	from = ship.shp_own;
    }

    /* Limit to FLAK_GUN_MAX guns of average tech factor */
    if (ngun > FLAK_GUN_MAX)
	total *= FLAK_GUN_MAX / ngun;

    gun = roundavg(total);
    if (gun > 0) {
	mpr(plane_owner, "Flak!  Ships firing %d flak guns...\n", gun);
	ac_fireflak(list, from, gun);
    }
}

static void
ac_landflak(struct emp_qelem *list, coord x, coord y)
{
    struct nstr_item ni;
    struct lndstr land;
    struct lchrstr *lcp;
    double flak, total, ngun;
    int aaf, gun;
    struct plist *plp;
    natid plane_owner;
    natid from;

    plp = (struct plist *)list->q_forw;
    plane_owner = plp->plane.pln_own;

    total = ngun = 0;
    snxtitem_xy(&ni, EF_LAND, x, y);
    while (!QEMPTY(list) && nxtitem(&ni, &land)) {
	if (land.lnd_own == 0 || land.lnd_own == plane_owner)
	    continue;
	lcp = &lchr[(int)land.lnd_type];
	aaf = lnd_aaf(&land);
	if ((lcp->l_flags & L_FLAK) == 0 || aaf == 0)
	    continue;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    continue;
	if (relations_with(land.lnd_own, plane_owner) > HOSTILE)
	    continue;
	flak = aaf * 1.5 * land.lnd_effic / 100.0;
	ngun += flak;
	total += techfact(land.lnd_tech, flak * 2.0);

	mpr(land.lnd_own, "firing flak guns from unit %s (AA rating %d)\n",
	    prland(&land), aaf);
	from = land.lnd_own;
    }

    /* Limit to FLAK_GUN_MAX guns of average tech factor */
    if (ngun > FLAK_GUN_MAX)
	total *= FLAK_GUN_MAX / ngun;

    gun = roundavg(total);
    if (gun > 0) {
	mpr(plane_owner, "Flak!  Land units firing %d flak guns...\n", gun);
	ac_fireflak(list, from, gun);
    }
}

/*
 * Called from shipflak, landflak, and doflak.
 */
static void
ac_fireflak(struct emp_qelem *list, natid from, int guns)
{
    struct plist *plp;
    int n, disp;
    struct emp_qelem *qp;
    struct emp_qelem *next;
    char msg[14];

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	n = ac_flak_dam(guns, pln_def(&plp->plane), plp->pcp->pl_flags);
	disp = ac_damage_plane(&plp->plane, from, n, 1, msg);
	mpr(plp->plane.pln_own, "    %s takes %d%s%s.\n",
	    prplane(&plp->plane), n, *msg ? " --" : "", msg);
	ac_putplane(plp, disp);
    }
}

/*
 * Calculate flak damage
 */
int
ac_flak_dam(int guns, int def, int pl_flags)
{
    int flak, dam;
    float mult;
    /*				   <-7      -7     -6     -5     -4 */
    static float flaktable[18] = { 0.132f, 0.20f, 0.20f, 0.25f, 0.30f,
    /*    -3     -2     -1      0     +1     +2     +3     +4 */
	 0.35f, 0.40f, 0.45f, 0.50f, 0.50f, 0.55f, 0.60f, 0.65f,
    /*    +5    +6     +7     +8    >+8 */
	 0.70f,0.75f, 0.80f, 0.85f, 1.1305f };
    enum { FLAK_MAX = ARRAY_SIZE(flaktable) - 1 };

    flak = guns - def;
    if ((pl_flags & P_T) == 0)
	flak--;

    if (flak > 8)
	mult = flaktable[FLAK_MAX];
    else if (flak < -7)
	mult = flaktable[0];
    else {
	flak += 8;
	mult = flaktable[flak];
    }
    mult *= flakscale;
    dam = (int)((roll(8) + 2) * mult);
    if (dam > 100)
	dam = 100;
    return dam;
}

/*
 * Get planes available for interception duties.
 */
static void
getilists(struct emp_qelem *list, enum relations rel[], natid intruder)
{
    natid cn;
    struct plchrstr *pcp;
    struct plnstr plane;
    struct nstr_item ni;
    struct plist *ip;

    rel[0] = NEUTRAL;
    for (cn = 1; cn < MAXNOC; cn++) {
	rel[cn] = relations_with(cn, intruder);
	emp_initque(&list[cn]);
    }

    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &plane)) {
	if (rel[plane.pln_own] > HOSTILE)
	    continue;
	pcp = &plchr[(int)plane.pln_type];
	if ((pcp->pl_flags & P_F) == 0)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	if (plane.pln_mission && plane.pln_mission != MI_AIR_DEFENSE)
	    continue;
	if (plane.pln_mobil <= 0)
	    continue;
	if (plane.pln_effic < 40)
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, &plane))
		continue;
	}
	if (!pln_airbase_ok(&plane, 0, 0))
	    continue;
	/* got one! */
	ip = malloc(sizeof(*ip));
	ip->load = 0;
	ip->pstage = PLG_HEALTHY;
	ip->pcp = &plchr[(int)plane.pln_type];
	ip->plane = plane;
	emp_insque(&ip->queue, &list[plane.pln_own]);
    }
}

static int
do_evade(struct emp_qelem *bomb_list, struct emp_qelem *esc_list)
{
    struct emp_qelem *qp;
    double evade;
    struct plist *plp;

    evade = 100.0;
    for (qp = bomb_list->q_forw; qp != bomb_list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if (evade > plp->pcp->pl_stealth / 100.0)
	    evade = plp->pcp->pl_stealth / 100.0;
    }
    for (qp = esc_list->q_forw; qp != esc_list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if (evade > plp->pcp->pl_stealth / 100.0)
	    evade = plp->pcp->pl_stealth / 100.0;
    }

    if (chance(evade))
	return 1;

    return 0;
}
