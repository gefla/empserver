/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mission.c: Mission subroutines for planes/ships/units
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1996-2000
 *     Markus Armbruster, 2003-2009
 */

#include <config.h>

#include <stdlib.h>
#include "empobj.h"
#include "file.h"
#include "item.h"
#include "misc.h"
#include "mission.h"
#include "nsc.h"
#include "optlist.h"
#include "path.h"
#include "player.h"
#include "prototypes.h"
#include "queue.h"
#include "xy.h"

struct genlist {
    struct emp_qelem queue;	/* list of units */
    struct empobj *thing;	/* thing's struct */
};

struct airport {
    struct emp_qelem queue;
    coord x, y;
    natid own;
};

static void add_airport(struct emp_qelem *, coord, coord);
static int air_damage(struct emp_qelem *, coord, coord, int, natid,
		      char *, int);
static void build_mission_list(struct genlist[],
			       unsigned char[], unsigned char[],
			       coord, coord, int);
static void build_mission_list_type(struct genlist[], unsigned char[],
				    coord, coord, int, int);
static void divide(struct emp_qelem *, struct emp_qelem *, coord, coord);
static int dosupport(coord, coord, natid, natid, int);
static int find_airport(struct emp_qelem *, coord, coord);
static void mission_pln_arm(struct emp_qelem *, coord, coord, int,
			    int, struct ichrstr *);
static void mission_pln_sel(struct emp_qelem *, int, int, int);
static int perform_mission_land(int, struct lndstr *, coord, coord,
				natid, int, char *, int);
static int perform_mission_ship(int, struct shpstr *, coord, coord,
				natid, int, char *, int);
static int perform_mission_msl(int, struct emp_qelem *, coord, coord,
			       natid, int);
static int perform_mission_bomb(int, struct emp_qelem *, coord, coord,
				natid, int, char *, int, int);
static int perform_mission(coord, coord, natid, struct emp_qelem *,
			   int, char *, int);

static int
tally_dam(int dam, int newdam)
{
    return dam < 0 ? newdam : dam + newdam;
}

/*
 * Interdict commodities & transported planes
 */
int
ground_interdict(coord x, coord y, natid victim, char *s)
{
    int cn;
    int dam = 0, newdam;
    unsigned char act[MAXNOC];
    struct genlist mi[MAXNOC];

    memset(mi, 0, sizeof(mi));
    act[0] = 0;
    for (cn = 1; cn < MAXNOC; cn++) {
	act[cn] = relations_with(cn, victim) <= HOSTILE;
	emp_initque((struct emp_qelem *)&mi[cn]);
    }

    build_mission_list(mi, act, act, x, y, MI_INTERDICT);

    for (cn = 1; cn < MAXNOC; cn++) {
	if (QEMPTY(&mi[cn].queue))
	    continue;

	newdam = perform_mission(x, y, victim, &mi[cn].queue,
				 MI_INTERDICT, s, SECT_HARDTARGET);
	if (newdam > 0) {
	    dam += newdam;
	    mpr(victim, "%s interdiction mission does %d damage!\n",
		cname(cn), newdam);
	}
    }
    if (dam) {
	collateral_damage(x, y, dam);
    }
    return dam;
}

int
collateral_damage(coord x, coord y, int dam)
{
    int coll;
    struct sctstr sect;

    if (!dam)
	return 0;

    getsect(x, y, &sect);
    if (sect.sct_own) {
	coll = ldround((double)dam * collateral_dam, 1);
	if (coll == 0)
	    return 0;
	mpr(sect.sct_own, "%s takes %d%% collateral damage\n",
	    xyas(x, y, sect.sct_own), coll);
	sectdamage(&sect, coll);
	putsect(&sect);
	return coll;
    }
    return 0;
}

static int
only_subs(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct genlist *glp;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	glp = (struct genlist *)qp;

	if (glp->thing->ef_type != EF_SHIP)
	    return 0;
	if (!(mchr[glp->thing->type].m_flags & M_SUB))
	    return 0;
	/* It's a sub! */
    }
    /* They were all subs! */
    return 1;
}


