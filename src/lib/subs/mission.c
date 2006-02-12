/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "nat.h"
#include "nsc.h"
#include "file.h"
#include "path.h"
#include "mission.h"
#include "genitem.h"
#include "news.h"
#include "item.h"
#include <fcntl.h>
#include "damage.h"
#include "prototypes.h"
#include "optlist.h"

struct airport {
    struct emp_qelem queue;
    coord x, y;
    natid own;
};

static void add_airport(struct emp_qelem *, coord, coord);
static int air_damage(struct emp_qelem *, coord, coord, int, natid,
		      s_char *, int);
static void build_mission_list(struct genlist *, coord, coord, int, natid);
static void build_mission_list_type(struct genlist *, coord, coord, int,
				    int, natid);
static void divide(struct emp_qelem *, struct emp_qelem *, coord, coord);
static int dosupport(struct genlist *, coord, coord, natid, natid);
static int find_airport(struct emp_qelem *, coord, coord);
static int mission_pln_arm(struct emp_qelem *, coord, coord, int,
			   int, struct ichrstr *, int, int, int *);
static void mission_pln_sel(struct emp_qelem *, int, int, int);
static int perform_mission(coord, coord, natid, struct emp_qelem *, int,
			   s_char *, int);

/*
 * Interdict commodities & transported planes
 */
int
ground_interdict(coord x, coord y, natid victim, s_char *s)
{
    int cn;
    int dam = 0, newdam, rel;
    struct genlist mi[MAXNOC];
    int z;

    memset(mi, 0, sizeof(mi));
    for (z = 1; z < MAXNOC; z++)
	emp_initque((struct emp_qelem *)&mi[z]);

    build_mission_list(mi, x, y, MI_INTERDICT, victim);

    for (cn = 1; cn < MAXNOC; cn++) {
	rel = getrel(getnatp(cn), victim);
	if (rel > HOSTILE)
	    continue;

	if (QEMPTY(&mi[cn].queue))
	    continue;

	newdam = perform_mission(x, y, victim, &mi[cn].queue,
				 MI_INTERDICT, s, SECT_HARDTARGET);
	dam += newdam;
	if (newdam)
	    mpr(victim, "%s interdiction mission does %d damage!\n",
		cname(cn), newdam);
    }
    if (dam) {
	collateral_damage(x, y, dam, 0);
    }
    return dam;
}

int
collateral_damage(coord x, coord y, int dam, struct emp_qelem *list)
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
	sectdamage(&sect, coll, list);
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
    struct mchrstr *mcp;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	glp = (struct genlist *)qp;

	if (glp->type != EF_SHIP)
	    return 0;
	mcp = (struct mchrstr *)glp->cp;
	if (!(mcp->m_flags & M_SUB))
	    return 0;
	/* It's a sub! */
    }
    /* They were all subs! */
    return 1;
}


/*
 *  Interdict ships & land units
 *
 */
int
unit_interdict(coord x, coord y, natid victim, s_char *s, int hardtarget,
	       int mission)
{
    int cn;
    int dam = 0, newdam;
    struct genlist mi[MAXNOC];
    int z;
    int osubs;

    memset(mi, 0, sizeof(mi));
    for (z = 1; z < MAXNOC; z++)
	emp_initque((struct emp_qelem *)&mi[z]);

    build_mission_list(mi, x, y, mission, victim);

    for (cn = 1; cn < MAXNOC; cn++) {
	if (cn == victim)
	    continue;
	if (mission == MI_SINTERDICT) {
	    if (getrel(getnatp(cn), victim) >= FRIENDLY)
		continue;
	} else if (getrel(getnatp(cn), victim) > HOSTILE)
	    continue;

	if (QEMPTY(&mi[cn].queue))
	    continue;

	osubs = only_subs(&mi[cn].queue);
	newdam = perform_mission(x, y, victim, &mi[cn].queue,
				 mission, s, hardtarget);
	dam += newdam;
	if (newdam) {
	    /* If only subs responded, then we don't know who's
	       subs they are */
	    if (osubs) {
		mpr(victim,
		    "Enemy interdiction mission does %d damage!\n",
		    newdam);
	    } else {
		mpr(victim,
		    "%s interdiction mission does %d damage!\n",
		    cname(cn), newdam);
	    }
	}
    }
    if (dam) {
	collateral_damage(x, y, dam, 0);
    }
    return dam;
}

/*
 *  Perform a mission against victim, on behalf of actee
 */
int
off_support(coord x, coord y, natid victim, natid actee)
{
    int dam = 0;
    struct genlist mi[MAXNOC];
    int z;

    memset(mi, 0, sizeof(mi));
    for (z = 1; z < MAXNOC; z++)
	emp_initque((struct emp_qelem *)&mi[z]);

    build_mission_list(mi, x, y, MI_SUPPORT, victim);
    build_mission_list(mi, x, y, MI_OSUPPORT, victim);

    dam = dosupport(mi, x, y, victim, actee);
    return dam;
}

/*
 *  Perform a mission against victim, on behalf of actee
 */
int
def_support(coord x, coord y, natid victim, natid actee)
{
    int dam = 0;
    struct genlist mi[MAXNOC];
    int z;

    memset(mi, 0, sizeof(mi));
    for (z = 1; z < MAXNOC; z++)
	emp_initque((struct emp_qelem *)&mi[z]);

    build_mission_list(mi, x, y, MI_SUPPORT, victim);
    build_mission_list(mi, x, y, MI_DSUPPORT, victim);

    dam = dosupport(mi, x, y, victim, actee);
    return dam;
}