/*
 *  Interdict ships & land units
 */
int
unit_interdict(coord x, coord y, natid victim, char *s, int hardtarget,
	       int mission)
{
    int cn, rel, newdam, osubs;
    int dam = -1;
    unsigned char plane_act[MAXNOC], other_act[MAXNOC];
    struct genlist mi[MAXNOC];

    memset(mi, 0, sizeof(mi));
    other_act[0] = plane_act[0] = 0;
    for (cn = 1; cn < MAXNOC; cn++) {
	rel = getrel(getnatp(cn), victim);
	other_act[cn] = rel <= HOSTILE;
	plane_act[cn] = mission == MI_SINTERDICT
	    ? cn != victim && rel <= NEUTRAL : other_act[cn];
	emp_initque((struct emp_qelem *)&mi[cn]);
    }

    build_mission_list(mi, other_act, plane_act, x, y, mission);

    for (cn = 1; cn < MAXNOC; cn++) {
	if (QEMPTY(&mi[cn].queue))
	    continue;

	osubs = only_subs(&mi[cn].queue);
	newdam = perform_mission(x, y, victim, &mi[cn].queue,
				 mission, s, hardtarget);
	dam = tally_dam(dam, newdam);
	if (newdam > 0)
	    mpr(victim, "%s interdiction mission does %d damage!\n",
		osubs ? "Enemy" : cname(cn), newdam);
    }
    if (dam > 0)
	collateral_damage(x, y, dam);
    return dam;
}

/*
 *  Perform a mission against victim, on behalf of actee
 */
int
off_support(coord x, coord y, natid victim, natid actee)
{
    return dosupport(x, y, victim, actee, MI_OSUPPORT);
}

/*
 *  Perform a mission against victim, on behalf of actee
 */
int
def_support(coord x, coord y, natid victim, natid actee)
{
    return dosupport(x, y, victim, actee, MI_DSUPPORT);
}

/*
 * Perform support missions in X,Y against VICTIM for ACTEE.
 * MISSION is either MI_OSUPPORT or MI_DSUPPORT.
 * Return total damage.
 */
static int
dosupport(coord x, coord y, natid victim, natid actee, int mission)
{
    int cn;
    unsigned char act[MAXNOC];
    struct genlist mi[MAXNOC];
    int newdam;
    int dam = 0;

    memset(mi, 0, sizeof(mi));
    act[0] = 0;
    for (cn = 1; cn < MAXNOC; cn++) {
	act[cn] = feels_like_helping(cn, actee, victim);
	emp_initque((struct emp_qelem *)&mi[cn]);
    }

    build_mission_list(mi, act, act, x, y, MI_SUPPORT);
    build_mission_list(mi, act, act, x, y, mission);

    for (cn = 1; cn < MAXNOC; cn++) {
	if (QEMPTY(&mi[cn].queue))
	    continue;

	newdam = perform_mission(x, y, victim, &mi[cn].queue, MI_SUPPORT,
				 "", SECT_HARDTARGET);
	if (newdam > 0)
	    dam += newdam;
    }
    return dam;
}

static void
build_mission_list(struct genlist mi[],
		   unsigned char other_act[], unsigned char plane_act[],
		   coord x, coord y, int mission)
{
    build_mission_list_type(mi, other_act, x, y, mission, EF_LAND);
    build_mission_list_type(mi, other_act, x, y, mission, EF_SHIP);
    build_mission_list_type(mi, plane_act, x, y, mission, EF_PLANE);
}

static void
build_mission_list_type(struct genlist mi[], unsigned char act[],
			coord x, coord y, int mission, int type)
{
    struct nstr_item ni;
    struct genlist *glp;
    struct empobj *gp;
    union empobj_storage item;

    snxtitem_all(&ni, type);
    while (nxtitem(&ni, &item)) {
	gp = (struct empobj *)&item;

	if (!act[gp->own])
	    continue;

	if (gp->mobil < 1)
	    continue;

	if ((gp->mission != mission) && (mission != MI_SINTERDICT))
	    continue;

	if ((gp->mission != mission) && (mission == MI_SINTERDICT) &&
	    (gp->mission != MI_INTERDICT))
	    continue;

	if (!in_oparea(gp, x, y))
	    continue;

	glp = malloc(sizeof(struct genlist));
	memset(glp, 0, sizeof(struct genlist));
	glp->thing = malloc(sizeof(item));
	memcpy(glp->thing, &item, sizeof(item));
	emp_insque(&glp->queue, &mi[gp->own].queue);
    }
}

static void
find_escorts(coord x, coord y, natid cn, struct emp_qelem *escorts)
{
    struct nstr_item ni;
    struct plist *plp;
    struct plnstr plane;

    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own != cn)
	    continue;
	if (plane.pln_mission != MI_ESCORT)
	    continue;
	if (!in_oparea((struct empobj *)&plane, x, y))
	    continue;
	plp = malloc(sizeof(struct plist));
	memset(plp, 0, sizeof(struct plist));
	plp->pcp = &plchr[(int)plane.pln_type];
	plp->plane = plane;
	emp_insque(&plp->queue, escorts);
    }
}

static int
perform_mission(coord x, coord y, natid victim, struct emp_qelem *list,
		int mission, char *s, int hardtarget)
{
    struct emp_qelem *qp, missiles, bombers;
    struct genlist *glp;
    struct plist *plp;
    struct plchrstr *pcp;
    int dam = -1;
    int targeting_ships = *s == 's'; /* "subs" or "ships" FIXME gross! */

    emp_initque(&missiles);
    emp_initque(&bombers);

    for (qp = list->q_forw; qp != list; ) {
	glp = (struct genlist *)qp;
	qp = qp->q_forw;

	if (glp->thing->ef_type == EF_LAND) {
	    dam = perform_mission_land(dam, (struct lndstr *)glp->thing,
				       x, y, victim, mission, s,
				       targeting_ships);
	} else if (glp->thing->ef_type == EF_SHIP) {
	    dam = perform_mission_ship(dam, (struct shpstr *)glp->thing,
				       x, y, victim, mission, s,
				       targeting_ships);
	} else if (glp->thing->ef_type == EF_PLANE) {
	    pcp = &plchr[glp->thing->type];
	    if ((pcp->pl_flags & P_M)
		&& (hardtarget != SECT_HARDTARGET
		    || (pcp->pl_flags & P_MAR)))
		;      /* units have their own missile interdiction */
	    else {
		/* save planes for later */
		plp = malloc(sizeof(struct plist));

		memset(plp, 0, sizeof(struct plist));
		plp->pcp = pcp;
		memcpy(&plp->plane, glp->thing, sizeof(struct plnstr));
		if (plp->pcp->pl_flags & P_M)
		    emp_insque(&plp->queue, &missiles);
		else
		    emp_insque(&plp->queue, &bombers);
	    }
	} else {
	    CANT_REACH();
	    break;
	}
	free(glp->thing);
	free(glp);
    }

    dam = perform_mission_msl(dam, &missiles, x, y, victim, hardtarget);
    dam = perform_mission_bomb(dam, &bombers, x, y, victim, mission, s,
			       hardtarget, targeting_ships);
    return dam;
}

static int
perform_mission_land(int dam, struct lndstr *lp, coord x, coord y,
		     natid victim, int mission, char *s,
		     int targeting_ships)
{
    int md, range, dam2;

    if (mission == MI_SINTERDICT)
	return dam;

    md = mapdist(x, y, lp->lnd_x, lp->lnd_y);

    if (mission == MI_INTERDICT && md > land_max_interdiction_range)
	return dam;

    range = roundrange(lnd_fire_range(lp));
    if (md > range)
	return dam;

    dam2 = lnd_fire(lp);
    putland(lp->lnd_uid, lp);
    if (dam2 < 0)
	return dam;

    if (targeting_ships) {
	if (chance(lnd_acc(lp) / 100.0))
	    dam2 = ldround(dam2 / 2.0, 1);
    }
    if (targeting_ships)
	nreport(lp->lnd_own, N_SHP_SHELL, victim, 1);
    else
	nreport(lp->lnd_own, N_SCT_SHELL, victim, 1);
    wu(0, lp->lnd_own,
       "%s fires at %s %s at %s\n",
       prland(lp), cname(victim), s, xyas(x, y, lp->lnd_own));

    mpr(victim, "%s %s fires at you at %s\n",
	cname(lp->lnd_own), prland(lp), xyas(x, y, victim));

    return tally_dam(dam, dam2);
}