static int
dosupport(struct genlist *mi, coord x, coord y, natid victim, natid actee)
{
    int cn;
    int rel;
    int dam = 0;

    for (cn = 1; cn < MAXNOC; cn++) {
	rel = getrel(getnatp(cn), actee);
	if ((cn != actee) && (rel != ALLIED))
	    continue;
	rel = getrel(getnatp(cn), victim);
	if ((cn != actee) && (rel != AT_WAR))
	    continue;

	if (QEMPTY(&mi[cn].queue))
	    continue;

	dam += perform_mission(x, y, victim, &mi[cn].queue, MI_SUPPORT,
			       "", SECT_HARDTARGET);
    }
    return dam;
}

static void
build_mission_list(struct genlist *mi, coord x, coord y, int mission,
		   natid victim)
{
    build_mission_list_type(mi, x, y, mission, EF_LAND, victim);
    build_mission_list_type(mi, x, y, mission, EF_SHIP, victim);
    build_mission_list_type(mi, x, y, mission, EF_PLANE, victim);
}

static void
build_mission_list_type(struct genlist *mi, coord x, coord y, int mission,
			int type, natid victim)
{
    struct nstr_item ni;
    struct genlist *glp;
    struct genitem *gp;
    union {
	struct shpstr u_sp;
	struct lndstr u_lp;
	struct plnstr u_pp;
    } u_block;
    s_char *block;
    int dist, size;
    int radius;
    int relat;
    struct sctstr sect;

/*
        size = MAX(sizeof(struct shpstr),sizeof(struct lndstr));
        size = MAX(size,sizeof(struct plnstr));
        block = malloc(size);
 */
    size = sizeof(u_block);
    block = (s_char *)&u_block;

    snxtitem_all(&ni, type);
    while (nxtitem(&ni, block)) {
	gp = (struct genitem *)block;

	if (gp->own == 0)
	    continue;

	if (gp->mobil < 1)
	    continue;

	if ((gp->mission != mission) && (mission != MI_SINTERDICT))
	    continue;

	if ((gp->mission != mission) && (mission == MI_SINTERDICT) &&
	    (gp->mission != MI_INTERDICT))
	    continue;

	relat = getrel(getnatp(gp->own), victim);
	if (mission == MI_SINTERDICT) {
	    if (relat >= FRIENDLY)
		continue;
	    else if (type != EF_PLANE && relat > HOSTILE)
		continue;
	} else if (relat > HOSTILE)
	    continue;

	dist = mapdist(x, y, gp->opx, gp->opy);

	radius = gp->radius;
	if (mission != MI_RESERVE)	/* XXX */
	    oprange(gp, type, &radius);

	if (dist > radius)
	    continue;

	/* Ok, it is within the operations range. */
	/* Now check from where the object actually is */
	dist = mapdist(x, y, gp->x, gp->y);
	radius = 999;
	oprange(gp, type, &radius);
	if (dist > radius)
	    continue;
	/* Ok, the object can get to where the x,y is */

	if (opt_SLOW_WAR) {
	    if (mission != MI_AIR_DEFENSE) {
		getsect(x, y, &sect);
		if (getrel(getnatp(gp->own), sect.sct_own) > AT_WAR) {

		    /*
		     * If the player->owner of the unit isn't at war
		     * with the victim, and doesn't own the
		     * sect being acted upon, and isn't the
		     * old player->owner of that sect, bounce them.
		     */
		    if (sect.sct_type != SCT_WATER &&
			sect.sct_own != gp->own &&
			sect.sct_oldown != gp->own)
			continue;
		}
	    }
	}

	glp = malloc(sizeof(struct genlist));
	memset(glp, 0, sizeof(struct genlist));
	glp->x = gp->x;
	glp->y = gp->y;
	glp->type = type;
	switch (type) {
	case EF_LAND:
	    glp->cp = (s_char *)&lchr[(int)gp->type];
	    break;
	case EF_SHIP:
	    glp->cp = (s_char *)&mchr[(int)gp->type];
	    break;
	case EF_PLANE:
	    glp->cp = (s_char *)&plchr[(int)gp->type];
	    break;
	}
	glp->thing = malloc(size);
	memcpy(glp->thing, block, size);
	emp_insque(&glp->queue, &mi[gp->own].queue);
    }
}