static int
perform_mission_ship(int dam, struct shpstr *sp, coord x, coord y,
		     natid victim, int mission, char *s,
		     int targeting_ships)
{
    struct mchrstr *mcp = &mchr[sp->shp_type];
    double vrange, hitchance;
    int md, range, dam2;

    md = mapdist(x, y, sp->shp_x, sp->shp_y);

    if ((mission == MI_INTERDICT || mission == MI_SINTERDICT)
	&& md > ship_max_interdiction_range)
	return dam;

    if (mission == MI_SINTERDICT) {
	if (!(mcp->m_flags & M_SONAR))
	    return dam;
	if (!(mcp->m_flags & M_DCH) && !(mcp->m_flags & M_SUBT))
	    return dam;
	vrange = techfact(sp->shp_tech, mcp->m_vrnge);
	vrange *= sp->shp_effic / 200.0;
	if (md > vrange)
	    return dam;
	/* can't look all the time */
	if (chance(0.5))
	    return dam;
    }
    if (mcp->m_flags & M_SUB) {
	if (!targeting_ships)
	    return dam;		/* subs interdict only ships */
	range = roundrange(torprange(sp));
	if (md > range)
	    return dam;
	if (!line_of_sight(NULL, x, y, sp->shp_x, sp->shp_y))
	    return dam;
	dam2 = shp_torp(sp, 1);
	putship(sp->shp_uid, sp);
	if (dam2 < 0)
	    return dam;
	hitchance = shp_torp_hitchance(sp, md);

	wu(0, sp->shp_own,
	   "%s locking on %s %s in %s\n",
	   prship(sp), cname(victim), s, xyas(x, y, sp->shp_own));
	wu(0, sp->shp_own,
	   "\tEffective torpedo range is %d.0\n", range);
	wu(0, sp->shp_own,
	   "\tWhooosh... Hitchance = %d%%\n",
	   (int)(hitchance * 100));

	if (!chance(hitchance)) {
	    wu(0, sp->shp_own, "\tMissed\n");
	    mpr(victim,
		"Incoming torpedo sighted @ %s missed (whew)!\n",
		xyas(x, y, victim));
	    return tally_dam(dam, 0);
	}
	wu(0, sp->shp_own, "\tBOOM!...\n");
	nreport(victim, N_TORP_SHIP, 0, 1);
	wu(0, sp->shp_own,
	   "\tTorpedo hit %s %s for %d damage\n",
	   cname(victim), s, dam2);

	mpr(victim,
	    "Incoming torpedo sighted @ %s hits and does %d damage!\n",
	    xyas(x, y, victim), dam2);
    } else {
	range = roundrange(shp_fire_range(sp));
	if (md > range)
	    return dam;
	if (mission == MI_SINTERDICT)
	    dam2 = shp_dchrg(sp);
	else
	    dam2 = shp_fire(sp);
	putship(sp->shp_uid, sp);
	if (dam2 < 0)
	    return dam;
	if (targeting_ships)
	    nreport(sp->shp_own, N_SHP_SHELL, victim, 1);
	else
	    nreport(sp->shp_own, N_SCT_SHELL, victim, 1);
	wu(0, sp->shp_own,
	   "%s fires at %s %s at %s\n",
	   prship(sp), cname(victim), s, xyas(x, y, sp->shp_own));

	mpr(victim, "%s %s fires at you at %s\n",
	    cname(sp->shp_own), prship(sp), xyas(x, y, victim));
    }

    return tally_dam(dam, dam2);
}

static int
perform_mission_msl(int dam, struct emp_qelem *missiles, coord x, coord y,
		    natid victim, int hardtarget)
{
    int performed, air_dam, sublaunch, dam2;
    struct emp_qelem *qp, *newqp;
    struct plist *plp;

    /*
     * Missiles, except for interdiction of ships or land units,
     * because that happens elsewhere, in shp_missile_interdiction()
     * and lnd_missile_interdiction().
     */
    performed = air_dam = 0;
    for (qp = missiles->q_back; qp != missiles; qp = newqp) {
	newqp = qp->q_back;
	plp = (struct plist *)qp;

	if (air_dam < 100
	    && !CANT_HAPPEN(hardtarget != SECT_HARDTARGET
			    || (plp->pcp->pl_flags & P_MAR))
	    && mission_pln_equip(plp, NULL, 'p') >= 0) {
	    if (msl_launch(&plp->plane, EF_SECTOR, "sector", x, y, victim,
			   &sublaunch) < 0)
		goto use_up_msl;
	    performed = 1;
	    if (!msl_hit(&plp->plane, SECT_HARDTARGET, EF_SECTOR,
			 N_SCT_MISS, N_SCT_SMISS, sublaunch, victim))
		CANT_REACH();
	    dam2 = pln_damage(&plp->plane, 'p', 1);
	    air_dam += dam2;
	use_up_msl:
	    plp->plane.pln_effic = 0;
	    putplane(plp->plane.pln_uid, &plp->plane);
	}
	emp_remque(qp);
	free(qp);
    }

    return performed ? tally_dam(dam, air_dam) : dam;
}

static int
perform_mission_bomb(int dam, struct emp_qelem *bombers, coord x, coord y,
		     natid victim, int mission, char *s, int hardtarget,
		     int targeting_ships)
{
    struct emp_qelem *qp, *newqp, escorts, airp, b, e;
    struct plist *plp;
    int plane_owner, performed, air_dam, md;

    emp_initque(&escorts);
    emp_initque(&airp);

    if (QEMPTY(bombers))
	return dam;

    plp = (struct plist *)bombers->q_forw;
    plane_owner = plp->plane.pln_own;

    /*
     * If there are planes performing an
     * interdict or support mission, find
     * some escorts for them, if possible.
     * Up to 2 per bomber, if possible.
     */
    find_escorts(x, y, plane_owner, &escorts);

    if (mission == MI_SINTERDICT)
	mission_pln_sel(bombers, P_T | P_A, 0, hardtarget);
    else
	mission_pln_sel(bombers, P_T, P_A, SECT_HARDTARGET);

    mission_pln_sel(&escorts, P_ESC | P_F, 0, SECT_HARDTARGET);

    for (qp = bombers->q_forw; qp != bombers; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if (!find_airport(&airp, plp->plane.pln_x, plp->plane.pln_y))
	    add_airport(&airp, plp->plane.pln_x, plp->plane.pln_y);
    }

    performed = air_dam = 0;
    for (qp = airp.q_forw; qp != (&airp); qp = qp->q_forw) {
	struct airport *air;
	char buf[512];
	char *pp;

	air = (struct airport *)qp;
	md = mapdist(x, y, air->x, air->y);

	emp_initque(&b);
	emp_initque(&e);

	/* Split off the bombers at this base into b */
	divide(bombers, &b, air->x, air->y);

	/* Split off the escorts at this base into e */
	divide(&escorts, &e, air->x, air->y);

	mission_pln_arm(&b, air->x, air->y, 2 * md, 'p', NULL);

	if (QEMPTY(&b))
	    continue;

	mission_pln_arm(&e, air->x, air->y, 2 * md, 'e', NULL);

	pp = BestAirPath(buf, air->x, air->y, x, y);
	if (CANT_HAPPEN(!pp))
	    continue;
	performed = 1;
	wu(0, plane_owner, "Flying %s mission from %s to %s\n",
	   mission_name(mission),
	   xyas(air->x, air->y, plane_owner),
	   xyas(x, y, plane_owner));
	if (air->own && (air->own != plane_owner)) {
	    wu(0, air->own, "%s is flying %s mission from %s to %s\n",
	       cname(plane_owner), mission_name(mission),
	       xyas(air->x, air->y, air->own),
	       xyas(x, y, air->own));
	}

	ac_encounter(&b, &e, air->x, air->y, pp, 0);

	if (!QEMPTY(&b))
	    air_dam +=
		air_damage(&b, x, y, mission, victim, s, hardtarget);

	pln_put(&b);
	pln_put(&e);
    }

    if (air_dam > 0) {
	if (targeting_ships)
	    nreport(plane_owner, N_SHP_BOMB, victim, 1);
	else
	    nreport(plane_owner, N_SCT_BOMB, victim, 1);
    }

    qp = escorts.q_forw;
    while (qp != (&escorts)) {
	newqp = qp->q_forw;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }

    qp = bombers->q_forw;
    while (qp != bombers) {
	newqp = qp->q_forw;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }

    return performed ? tally_dam(dam, air_dam) : dam;
}