static void
find_escorts(coord x, coord y, natid cn, struct emp_qelem *escorts)
{
    struct nstr_item ni;
    struct plist *plp;
    struct plnstr plane;
    int dist;

    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own != cn)
	    continue;

	if (plane.pln_mission != MI_ESCORT)
	    continue;

	dist = mapdist(x, y, plane.pln_x, plane.pln_y);

	if (dist > ((int)((float)plane.pln_range / 2.0)))
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
		int mission, s_char *s, int hardtarget)
{
    struct emp_qelem *qp, missiles, bombers, escorts, airp, b, e;
    struct emp_qelem *newqp;
    struct genlist *glp;
    struct plist *plp;
    struct genitem *gp;
    struct lndstr *lp;
    struct shpstr *sp;
    struct sctstr sect;
    struct mchrstr *mcp;
    struct plchrstr *pcp;
    int dam = 0, dam2, mission_flags, tech;
    natid plane_owner = 0;
    int gun, shell, md, air_dam = 0;
    double range2, prb, range, mobcost, hitchance;

    getsect(x, y, &sect);

    emp_initque(&missiles);
    emp_initque(&bombers);
    emp_initque(&escorts);
    emp_initque(&airp);

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	glp = (struct genlist *)qp;
	gp = (struct genitem *)glp->thing;

	md = mapdist(x, y, gp->x, gp->y);

	if (glp->type == EF_LAND) {
	    lp = (struct lndstr *)glp->thing;

	    if (lp->lnd_effic < LAND_MINFIREEFF)
		continue;

	    if (mission == MI_SINTERDICT)
		continue;

	    if ((mission == MI_INTERDICT) &&
		(md > land_max_interdiction_range))
		continue;

	    if (md > (lp->lnd_frg / 2))
		continue;

	    if ((lp->lnd_ship != -1) || (lp->lnd_land != -1))
		continue;

	    if (lnd_getmil(lp) < 1)
		continue;

	    range = techfact((int)lp->lnd_tech, (double)lp->lnd_frg / 2.0);
	    range2 = (double)roundrange(range);

	    if (md > range2)
		continue;

	    shell = lp->lnd_item[I_SHELL];
	    gun = lp->lnd_item[I_GUN];
	    if (shell == 0 || gun == 0)
		continue;

	    if (has_supply(lp)) {
		use_supply(lp);
		putland(lp->lnd_uid, lp);
		dam2 = ldround(landunitgun(lp->lnd_effic, lp->lnd_dam, gun,
					   lp->lnd_ammo, shell), 1);
		if (sect.sct_type == SCT_WATER) {
		    double dam3 = (double)dam2;
		    if (chance(((double)lp->lnd_acc) / 100.0))
			dam2 = ldround((dam3 / 2.0), 1);
		}
		dam += dam2;
		if (sect.sct_type == SCT_WATER)
		    nreport(lp->lnd_own, N_SHP_SHELL, victim, 1);
		else
		    nreport(lp->lnd_own, N_SCT_SHELL, victim, 1);
		wu(0, lp->lnd_own,
		   "%s fires at %s %s at %s\n",
		   prland(lp), cname(victim), s, xyas(x, y, lp->lnd_own));

		mpr(victim, "%s %s fires at you at %s\n",
		    cname(lp->lnd_own), prland(lp), xyas(x, y, victim));
	    }
	} else if (glp->type == EF_SHIP) {
	    sp = (struct shpstr *)glp->thing;
	    mcp = (struct mchrstr *)glp->cp;

	    if (sp->shp_effic < 60)
		continue;
	    if (sp->shp_frnge == 0)
		continue;
	    if (((mission == MI_INTERDICT) ||
		 (mission == MI_SINTERDICT)) &&
		(md > ship_max_interdiction_range))
		continue;
	    if (sp->shp_item[I_MILIT] < 1)
		continue;
/*
  if ((mcp->m_flags & M_SUB) &&
  (sect.sct_type != SCT_WATER))
  continue;
*/
	    if (mission == MI_SINTERDICT) {
		if (!(mcp->m_flags & M_SONAR))
		    continue;
		if (!(mcp->m_flags & M_DCH) && !(mcp->m_flags & M_SUBT))
		    continue;
		range2 = techfact(sp->shp_tech, (double)mcp->m_vrnge);
		range2 *= (double)sp->shp_effic / 200.0;
		if (md > range2)
		    continue;
		/* can't look all the time */
		if (chance(0.5))
		    continue;
	    }
	    if (mcp->m_flags & M_SUB) {
/* If we aren't shooting at "subs" or "ships" don't fire at all from
   a sub. */
		if (*s != 's')
		    continue;
		if (sp->shp_mobil < 0)
		    continue;
		gun = sp->shp_item[I_GUN];
		if (gun < 1)
		    continue;
		shell = sp->shp_item[I_SHELL];
		if (shell < SHP_TORP_SHELLS)
		    shell += supply_commod(sp->shp_own,
					   sp->shp_x, sp->shp_y, I_SHELL,
					   SHP_TORP_SHELLS - shell);
		if (shell < SHP_TORP_SHELLS)
		    continue;

		range = sp->shp_effic * techfact(sp->shp_tech,
						 ((double)sp->shp_frnge)) /
		    100.0;

		range2 = (double)roundrange(range);
		if (md > range)
		    continue;

		if (!line_of_sight(NULL, x, y, gp->x, gp->y))
		    continue;
		sp->shp_item[I_SHELL] = shell - SHP_TORP_SHELLS;
		mobcost = sp->shp_effic * 0.01 * sp->shp_speed;
		mobcost = (480.0 / (mobcost +
				    techfact(sp->shp_tech, mobcost)));
		sp->shp_mobil -= mobcost;
		putship(sp->shp_uid, sp);
		hitchance = DTORP_HITCHANCE(md, sp->shp_visib);

		wu(0, sp->shp_own,
		   "%s locking on %s %s in %s\n",
		   prship(sp), cname(victim), s, xyas(x, y, sp->shp_own));
		wu(0, sp->shp_own,
		   "\tEffective torpedo range is %.1f\n", range);
		wu(0, sp->shp_own,
		   "\tWhooosh... Hitchance = %d%%\n",
		   (int)(hitchance * 100));

		if (hitchance < 1.0 && !chance(hitchance)) {
		    wu(0, sp->shp_own, "\tMissed\n");
		    mpr(victim,
			"Incoming torpedo sighted @ %s missed (whew)!\n",
			xyas(x, y, victim));
		    continue;
		}
		wu(0, sp->shp_own, "\tBOOM!...\n");
		dam2 = TORP_DAMAGE();

		dam += dam2;
		nreport(victim, N_TORP_SHIP, 0, 1);
		wu(0, sp->shp_own,
		   "\tTorpedo hit %s %s for %d damage\n",
		   cname(victim), s, dam2);

		mpr(victim,
		    "Incoming torpedo sighted @ %s hits and does %d damage!\n",
		    xyas(x, y, victim), dam2);
	    } else {
		range = techfact(sp->shp_tech, (double)mcp->m_frnge / 2.0);
		range2 = (double)roundrange(range);
		if (md > range2)
		    continue;
		gun = sp->shp_item[I_GUN];
		gun = MIN(gun, sp->shp_glim);
		shell = sp->shp_item[I_SHELL];
		if (shell < gun)
		    shell += supply_commod(sp->shp_own,
					   sp->shp_x, sp->shp_y, I_SHELL,
					   gun - shell);
		gun = MIN(gun, shell);
		gun = MIN(gun, sp->shp_item[I_MILIT] / 2.0);
		if (gun == 0)
		    continue;
		gun = MAX(gun, 1);
		dam2 = seagun(sp->shp_effic, gun);
		if (range2 == 0.0)
		    prb = 1.0;
		else
		    prb = ((double)md) / range2;
		prb *= prb;
		if (chance(prb))
		    dam2 = (int)((float)dam2 / 2.0);
		dam += dam2;
		if (sect.sct_type == SCT_WATER)
		    nreport(sp->shp_own, N_SHP_SHELL, victim, 1);
		else
		    nreport(sp->shp_own, N_SCT_SHELL, victim, 1);
		wu(0, sp->shp_own,
		   "%s fires at %s %s at %s\n",
		   prship(sp), cname(victim), s, xyas(x, y, sp->shp_own));

		mpr(victim, "%s %s fires at you at %s\n",
		    cname(sp->shp_own), prship(sp), xyas(x, y, victim));

		sp->shp_item[I_SHELL] = shell - gun;
		putship(sp->shp_uid, sp);
	    }
	} else if (glp->type == EF_PLANE) {
	    pcp = (struct plchrstr *)glp->cp;
	    if (pcp->pl_flags & P_M)
		/* units have their own missile interdiction */
		if (hardtarget != SECT_HARDTARGET || pcp->pl_flags & P_MAR)
		    continue;

	    /* save planes for later */
	    plp = malloc(sizeof(struct plist));

	    memset(plp, 0, sizeof(struct plist));
	    plp->pcp = pcp;
	    memcpy(&plp->plane, glp->thing, sizeof(struct plnstr));
	    if (plp->pcp->pl_flags & P_M)
		emp_insque(&plp->queue, &missiles);
	    else
		emp_insque(&plp->queue, &bombers);
	    plane_owner = plp->plane.pln_own;
	}
    }
    if (!QEMPTY(&missiles)) {
	/* I arbitrarily chose 100 mindam -KHS */
	dam +=
	    msl_launch_mindam(&missiles, x, y, hardtarget, EF_SECTOR, 100,
			      "sector", victim, mission);
	qp = missiles.q_forw;
	while (qp != (&missiles)) {
	    newqp = qp->q_forw;
	    emp_remque(qp);
	    free(qp);
	    qp = newqp;
	}
    }

    if (QEMPTY(&bombers)) {
	qp = list->q_forw;
	while (qp != list) {
	    glp = (struct genlist *)qp;
	    qp = qp->q_forw;

	    free(glp->thing);
	    free(glp);
	}
	return dam;
    }
    /*
     * If there are planes performing an
     * interdict or support mission, find
     * some escorts for them, if possible.
     * Up to 2 per bomber, if possible.
     */
    find_escorts(x, y, plane_owner, &escorts);

    if (mission == MI_SINTERDICT)
	mission_pln_sel(&bombers, P_T | P_A, 0, hardtarget);
    else
	mission_pln_sel(&bombers, P_T, P_A, SECT_HARDTARGET);

    mission_pln_sel(&escorts, P_ESC | P_F, 0, SECT_HARDTARGET);

    for (qp = bombers.q_forw; qp != (&bombers); qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if (!find_airport(&airp, plp->plane.pln_x, plp->plane.pln_y))
	    add_airport(&airp, plp->plane.pln_x, plp->plane.pln_y);
    }

    for (qp = airp.q_forw; qp != (&airp); qp = qp->q_forw) {
	struct airport *air;
	s_char pp[512];

	air = (struct airport *)qp;
	md = mapdist(x, y, air->x, air->y);

	emp_initque(&b);
	emp_initque(&e);

	/* Split off the bombers at this base into b */
	divide(&bombers, &b, air->x, air->y);

	/* Split off the escorts at this base into e */
	divide(&escorts, &e, air->x, air->y);

	tech = 0;
	mission_flags = 0;
	mission_flags |= P_X;	/* stealth (shhh) */
	mission_flags |= P_H;	/* gets turned off if not all choppers */

	mission_flags = mission_pln_arm(&b, air->x, air->y, 2 * md, 'p', 0,
					0, mission_flags, &tech);

	if (QEMPTY(&b)) {
	    continue;
	}

	mission_flags = mission_pln_arm(&e, air->x, air->y, 2 * md, 'p', 0,
					P_F | P_ESC, mission_flags, &tech);

	BestAirPath(pp, air->x, air->y, x, y);
	wu(0, plane_owner, "Flying %s mission from %s\n",
	   mission_name(mission), xyas(air->x, air->y, plane_owner));
	if (air->own && (air->own != plane_owner)) {
	    wu(0, air->own, "%s is flying %s mission from %s\n",
	       cname(plane_owner), mission_name(mission),
	       xyas(air->x, air->y, air->own));
	}

	ac_encounter(&b, &e, air->x, air->y, pp, mission_flags, 0, 0, 0);

	if (!QEMPTY(&b))
	    air_dam +=
		air_damage(&b, x, y, mission, victim, s, hardtarget);

	pln_put(&b);
	pln_put(&e);
    }

    if (air_dam > 0) {
	dam += air_dam;
	if (sect.sct_type == SCT_WATER)
	    nreport(plane_owner, N_SHP_BOMB, victim, 1);
	else
	    nreport(plane_owner, N_SCT_BOMB, victim, 1);
    }

    /* free up all this memory */
    qp = list->q_forw;
    while (qp != list) {
	glp = (struct genlist *)qp;
	qp = qp->q_forw;

	free(glp->thing);
	free(glp);
    }

    qp = escorts.q_forw;
    while (qp != (&escorts)) {
	newqp = qp->q_forw;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }

    qp = bombers.q_forw;
    while (qp != (&bombers)) {
	newqp = qp->q_forw;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }

    return dam;
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