int
cando(int mission, int type)
{
    switch (mission) {
    case MI_ESCORT:
	if (type == EF_PLANE)
	    return 1;
	return 0;
    case MI_AIR_DEFENSE:
	if (type == EF_PLANE)
	    return 1;
	return 0;
    case MI_SINTERDICT:
	if ((type == EF_PLANE) || (type == EF_SHIP))
	    return 1;
	return 0;
    case MI_INTERDICT:
	return 1;
    case MI_SUPPORT:
    case MI_OSUPPORT:
    case MI_DSUPPORT:
	if (type == EF_PLANE)
	    return 1;
	return 0;
    case MI_RESERVE:
	if (type == EF_LAND)
	    return 1;
	return 0;
    }

    return 0;
}

char *
mission_name(int mission)
{
    switch (mission) {
    case MI_INTERDICT:
	return "an interdiction";
    case MI_SUPPORT:
	return "a support";
    case MI_OSUPPORT:
	return "an offensive support";
    case MI_DSUPPORT:
	return "a defensive support";
    case MI_RESERVE:
	return "a reserve";
    case MI_ESCORT:
	return "an escort";
    case MI_SINTERDICT:
	return "a sub interdiction";
    case MI_AIR_DEFENSE:
	return "an air defense";
    }
    CANT_REACH();
    return "a mysterious";
}

/*
 * Maximum distance GP can perform its mission.
 * Note: this has nothing to do with the radius of the op-area.
 * oprange() governs where the unit *can* strike, the op-area governs
 * where the player wants it to strike.
 */
int
oprange(struct empobj *gp)
{
    switch (gp->ef_type) {
    case EF_SHIP:
	return ldround(shp_fire_range((struct shpstr *)gp), 1);
    case EF_LAND:
	if (gp->mission == MI_RESERVE)
	    return lnd_reaction_range((struct lndstr *)gp);
	return ldround(lnd_fire_range((struct lndstr *)gp), 1);
    case EF_PLANE:
	/* missiles go one way, so we can use all the range */
	if (plchr[(int)gp->type].pl_flags & P_M)
	    return ((struct plnstr *)gp)->pln_range;
	return ((struct plnstr *)gp)->pln_range / 2;
    }
    CANT_REACH();
    return -1;
}

/*
 * Does GP's mission op area cover X,Y?
 */
int
in_oparea(struct empobj *gp, coord x, coord y)
{
    return mapdist(x, y, gp->opx, gp->opy) <= gp->radius
	&& mapdist(x, y, gp->x, gp->y) <= oprange(gp);
}

/*
 *  Remove all planes who cannot go on
 *  the mission from the plane list.
 */
static void
mission_pln_sel(struct emp_qelem *list, int wantflags, int nowantflags,
		int hardtarget)
{
    struct emp_qelem *qp, *next;
    struct plnstr *pp;
    struct plchrstr *pcp;
    struct plist *plp;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	pp = &plp->plane;
	pcp = plp->pcp;

	if (pp->pln_effic < 40) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}

	if (pp->pln_mobil < 1) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}

	if (opt_MARKET) {
	    if (ontradingblock(EF_PLANE, pp)) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
	}

	if (!pln_capable(pp, wantflags, nowantflags)) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}

	if (!pln_airbase_ok(pp, 0, 0)) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}

	if (pcp->pl_flags & P_A) {
	    if (roll(100) > pln_identchance(pp, hardtarget, EF_SHIP)) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
	}

	putplane(pp->pln_uid, pp);
    }
}

/*
 * Arm only the planes at x,y
 */
static void
mission_pln_arm(struct emp_qelem *list, coord x, coord y, int dist,
		int mission, struct ichrstr *ip)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct plist *plp;
    struct plnstr *pp;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;
	pp = &plp->plane;

	if (pp->pln_x != x)
	    continue;
	if (pp->pln_y != y)
	    continue;

	if (CANT_HAPPEN(pp->pln_flags & PLN_LAUNCHED)
	    || mission_pln_equip(plp, ip, mission) < 0) {
	    emp_remque(qp);
	    free(qp);
	    continue;
	}

	pp->pln_flags |= PLN_LAUNCHED;
	pp->pln_mobil -= pln_mobcost(dist, pp, mission);
	putplane(pp->pln_uid, pp);
    }
}

int
mission_pln_equip(struct plist *plp, struct ichrstr *ip, char mission)
{
    struct plchrstr *pcp;
    struct plnstr *pp;
    int load, needed;
    struct lndstr land;
    struct shpstr ship;
    struct sctstr sect;
    i_type itype;
    short *item;

    pp = &plp->plane;
    pcp = plp->pcp;
    if (pp->pln_ship >= 0) {
	getship(pp->pln_ship, &ship);
	item = ship.shp_item;
    } else if (pp->pln_land >= 0) {
	getland(pp->pln_land, &land);
	item = land.lnd_item;
    } else {
	getsect(pp->pln_x, pp->pln_y, &sect);
	item = sect.sct_item;
    }
    if (pcp->pl_fuel > item[I_PETROL]) {
	return -1;
    }
    item[I_PETROL] -= pcp->pl_fuel;
    load = pln_load(pp);
    itype = I_NONE;
    switch (mission) {
    case 'p':		/* pinpoint bomb */
	itype = I_SHELL;
	break;
    case 'i':		/* missile interception */
	if (load)
	    itype = I_SHELL;
	break;
    case 'e':		/* escort */
    case 0:		/* plane interception */
	load = 0;
	break;
    default:
	CANT_REACH();
	load = 0;
    }

    if (itype != I_NONE) {
	needed = load / ichr[itype].i_lbs;
	if (needed <= 0)
	    return -1;
	if (CANT_HAPPEN(nuk_on_plane(pp) >= 0))
	    return -1;
	if (itype == I_SHELL && item[itype] < needed) {
	    if (pp->pln_ship >= 0)
		shp_supply(&ship, I_SHELL, needed);
	    else if (pp->pln_land >= 0)
		lnd_supply(&land, I_SHELL, needed);
	    else
		sct_supply(&sect, I_SHELL, needed);
	}
	if (item[itype] < needed)
	    return -1;
	item[itype] -= needed;
	plp->load = needed;
    }

    if (pp->pln_ship >= 0)
	putship(ship.shp_uid, &ship);
    else if (pp->pln_land >= 0)
	putland(land.lnd_uid, &land);
    else
	putsect(&sect);
    return 0;
}

/*
 *  Return 1 if this x,y pair is in the list
 */
static int
find_airport(struct emp_qelem *airp, coord x, coord y)
{
    struct emp_qelem *qp;
    struct airport *a;

    for (qp = airp->q_forw; qp != airp; qp = qp->q_forw) {
	a = (struct airport *)qp;
	if ((a->x == x) && (a->y == y))
	    return 1;
    }

    return 0;
}