s_char *
nameofitem(struct genitem *gp, int type)
{
    switch (type) {
    case EF_SHIP:
	return prship((struct shpstr *)gp);
    case EF_PLANE:
	return prplane((struct plnstr *)gp);
    case EF_LAND:
	return prland((struct lndstr *)gp);
    }
    return NULL;
}

s_char *
mission_name(short int mission)
{
    switch (mission) {
    case MI_INTERDICT:
	return "an interdiction";
    case MI_SUPPORT:
	return "a support";
    case MI_OSUPPORT:
	return "a offensive support";
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
    return "a mysterious";
}

void
show_mission(int type, struct nstr_item *np)
{
    size_t size;
    int first = 1, radius;
    s_char *block;
    struct genitem *gp;

    size = MAX(sizeof(struct lndstr), sizeof(struct plnstr));
    size = MAX(size, sizeof(struct shpstr));
    block = malloc(size);

    while (nxtitem(np, block)) {
	gp = (struct genitem *)block;
	if (!player->owner || gp->own == 0)
	    continue;

	if (first) {
	    pr("Thing                         x,y   op-sect rad mission\n");
	    first = 0;
	}
	pr("%-25s", nameofitem(gp, type));
	prxy(" %3d,%-3d", gp->x, gp->y, player->cnum);
	if (gp->mission == MI_INTERDICT || gp->mission == MI_SUPPORT ||
	    gp->mission == MI_OSUPPORT ||
	    gp->mission == MI_DSUPPORT || gp->mission == MI_AIR_DEFENSE) {
	    radius = 999;
	    oprange(gp, type, &radius);
	    prxy(" %3d,%-3d", gp->opx, gp->opy, player->cnum);
	    if (radius < gp->radius)
		pr("  %4d", radius);
	    else
		pr("  %4d", gp->radius);
	} else if (gp->mission == MI_RESERVE) {
	    struct sctstr sect;
	    int plus = 2;

	    getsect(gp->x, gp->y, &sect);
	    if ((sect.sct_type == SCT_HEADQ) && (sect.sct_effic >= 60))
		plus++;

	    if (((struct lndstr *)block)->lnd_rad_max == 0)
		plus = 0;
	    else
		plus += ((struct lndstr *)block)->lnd_rad_max;
	    prxy(" %3d,%-3d", gp->x, gp->y, player->cnum);
	    pr("  %4d", plus);
	} else if (gp->mission == MI_ESCORT) {
	    pr("        ");
	    pr("  %4d", (int)
	       ((float)((struct plnstr *)block)->pln_range / 2.0)
		);
	} else
	    pr("              ");
	if (gp->mission)
	    pr(" is on %s mission\n", mission_name(gp->mission));
	else
	    pr(" has no mission.\n");
    }
}

int
oprange(struct genitem *gp, int type, int *radius)
{
    int range;
    struct shpstr ship;
    struct lndstr land;
    struct plnstr plane;

    switch (type) {
    case EF_SHIP:
	getship(gp->uid, &ship);
	range = ldround(techfact(gp->tech,
				 (double)ship.shp_frnge / 2.0), 1);
	break;
    case EF_LAND:
	getland(gp->uid, &land);
	range = ldround(techfact((int)land.lnd_tech,
				 (double)land.lnd_frg / 2.0), 1);
	break;
    case EF_PLANE:
	getplane(gp->uid, &plane);
	/* missiles go one way, so we can use all the range */
	if (plchr[(int)plane.pln_type].pl_flags & P_M)
	    range = plane.pln_range;
	else
	    range = ldround((double)plane.pln_range / 2.0, 1);;
	break;
    }

    if ((*radius) > range)
	*radius = range;

    return range;
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
    struct shpstr ship;
    struct lndstr land;
    struct sctstr sect;
    struct plchrstr *pcp;
    struct plist *plp;
    int y, bad, bad1;
    unsigned int x;

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
	    if (ontradingblock(EF_PLANE, (int *)pp)) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
	}

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
	    if (bad) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
	    if (bad1 == 2) {
		if ((pcp->pl_flags & P_ESC) || (pcp->pl_flags & P_F))
		    bad1 = 0;
	    }
	    if (bad1 == 1) {
		if ((pcp->pl_flags & P_E) ||
		    (pcp->pl_flags & P_K) || (pcp->pl_flags & P_L))
		    bad1 = 0;
	    }
	    if (bad1) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
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
	    if (bad) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
	}
	if (pp->pln_ship >= 0) {
	    if (!getship(pp->pln_ship, &ship)) {
	      shipsunk:
		pp->pln_effic = 0;
		putplane(pp->pln_uid, pp);
		emp_remque(qp);
		free(qp);
		continue;
	    }
	    if (!can_be_on_ship(pp->pln_uid, ship.shp_uid)) {
		goto shipsunk;
	    }
	    if (ship.shp_effic < SHIP_MINEFF) {
		goto shipsunk;
	    }
	    /* Can't fly off of inefficient or non-owned, non-allied ships */
	    if ((ship.shp_effic < SHP_AIROPS_EFF) ||
		((ship.shp_own != pp->pln_own) &&
		 (getrel(getnatp(ship.shp_own), pp->pln_own) != ALLIED))) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
	}
	if (pp->pln_land >= 0) {
	    if (!getland(pp->pln_land, &land)) {
	      landdead:
		pp->pln_effic = 0;
		putplane(pp->pln_uid, pp);
		emp_remque(qp);
		free(qp);
		continue;
	    }
	    if (!(pcp->pl_flags & P_E))
		goto landdead;
	    if (land.lnd_effic < LAND_MINEFF)
		goto landdead;

	    /* Can't fly off of inefficient or non-owned, non-allied units */
	    if ((land.lnd_effic < LND_AIROPS_EFF) ||
		((land.lnd_own != pp->pln_own) &&
		 (getrel(getnatp(land.lnd_own), pp->pln_own) != ALLIED))) {
		emp_remque(qp);
		free(qp);
		continue;
	    }

	    /* Can't fly off units in ships or other units */
	    if ((land.lnd_ship >= 0) || (land.lnd_land >= 0)) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
	}
	/* Now, check the sector status if not on a plane or unit */
	if ((pp->pln_ship < 0) && (pp->pln_land < 0)) {
	    /* If we can't get the sector, we can't check it, and can't fly */
	    if (!getsect(pp->pln_x, pp->pln_y, &sect)) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
	    /* First, check allied status */
	    /* Can't fly from non-owned sectors or non-allied sectors */
	    if ((sect.sct_own != pp->pln_own) &&
		(getrel(getnatp(sect.sct_own), pp->pln_own) != ALLIED)) {
		emp_remque(qp);
		free(qp);
		continue;
	    }
	    /* non-vtol plane */
	    if ((pcp->pl_flags & P_V) == 0) {
		if ((sect.sct_type != SCT_AIRPT) || (sect.sct_effic < 40)) {
		    emp_remque(qp);
		    free(qp);
		    continue;
		}
	    }
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
 *
 */
static int
mission_pln_arm(struct emp_qelem *list, coord x, coord y, int dist,
		int mission, struct ichrstr *ip, int flags,
		int mission_flags, int *tech)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct plist *plp;

    if (*tech == 0)
	*tech = 9999;
    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	plp = (struct plist *)qp;

	if (plp->plane.pln_x != x)
	    continue;
	if (plp->plane.pln_y != y)
	    continue;

	if (mission_pln_equip(plp, ip, flags, mission) < 0) {
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
	if (*tech > plp->plane.pln_tech)
	    *tech = plp->plane.pln_tech;
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

	/*
	 *      Mob costs for missions are 1/2 normal
	 *       Not anymore. :)
	 */
/*	plp->plane.pln_mobil -= pln_mobcost(dist,&plp->plane,flags)/2;*/
	plp->plane.pln_mobil -= pln_mobcost(dist, &plp->plane, flags);

    }
    return mission_flags;
}