/* #*# This needs to be changed to include acc's -KHS */
static void
add_airport(struct emp_qelem *airp, coord x, coord y)
{
    struct airport *a;
    struct sctstr sect;

    a = malloc(sizeof(struct airport));

    a->x = x;
    a->y = y;
    getsect(x, y, &sect);
    a->own = sect.sct_own;

    emp_insque((struct emp_qelem *)a, airp);
}

/*
 *  Take all the planes in list 1 that
 *  are at x,y, and put them into list 2.
 */
static void
divide(struct emp_qelem *l1, struct emp_qelem *l2, coord x, coord y)
{
    struct emp_qelem *qp, *next;
    struct plist *plp;

    for (qp = l1->q_forw; qp != l1; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;

	if (plp->plane.pln_x != x)
	    continue;
	if (plp->plane.pln_y != y)
	    continue;

	emp_remque(qp);
	emp_insque(qp, l2);
    }
}

static int
air_damage(struct emp_qelem *bombers, coord x, coord y, int mission,
	   natid victim, char *s, int hardtarget)
{
    struct emp_qelem *qp;
    struct plist *plp;
    struct plnstr *pp;
    int newdam, dam = 0;
    int hitchance;

    for (qp = bombers->q_forw; qp != bombers; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	pp = &plp->plane;

	if ((mission == MI_SINTERDICT) && !(plp->pcp->pl_flags & P_A))
	    continue;

	if (!plp->load)
	    continue;

	if (plp->pcp->pl_flags & P_A) {
	    if (roll(100) > pln_identchance(pp, hardtarget, EF_SHIP)) {
		wu(0, pp->pln_own,
		   "\t%s detects sub movement in %s\n",
		   prplane(pp), xyas(x, y, pp->pln_own));
		continue;
	    }
	    if (relations_with(pp->pln_own, victim) > HOSTILE) {
		wu(0, pp->pln_own,
		   "\t%s tracks %s %s at %s\n",
		   prplane(pp), cname(victim), s, xyas(x, y, pp->pln_own));
		continue;
	    }
	    wu(0, pp->pln_own,
	       "\t%s depth-charging %s %s in %s\n",
	       prplane(pp), cname(victim), s, xyas(x, y, pp->pln_own));
	} else {
	    wu(0, pp->pln_own,
	       "\t%s pinbombing %s %s in %s\n",
	       prplane(pp), cname(victim), s, xyas(x, y, pp->pln_own));
	}
	hitchance = pln_hitchance(pp, hardtarget, EF_SHIP);
	if (nuk_on_plane(&plp->plane) >= 0)
	    hitchance = 100;
	else if (hardtarget != SECT_HARDTARGET)
	    wu(0, pp->pln_own, "\t\t%d%% hitchance...", hitchance);
	if (roll(100) <= hitchance) {
	    newdam = pln_damage(&plp->plane, 'p', 1);
	    wu(0, pp->pln_own,
	       "\t\thit %s %s for %d damage\n",
	       cname(victim), s, newdam);
	    dam += newdam;
	} else {
	    newdam = pln_damage(&plp->plane, 'p', 0);
	    wu(0, pp->pln_own, "missed\n");
	    if (mission == MI_SINTERDICT) {
		mpr(victim,
		    "RUMBLE... your sub in %s hears a depth-charge explode nearby\n",
		    xyas(x, y, victim));
	    } else if (*s == 's') {
		mpr(victim, "SPLASH!  Bombs miss your %s in %s\n",
		    s, xyas(x, y, victim));
	    } else {
		mpr(victim, "SPLAT!  Bombs miss your %s in %s\n",
		    s, xyas(x, y, victim));
	    }
	    /* Now, even though we missed, the bombs
	       land somewhere. */
	    collateral_damage(x, y, newdam);
	}

	/* use up missiles */
	if (plp->pcp->pl_flags & P_M)
	    pp->pln_effic = 0;
    }

    return dam;
}