int
mission_pln_equip(struct plist *plp, struct ichrstr *ip, int flags,
		  s_char mission)
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
    rval = 0;
    if (!(flags & P_F)) {
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
	    if ((pcp->pl_flags & P_C) == 0 || ip == 0)
		break;
	    itype = ip->i_vtype;
	    needed = (pp->pln_load * 2) / ip->i_lbs;
	    break;
	case 'd':
	    if ((pcp->pl_flags & P_C) == 0 || ip == 0)
		break;
	    itype = ip->i_vtype;
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
	case 'i':		/* missile interception */
	    if (pp->pln_load) {
		itype = I_SHELL;
		needed = pp->pln_load;
	    }
	    break;
	default:
	    break;
	}
	if (rval < 0 || (itype != I_NONE && needed <= 0)) {
	    return -1;
	}
	if (itype != I_NONE) {
	    if (itype == I_SHELL && item[itype] < needed)
		item[itype] += supply_commod(plp->plane.pln_own,
					     plp->plane.pln_x,
					     plp->plane.pln_y,
					     I_SHELL, needed);
	    if (item[itype] < needed)
		return -1;
	    item[itype] -= needed;
	}
	if (itype == I_SHELL && (mission == 's' || mission == 'p'))
	    plp->bombs = needed;
	else
	    plp->misc = needed;
    }
    if (pp->pln_ship >= 0)
	putship(ship.shp_uid, &ship);
    else if (pp->pln_land >= 0)
	putland(land.lnd_uid, &land);
    else
	putsect(&sect);
    return rval;
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
	   natid victim, s_char *s, int hardtarget)
{
    struct emp_qelem *qp;
    struct plist *plp;
    struct plnstr *pp;
    int newdam, dam = 0;
    int hitchance;
    int nukedam;

    for (qp = bombers->q_forw; qp != bombers; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	pp = &plp->plane;

	if ((mission == MI_SINTERDICT) && !(plp->pcp->pl_flags & P_A))
	    continue;

	if (!plp->bombs)
	    continue;

	newdam = 0;
	if (plp->pcp->pl_flags & P_A) {
	    if (roll(100) > pln_identchance(pp, hardtarget, EF_SHIP)) {
		wu(0, pp->pln_own,
		   "\t%s detects sub movement in %s\n",
		   prplane(pp), xyas(x, y, pp->pln_own));
		continue;
	    }
	    if (getrel(getnatp(pp->pln_own), victim) > HOSTILE) {
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
	if (plp->plane.pln_nuketype != -1)
	    hitchance = 100;
	else if (hardtarget != SECT_HARDTARGET)
	    wu(0, pp->pln_own, "\t\t%d%% hitchance...", hitchance);
	/* Always calculate damage */
	if (roll(100) <= hitchance) {
	    newdam = pln_damage(&plp->plane, x, y, 'p', &nukedam, 1);
	    if (nukedam) {
		if (mission == MI_INTERDICT) {
		    wu(0, pp->pln_own,
		       "\t\tnuclear warhead on plane %s does %d damage to %s %s\n",
		       prplane(pp), nukedam, cname(victim), s);
		    dam += nukedam;
		}
	    } else {
		wu(0, pp->pln_own,
		   "\t\thit %s %s for %d damage\n",
		   cname(victim), s, newdam);
		dam += newdam;
	    }
	} else {
	    newdam = pln_damage(&plp->plane, x, y, 'p', &nukedam, 0);
	    wu(0, pp->pln_own, "missed\n");
	    if (mission == MI_SINTERDICT) {
		mpr(victim,
		    "RUMBLE... your sub in %s hears a depth-charge explode nearby\n",
		    xyas(x, y, victim));
	    } else if (*s == 's') {
		mpr(victim,
		    "SPLASH!  Bombs miss your %s in %s\n",
		    s, xyas(x, y, victim));
	    } else {
		mpr(victim, "SPLAT!  Bombs miss your %s in %s\n",
		    s, xyas(x, y, victim));
	    }
	    /* Now, even though we missed, the bombs
	       land somewhere. */
	    collateral_damage(x, y, newdam, bombers);
	}

	/* use up missiles */
	if (plp->pcp->pl_flags & P_M) {
	    makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x,
		     pp->pln_y);
	    pp->pln_own = 0;
	}
    }

    return dam;
}

/*
 * Check to see if anyone hostile to the victim
 * is running an air defense mission on this
 * sector. If so, do air combat
 */
int
air_defense(coord x, coord y, natid victim, struct emp_qelem *bomb_list,
	    struct emp_qelem *esc_list)
{
    int dam = 0, cn;
    int mission_flags, tech, combat = 0, rel, dist, z;
    struct emp_qelem *qp, interceptors, airp, i, empty, *next;
    struct plist *plp;
    struct genlist *glp;
    struct genitem *gp;
    struct genlist mi[MAXNOC];
    s_char path[512];
    int count;
    int tcount;

    count = 0;
    for (qp = bomb_list->q_forw; qp != bomb_list; qp = qp->q_forw)
	count++;
    for (qp = esc_list->q_forw; qp != esc_list; qp = qp->q_forw)
	count++;

    memset(mi, 0, sizeof(mi));
    for (z = 1; z < MAXNOC; z++)
	emp_initque((struct emp_qelem *)&mi[z]);

    build_mission_list_type(mi, x, y, MI_AIR_DEFENSE, EF_PLANE, victim);

    for (cn = 1; cn < MAXNOC; cn++) {
	/* Check our relations */
	rel = getrel(getnatp(cn), victim);

	if (rel > HOSTILE)
	    continue;

	if (QEMPTY(&mi[cn].queue))
	    continue;

	/* Ok, make a list of all the interceptors.  Note that this *copies* the
	 * list from the mission creation.  This list must be deleted later. */
	emp_initque(&interceptors);
	for (qp = mi[cn].queue.q_forw; qp != (&mi[cn].queue); qp = next) {
	    next = qp->q_forw;

	    glp = (struct genlist *)qp;
	    gp = (struct genitem *)glp->thing;
	    plp = (struct plist *)qp;

	    dist = mapdist(x, y, gp->x, gp->y);

	    plp = malloc(sizeof(struct plist));
	    memset(plp, 0, sizeof(struct plist));
	    plp->pcp = (struct plchrstr *)glp->cp;
	    memcpy(&plp->plane, glp->thing, sizeof(struct plnstr));

	    /* missiles go one way, so we can use all the range */
	    if (!(plp->pcp->pl_flags & P_M))
		dist *= 2;
	    /* If it's out of range, free it and continue on */
	    if (dist > plp->plane.pln_range) {
		free(plp);
		continue;
	    }
	    emp_insque(&plp->queue, &interceptors);
	}

	/* Remove those who cannot go */
	mission_pln_sel(&interceptors, P_F, 0, SECT_HARDTARGET);

	if (QEMPTY(&interceptors))
	    continue;

	/* Now, delete all the extras, but delete the first ones, not the last ones, so
	 * that the higher numbered planes go into battle (they should be the better ones
	 * at fighting, if all went well.) */
	tcount = 0;
	for (qp = interceptors.q_forw; qp != (&interceptors);
	     qp = qp->q_forw)
	    tcount++;
	tcount -= (count * 2);
	/* Just in case there are more incoming than we have */
	if (tcount < 0)
	    tcount = 0;
	for (qp = interceptors.q_forw; qp != (&interceptors); qp = next) {
	    next = qp->q_forw;
	    if (tcount) {
		tcount--;
		/* Free it up and continue */
		emp_remque(qp);
		glp = (struct genlist *)qp;
		free(glp);
	    }
	}

	/* Now, make a list of all the airports these planes are coming from */
	emp_initque(&airp);
	for (qp = interceptors.q_forw; qp != (&interceptors);
	     qp = qp->q_forw) {
	    plp = (struct plist *)qp;
	    if (!find_airport(&airp, plp->plane.pln_x, plp->plane.pln_y))
		add_airport(&airp, plp->plane.pln_x, plp->plane.pln_y);
	}

	/* Now, fly them out one airport at a time */
	for (qp = airp.q_forw; qp != (&airp); qp = qp->q_forw) {
	    struct airport *air;

	    air = (struct airport *)qp;
	    dist = mapdist(x, y, air->x, air->y);

	    emp_initque(&i);

	    /* Split off the interceptors at this base into i */
	    divide(&interceptors, &i, air->x, air->y);

	    tech = 0;
	    mission_flags = 0;
	    mission_flags |= P_X;	/* stealth (shhh) */
	    /* gets turned off if not all choppers */
	    mission_flags |= P_H;
	    sam_intercept(bomb_list, &i, cn, victim, x, y, 0);
	    sam_intercept(esc_list, &i, cn, victim, x, y, 1);

	    /* Did we run out of interceptors? */
	    if (QEMPTY(&i))
		continue;
	    /* Did we run out of bombers? */
	    if (QEMPTY(bomb_list)) {
		/* Yes, so we have to put the rest of the interceptors back, and
		   then continue, or we leak memory */
		pln_put(&i);
		continue;
	    }
	    mission_flags =
		mission_pln_arm(&i, air->x, air->y, 2 * dist, 'r', 0, P_F,
				mission_flags, &tech);

	    /* Did we run out of interceptors? */
	    if (QEMPTY(&i))
		continue;
	    /* Did we run out of bombers? */
	    if (QEMPTY(bomb_list)) {
		/* Yes, so we have to put the rest of the interceptors back, and
		   then continue, or we leak memory */
		pln_put(&i);
		continue;
	    }

	    BestAirPath(path, air->x, air->y, x, y);
	    wu(0, cn, "Flying %s mission from %s\n",
	       mission_name(MI_AIR_DEFENSE), xyas(air->x, air->y, cn));
	    if (air->own && (air->own != cn)) {
		wu(0, air->own, "%s is flying %s mission from %s\n",
		   cname(cn), mission_name(MI_AIR_DEFENSE),
		   xyas(air->x, air->y, air->own));
	    }

	    /* Now, fly the planes to the sector */
	    emp_initque(&empty);
	    ac_encounter(&i, &empty, air->x, air->y,
			 path, mission_flags, 1, bomb_list, esc_list);

	    /* If none made it, continue */
	    if (QEMPTY(&i))
		continue;

	    /* Some made it, so now they get to try to fight. */
	    /* Intercept the escorts first */
	    combat = 0;
	    if (!QEMPTY(esc_list)) {
		mpr(victim, "%s air defense planes intercept!\n",
		    cname(cn));
		ac_combat_headers(victim, cn);
		ac_airtoair(esc_list, &i);
		combat = 1;
	    }
	    /* Now intercept the bombers */
	    if (!QEMPTY(bomb_list)) {
		if (!combat) {
		    mpr(victim, "%s air defense planes intercept!\n",
			cname(cn));
		    ac_combat_headers(victim, cn);
		}
		ac_airtoair(bomb_list, &i);
		PR(cn, "\n");
		PR(victim, "\n");
	    }

	    pln_put(&i);
	}
    }

    /* We have to free all of these, if they are still there, otherwise they get
       lost and we leak memory all over the place. */
    for (cn = 1; cn < MAXNOC; cn++) {
	/* free up all this memory if it's still there */
	for (qp = mi[cn].queue.q_forw; qp != (&mi[cn].queue); qp = next) {
	    next = qp->q_forw;
	    glp = (struct genlist *)qp;
	    free(glp->thing);
	    free(glp);
	}
    }

    return dam;
}
